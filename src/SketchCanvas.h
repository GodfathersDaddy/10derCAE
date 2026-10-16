#pragma once

#include "SketchModel.h"

#include <QGraphicsView>

class QGraphicsLineItem;
class QGraphicsPathItem;

class SketchCanvas final : public QGraphicsView {
    Q_OBJECT
public:
    enum class Tool { Select, Line, Rectangle, Circle, Arc };

    explicit SketchCanvas(QWidget* parent = nullptr);
    void setTool(Tool tool);
    Tool tool() const;
    void setModel(const SketchModel& model);
    const SketchModel& model() const;
    void clearSketch();
    void setPlane(const QString& plane);
    QString plane() const;
    QStringList selectedEntityIds() const;
    bool applyConstraint(SketchConstraintType type, double value = 0.0);

signals:
    void modelChanged();
    void solveStateChanged(SketchSolveState state, int degreesOfFreedom, const QString& message);
    void toolChanged(Tool tool);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct SnapTarget { QPointF point; QString entityId; int pointIndex = 0; bool endpoint = false; };
    SnapTarget snapTarget(const QPointF& point) const;
    QPointF snapped(const QPointF& point, bool includeEndpoints = true) const;
    void finishEntity(const QPointF& end);
    void rebuildScene();
    void updateSolveState();
    void cancelPending();

    SketchModel m_model;
    Tool m_tool = Tool::Select;
    bool m_drawing = false;
    QPointF m_start;
    SnapTarget m_startSnap;
    QGraphicsLineItem* m_previewLine = nullptr;
    QGraphicsEllipseItem* m_previewEllipse = nullptr;
    QGraphicsPathItem* m_previewPath = nullptr;
    QPointF m_arcPoint;
    int m_arcStage = 0;
    double m_grid = 5.0;
    QString m_plane = QStringLiteral("XY");
    bool m_panning = false;
    QPoint m_lastPanPosition;
};

