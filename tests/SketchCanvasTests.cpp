#include "SketchCanvas.h"
#include "RibbonWidget.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QToolButton>
#include <cstdlib>

namespace {
void click(QWidget* target, const QPoint& position) {
    QMouseEvent press(QEvent::MouseButtonPress, position, target->mapToGlobal(position),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(target, &press);
    QMouseEvent release(QEvent::MouseButtonRelease, position, target->mapToGlobal(position),
        Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(target, &release);
}
void key(QWidget* target, int value) {
    QKeyEvent press(QEvent::KeyPress, value, Qt::NoModifier);
    QApplication::sendEvent(target, &press);
    QKeyEvent release(QEvent::KeyRelease, value, Qt::NoModifier);
    QApplication::sendEvent(target, &release);
}
void middleDrag(QWidget* target, const QPoint& from, const QPoint& to) {
    QMouseEvent press(QEvent::MouseButtonPress, from, target->mapToGlobal(from), Qt::MiddleButton, Qt::MiddleButton, Qt::NoModifier);
    QApplication::sendEvent(target, &press);
    QMouseEvent move(QEvent::MouseMove, to, target->mapToGlobal(to), Qt::NoButton, Qt::MiddleButton, Qt::NoModifier);
    QApplication::sendEvent(target, &move);
    QMouseEvent release(QEvent::MouseButtonRelease, to, target->mapToGlobal(to), Qt::MiddleButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(target, &release);
}
}

int main(int argc, char** argv) {
    QApplication application(argc, argv);
    RibbonWidget ribbon;
    ribbon.setWorkspace(WorkspaceId::Part);
    auto* start = ribbon.findChild<QToolButton*>(QStringLiteral("startButton"));
    if (!start) std::_Exit(16);
    int startChanges = 0;
    QObject::connect(&ribbon, &RibbonWidget::workspaceChanged, [&](WorkspaceId workspace) {
        if (workspace == WorkspaceId::Start) ++startChanges;
    });
    start->click();
    start->click();
    if (ribbon.workspace() != WorkspaceId::Start || startChanges != 1) std::_Exit(17);
    SketchCanvas canvas;
    canvas.resize(800, 600);

    canvas.setTool(SketchCanvas::Tool::Rectangle);
    const QPoint first = canvas.mapFromScene(QPointF(-40, -20));
    const QPoint second = canvas.mapFromScene(QPointF(40, 20));
    click(canvas.viewport(), first);
    click(canvas.viewport(), second);
    if (canvas.model().entities().size() != 4 || !canvas.model().isClosedProfile()) std::_Exit(1);
    int coincidentCount = 0;
    for (const auto& constraint : canvas.model().constraints()) if (constraint.type == SketchConstraintType::Coincident) ++coincidentCount;
    if (coincidentCount != 4) std::_Exit(9);

    canvas.setTool(SketchCanvas::Tool::Line);
    click(canvas.viewport(), canvas.mapFromScene(QPointF(40, 20)));
    click(canvas.viewport(), canvas.mapFromScene(QPointF(70, 35)));
    coincidentCount = 0;
    for (const auto& constraint : canvas.model().constraints()) if (constraint.type == SketchConstraintType::Coincident) ++coincidentCount;
    if (canvas.model().entities().size() != 5 || coincidentCount != 5) std::_Exit(10);

    canvas.setTool(SketchCanvas::Tool::Line);
    click(canvas.viewport(), canvas.mapFromScene(QPointF(80, 80)));
    key(&canvas, Qt::Key_Escape);
    if (canvas.tool() != SketchCanvas::Tool::Select || canvas.model().entities().size() != 5) std::_Exit(2);

    auto items = canvas.scene()->items();
    if (items.isEmpty()) std::_Exit(3);
    QGraphicsItem* selectable = nullptr;
    for (auto* item : items) if (item->flags().testFlag(QGraphicsItem::ItemIsSelectable)) { selectable = item; break; }
    if (!selectable) std::_Exit(3);
    selectable->setSelected(true);
    if (!canvas.applyConstraint(SketchConstraintType::Distance, 50.0)) std::_Exit(4);
    bool hasDimension = false;
    for (const auto& constraint : canvas.model().constraints()) if (constraint.type == SketchConstraintType::Distance) hasDimension = true;
    if (!hasDimension) std::_Exit(5);

    const QPointF beforePan = canvas.mapToScene(canvas.viewport()->rect().center());
    middleDrag(canvas.viewport(), QPoint(300, 250), QPoint(350, 275));
    const QPointF afterPan = canvas.mapToScene(canvas.viewport()->rect().center());
    if (QLineF(beforePan, afterPan).length() < 1.0) std::_Exit(6);

    items = canvas.scene()->items();
    selectable = nullptr;
    for (auto* item : items) if (item->flags().testFlag(QGraphicsItem::ItemIsSelectable)) { selectable = item; break; }
    if (!selectable) std::_Exit(7);
    selectable->setSelected(true);
    key(&canvas, Qt::Key_Delete);
    if (canvas.model().entities().size() != 4) std::_Exit(8);

    SketchCanvas arcCanvas;
    arcCanvas.resize(800, 600);
    arcCanvas.setTool(SketchCanvas::Tool::Arc);
    click(arcCanvas.viewport(), arcCanvas.mapFromScene(QPointF(-30, 0)));
    click(arcCanvas.viewport(), arcCanvas.mapFromScene(QPointF(0, 30)));
    click(arcCanvas.viewport(), arcCanvas.mapFromScene(QPointF(30, 0)));
    if (arcCanvas.model().entities().size() != 1 || arcCanvas.model().entities()[0].type != SketchEntityType::Arc) std::_Exit(11);

    SketchModel symmetryModel;
    const QString symmetrySource = symmetryModel.addLine({2, 1}, {5, 4});
    symmetryModel.addConstraint(SketchConstraintType::Fixed, symmetrySource);
    symmetryModel.addLine({-9, 1}, {-7, 4});
    symmetryModel.addLine({0, -10}, {0, 10}, true);
    SketchCanvas symmetryCanvas;
    symmetryCanvas.setModel(symmetryModel);
    for (auto* item : symmetryCanvas.scene()->items())
        if (!item->data(1).toString().isEmpty()) item->setSelected(true);
    if (!symmetryCanvas.applyConstraint(SketchConstraintType::Symmetric)) std::_Exit(12);
    if (QLineF(symmetryCanvas.model().entities()[1].first, QPointF(-2, 1)).length() > 1e-7
        || QLineF(symmetryCanvas.model().entities()[1].second, QPointF(-5, 4)).length() > 1e-7) std::_Exit(13);

    SketchModel tangentModel;
    tangentModel.addCircle({0, 0}, 10.0);
    tangentModel.addCircle({3, 9}, 4.0);
    SketchCanvas tangentCanvas;
    tangentCanvas.setModel(tangentModel);
    for (auto* item : tangentCanvas.scene()->items())
        if (!item->data(1).toString().isEmpty()) item->setSelected(true);
    if (!tangentCanvas.applyConstraint(SketchConstraintType::InternalTangent)) std::_Exit(14);
    if (std::abs(QLineF(tangentCanvas.model().entities()[0].first, tangentCanvas.model().entities()[1].first).length() - 6.0) > 1e-7)
        std::_Exit(15);
    std::_Exit(0);
}
