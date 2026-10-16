#include "SketchCanvas.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPathItem>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace {
constexpr int EntityIdRole = 1;
QPainterPath arcPath(const QPointF& start, const QPointF& through, const QPointF& end) {
    const double determinant = 2.0 * (start.x() * (through.y() - end.y())
        + through.x() * (end.y() - start.y()) + end.x() * (start.y() - through.y()));
    QPainterPath path(start);
    if (std::abs(determinant) < 1.0e-9) { path.lineTo(end); return path; }
    const double startSquared = start.x() * start.x() + start.y() * start.y();
    const double throughSquared = through.x() * through.x() + through.y() * through.y();
    const double endSquared = end.x() * end.x() + end.y() * end.y();
    const QPointF center(
        (startSquared * (through.y() - end.y()) + throughSquared * (end.y() - start.y())
            + endSquared * (start.y() - through.y())) / determinant,
        (startSquared * (end.x() - through.x()) + throughSquared * (start.x() - end.x())
            + endSquared * (through.x() - start.x())) / determinant);
    const double radius = QLineF(center, start).length();
    const auto angle = [&center](const QPointF& point) { return qRadiansToDegrees(std::atan2(center.y() - point.y(), point.x() - center.x())); };
    const auto positive = [](double value) { while (value < 0.0) value += 360.0; while (value >= 360.0) value -= 360.0; return value; };
    const double startAngle = angle(start);
    const double throughDelta = positive(angle(through) - startAngle);
    const double endDelta = positive(angle(end) - startAngle);
    const double sweep = throughDelta <= endDelta ? endDelta : endDelta - 360.0;
    path.arcTo(QRectF(center.x() - radius, center.y() - radius, radius * 2.0, radius * 2.0), startAngle, sweep);
    return path;
}
}

SketchCanvas::SketchCanvas(QWidget* parent) : QGraphicsView(parent) {
    setObjectName(QStringLiteral("sketchCanvas"));
    setScene(new QGraphicsScene(this));
    scene()->setSceneRect(-5000, -5000, 10000, 10000);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setBackgroundBrush(QColor(QStringLiteral("#fbfcfd")));
    setFocusPolicy(Qt::StrongFocus);
    scale(2.0, -2.0);
}

void SketchCanvas::setTool(Tool value) {
    cancelPending();
    m_tool = value;
    setDragMode(value == Tool::Select ? QGraphicsView::RubberBandDrag : QGraphicsView::NoDrag);
    setCursor(value == Tool::Select ? Qt::ArrowCursor : Qt::CrossCursor);
    emit toolChanged(value);
}

SketchCanvas::Tool SketchCanvas::tool() const { return m_tool; }
void SketchCanvas::setModel(const SketchModel& value) { m_model = value; rebuildScene(); updateSolveState(); }
const SketchModel& SketchCanvas::model() const { return m_model; }
void SketchCanvas::clearSketch() { m_model = {}; cancelPending(); rebuildScene(); updateSolveState(); emit modelChanged(); }
void SketchCanvas::setPlane(const QString& value) { if (value == QStringLiteral("XY") || value == QStringLiteral("XZ") || value == QStringLiteral("YZ")) m_plane = value; viewport()->update(); }
QString SketchCanvas::plane() const { return m_plane; }

QStringList SketchCanvas::selectedEntityIds() const {
    QStringList result;
    for (auto* item : scene()->selectedItems()) {
        const QString id = item->data(EntityIdRole).toString();
        if (!id.isEmpty() && !result.contains(id)) result.push_back(id);
    }
    return result;
}

bool SketchCanvas::applyConstraint(SketchConstraintType type, double value) {
    const QStringList ids = selectedEntityIds();
    if (type == SketchConstraintType::Symmetric) {
        if (ids.size() != 3) return false;
        QString axis;
        QStringList geometry;
        for (const QString& id : ids) {
            const auto entity = std::find_if(m_model.entities().cbegin(), m_model.entities().cend(),
                [&id](const SketchEntity& candidate) { return candidate.id == id; });
            if (entity != m_model.entities().cend() && entity->type == SketchEntityType::Line && entity->construction && axis.isEmpty())
                axis = id;
            else geometry.push_back(id);
        }
        // If no construction line is present, the third selected line is the axis.
        if (axis.isEmpty()) { axis = ids[2]; geometry = {ids[0], ids[1]}; }
        if (geometry.size() != 2 || m_model.addSymmetryConstraint(geometry[0], geometry[1], axis).isEmpty()) return false;
        updateSolveState();
        rebuildScene();
        emit modelChanged();
        return true;
    }
    const bool binary = type == SketchConstraintType::Coincident || type == SketchConstraintType::Parallel
        || type == SketchConstraintType::Perpendicular || type == SketchConstraintType::Equal
        || type == SketchConstraintType::Concentric || type == SketchConstraintType::Tangent || type == SketchConstraintType::InternalTangent
        || type == SketchConstraintType::PointDistance || type == SketchConstraintType::Angle;
    if (ids.size() < (binary ? 2 : 1)) return false;
    const QString id = m_model.addConstraint(type, ids[0], binary ? ids[1] : QString{}, 1, 0, value);
    if (id.isEmpty()) return false;
    updateSolveState();
    rebuildScene();
    emit modelChanged();
    return true;
}

void SketchCanvas::drawBackground(QPainter* painter, const QRectF& rect) {
    QGraphicsView::drawBackground(painter, rect);
    const int left = static_cast<int>(std::floor(rect.left() / m_grid));
    const int right = static_cast<int>(std::ceil(rect.right() / m_grid));
    const int top = static_cast<int>(std::floor(rect.top() / m_grid));
    const int bottom = static_cast<int>(std::ceil(rect.bottom() / m_grid));
    QPen minor(QColor(QStringLiteral("#dce3e8")), 0);
    QPen major(QColor(QStringLiteral("#b7c3cc")), 0);
    for (int x = left; x <= right; ++x) {
        painter->setPen(x % 10 == 0 ? major : minor);
        painter->drawLine(QPointF(x * m_grid, rect.top()), QPointF(x * m_grid, rect.bottom()));
    }
    for (int y = top; y <= bottom; ++y) {
        painter->setPen(y % 10 == 0 ? major : minor);
        painter->drawLine(QPointF(rect.left(), y * m_grid), QPointF(rect.right(), y * m_grid));
    }
    painter->setPen(QPen(QColor(QStringLiteral("#d74d4d")), 0));
    painter->drawLine(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
    painter->setPen(QPen(QColor(QStringLiteral("#4a9b65")), 0));
    painter->drawLine(QPointF(0, rect.top()), QPointF(0, rect.bottom()));
}

QPointF SketchCanvas::snapped(const QPointF& raw, bool includeEndpoints) const {
    if (includeEndpoints) return snapTarget(raw).point;
    return QPointF(std::round(raw.x() / m_grid) * m_grid, std::round(raw.y() / m_grid) * m_grid);
}

SketchCanvas::SnapTarget SketchCanvas::snapTarget(const QPointF& raw) const {
    QPointF best(std::round(raw.x() / m_grid) * m_grid, std::round(raw.y() / m_grid) * m_grid);
    double bestDistance = std::hypot(best.x() - raw.x(), best.y() - raw.y());
    SnapTarget result{best, {}, 0, false};
    for (const auto& entity : m_model.entities()) {
        const QList<QPointF> candidates = entity.type == SketchEntityType::Line ? QList<QPointF>{entity.first, entity.second}
            : entity.type == SketchEntityType::Arc ? QList<QPointF>{entity.first, entity.third} : QList<QPointF>{entity.first};
        for (int index = 0; index < candidates.size(); ++index) {
            const QPointF& candidate = candidates[index];
            const double distance = std::hypot(candidate.x() - raw.x(), candidate.y() - raw.y());
            if (distance <= bestDistance && distance < 4.0) {
                bestDistance = distance;
                result = {candidate, entity.id, index, true};
            }
        }
    }
    return result;
}

void SketchCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_lastPanPosition = event->position().toPoint();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    if (event->button() != Qt::LeftButton || m_tool == Tool::Select) { QGraphicsView::mousePressEvent(event); return; }
    const SnapTarget target = snapTarget(mapToScene(event->position().toPoint()));
    const QPointF point = target.point;
    if (m_tool == Tool::Arc && m_drawing && m_arcStage == 1) {
        if (QLineF(m_start, point).length() < 1.0e-6) return;
        m_arcPoint = point;
        m_arcStage = 2;
        if (m_previewLine) { scene()->removeItem(m_previewLine); delete m_previewLine; m_previewLine = nullptr; }
        m_previewPath = scene()->addPath(arcPath(m_start, m_arcPoint, point), QPen(QColor(QStringLiteral("#087f83")), 0, Qt::DashLine));
        event->accept();
        return;
    }
    if (m_tool == Tool::Arc && m_drawing && m_arcStage == 2) {
        if (QLineF(m_start, point).length() < 1.0e-6) return;
        const QString id = m_model.addArc(m_start, m_arcPoint, point);
        if (m_startSnap.endpoint) m_model.addConstraint(SketchConstraintType::Coincident, id, m_startSnap.entityId, 0, m_startSnap.pointIndex);
        if (target.endpoint) m_model.addConstraint(SketchConstraintType::Coincident, id, target.entityId, 2, target.pointIndex);
        cancelPending(); m_model.solve(); rebuildScene(); updateSolveState(); emit modelChanged();
        event->accept();
        return;
    }
    if (!m_drawing) {
        m_drawing = true;
        m_start = point;
        m_startSnap = target;
        if (m_tool == Tool::Arc) m_arcStage = 1;
        if (m_tool == Tool::Circle) m_previewEllipse = scene()->addEllipse(QRectF(point, QSizeF()), QPen(QColor(QStringLiteral("#087f83")), 0));
        else m_previewLine = scene()->addLine(QLineF(point, point), QPen(QColor(QStringLiteral("#087f83")), 0, Qt::DashLine));
    } else finishEntity(point);
    event->accept();
}

void SketchCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (m_panning) {
        const QPoint position = event->position().toPoint();
        const QPoint delta = position - m_lastPanPosition;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        m_lastPanPosition = position;
        event->accept();
        return;
    }
    if (m_drawing) {
        const QPointF point = snapped(mapToScene(event->position().toPoint()));
        if (m_previewLine) m_previewLine->setLine(QLineF(m_start, point));
        if (m_previewEllipse) {
            const double radius = std::hypot(point.x() - m_start.x(), point.y() - m_start.y());
            m_previewEllipse->setRect(m_start.x() - radius, m_start.y() - radius, radius * 2.0, radius * 2.0);
        }
        if (m_previewPath && m_arcStage == 2) m_previewPath->setPath(arcPath(m_start, m_arcPoint, point));
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void SketchCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton && m_panning) {
        m_panning = false;
        setCursor(m_tool == Tool::Select ? Qt::ArrowCursor : Qt::CrossCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void SketchCanvas::finishEntity(const QPointF& end) {
    if (QLineF(m_start, end).length() < 1.0e-6) { cancelPending(); return; }
    if (m_tool == Tool::Line) {
        const SnapTarget endSnap = snapTarget(end);
        const QString id = m_model.addLine(m_start, end);
        if (m_startSnap.endpoint) m_model.addConstraint(SketchConstraintType::Coincident, id, m_startSnap.entityId, 0, m_startSnap.pointIndex);
        if (endSnap.endpoint) m_model.addConstraint(SketchConstraintType::Coincident, id, endSnap.entityId, 1, endSnap.pointIndex);
        const QPointF delta = end - m_start;
        if (std::abs(delta.y()) < 1.0) m_model.addConstraint(SketchConstraintType::Horizontal, id);
        else if (std::abs(delta.x()) < 1.0) m_model.addConstraint(SketchConstraintType::Vertical, id);
    } else if (m_tool == Tool::Rectangle) {
        const QString a = m_model.addLine(m_start, {end.x(), m_start.y()});
        const QString b = m_model.addLine({end.x(), m_start.y()}, end);
        const QString c = m_model.addLine(end, {m_start.x(), end.y()});
        const QString d = m_model.addLine({m_start.x(), end.y()}, m_start);
        m_model.addConstraint(SketchConstraintType::Horizontal, a);
        m_model.addConstraint(SketchConstraintType::Vertical, b);
        m_model.addConstraint(SketchConstraintType::Horizontal, c);
        m_model.addConstraint(SketchConstraintType::Vertical, d);
        m_model.addConstraint(SketchConstraintType::Coincident, a, b, 1, 0);
        m_model.addConstraint(SketchConstraintType::Coincident, b, c, 1, 0);
        m_model.addConstraint(SketchConstraintType::Coincident, c, d, 1, 0);
        m_model.addConstraint(SketchConstraintType::Coincident, d, a, 1, 0);
    } else if (m_tool == Tool::Circle) {
        m_model.addCircle(m_start, QLineF(m_start, end).length());
    }
    cancelPending();
    m_model.solve();
    rebuildScene();
    updateSolveState();
    emit modelChanged();
}

void SketchCanvas::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) { cancelPending(); setTool(Tool::Select); event->accept(); return; }
    if (event->key() == Qt::Key_Delete) {
        const QStringList ids = selectedEntityIds();
        for (const QString& id : ids) m_model.removeEntity(id);
        rebuildScene(); updateSolveState(); emit modelChanged(); event->accept(); return;
    }
    QGraphicsView::keyPressEvent(event);
}

void SketchCanvas::wheelEvent(QWheelEvent* event) {
    const double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    const double current = transform().m11();
    if ((factor > 1.0 && current < 20.0) || (factor < 1.0 && current > 0.2)) scale(factor, factor);
    event->accept();
}

void SketchCanvas::cancelPending() {
    m_drawing = false;
    if (m_previewLine) { scene()->removeItem(m_previewLine); delete m_previewLine; m_previewLine = nullptr; }
    if (m_previewEllipse) { scene()->removeItem(m_previewEllipse); delete m_previewEllipse; m_previewEllipse = nullptr; }
    if (m_previewPath) { scene()->removeItem(m_previewPath); delete m_previewPath; m_previewPath = nullptr; }
    m_arcStage = 0;
}

void SketchCanvas::rebuildScene() {
    cancelPending();
    scene()->clear();
    for (const auto& entity : m_model.entities()) {
        QGraphicsItem* item = nullptr;
        const QPen pen(entity.fixed ? QColor(QStringLiteral("#2667a5")) : QColor(QStringLiteral("#222d36")), 0);
        if (entity.type == SketchEntityType::Line) item = scene()->addLine(QLineF(entity.first, entity.second), pen);
        else if (entity.type == SketchEntityType::Arc) item = scene()->addPath(arcPath(entity.first, entity.second, entity.third), pen);
        else item = scene()->addEllipse(entity.first.x() - entity.radius, entity.first.y() - entity.radius,
            entity.radius * 2.0, entity.radius * 2.0, pen);
        item->setData(EntityIdRole, entity.id);
        item->setFlag(QGraphicsItem::ItemIsSelectable);
    }
    for (const auto& constraint : m_model.constraints()) {
        const auto entity = std::find_if(m_model.entities().cbegin(), m_model.entities().cend(),
            [&](const SketchEntity& candidate) { return candidate.id == constraint.entityA; });
        if (entity == m_model.entities().cend()) continue;
        QString glyph;
        switch (constraint.type) {
        case SketchConstraintType::Horizontal: glyph = QStringLiteral("H"); break;
        case SketchConstraintType::Vertical: glyph = QStringLiteral("V"); break;
        case SketchConstraintType::Parallel: glyph = QStringLiteral("∥"); break;
        case SketchConstraintType::Perpendicular: glyph = QStringLiteral("⊥"); break;
        case SketchConstraintType::Coincident: glyph = QStringLiteral("●"); break;
        case SketchConstraintType::Equal: glyph = QStringLiteral("="); break;
        case SketchConstraintType::Concentric: glyph = QStringLiteral("◎"); break;
        case SketchConstraintType::Tangent: glyph = QStringLiteral("T"); break;
        case SketchConstraintType::InternalTangent: glyph = QStringLiteral("Tᵢ"); break;
        case SketchConstraintType::Symmetric: glyph = QStringLiteral("↔"); break;
        case SketchConstraintType::Fixed: glyph = QStringLiteral("🔒"); break;
        case SketchConstraintType::Distance: glyph = QStringLiteral("%1 mm").arg(constraint.value, 0, 'f', 2); break;
        case SketchConstraintType::PointDistance: glyph = QStringLiteral("↔ %1 mm").arg(constraint.value, 0, 'f', 2); break;
        case SketchConstraintType::Angle: glyph = QStringLiteral("∠ %1°").arg(constraint.value, 0, 'f', 2); break;
        case SketchConstraintType::Radius: glyph = QStringLiteral("R %1 mm").arg(constraint.value, 0, 'f', 2); break;
        }
        auto* label = scene()->addSimpleText(glyph);
        label->setBrush(QColor(QStringLiteral("#087f83")));
        label->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        label->setPos(entity->type == SketchEntityType::Line ? (entity->first + entity->second) / 2.0 : entity->first + QPointF(entity->radius, 0));
    }
    auto* planeLabel = scene()->addSimpleText(QStringLiteral("%1 plane").arg(m_plane));
    planeLabel->setBrush(QColor(QStringLiteral("#607284")));
    planeLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    planeLabel->setPos(mapToScene(viewport()->rect().topLeft()) + QPointF(10, -10));
}

void SketchCanvas::updateSolveState() {
    const auto result = m_model.solve();
    emit solveStateChanged(result.state, result.degreesOfFreedom, result.message);
}
