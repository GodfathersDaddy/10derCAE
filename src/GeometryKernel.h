#pragma once

#include <TopoDS_Shape.hxx>
#include <QString>
#include <memory>

class KernelShape {
public:
    explicit KernelShape(TopoDS_Shape shape);
    const TopoDS_Shape& nativeShape() const;
    bool isNull() const;
private:
    TopoDS_Shape m_shape;
};
class SketchModel;

class GeometryKernel {
public:
    static std::shared_ptr<KernelShape> makeDemoPart(double length = 120.0, double width = 80.0,
        double thickness = 8.0, double bossRadius = 18.0, double bossHeight = 24.0);
    static std::shared_ptr<KernelShape> extrudeRectangle(double width, double height, double distance,
        QString* error = nullptr);
    static std::shared_ptr<KernelShape> extrudeSketch(const SketchModel& sketch, double distance,
        const QString& plane = QStringLiteral("XY"), QString* error = nullptr);
    static std::shared_ptr<KernelShape> filletAllEdges(const std::shared_ptr<KernelShape>& shape,
        double radius, QString* error = nullptr);
    static std::shared_ptr<KernelShape> chamferAllEdges(const std::shared_ptr<KernelShape>& shape,
        double distance, QString* error = nullptr);
    static bool exportStep(const std::shared_ptr<KernelShape>& shape, const QString& path, QString* error = nullptr);
    static bool exportStl(const std::shared_ptr<KernelShape>& shape, const QString& path, QString* error = nullptr);
};
