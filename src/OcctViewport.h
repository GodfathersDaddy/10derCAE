#pragma once

#include <QPoint>
#include <QImage>
#include <QWidget>
#include <memory>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

class KernelShape;

class OcctViewport final : public QWidget {
    Q_OBJECT
public:
    enum class StandardView { Isometric, Front, Back, Left, Right, Top, Bottom };

    explicit OcctViewport(QWidget* parent = nullptr);
    ~OcctViewport() override;
    void setShape(const std::shared_ptr<KernelShape>& shape);
    void fitAll();
    void homeView();
    void setStandardView(StandardView orientation);
    void toggleProjection();
    bool isPerspective() const;
    bool dumpImage(const QString& path) const;

signals:
    void selectionChanged(const QString& description);
    void navigationChanged();

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void initializeOcct();
    void createNavigationOverlay();
    QImage renderFrame() const;

    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    Handle(AIS_Shape) m_presentedShape;
    std::shared_ptr<KernelShape> m_shape;
    QPoint m_lastMouse;
    bool m_rotating = false;
    bool m_panning = false;
    bool m_perspective = false;
    QWidget* m_navigation = nullptr;
};
