#pragma once

#include <QPoint>
#include <QWidget>

class CadViewport final : public QWidget {
    Q_OBJECT
public:
    explicit CadViewport(QWidget* parent = nullptr);

public slots:
    void homeView();
    void setFeatureCount(int count);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QPoint m_lastPointer;
    double m_yaw = -28.0;
    double m_pitch = 23.0;
    double m_zoom = 1.0;
    int m_featureCount = 0;
};

