#include "GeometryKernel.h"
#include "SketchModel.h"

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <cmath>
#include <iostream>

namespace {
double volume(const std::shared_ptr<KernelShape>& shape) {
    GProp_GProps properties;
    BRepGProp::VolumeProperties(shape->nativeShape(), properties);
    return properties.Mass();
}
}

int main() {
    QString error;
    const auto box = GeometryKernel::extrudeRectangle(100.0, 60.0, 20.0, &error);
    if (!box || box->isNull() || std::abs(volume(box) - 120000.0) > 0.01) {
        std::cerr << "Rectangle extrusion failed\n";
        return 1;
    }
    if (GeometryKernel::extrudeRectangle(-1.0, 60.0, 20.0, &error) || error.isEmpty()) {
        std::cerr << "Invalid extrusion was accepted\n";
        return 2;
    }
    error.clear();
    const auto fillet = GeometryKernel::filletAllEdges(box, 2.0, &error);
    if (!fillet || fillet->isNull() || volume(fillet) >= volume(box)) {
        std::cerr << "Fillet failed\n";
        return 3;
    }
    error.clear();
    const auto chamfer = GeometryKernel::chamferAllEdges(box, 2.0, &error);
    if (!chamfer || chamfer->isNull() || volume(chamfer) >= volume(box)) {
        std::cerr << "Chamfer failed\n";
        return 4;
    }
    SketchModel rectangle;
    rectangle.addLine({0, 0}, {40, 0});
    rectangle.addLine({40, 0}, {40, 20});
    rectangle.addLine({40, 20}, {0, 20});
    rectangle.addLine({0, 20}, {0, 0});
    const auto profileSolid = GeometryKernel::extrudeSketch(rectangle, 10.0, QStringLiteral("XY"), &error);
    if (!profileSolid || std::abs(volume(profileSolid) - 8000.0) > 0.01) return 5;
    SketchModel circle;
    circle.addCircle({0, 0}, 10.0);
    const auto cylinder = GeometryKernel::extrudeSketch(circle, 12.0, QStringLiteral("YZ"), &error);
    if (!cylinder || std::abs(volume(cylinder) - (1200.0 * std::acos(-1.0))) > 0.05) return 6;
    SketchModel halfCircle;
    halfCircle.addArc({-10, 0}, {0, 10}, {10, 0});
    halfCircle.addLine({10, 0}, {-10, 0});
    const auto halfCylinder = GeometryKernel::extrudeSketch(halfCircle, 8.0, QStringLiteral("XZ"), &error);
    if (!halfCylinder || std::abs(volume(halfCylinder) - (400.0 * std::acos(-1.0))) > 0.05) return 7;
    return 0;
}
