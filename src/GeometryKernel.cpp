#include "GeometryKernel.h"
#include "SketchModel.h"

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Standard_Failure.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_StepModelType.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Writer.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <GC_MakeArcOfCircle.hxx>

KernelShape::KernelShape(TopoDS_Shape value) : m_shape(std::move(value)) {}
const TopoDS_Shape& KernelShape::nativeShape() const { return m_shape; }
bool KernelShape::isNull() const { return m_shape.IsNull(); }

std::shared_ptr<KernelShape> GeometryKernel::makeDemoPart(double length, double width,
    double thickness, double bossRadius, double bossHeight) {
    const auto plate = BRepPrimAPI_MakeBox(length, width, thickness).Shape();
    const gp_Ax2 axis(gp_Pnt(length / 2.0, width / 2.0, thickness), gp_Dir(0, 0, 1));
    BRepAlgoAPI_Fuse fuse(plate, BRepPrimAPI_MakeCylinder(axis, bossRadius, bossHeight).Shape());
    fuse.Build();
    return fuse.IsDone() ? std::make_shared<KernelShape>(fuse.Shape()) : nullptr;
}

std::shared_ptr<KernelShape> GeometryKernel::extrudeRectangle(double width, double height,
    double distance, QString* error) {
    if (width <= 0.0 || height <= 0.0 || distance <= 0.0) {
        if (error) *error = QStringLiteral("Width, height and distance must be positive.");
        return nullptr;
    }
    const auto result = BRepPrimAPI_MakeBox(width, height, distance).Shape();
    return std::make_shared<KernelShape>(result);
}

std::shared_ptr<KernelShape> GeometryKernel::extrudeSketch(const SketchModel& sketch, double distance,
    const QString& plane, QString* error) {
    if (distance <= 0.0) { if (error) *error = QStringLiteral("Extrusion distance must be positive."); return nullptr; }
    try {
        TopoDS_Face face;
        const gp_Dir normal = plane == QStringLiteral("XZ") ? gp_Dir(0, -1, 0)
            : plane == QStringLiteral("YZ") ? gp_Dir(1, 0, 0) : gp_Dir(0, 0, 1);
        const auto mapPoint = [&plane](const QPointF& point) {
            if (plane == QStringLiteral("XZ")) return gp_Pnt(point.x(), 0, point.y());
            if (plane == QStringLiteral("YZ")) return gp_Pnt(0, point.x(), point.y());
            return gp_Pnt(point.x(), point.y(), 0);
        };
        const auto& entities = sketch.entities();
        if (entities.size() == 1 && entities[0].type == SketchEntityType::Circle) {
            const auto& circle = entities[0];
            BRepBuilderAPI_MakeWire wire(BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(mapPoint(circle.first), normal), circle.radius)));
            face = BRepBuilderAPI_MakeFace(wire.Wire());
        } else {
            if (!sketch.isClosedProfile()) { if (error) *error = QStringLiteral("The sketch profile is not closed."); return nullptr; }
            BRepBuilderAPI_MakeWire wire;
            for (const auto& entity : entities) if (!entity.construction) {
                if (entity.type == SketchEntityType::Line)
                    wire.Add(BRepBuilderAPI_MakeEdge(mapPoint(entity.first), mapPoint(entity.second)));
                else if (entity.type == SketchEntityType::Arc) {
                    GC_MakeArcOfCircle arc(mapPoint(entity.first), mapPoint(entity.second), mapPoint(entity.third));
                    if (!arc.IsDone()) { if (error) *error = QStringLiteral("An arc is degenerate."); return nullptr; }
                    wire.Add(BRepBuilderAPI_MakeEdge(arc.Value()));
                }
            }
            if (!wire.IsDone()) { if (error) *error = QStringLiteral("The sketch wire could not be built."); return nullptr; }
            face = BRepBuilderAPI_MakeFace(wire.Wire());
        }
        BRepPrimAPI_MakePrism prism(face, gp_Vec(normal) * distance);
        prism.Build();
        if (prism.IsDone()) return std::make_shared<KernelShape>(prism.Shape());
    } catch (const Standard_Failure&) {}
    if (error) *error = QStringLiteral("The sketch could not be extruded.");
    return nullptr;
}

std::shared_ptr<KernelShape> GeometryKernel::filletAllEdges(const std::shared_ptr<KernelShape>& shape,
    double radius, QString* error) {
    if (!shape || shape->isNull() || radius <= 0.0) {
        if (error) *error = QStringLiteral("A valid solid and positive radius are required.");
        return nullptr;
    }
    try {
        BRepFilletAPI_MakeFillet builder(shape->nativeShape());
        for (TopExp_Explorer edge(shape->nativeShape(), TopAbs_EDGE); edge.More(); edge.Next())
            builder.Add(radius, TopoDS::Edge(edge.Current()));
        builder.Build();
        if (builder.IsDone()) return std::make_shared<KernelShape>(builder.Shape());
    } catch (const Standard_Failure&) {}
    if (error) *error = QStringLiteral("The selected radius cannot be applied to all edges.");
    return nullptr;
}

std::shared_ptr<KernelShape> GeometryKernel::chamferAllEdges(const std::shared_ptr<KernelShape>& shape,
    double distance, QString* error) {
    if (!shape || shape->isNull() || distance <= 0.0) {
        if (error) *error = QStringLiteral("A valid solid and positive distance are required.");
        return nullptr;
    }
    try {
        BRepFilletAPI_MakeChamfer builder(shape->nativeShape());
        for (TopExp_Explorer edge(shape->nativeShape(), TopAbs_EDGE); edge.More(); edge.Next())
            builder.Add(distance, TopoDS::Edge(edge.Current()));
        builder.Build();
        if (builder.IsDone()) return std::make_shared<KernelShape>(builder.Shape());
    } catch (const Standard_Failure&) {}
    if (error) *error = QStringLiteral("The selected distance cannot be applied to all edges.");
    return nullptr;
}

bool GeometryKernel::exportStep(const std::shared_ptr<KernelShape>& shape, const QString& path, QString* error) {
    if (!shape || shape->isNull()) { if (error) *error = QStringLiteral("No geometry to export."); return false; }
    STEPControl_Writer writer;
    const bool ok = writer.Transfer(shape->nativeShape(), STEPControl_AsIs) == IFSelect_RetDone
        && writer.Write(path.toUtf8().constData()) == IFSelect_RetDone;
    if (!ok && error) *error = QStringLiteral("STEP export failed.");
    return ok;
}

bool GeometryKernel::exportStl(const std::shared_ptr<KernelShape>& shape, const QString& path, QString* error) {
    if (!shape || shape->isNull()) { if (error) *error = QStringLiteral("No geometry to export."); return false; }
    BRepMesh_IncrementalMesh mesh(shape->nativeShape(), 0.1, false, 0.35, true);
    StlAPI_Writer writer;
    const bool ok = writer.Write(shape->nativeShape(), path.toUtf8().constData());
    if (!ok && error) *error = QStringLiteral("STL export failed.");
    return ok;
}
