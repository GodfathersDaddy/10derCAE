#include "OcctViewport.h"

#include "GeometryKernel.h"

#include <Aspect_DisplayConnection.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <Graphic3d_Camera.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Quantity_Color.hxx>
#include <WNT_Window.hxx>
#include <Image_PixMap.hxx>
#include <Image_Format.hxx>

#include <QGridLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QShowEvent>
#include <QWheelEvent>
#include <algorithm>
#include <cstring>

OcctViewport::OcctViewport(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("occtViewport"));
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    createNavigationOverlay();
}

OcctViewport::~OcctViewport() {
    if (!m_context.IsNull()) m_context->RemoveAll(false);
    m_presentedShape.Nullify();
    m_context.Nullify();
    m_view.Nullify();
    m_viewer.Nullify();
}

void OcctViewport::initializeOcct() {
    if (!m_view.IsNull()) return;
    Handle(OpenGl_GraphicDriver) driver = new OpenGl_GraphicDriver(Handle(Aspect_DisplayConnection)());
    m_viewer = new V3d_Viewer(driver);
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();
    m_viewer->SetDefaultBackgroundColor(Quantity_NOC_GRAY90);
    m_view = m_viewer->CreateView();
    Handle(WNT_Window) window = new WNT_Window(reinterpret_cast<Aspect_Handle>(winId()));
    m_view->SetWindow(window);
    if (!window->IsMapped()) window->Map();
    m_view->MustBeResized();
    m_view->SetBgGradientColors(Quantity_Color(0.90, 0.93, 0.96, Quantity_TOC_RGB),
        Quantity_Color(0.99, 0.99, 1.0, Quantity_TOC_RGB), Aspect_GFM_VER, true);
    m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GRAY40, 0.075, V3d_ZBUFFER);
    m_context = new AIS_InteractiveContext(m_viewer);
    m_context->SetAutomaticHilight(true);
    if (m_shape) setShape(m_shape);
    homeView();
    m_view->Invalidate();
    m_view->Update();
}

void OcctViewport::setShape(const std::shared_ptr<KernelShape>& shape) {
    m_shape = shape;
    if (m_context.IsNull() || !shape || shape->isNull()) return;
    if (!m_presentedShape.IsNull()) m_context->Remove(m_presentedShape, false);
    m_presentedShape = new AIS_Shape(shape->nativeShape());
    m_presentedShape->SetColor(Quantity_Color(0.72, 0.78, 0.84, Quantity_TOC_RGB));
    m_presentedShape->SetMaterial(Graphic3d_NOM_SATIN);
    m_presentedShape->Attributes()->SetFaceBoundaryDraw(true);
    m_presentedShape->Attributes()->SetFaceBoundaryAspect(
        new Prs3d_LineAspect(Quantity_NOC_GRAY30, Aspect_TOL_SOLID, 1.0));
    m_context->Display(m_presentedShape, AIS_Shaded, 0, false);
    m_context->UpdateCurrentViewer();
    fitAll();
    m_view->Update();
}

void OcctViewport::fitAll() {
    if (m_view.IsNull()) return;
    m_view->FitAll(0.08, false);
    m_view->ZFitAll();
    m_view->Redraw();
    emit navigationChanged();
}

void OcctViewport::homeView() {
    if (m_view.IsNull()) return;
    m_view->SetProj(V3d_XposYnegZpos);
    fitAll();
}

void OcctViewport::setStandardView(StandardView orientation) {
    if (m_view.IsNull()) return;
    switch (orientation) {
    case StandardView::Front: m_view->SetProj(V3d_Yneg); break;
    case StandardView::Back: m_view->SetProj(V3d_Ypos); break;
    case StandardView::Left: m_view->SetProj(V3d_Xpos); break;
    case StandardView::Right: m_view->SetProj(V3d_Xneg); break;
    case StandardView::Top: m_view->SetProj(V3d_Zpos); break;
    case StandardView::Bottom: m_view->SetProj(V3d_Zneg); break;
    case StandardView::Isometric: m_view->SetProj(V3d_XposYnegZpos); break;
    }
    fitAll();
}

void OcctViewport::toggleProjection() {
    if (m_view.IsNull()) return;
    m_perspective = !m_perspective;
    m_view->Camera()->SetProjectionType(m_perspective
        ? Graphic3d_Camera::Projection_Perspective : Graphic3d_Camera::Projection_Orthographic);
    fitAll();
}

bool OcctViewport::isPerspective() const { return m_perspective; }

bool OcctViewport::dumpImage(const QString& path) const {
    return !m_view.IsNull() && m_view->Dump(path.toUtf8().constData());
}

void OcctViewport::showEvent(QShowEvent* event) { QWidget::showEvent(event); initializeOcct(); }
QImage OcctViewport::renderFrame() const {
    if (m_view.IsNull() || width() <= 0 || height() <= 0) return {};
    const qreal ratio = devicePixelRatioF();
    const int imageWidth = std::max(1, qRound(width() * ratio));
    const int imageHeight = std::max(1, qRound(height() * ratio));
    Image_PixMap pixels;
    if (!m_view->ToPixMap(pixels, imageWidth, imageHeight)) return {};
    QImage::Format format = QImage::Format_Invalid;
    if (pixels.Format() == Image_Format_RGB) format = QImage::Format_RGB888;
    else if (pixels.Format() == Image_Format_BGR) format = QImage::Format_BGR888;
    else if (pixels.Format() == Image_Format_RGBA) format = QImage::Format_RGBA8888;
    else if (pixels.Format() == Image_Format_BGRA) format = QImage::Format_ARGB32;
    else if (pixels.Format() == Image_Format_RGB32 || pixels.Format() == Image_Format_BGR32) format = QImage::Format_RGB32;
    if (format == QImage::Format_Invalid) return {};
    QImage result(imageWidth, imageHeight, format);
    for (int y = 0; y < imageHeight; ++y) {
        const int sourceRow = pixels.IsTopDown() ? y : imageHeight - 1 - y;
        std::memcpy(result.scanLine(y), pixels.Row(sourceRow),
            std::min(static_cast<size_t>(result.bytesPerLine()), pixels.SizeRowBytes()));
    }
    result.setDevicePixelRatio(ratio);
    return result;
}

void OcctViewport::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(QStringLiteral("#eef2f5")));
    const QImage frame = renderFrame();
    if (!frame.isNull()) painter.drawImage(QPoint(0, 0), frame);
}
void OcctViewport::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (!m_view.IsNull()) {
        m_view->MustBeResized();
        m_view->Invalidate();
        m_view->Update();
    }
    if (m_navigation) m_navigation->move(width() - m_navigation->width() - 14, 14);
}

void OcctViewport::mousePressEvent(QMouseEvent* event) {
    m_lastMouse = event->position().toPoint();
    if (event->button() == Qt::MiddleButton) {
        m_panning = event->modifiers().testFlag(Qt::ShiftModifier);
        m_rotating = !m_panning;
        if (m_rotating && !m_view.IsNull()) m_view->StartRotation(m_lastMouse.x(), m_lastMouse.y());
    } else if (event->button() == Qt::LeftButton && !m_context.IsNull()) {
        m_context->SelectDetected(AIS_SelectionScheme_Replace);
        emit selectionChanged(m_context->NbSelected() > 0 ? QStringLiteral("1") : QStringLiteral("0"));
    }
    QWidget::mousePressEvent(event);
}

void OcctViewport::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) { m_rotating = false; m_panning = false; }
    QWidget::mouseReleaseEvent(event);
}

void OcctViewport::mouseMoveEvent(QMouseEvent* event) {
    const QPoint point = event->position().toPoint();
    if (!m_view.IsNull() && m_rotating) m_view->Rotation(point.x(), point.y());
    else if (!m_view.IsNull() && m_panning) {
        m_view->Pan(point.x() - m_lastMouse.x(), m_lastMouse.y() - point.y());
        m_lastMouse = point;
    } else if (!m_context.IsNull() && !m_view.IsNull()) {
        m_context->MoveTo(point.x(), point.y(), m_view, true);
    }
    update();
    QWidget::mouseMoveEvent(event);
}

void OcctViewport::wheelEvent(QWheelEvent* event) {
    if (!m_view.IsNull()) {
        const QPoint point = event->position().toPoint();
        const int amount = event->angleDelta().y() / 8;
        m_view->StartZoomAtPoint(point.x(), point.y());
        m_view->ZoomAtPoint(0, 0, amount, amount);
        emit navigationChanged();
        update();
    }
    event->accept();
}

void OcctViewport::createNavigationOverlay() {
    m_navigation = new QWidget(this);
    m_navigation->setObjectName(QStringLiteral("viewCube"));
    m_navigation->setFixedSize(116, 116);
    auto* grid = new QGridLayout(m_navigation);
    grid->setContentsMargins(4, 4, 4, 4);
    grid->setSpacing(2);
    auto add = [this, grid](const QString& text, int row, int column, StandardView view) {
        auto* button = new QPushButton(text, m_navigation);
        button->setFixedSize(34, 30);
        connect(button, &QPushButton::clicked, this, [this, view] { setStandardView(view); });
        grid->addWidget(button, row, column);
    };
    add(QStringLiteral("T"), 0, 1, StandardView::Top);
    add(QStringLiteral("L"), 1, 0, StandardView::Left);
    add(QStringLiteral("F"), 1, 1, StandardView::Front);
    add(QStringLiteral("R"), 1, 2, StandardView::Right);
    add(QStringLiteral("B"), 2, 1, StandardView::Bottom);
    add(QStringLiteral("ISO"), 2, 2, StandardView::Isometric);
    m_navigation->raise();
}
