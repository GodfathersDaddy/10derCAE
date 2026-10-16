#include "SketchModel.h"
#include <QLineF>
#include <cmath>
#include <iostream>

int main() {
    SketchModel sketch;
    const QString a = sketch.addLine({0, 0}, {9, 1});
    const QString b = sketch.addLine({9, 1}, {8, 10});
    sketch.addConstraint(SketchConstraintType::Horizontal, a);
    sketch.addConstraint(SketchConstraintType::Perpendicular, a, b);
    sketch.addConstraint(SketchConstraintType::Coincident, a, b, 1, 0);
    auto result = sketch.solve();
    if (result.state == SketchSolveState::OverConstrained) return 1;
    const auto& entities = sketch.entities();
    if (std::abs(entities[0].first.y() - entities[0].second.y()) > 1e-7) return 2;
    const QPointF av = entities[0].second - entities[0].first;
    const QPointF bv = entities[1].second - entities[1].first;
    if (std::abs(QPointF::dotProduct(av, bv)) > 1e-6) return 3;

    SketchModel rectangle;
    const QString l1 = rectangle.addLine({0, 0}, {10, 0});
    const QString l2 = rectangle.addLine({10, 0}, {10, 5});
    const QString l3 = rectangle.addLine({10, 5}, {0, 5});
    const QString l4 = rectangle.addLine({0, 5}, {0, 0});
    rectangle.addConstraint(SketchConstraintType::Horizontal, l1);
    rectangle.addConstraint(SketchConstraintType::Vertical, l2);
    rectangle.addConstraint(SketchConstraintType::Horizontal, l3);
    rectangle.addConstraint(SketchConstraintType::Vertical, l4);
    if (!rectangle.isClosedProfile()) return 4;

    QString error;
    const auto restored = SketchModel::fromJson(rectangle.toJson(), &error);
    if (!error.isEmpty() || restored.entities().size() != 4 || restored.constraints().size() != 4 || !restored.isClosedProfile()) return 5;

    SketchModel conflict;
    const QString line = conflict.addLine({0, 0}, {10, 0});
    conflict.addConstraint(SketchConstraintType::Fixed, line);
    conflict.addConstraint(SketchConstraintType::Vertical, line);
    if (conflict.solve().state != SketchSolveState::OverConstrained) return 6;

    SketchModel dimensions;
    const QString da = dimensions.addLine({0, 0}, {10, 0});
    const QString db = dimensions.addLine({10, 0}, {14, 4});
    dimensions.addConstraint(SketchConstraintType::Fixed, da);
    dimensions.addConstraint(SketchConstraintType::Angle, da, db, 0, 0, 90.0);
    dimensions.addConstraint(SketchConstraintType::PointDistance, da, db, 1, 0, 5.0);
    const auto dimensionResult = dimensions.solve();
    if (dimensionResult.state == SketchSolveState::OverConstrained) return 7;
    const auto& solved = dimensions.entities();
    const QPointF firstDirection = solved[0].second - solved[0].first;
    const QPointF secondDirection = solved[1].second - solved[1].first;
    if (std::abs(QPointF::dotProduct(firstDirection, secondDirection)) > 1e-6) return 8;
    if (std::abs(QLineF(solved[0].second, solved[1].first).length() - 5.0) > 1e-6) return 9;
    QString dimensionError;
    const auto dimensionCopy = SketchModel::fromJson(dimensions.toJson(), &dimensionError);
    if (!dimensionError.isEmpty() || dimensionCopy.constraints().size() != 3) return 10;

    SketchModel arcProfile;
    const QString arc = arcProfile.addArc({-10, 0}, {0, 10}, {10, 0});
    const QString diameter = arcProfile.addLine({10, 0}, {-10, 0});
    arcProfile.addConstraint(SketchConstraintType::Coincident, arc, diameter, 2, 0);
    arcProfile.addConstraint(SketchConstraintType::Coincident, diameter, arc, 1, 0);
    if (!arcProfile.isClosedProfile()) return 11;
    QString arcError;
    const auto arcCopy = SketchModel::fromJson(arcProfile.toJson(), &arcError);
    if (!arcError.isEmpty() || arcCopy.entities().size() != 2 || arcCopy.entities()[0].type != SketchEntityType::Arc
        || !arcCopy.isClosedProfile()) return 12;

    SketchModel curveConstraints;
    const QString fixedCircle = curveConstraints.addCircle({0, 0}, 10.0);
    const QString movingCircle = curveConstraints.addCircle({30, 5}, 4.0);
    curveConstraints.addConstraint(SketchConstraintType::Fixed, fixedCircle);
    curveConstraints.addConstraint(SketchConstraintType::Concentric, fixedCircle, movingCircle);
    curveConstraints.addConstraint(SketchConstraintType::Radius, movingCircle, {}, 0, 0, 6.0);
    if (curveConstraints.solve().state == SketchSolveState::OverConstrained) return 13;
    if (QLineF(curveConstraints.entities()[0].first, curveConstraints.entities()[1].first).length() > 1e-7
        || std::abs(curveConstraints.entities()[1].radius - 6.0) > 1e-7) return 14;

    SketchModel tangent;
    const QString tangentLine = tangent.addLine({-20, 0}, {20, 0});
    const QString tangentCircle = tangent.addCircle({0, 8}, 3.0);
    tangent.addConstraint(SketchConstraintType::Fixed, tangentLine);
    tangent.addConstraint(SketchConstraintType::Tangent, tangentLine, tangentCircle);
    tangent.solve();
    if (std::abs(std::abs(tangent.entities()[1].first.y()) - 3.0) > 1e-7) return 15;

    SketchModel arcRadiusModel;
    const QString radiusArc = arcRadiusModel.addArc({-10, 0}, {0, 10}, {10, 0});
    const QString radiusConstraint = arcRadiusModel.addConstraint(SketchConstraintType::Radius, radiusArc, {}, 0, 0, 20.0);
    if (radiusConstraint.isEmpty() || arcRadiusModel.addConstraint(SketchConstraintType::Radius, radiusArc, {}, 0, 0, 20.0) != radiusConstraint
        || arcRadiusModel.constraints().size() != 1) return 16;
    arcRadiusModel.solve();
    if (std::abs(QLineF(QPointF(0, 0), arcRadiusModel.entities()[0].first).length() - 20.0) > 1e-6) return 17;

    SketchModel symmetry;
    const QString original = symmetry.addLine({2, 1}, {5, 4});
    const QString mirrored = symmetry.addLine({-1, 8}, {-3, 9});
    const QString axis = symmetry.addLine({0, -10}, {0, 10}, true);
    symmetry.addConstraint(SketchConstraintType::Fixed, original);
    symmetry.addConstraint(SketchConstraintType::Fixed, axis);
    const QString symmetryId = symmetry.addSymmetryConstraint(original, mirrored, axis);
    if (symmetryId.isEmpty() || symmetry.addSymmetryConstraint(original, mirrored, axis) != symmetryId) return 18;
    if (symmetry.solve().state == SketchSolveState::OverConstrained) return 19;
    if (QLineF(symmetry.entities()[1].first, QPointF(-2, 1)).length() > 1e-7
        || QLineF(symmetry.entities()[1].second, QPointF(-5, 4)).length() > 1e-7) return 20;
    QString symmetryError;
    const auto symmetryCopy = SketchModel::fromJson(symmetry.toJson(), &symmetryError);
    if (!symmetryError.isEmpty() || symmetryCopy.constraints().size() != 3
        || symmetryCopy.constraints().last().entityC != axis) return 21;
    SketchModel invalidSymmetry;
    const QString onlyLine = invalidSymmetry.addLine({0, 0}, {1, 0});
    if (!invalidSymmetry.addSymmetryConstraint(onlyLine, onlyLine, onlyLine).isEmpty()) return 22;

    SketchModel internalTangent;
    const QString outerCircle = internalTangent.addCircle({0, 0}, 10.0);
    const QString innerCircle = internalTangent.addCircle({2, 7}, 4.0);
    internalTangent.addConstraint(SketchConstraintType::Fixed, outerCircle);
    const QString internalId = internalTangent.addConstraint(SketchConstraintType::InternalTangent, outerCircle, innerCircle);
    if (internalId.isEmpty() || internalTangent.solve().state == SketchSolveState::OverConstrained) return 23;
    if (std::abs(QLineF(internalTangent.entities()[0].first, internalTangent.entities()[1].first).length() - 6.0) > 1e-7) return 24;
    if (!internalTangent.addConstraint(SketchConstraintType::InternalTangent, outerCircle, innerCircle).startsWith(QStringLiteral("constraint-"))
        || internalTangent.constraints().size() != 2) return 25;
    QString internalError;
    const auto internalCopy = SketchModel::fromJson(internalTangent.toJson(), &internalError);
    if (!internalError.isEmpty() || internalCopy.constraints().last().type != SketchConstraintType::InternalTangent) return 26;
    const QString invalidLine = internalTangent.addLine({0, 0}, {5, 0});
    if (!internalTangent.addConstraint(SketchConstraintType::InternalTangent, invalidLine, innerCircle).isEmpty()) return 27;

    SketchModel reverseInternal;
    const QString movableOuter = reverseInternal.addCircle({20, 0}, 10.0);
    const QString fixedInner = reverseInternal.addCircle({0, 0}, 4.0);
    reverseInternal.addConstraint(SketchConstraintType::Fixed, fixedInner);
    reverseInternal.addConstraint(SketchConstraintType::InternalTangent, movableOuter, fixedInner);
    if (reverseInternal.solve().state == SketchSolveState::OverConstrained
        || std::abs(QLineF(reverseInternal.entities()[0].first, reverseInternal.entities()[1].first).length() - 6.0) > 1e-7) return 28;
    for (bool firstArc : {false, true}) for (bool secondArc : {false, true}) {
        SketchModel curves;
        const QString outer = firstArc ? curves.addArc({-10, 0}, {0, 10}, {10, 0}) : curves.addCircle({0, 0}, 10);
        const QString inner = secondArc ? curves.addArc({-4, 0}, {0, 4}, {4, 0}) : curves.addCircle({0, 0}, 4);
        curves.addConstraint(SketchConstraintType::Fixed, outer);
        if (curves.addConstraint(SketchConstraintType::InternalTangent, outer, inner).isEmpty()
            || curves.solve().state == SketchSolveState::OverConstrained) return 29;
        const auto& moved = curves.entities()[1];
        const QPointF center = secondArc ? (moved.first + moved.third) / 2.0 : moved.first;
        if (std::abs(QLineF(QPointF(), center).length() - 6.0) > 1e-7) return 30;
        QString error;
        auto copy = SketchModel::fromJson(curves.toJson(), &error);
        if (!error.isEmpty() || copy.solve().state == SketchSolveState::OverConstrained) return 31;
    }
    SketchModel equalCircles;
    const QString equalA = equalCircles.addCircle({0, 0}, 5);
    const QString equalB = equalCircles.addCircle({4, 0}, 5);
    if (!equalCircles.addConstraint(SketchConstraintType::InternalTangent, equalA, equalB).isEmpty()) return 32;
    return 0;
}
