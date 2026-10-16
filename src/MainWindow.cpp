#include "MainWindow.h"

#include "GeometryKernel.h"
#include "I18n.h"
#include "OcctViewport.h"
#include "RibbonWidget.h"
#include "SketchCanvas.h"
#include "SketchModel.h"
#include "WorkspaceCatalog.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QFile>
#include <QImage>
#include <QInputDialog>
#include <QPainter>
#include <QScreen>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace {
QString tx(const char* key) { return I18n::instance().text(QString::fromLatin1(key)); }
double arcRadius(const SketchEntity& entity) {
    const double ab = QLineF(entity.first, entity.second).length();
    const double bc = QLineF(entity.second, entity.third).length();
    const double ca = QLineF(entity.third, entity.first).length();
    const double twiceArea = std::abs((entity.second.x() - entity.first.x()) * (entity.third.y() - entity.first.y())
        - (entity.second.y() - entity.first.y()) * (entity.third.x() - entity.first.x()));
    return twiceArea > 1.0e-10 ? (ab * bc * ca) / (2.0 * twiceArea) : 0.0;
}
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_modelShape = GeometryKernel::makeDemoPart();
    buildUi();
    restoreSettings();
    setWorkspace(m_workspace);
}

void MainWindow::buildUi() {
    resize(1440, 920);
    setMinimumSize(1100, 700);
    setDockOptions(AllowNestedDocks | AllowTabbedDocks | AnimatedDocks);
    m_ribbon = new RibbonWidget(this);
    setMenuWidget(m_ribbon);
    buildDocks();
    buildPages();
    connectRibbon();

    m_statusMessage = new QLabel(this);
    m_statusWorkspace = new QLabel(this);
    m_statusUnits = new QLabel(this);
    statusBar()->addWidget(m_statusMessage, 1);
    statusBar()->addPermanentWidget(m_statusWorkspace);
    statusBar()->addPermanentWidget(m_statusUnits);
    applyStyle();
    retranslate();
}

void MainWindow::buildDocks() {
    m_browserDock = new QDockWidget(this);
    m_browserDock->setObjectName(QStringLiteral("browserDock"));
    m_browser = new QTreeWidget(m_browserDock);
    m_browser->setHeaderHidden(true);
    m_browserDock->setWidget(m_browser);
    m_browserDock->setMinimumWidth(225);
    addDockWidget(Qt::LeftDockWidgetArea, m_browserDock);

    m_propertiesDock = new QDockWidget(this);
    m_propertiesDock->setObjectName(QStringLiteral("propertiesDock"));
    m_properties = new QTreeWidget(m_propertiesDock);
    m_properties->setColumnCount(2);
    m_properties->setHeaderHidden(true);
    m_properties->setAlternatingRowColors(true);
    m_propertiesDock->setWidget(m_properties);
    m_propertiesDock->setMinimumWidth(255);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
}

void MainWindow::buildPages() {
    m_pages = new QStackedWidget(this);
    m_startPage = createStartPage();
    m_viewport = new OcctViewport(m_pages);
    m_viewport->setShape(m_modelShape);
    m_sketchCanvas = new SketchCanvas(m_pages);
    m_drawingPage = createDrawingPage();
    m_systemPage = createSystemPage();
    m_pages->addWidget(m_startPage);
    m_pages->addWidget(m_viewport);
    m_pages->addWidget(m_sketchCanvas);
    m_pages->addWidget(m_drawingPage);
    m_pages->addWidget(m_systemPage);
    setCentralWidget(m_pages);
    connect(m_viewport, &OcctViewport::selectionChanged, this, [this](const QString& count) {
        m_statusMessage->setText(tx("status.selected").replace(QStringLiteral("{0}"), count));
    });
    connect(m_sketchCanvas, &SketchCanvas::solveStateChanged, this,
        [this](SketchSolveState, int degrees, const QString& message) {
            m_statusMessage->setText(QStringLiteral("%1 | DOF: %2").arg(message).arg(degrees));
        });
}

QWidget* MainWindow::createStartPage() {
    auto* page = new QWidget(this);
    page->setObjectName(QStringLiteral("startPage"));
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(70, 55, 70, 55);
    auto* heading = new QLabel(page);
    heading->setObjectName(QStringLiteral("startHeading"));
    heading->setProperty("i18n", QStringLiteral("start.heading"));
    auto* subheading = new QLabel(page);
    subheading->setObjectName(QStringLiteral("startSubheading"));
    subheading->setProperty("i18n", QStringLiteral("start.subheading"));
    layout->addWidget(heading);
    layout->addWidget(subheading);
    auto* cards = new QHBoxLayout;
    for (const auto& workspace : WorkspaceCatalog::all()) {
        if (workspace.id == WorkspaceId::Start) continue;
        auto* button = new QPushButton(page);
        button->setObjectName(QStringLiteral("startCard"));
        button->setProperty("i18n", workspace.labelKey);
        button->setMinimumSize(125, 85);
        connect(button, &QPushButton::clicked, this, [this, id = workspace.id] { setWorkspace(id); });
        cards->addWidget(button);
    }
    layout->addSpacing(30);
    layout->addLayout(cards);
    layout->addStretch();
    return page;
}

QWidget* MainWindow::createDrawingPage() {
    auto* view = new QGraphicsView(this);
    view->setObjectName(QStringLiteral("drawingCanvas"));
    auto* scene = new QGraphicsScene(view);
    scene->setSceneRect(0, 0, 1040, 735);
    scene->addRect(22, 22, 996, 691, QPen(QColor(QStringLiteral("#252b33")), 2), QBrush(Qt::white));
    scene->addRect(760, 620, 258, 93, QPen(QColor(QStringLiteral("#252b33")), 1));
    auto* label = scene->addText(QStringLiteral("A3"));
    label->setPos(960, 675);
    view->setScene(scene);
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    return view;
}

QWidget* MainWindow::createSystemPage() {
    auto* view = new QGraphicsView(this);
    view->setObjectName(QStringLiteral("systemCanvas"));
    auto* scene = new QGraphicsScene(view);
    scene->setSceneRect(0, 0, 1000, 620);
    const QList<QPair<QRectF, QString>> blocks = {
        {QRectF(90, 250, 180, 90), QStringLiteral("system.source")},
        {QRectF(410, 220, 220, 150), QStringLiteral("system.plant")},
        {QRectF(770, 250, 150, 90), QStringLiteral("system.scope")}};
    for (const auto& [rect, key] : blocks) {
        scene->addRect(rect, QPen(QColor(QStringLiteral("#397c8c")), 2), QBrush(QColor(QStringLiteral("#f7fbfc"))));
        auto* text = scene->addText(I18n::instance().text(key));
        text->setData(0, key);
        text->setPos(rect.x() + 12, rect.y() + 12);
    }
    scene->addLine(270, 295, 410, 295, QPen(QColor(QStringLiteral("#397c8c")), 3));
    scene->addLine(630, 295, 770, 295, QPen(QColor(QStringLiteral("#397c8c")), 3));
    view->setScene(scene);
    return view;
}

void MainWindow::connectRibbon() {
    connect(m_ribbon, &RibbonWidget::workspaceChanged, this, &MainWindow::setWorkspace);
    connect(m_ribbon, &RibbonWidget::commandTriggered, this, &MainWindow::triggerCommand);
    connect(m_ribbon, &RibbonWidget::newRequested, this, [this] { newDocument(); });
    connect(m_ribbon, &RibbonWidget::openRequested, this, &MainWindow::openDocument);
    connect(m_ribbon, &RibbonWidget::saveRequested, this, [this] { saveDocument(); });
    connect(m_ribbon, &RibbonWidget::saveAsRequested, this, [this] { saveDocument(true); });
    connect(m_ribbon, &RibbonWidget::exportStepRequested, this, [this] { exportShape(QStringLiteral("step")); });
    connect(m_ribbon, &RibbonWidget::exportStlRequested, this, [this] { exportShape(QStringLiteral("stl")); });
    connect(m_ribbon, &RibbonWidget::undoRequested, this, &MainWindow::undo);
    connect(m_ribbon, &RibbonWidget::redoRequested, this, &MainWindow::redo);
    connect(m_ribbon, &RibbonWidget::languageChanged, this, &MainWindow::changeLanguage);
}

void MainWindow::applyStyle() {
    setStyleSheet(QStringLiteral(R"(
        QMainWindow, #startPage { background: #eef2f5; color: #263544; }
        #ribbon { background: #f8f9fb; border-bottom: 1px solid #bfc8d1; }
        #ribbonHeader { background: #273849; color: white; }
        #applicationTitle { color: white; font-weight: 600; padding-left: 8px; }
        #fileButton { background: #087f83; color: white; padding: 5px 18px; border: 0; }
        #workspaceTabs::pane { border: 0; background: #f8f9fb; }
        #workspaceTabs QTabBar::tab { padding: 6px 15px; background: #e8edf1; color: #263544; }
        #workspaceTabs QTabBar::tab:selected { background: #ffffff; color: #087f83; border-top: 3px solid #087f83; }
        #ribbonPanel { border-right: 1px solid #d6dde3; }
        #panelCaption { color: #667585; font-size: 10px; }
        QToolButton { color: #263544; padding: 3px; border: 1px solid transparent; }
        QToolButton:hover { background: #e1eef0; border-color: #9fc7ca; }
        QDockWidget::title { background: #34485c; color: white; padding: 7px; }
        QTreeWidget { border: 0; background: #fbfcfd; alternate-background-color: #f1f4f6; }
        #startHeading { font-size: 29px; font-weight: 300; color: #203143; }
        #startSubheading { font-size: 14px; color: #607284; }
        #startCard { background: white; border: 1px solid #ccd5dd; border-radius: 3px; font-size: 14px; }
        #startCard:hover { border: 2px solid #087f83; background: #f4fbfb; }
        #viewCube { background: rgba(248,250,252,225); border: 1px solid #a8b4c0; border-radius: 5px; }
        #viewCube QPushButton { padding: 0; color: #36495a; }
        QStatusBar { background: #e6ebef; color: #445565; }
    )"));
}

WorkspaceId MainWindow::currentWorkspace() const { return m_workspace; }
void MainWindow::setLanguage(AppLanguage language) { changeLanguage(language); }
bool MainWindow::dumpViewport(const QString& path) const { return m_viewport->dumpImage(path); }

bool MainWindow::captureWindow(const QString& path) const {
    const QRect frame = frameGeometry();
    QPixmap capture = screen()->grabWindow(0, frame.x(), frame.y(), frame.width(), frame.height());
    return capture.save(path);
}

void MainWindow::setWorkspace(WorkspaceId workspace) {
    m_workspace = workspace;
    m_document.setActiveWorkspace(workspace);
    if (m_ribbon->workspace() != workspace) m_ribbon->setWorkspace(workspace);
    if (workspace == WorkspaceId::Start) m_pages->setCurrentWidget(m_startPage);
    else if (workspace == WorkspaceId::Sketch) m_pages->setCurrentWidget(m_sketchCanvas);
    else if (workspace == WorkspaceId::Drawing) m_pages->setCurrentWidget(m_drawingPage);
    else if (workspace == WorkspaceId::SystemModel) m_pages->setCurrentWidget(m_systemPage);
    else m_pages->setCurrentWidget(m_viewport);
    refreshUi();
}

void MainWindow::triggerCommand(CommandId command) {
    if (command == CommandId::OpenDocument) { openDocument(); return; }
    if (command == CommandId::NewPart) { newDocument(WorkspaceId::Part); return; }
    if (command == CommandId::ViewHome) { m_viewport->homeView(); return; }
    if (command == CommandId::ViewFit) { m_viewport->fitAll(); return; }
    if (command == CommandId::ToggleProjection) { m_viewport->toggleProjection(); return; }
    if (command == CommandId::CreateSketch) { beginSketch(); return; }
    if (command == CommandId::Line) { m_sketchCanvas->setTool(SketchCanvas::Tool::Line); return; }
    if (command == CommandId::Rectangle) { m_sketchCanvas->setTool(SketchCanvas::Tool::Rectangle); return; }
    if (command == CommandId::Circle) { m_sketchCanvas->setTool(SketchCanvas::Tool::Circle); return; }
    if (command == CommandId::Arc) { m_sketchCanvas->setTool(SketchCanvas::Tool::Arc); return; }
    if (command == CommandId::Constraints) { applySketchConstraint(); return; }
    if (command == CommandId::Dimension) { applySketchDimension(); return; }
    if (command == CommandId::FinishSketch) { finishSketch(); return; }
    if (command == CommandId::Extrude) { extrudeSketch(); return; }
    if (command == CommandId::Fillet) { applyEdgeFeature(true); return; }
    if (command == CommandId::Chamfer) { applyEdgeFeature(false); return; }
    const auto& definition = WorkspaceCatalog::get(m_workspace);
    for (const auto& item : definition.commands) if (item.id == command) {
        QString message = tx("status.placeholder");
        message.replace(QStringLiteral("{0}"), I18n::instance().text(item.labelKey));
        m_statusMessage->setText(message);
        return;
    }
}

void MainWindow::retranslate() {
    m_ribbon->retranslate();
    m_browserDock->setWindowTitle(tx("dock.browser"));
    m_propertiesDock->setWindowTitle(tx("dock.properties"));
    for (auto* label : m_startPage->findChildren<QLabel*>())
        if (label->property("i18n").isValid()) label->setText(I18n::instance().text(label->property("i18n").toString()));
    for (auto* button : m_startPage->findChildren<QPushButton*>())
        if (button->property("i18n").isValid()) button->setText(I18n::instance().text(button->property("i18n").toString()));
    if (auto* view = qobject_cast<QGraphicsView*>(m_systemPage))
        for (auto* item : view->scene()->items()) if (auto* text = qgraphicsitem_cast<QGraphicsTextItem*>(item))
            if (text->data(0).isValid()) text->setPlainText(I18n::instance().text(text->data(0).toString()));
    m_statusMessage->setText(tx("app.ready"));
    refreshUi();
}

void MainWindow::refreshUi() {
    refreshBrowser();
    refreshProperties();
    updateWindowTitle();
    m_statusWorkspace->setText(I18n::instance().text(WorkspaceCatalog::get(m_workspace).labelKey));
    m_statusUnits->setText(m_document.units());
}

void MainWindow::refreshBrowser() {
    m_browser->clear();
    auto* root = new QTreeWidgetItem(m_browser, {m_document.name()});
    root->setExpanded(true);
    const auto& definition = WorkspaceCatalog::get(m_workspace);
    for (const auto& key : definition.browserKeys) {
        auto* item = new QTreeWidgetItem(root, {I18n::instance().text(key)});
        item->setExpanded(true);
        if (key == QStringLiteral("browser.features") || key == QStringLiteral("browser.solids"))
            for (const auto& feature : m_document.features()) new QTreeWidgetItem(item, {feature.name});
    }
}

void MainWindow::refreshProperties() {
    m_properties->clear();
    const auto& definition = WorkspaceCatalog::get(m_workspace);
    for (const auto& key : definition.propertyKeys) {
        QString value = tx("value.notConfigured");
        if (key == QStringLiteral("property.document")) value = m_document.name();
        else if (key == QStringLiteral("property.units")) value = m_document.units();
        else if (key == QStringLiteral("property.application")) value = QStringLiteral("10derCAE");
        else if (key == QStringLiteral("property.version")) value = QApplication::applicationVersion();
        else if (key == QStringLiteral("property.license")) value = QStringLiteral("GPL-3.0-or-later");
        new QTreeWidgetItem(m_properties, {I18n::instance().text(key), value});
    }
    m_properties->resizeColumnToContents(0);
}

void MainWindow::updateWindowTitle() {
    setWindowTitle(QStringLiteral("10derCAE — %1%2").arg(m_document.name(), m_document.isModified() ? QStringLiteral(" *") : QString{}));
}

void MainWindow::changeLanguage(AppLanguage language) {
    I18n::instance().setLanguage(language);
    retranslate();
}

void MainWindow::newDocument(WorkspaceId workspace) {
    if (!confirmDiscard()) return;
    m_document = Document{};
    m_path.clear(); m_undo.clear(); m_redo.clear();
    setWorkspace(workspace);
}

void MainWindow::openDocument() {
    if (!confirmDiscard()) return;
    const QString path = QFileDialog::getOpenFileName(this, tx("dialog.open"), {}, QStringLiteral("10derCAE (*.10cae)"));
    if (path.isEmpty()) return;
    Document loaded;
    QString error;
    if (!Document::load(path, &loaded, &error)) { QMessageBox::critical(this, tx("dialog.error"), error); return; }
    m_document = loaded; m_path = path; m_undo.clear(); m_redo.clear();
    rebuildModel();
    setWorkspace(m_document.activeWorkspace());
    m_statusMessage->setText(tx("status.opened"));
}

bool MainWindow::saveDocument(bool choosePath) {
    QString path = m_path;
    if (choosePath || path.isEmpty()) path = QFileDialog::getSaveFileName(this, tx("dialog.save"), m_document.name() + QStringLiteral(".10cae"), QStringLiteral("10derCAE (*.10cae)"));
    if (path.isEmpty()) return false;
    if (!path.endsWith(QStringLiteral(".10cae"), Qt::CaseInsensitive)) path += QStringLiteral(".10cae");
    QString error;
    if (!m_document.save(path, &error)) { QMessageBox::critical(this, tx("dialog.error"), error); return false; }
    m_path = path; updateWindowTitle(); m_statusMessage->setText(tx("status.saved"));
    return true;
}

void MainWindow::exportShape(const QString& format) {
    const QString filter = format == QStringLiteral("step") ? QStringLiteral("STEP (*.step *.stp)") : QStringLiteral("STL (*.stl)");
    const QString path = QFileDialog::getSaveFileName(this, tx("file.exportStep"), m_document.name() + QStringLiteral(".") + format, filter);
    if (path.isEmpty()) return;
    QString error;
    const bool ok = format == QStringLiteral("step") ? GeometryKernel::exportStep(m_modelShape, path, &error) : GeometryKernel::exportStl(m_modelShape, path, &error);
    if (!ok) QMessageBox::critical(this, tx("dialog.error"), error);
}

void MainWindow::commitFeature(const Feature& feature) {
    m_undo.push_back(m_document);
    m_redo.clear();
    m_document.addFeature(feature);
    rebuildModel();
    refreshUi();
}

void MainWindow::beginSketch() {
    const QStringList planes{QStringLiteral("XY"), QStringLiteral("XZ"), QStringLiteral("YZ")};
    bool accepted = false;
    m_activeSketchPlane = QInputDialog::getItem(this, tr("Create sketch"), tr("Sketch plane"), planes, 0, false, &accepted);
    if (!accepted) return;
    m_sketchCanvas->clearSketch();
    m_sketchCanvas->setPlane(m_activeSketchPlane);
    m_sketchCanvas->setTool(SketchCanvas::Tool::Select);
    setWorkspace(WorkspaceId::Sketch);
}

void MainWindow::finishSketch() {
    const SketchModel& sketch = m_sketchCanvas->model();
    if (sketch.entities().isEmpty()) { setWorkspace(WorkspaceId::Part); return; }
    if (!sketch.isClosedProfile() && !(sketch.entities().size() == 1 && sketch.entities()[0].type == SketchEntityType::Circle)) {
        QMessageBox::warning(this, tr("Sketch"), tr("The profile must be closed before finishing the sketch."));
        return;
    }
    commitFeature({QStringLiteral("sketch-%1").arg(m_document.features().size() + 1), QStringLiteral("sketch2d"),
        tr("Sketch %1").arg(m_document.features().size() + 1), {{QStringLiteral("plane"), m_sketchCanvas->plane()},
        {QStringLiteral("model"), sketch.toJson()}}});
    setWorkspace(WorkspaceId::Part);
}

void MainWindow::applySketchDimension() {
    const QStringList ids = m_sketchCanvas->selectedEntityIds();
    if (ids.size() < 1 || ids.size() > 2) { QMessageBox::information(this, tr("Dimension"), tr("Select one entity, or two lines for angle/distance.")); return; }
    const auto& entities = m_sketchCanvas->model().entities();
    const auto entity = std::find_if(entities.cbegin(), entities.cend(), [&](const SketchEntity& item) { return item.id == ids[0]; });
    if (entity == entities.cend()) return;
    bool accepted = false;
    if (ids.size() == 2) {
        const QStringList modes{tr("Angle"), tr("Endpoint distance")};
        const QString mode = QInputDialog::getItem(this, tr("Dimension"), tr("Dimension type"), modes, 0, false, &accepted);
        if (!accepted) return;
        const bool angle = mode == modes[0];
        double initial = 90.0;
        if (!angle) {
            const auto second = std::find_if(entities.cbegin(), entities.cend(), [&](const SketchEntity& item) { return item.id == ids[1]; });
            if (second == entities.cend()) return;
            initial = QLineF(entity->second, second->first).length();
        }
        const double value = QInputDialog::getDouble(this, tr("Dimension"), angle ? tr("Angle (degrees)") : tr("Distance (mm)"),
            initial, angle ? -360.0 : 0.001, 100000.0, 3, &accepted);
        if (accepted && !m_sketchCanvas->applyConstraint(angle ? SketchConstraintType::Angle : SketchConstraintType::PointDistance, value))
            QMessageBox::warning(this, tr("Dimension"), tr("The dimension could not be applied."));
        return;
    }
    const bool radial = entity->type == SketchEntityType::Circle || entity->type == SketchEntityType::Arc;
    const double current = entity->type == SketchEntityType::Circle ? entity->radius
        : entity->type == SketchEntityType::Arc ? arcRadius(*entity) : QLineF(entity->first, entity->second).length();
    const double value = QInputDialog::getDouble(this, tr("Dimension"), radial ? tr("Radius (mm)") : tr("Length (mm)"),
        current, 0.001, 100000.0, 3, &accepted);
    if (accepted && !m_sketchCanvas->applyConstraint(radial ? SketchConstraintType::Radius : SketchConstraintType::Distance, value))
        QMessageBox::warning(this, tr("Dimension"), tr("The dimension could not be applied."));
}

void MainWindow::applySketchConstraint() {
    const QStringList labels{tr("Horizontal"), tr("Vertical"), tr("Parallel"), tr("Perpendicular"), tr("Coincident"), tr("Equal"), tr("Concentric"), tr("External tangent"), tr("Internal tangent"), tr("Symmetric"), tr("Fixed")};
    bool accepted = false;
    const QString selected = QInputDialog::getItem(this, tr("Geometric constraint"), tr("Constraint"), labels, 0, false, &accepted);
    if (!accepted) return;
    const int index = labels.indexOf(selected);
    const SketchConstraintType types[] = {SketchConstraintType::Horizontal, SketchConstraintType::Vertical,
        SketchConstraintType::Parallel, SketchConstraintType::Perpendicular, SketchConstraintType::Coincident,
        SketchConstraintType::Equal, SketchConstraintType::Concentric, SketchConstraintType::Tangent, SketchConstraintType::InternalTangent,
        SketchConstraintType::Symmetric, SketchConstraintType::Fixed};
    if (index < 0 || !m_sketchCanvas->applyConstraint(types[index]))
        QMessageBox::information(this, tr("Geometric constraint"), tr("Select the required sketch geometry first."));
}

void MainWindow::createRectangleSketch() {
    bool accepted = false;
    const double width = QInputDialog::getDouble(this, tr("Rectangle"), tr("Width (mm)"), 100.0, 0.001, 100000.0, 3, &accepted);
    if (!accepted) return;
    const double height = QInputDialog::getDouble(this, tr("Rectangle"), tr("Height (mm)"), 60.0, 0.001, 100000.0, 3, &accepted);
    if (!accepted) return;
    Feature feature{QStringLiteral("sketch-%1").arg(m_document.features().size() + 1),
        QStringLiteral("rectangleSketch"), tr("Sketch %1").arg(m_document.features().size() + 1),
        {{QStringLiteral("plane"), QStringLiteral("XY")}, {QStringLiteral("width"), width},
         {QStringLiteral("height"), height}, {QStringLiteral("centered"), true},
         {QStringLiteral("constraintState"), QStringLiteral("fullyConstrained")}}};
    commitFeature(feature);
    m_statusMessage->setText(tr("Fully constrained rectangle sketch created."));
}

void MainWindow::extrudeSketch() {
    const Feature* sketch = nullptr;
    for (auto it = m_document.features().crbegin(); it != m_document.features().crend(); ++it)
        if (it->type == QStringLiteral("rectangleSketch") || it->type == QStringLiteral("sketch2d")) { sketch = &*it; break; }
    if (!sketch) { QMessageBox::information(this, tr("Extrude"), tr("Create a rectangle sketch first.")); return; }
    bool accepted = false;
    const double distance = QInputDialog::getDouble(this, tr("Extrude"), tr("Distance (mm)"), 20.0, 0.001, 100000.0, 3, &accepted);
    if (!accepted) return;
    commitFeature({QStringLiteral("extrude-%1").arg(m_document.features().size() + 1),
        QStringLiteral("extrude"), tr("Extrusion %1").arg(m_document.features().size() + 1),
        {{QStringLiteral("profile"), sketch->id}, {QStringLiteral("distance"), distance},
         {QStringLiteral("operation"), QStringLiteral("newSolid")}}});
    setWorkspace(WorkspaceId::Part);
}

void MainWindow::applyEdgeFeature(bool fillet) {
    if (!m_modelShape || m_modelShape->isNull()) return;
    bool accepted = false;
    const double value = QInputDialog::getDouble(this, fillet ? tr("Fillet") : tr("Chamfer"),
        fillet ? tr("Radius (mm)") : tr("Distance (mm)"), 2.0, 0.001, 10000.0, 3, &accepted);
    if (!accepted) return;
    commitFeature({QStringLiteral("%1-%2").arg(fillet ? QStringLiteral("fillet") : QStringLiteral("chamfer")).arg(m_document.features().size() + 1),
        fillet ? QStringLiteral("fillet") : QStringLiteral("chamfer"),
        fillet ? tr("Fillet") : tr("Chamfer"), {{QStringLiteral("value"), value}, {QStringLiteral("edges"), QStringLiteral("all")}}});
}

void MainWindow::rebuildModel() {
    std::shared_ptr<KernelShape> shape;
    QHash<QString, Feature> sketches;
    QString error;
    for (const Feature& feature : m_document.features()) {
        if (feature.type == QStringLiteral("rectangleSketch") || feature.type == QStringLiteral("sketch2d")) sketches.insert(feature.id, feature);
        else if (feature.type == QStringLiteral("extrude")) {
            const Feature sketch = sketches.value(feature.parameters.value(QStringLiteral("profile")).toString());
            if (sketch.type == QStringLiteral("sketch2d")) {
                QString sketchError;
                const auto model = SketchModel::fromJson(sketch.parameters.value(QStringLiteral("model")).toObject(), &sketchError);
                shape = GeometryKernel::extrudeSketch(model, feature.parameters.value(QStringLiteral("distance")).toDouble(),
                    sketch.parameters.value(QStringLiteral("plane")).toString(QStringLiteral("XY")), &error);
            } else shape = GeometryKernel::extrudeRectangle(sketch.parameters.value(QStringLiteral("width")).toDouble(),
                    sketch.parameters.value(QStringLiteral("height")).toDouble(),
                    feature.parameters.value(QStringLiteral("distance")).toDouble(), &error);
        } else if (feature.type == QStringLiteral("fillet")) {
            auto changed = GeometryKernel::filletAllEdges(shape, feature.parameters.value(QStringLiteral("value")).toDouble(), &error);
            if (changed) shape = changed;
        } else if (feature.type == QStringLiteral("chamfer")) {
            auto changed = GeometryKernel::chamferAllEdges(shape, feature.parameters.value(QStringLiteral("value")).toDouble(), &error);
            if (changed) shape = changed;
        }
    }
    m_modelShape = shape ? shape : GeometryKernel::makeDemoPart();
    m_viewport->setShape(m_modelShape);
    if (!error.isEmpty()) m_statusMessage->setText(error);
}

void MainWindow::undo() {
    if (m_undo.isEmpty()) return;
    m_redo.push_back(m_document); m_document = m_undo.takeLast(); rebuildModel(); refreshUi();
}
void MainWindow::redo() {
    if (m_redo.isEmpty()) return;
    m_undo.push_back(m_document); m_document = m_redo.takeLast(); rebuildModel(); refreshUi();
}

bool MainWindow::confirmDiscard() {
    if (!m_document.isModified()) return true;
    const auto answer = QMessageBox::question(this, tx("dialog.unsaved.title"), tx("dialog.unsaved.text"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    return answer == QMessageBox::Save ? saveDocument() : answer == QMessageBox::Discard;
}

void MainWindow::restoreSettings() {
    QSettings settings;
    if (settings.contains(QStringLiteral("window/geometry"))) restoreGeometry(settings.value(QStringLiteral("window/geometry")).toByteArray());
    if (settings.contains(QStringLiteral("window/state"))) restoreState(settings.value(QStringLiteral("window/state")).toByteArray());
    I18n::instance().setLanguage(settings.value(QStringLiteral("ui/language"), QStringLiteral("de")).toString() == QStringLiteral("en") ? AppLanguage::English : AppLanguage::German);
    m_workspace = workspaceFromStableId(settings.value(QStringLiteral("ui/workspace"), QStringLiteral("start")).toString());
    retranslate();
}

void MainWindow::saveSettings() {
    QSettings settings;
    settings.setValue(QStringLiteral("window/geometry"), saveGeometry());
    settings.setValue(QStringLiteral("window/state"), saveState());
    settings.setValue(QStringLiteral("ui/language"), I18n::instance().code());
    settings.setValue(QStringLiteral("ui/workspace"), workspaceStableId(m_workspace));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (confirmDiscard()) { saveSettings(); event->accept(); }
    else event->ignore();
}
