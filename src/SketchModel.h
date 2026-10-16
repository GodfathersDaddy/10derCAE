#pragma once

#include <QJsonObject>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QVector>

enum class SketchEntityType { Line, Circle, Arc };
enum class SketchConstraintType { Coincident, Horizontal, Vertical, Parallel, Perpendicular, Equal, Concentric, Tangent, InternalTangent, Symmetric, Distance, PointDistance, Angle, Radius, Fixed };
enum class SketchSolveState { UnderConstrained, FullyConstrained, OverConstrained };

struct SketchEntity {
    QString id;
    SketchEntityType type = SketchEntityType::Line;
    QPointF first;
    QPointF second;
    QPointF third;
    double radius = 0.0;
    bool construction = false;
    bool fixed = false;
};

struct SketchConstraint {
    QString id;
    SketchConstraintType type = SketchConstraintType::Coincident;
    QString entityA;
    QString entityB;
    int pointA = 0;
    int pointB = 0;
    double value = 0.0;
    QString entityC;
};

struct SketchSolveResult {
    SketchSolveState state = SketchSolveState::UnderConstrained;
    int degreesOfFreedom = 0;
    double maximumResidual = 0.0;
    QString message;
    QStringList unsatisfiedConstraintIds;
};

class SketchModel {
public:
    QString addLine(const QPointF& start, const QPointF& end, bool construction = false);
    QString addCircle(const QPointF& center, double radius, bool construction = false);
    QString addArc(const QPointF& start, const QPointF& pointOnArc, const QPointF& end,
        bool construction = false);
    QString addConstraint(SketchConstraintType type, const QString& entityA,
        const QString& entityB = {}, int pointA = 0, int pointB = 0, double value = 0.0);
    QString addSymmetryConstraint(const QString& entityA, const QString& entityB, const QString& symmetryAxis);
    bool removeEntity(const QString& id);
    bool removeConstraint(const QString& id);
    SketchSolveResult solve(int maximumIterations = 100, double tolerance = 1.0e-7);
    bool isClosedProfile(double tolerance = 1.0e-6) const;
    const QVector<SketchEntity>& entities() const;
    const QVector<SketchConstraint>& constraints() const;
    QJsonObject toJson() const;
    static SketchModel fromJson(const QJsonObject& object, QString* error = nullptr);

private:
    SketchEntity* findEntity(const QString& id);
    const SketchEntity* findEntity(const QString& id) const;
    int estimateDegreesOfFreedom() const;
    QVector<SketchEntity> m_entities;
    QVector<SketchConstraint> m_constraints;
    int m_nextEntity = 1;
    int m_nextConstraint = 1;
};

