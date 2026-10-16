#include "CadViewport.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <algorithm>

CadViewport::CadViewport(QWidget* parent) : QWidget(parent) {
    setMinimumSize(560, 400);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void CadViewport::homeView() {
    m_yaw = -28.0;
    m_pitch = 23.0;
    m_zoom = 1.0;
    update();
}

void CadViewport::setFeatureCount(int count) {
    m_featureCount = count;
    update();
}

void CadViewport::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#f5f7fa"));

    painter.setPen(QPen(QColor("#e1e7ee"), 1));
    const int grid = std::max(24, static_cast<int>(42 * m_zoom));
    for (int x = width() / 2 % grid; x < width(); x += grid) painter.drawLine(x, 0, x, height());
    for (int y = height() / 2 % grid; y < height(); y += grid) painter.drawLine(0, y, width(), y);

    painter.translate(width() * 0.52, height() * 0.56);
    painter.rotate(m_yaw * 0.12);
    painter.scale(m_zoom, m_zoom);

    const QPointF a(-190, 35), b(70, 120), c(210, 25), d(-50, -60);
    QPainterPath side;
    side.moveTo(a); side.lineTo(b); side.lineTo(b + QPointF(0, 28));
    side.lineTo(a + QPointF(0, 28)); side.closeSubpath();
    painter.fillPath(side, QColor("#6f7d8e"));
    painter.setPen(QPen(QColor("#415063"), 1.3));
    painter.drawPath(side);

    QPolygonF plate({a, b, c, d});
    painter.setBrush(QColor("#b9c3d0"));
    painter.drawPolygon(plate);

    if (m_featureCount > 0) {
        painter.setBrush(QColor("#a8b4c3"));
        painter.drawEllipse(QPointF(15, 14), 58, 36);
        painter.setBrush(QColor("#8795a6"));
        QRectF boss(-43, -48, 116, 72);
        painter.drawPie(boss.translated(0, 43), 180 * 16, 180 * 16);
        painter.setBrush(QColor("#c3ccd7"));
        painter.drawEllipse(boss);
    }

    painter.resetTransform();
    painter.setPen(QColor("#59697b"));
    painter.drawText(22, 30, QStringLiteral("Orbit: mittlere Maustaste  •  Zoom: Mausrad"));
    painter.drawText(22, height() - 22,
                     QStringLiteral("Präziser B-Rep-Kern: Open CASCADE  •  Anzeigeadapter folgt"));

    const QRect cube(width() - 116, 24, 78, 78);
    painter.setBrush(QColor("#eef3f7"));
    painter.setPen(QColor("#728398"));
    painter.drawRect(cube);
    painter.drawText(cube, Qt::AlignCenter, QStringLiteral("OBEN\nVORN"));
}

void CadViewport::mousePressEvent(QMouseEvent* event) { m_lastPointer = event->pos(); }

void CadViewport::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons().testFlag(Qt::MiddleButton)) {
        const QPoint delta = event->pos() - m_lastPointer;
        m_yaw += delta.x();
        m_pitch += delta.y();
        m_lastPointer = event->pos();
        update();
    }
}

void CadViewport::wheelEvent(QWheelEvent* event) {
    m_zoom = std::clamp(m_zoom * (event->angleDelta().y() > 0 ? 1.1 : 0.9), 0.35, 3.5);
    update();
}

