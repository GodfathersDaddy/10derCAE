#include "RibbonWidget.h"

#include "WorkspaceCatalog.h"

#include <QComboBox>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QRegularExpression>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

RibbonWidget::RibbonWidget(QWidget* parent) : QWidget(parent) { build(); }

QString RibbonWidget::iconPath(const QString& name) const {
    const QString candidate = QStringLiteral(":/icons/v2/light/%1").arg(name);
    return QFile::exists(candidate) ? candidate : QStringLiteral(":/icons/v2/light/status-success.svg");
}

QToolButton* RibbonWidget::quickButton(const QString& icon, const QString& tooltip) {
    auto* button = new QToolButton(this);
    button->setIcon(QIcon(iconPath(icon)));
    button->setIconSize(QSize(18, 18));
    button->setToolTip(tooltip);
    button->setAutoRaise(true);
    return button;
}

void RibbonWidget::build() {
    setObjectName(QStringLiteral("ribbon"));
    setFixedHeight(174);
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* header = new QWidget(this);
    header->setObjectName(QStringLiteral("ribbonHeader"));
    m_headerLayout = new QHBoxLayout(header);
    m_headerLayout->setContentsMargins(8, 3, 8, 3);
    m_headerLayout->setSpacing(4);
    m_fileButton = new QToolButton(header);
    m_fileButton->setObjectName(QStringLiteral("fileButton"));
    m_fileButton->setPopupMode(QToolButton::InstantPopup);
    m_headerLayout->addWidget(m_fileButton);
    m_undoButton = quickButton(QStringLiteral("edit-undo.svg"), {});
    m_redoButton = quickButton(QStringLiteral("edit-redo.svg"), {});
    m_headerLayout->addWidget(m_undoButton);
    m_headerLayout->addWidget(m_redoButton);
    auto* title = new QLabel(QStringLiteral("10derCAE"), header);
    title->setObjectName(QStringLiteral("applicationTitle"));
    m_headerLayout->addWidget(title);
    m_headerLayout->addStretch();
    m_language = new QComboBox(header);
    m_language->setObjectName(QStringLiteral("languageSelector"));
    m_language->addItem(QStringLiteral("DE"), static_cast<int>(AppLanguage::German));
    m_language->addItem(QStringLiteral("EN"), static_cast<int>(AppLanguage::English));
    m_headerLayout->addWidget(m_language);
    root->addWidget(header);

    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName(QStringLiteral("workspaceTabs"));
    m_tabs->setDocumentMode(true);
    for (const auto& definition : WorkspaceCatalog::all())
        m_tabs->addTab(makeWorkspacePage(definition), {});
    root->addWidget(m_tabs);

    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (index >= 0 && index < WorkspaceCatalog::all().size())
            emit workspaceChanged(WorkspaceCatalog::all().at(index).id);
    });
    connect(m_undoButton, &QToolButton::clicked, this, &RibbonWidget::undoRequested);
    connect(m_redoButton, &QToolButton::clicked, this, &RibbonWidget::redoRequested);
    connect(m_language, &QComboBox::currentIndexChanged, this, [this](int) {
        emit languageChanged(static_cast<AppLanguage>(m_language->currentData().toInt()));
    });
    retranslate();
}

QWidget* RibbonWidget::makeWorkspacePage(const WorkspaceDefinition& definition) {
    auto* page = new QWidget(this);
    page->setProperty("contextual", definition.contextual);
    auto* layout = new QHBoxLayout(page);
    layout->setContentsMargins(7, 3, 7, 3);
    layout->setSpacing(3);
    QStringList panels;
    for (const auto& command : definition.commands)
        if (!panels.contains(command.panelKey)) panels.append(command.panelKey);
    for (const auto& panelKey : panels) {
        auto* panel = new QFrame(page);
        panel->setObjectName(QStringLiteral("ribbonPanel"));
        auto* panelLayout = new QVBoxLayout(panel);
        panelLayout->setContentsMargins(4, 2, 4, 1);
        panelLayout->setSpacing(1);
        auto* tools = new QWidget(panel);
        auto* toolLayout = new QHBoxLayout(tools);
        toolLayout->setContentsMargins(0, 0, 0, 0);
        toolLayout->setSpacing(1);
        for (const auto& command : definition.commands) {
            if (command.panelKey != panelKey) continue;
            auto* button = new QToolButton(tools);
            button->setObjectName(QStringLiteral("command_%1").arg(command.stableId));
            button->setProperty("labelKey", command.labelKey);
            button->setProperty("descriptionKey", command.descriptionKey);
            button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            button->setIcon(QIcon(iconPath(command.iconName)));
            button->setIconSize(QSize(28, 28));
            button->setMinimumWidth(64);
            if (!command.shortcut.isEmpty()) button->setShortcut(QKeySequence(command.shortcut));
            connect(button, &QToolButton::clicked, this, [this, id = command.id] { emit commandTriggered(id); });
            toolLayout->addWidget(button);
        }
        panelLayout->addWidget(tools);
        auto* caption = new QLabel(panel);
        caption->setAlignment(Qt::AlignCenter);
        caption->setObjectName(QStringLiteral("panelCaption"));
        caption->setProperty("labelKey", panelKey);
        panelLayout->addWidget(caption);
        layout->addWidget(panel);
    }
    layout->addStretch();
    return page;
}

void RibbonWidget::buildFileMenu() {
    auto* menu = new QMenu(m_fileButton);
    auto add = [menu, this](const QString& key, const QString& icon, auto signal) {
        auto* action = menu->addAction(QIcon(iconPath(icon)), I18n::instance().text(key));
        connect(action, &QAction::triggered, this, signal);
    };
    add(QStringLiteral("file.new"), QStringLiteral("sketch-create.svg"), &RibbonWidget::newRequested);
    add(QStringLiteral("file.open"), QStringLiteral("file-open.svg"), &RibbonWidget::openRequested);
    add(QStringLiteral("file.save"), QStringLiteral("file-save.svg"), &RibbonWidget::saveRequested);
    add(QStringLiteral("file.saveAs"), QStringLiteral("file-save.svg"), &RibbonWidget::saveAsRequested);
    menu->addSeparator();
    add(QStringLiteral("file.exportStep"), QStringLiteral("file-export.svg"), &RibbonWidget::exportStepRequested);
    add(QStringLiteral("file.exportStl"), QStringLiteral("file-export.svg"), &RibbonWidget::exportStlRequested);
    m_fileButton->setMenu(menu);
}

void RibbonWidget::retranslate() {
    auto& tr = I18n::instance();
    m_fileButton->setText(tr.text(QStringLiteral("file.menu")));
    m_undoButton->setToolTip(tr.text(QStringLiteral("edit.undo")));
    m_redoButton->setToolTip(tr.text(QStringLiteral("edit.redo")));
    for (int i = 0; i < WorkspaceCatalog::all().size(); ++i)
        m_tabs->setTabText(i, tr.text(WorkspaceCatalog::all().at(i).labelKey));
    for (auto* button : findChildren<QToolButton*>(QRegularExpression(QStringLiteral("command_.*")))) {
        button->setText(tr.text(button->property("labelKey").toString()));
        button->setToolTip(tr.text(button->property("descriptionKey").toString()));
    }
    for (auto* label : findChildren<QLabel*>(QStringLiteral("panelCaption")))
        label->setText(tr.text(label->property("labelKey").toString()));
    buildFileMenu();
    const int langIndex = m_language->findData(static_cast<int>(tr.language()));
    m_language->blockSignals(true);
    m_language->setCurrentIndex(langIndex);
    m_language->blockSignals(false);
}

void RibbonWidget::setWorkspace(WorkspaceId value) {
    for (int i = 0; i < WorkspaceCatalog::all().size(); ++i)
        if (WorkspaceCatalog::all().at(i).id == value) { m_tabs->setCurrentIndex(i); return; }
}

WorkspaceId RibbonWidget::workspace() const {
    const int index = m_tabs->currentIndex();
    return index >= 0 ? WorkspaceCatalog::all().at(index).id : WorkspaceId::Start;
}
