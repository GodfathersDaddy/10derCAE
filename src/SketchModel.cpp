#include "SketchModel.h"

#include <QJsonArray>
#include <QLineF>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace {
QString entityTypeName(SketchEntityType type) {
    if (type == SketchEntityType::Circle) return QStringLiteral("circle");
    if (type == SketchEntityType::Arc) return QStringLiteral("arc");
    return QStringLiteral("line");
}
SketchEntityType entityType(const QString& name) {
    if (name == QStringLiteral("circle")) return SketchEntityType::Circle;
    if (name == QStringLiteral("arc")) return SketchEntityType::Arc;
    return SketchEntityType::Line;
}
QString constraintTypeName(SketchConstraintType type) {
    switch (type) {
    case SketchConstraintType::Coincident: return QStringLiteral("coincident");
    case SketchConstraintType::Horizontal: return QStringLiteral("horizontal");
    case SketchConstraintType::Vertical: return QStringLiteral("vertical");
    case SketchConstraintType::Parallel: return QStringLiteral("parallel");
    case SketchConstraintType::Perpendicular: return QStringLiteral("perpendicular");
    case SketchConstraintType::Equal: return QStringLiteral("equal");
    case SketchConstraintType::Concentric: return QStringLiteral("concentric");
    case SketchConstraintType::Tangent: return QStringLiteral("tangent");
    case SketchConstraintType::InternalTangent: return QStringLiteral("internalTangent");
    case SketchConstraintType::Symmetric: return QStringLiteral("symmetric");
    case SketchConstraintType::Distance: return QStringLiteral("distance");
    case SketchConstraintType::PointDistance: return QStringLiteral("pointDistance");
    case SketchConstraintType::Angle: return QStringLiteral("angle");
    case SketchConstraintType::Radius: return QStringLiteral("radius");
    case SketchConstraintType::Fixed: return QStringLiteral("fixed");
    }
    return {};
}
SketchConstraintType constraintType(const QString& name) {
    if (name == QStringLiteral("horizontal")) return SketchConstraintType::Horizontal;
    if (name == QStringLiteral("vertical")) return SketchConstraintType::Vertical;
    if (name == QStringLiteral("parallel")) return SketchConstraintType::Parallel;
    if (name == QStringLiteral("perpendicular")) return SketchConstraintType::Perpendicular;
    if (name == QStringLiteral("equal")) return SketchConstraintType::Equal;
    if (name == QStringLiteral("concentric")) return SketchConstraintType::Concentric;
    if (name == QStringLiteral("tangent")) return SketchConstraintType::Tangent;
    if (name == QStringLiteral("internalTangent")) return SketchConstraintType::InternalTangent;
    if (name == QStringLiteral("symmetric")) return SketchConstraintType::Symmetric;
    if (name == QStringLiteral("distance")) return SketchConstraintType::Distance;
    if (name == QStringLiteral("pointDistance")) return SketchConstraintType::PointDistance;
    if (name == QStringLiteral("angle")) return SketchConstraintType::Angle;
    if (name == QStringLiteral("radius")) return SketchConstraintType::Radius;
    if (name == QStringLiteral("fixed")) return SketchConstraintType::Fixed;
    return SketchConstraintType::Coincident;
}
QPointF& point(SketchEntity& entity, int index) {
    if (index == 0) return entity.first;
    if (entity.type == SketchEntityType::Arc && index == 2) return entity.third;
    return entity.second;
}
double length(const SketchEntity& entity) { return std::hypot(entity.second.x() - entity.first.x(), entity.second.y() - entity.first.y()); }
double angle(const SketchEntity& entity) { return std::atan2(entity.second.y() - entity.first.y(), entity.second.x() - entity.first.x()); }
void setLineAngle(SketchEntity& entity, double radians) {
    const double size = length(entity);
    entity.second = entity.first + QPointF(std::cos(radians) * size, std::sin(radians) * size);
}
bool circleData(const SketchEntity& entity, QPointF* center, double* radius) {
    if (entity.type == SketchEntityType::Circle) { *center = entity.first; *radius = entity.radius; return true; }
    if (entity.type != SketchEntityType::Arc) return false;
    const QPointF& a = entity.first; const QPointF& b = entity.second; const QPointF& c = entity.third;
    const double d = 2.0 * (a.x() * (b.y() - c.y()) + b.x() * (c.y() - a.y()) + c.x() * (a.y() - b.y()));
    if (std::abs(d) < 1.0e-10) return false;
    const double aa = a.x() * a.x() + a.y() * a.y();
    const double bb = b.x() * b.x() + b.y() * b.y();
    const double cc = c.x() * c.x() + c.y() * c.y();
    *center = {(aa * (b.y() - c.y()) + bb * (c.y() - a.y()) + cc * (a.y() - b.y())) / d,
        (aa * (c.x() - b.x()) + bb * (a.x() - c.x()) + cc * (b.x() - a.x())) / d};
    *radius = QLineF(*center, a).length();
    return true;
}
void translateCurve(SketchEntity& entity, const QPointF& delta) {
    entity.first += delta; entity.second += delta;
    if (entity.type == SketchEntityType::Arc) entity.third += delta;
}
void setCurveRadius(SketchEntity& entity, double value) {
    if (entity.type == SketchEntityType::Circle) { entity.radius = value; return; }
    QPointF center; double current = 0.0;
    if (!circleData(entity, &center, &current) || current <= 1.0e-10) return;
    const double factor = value / current;
    entity.first = center + (entity.first - center) * factor;
    entity.second = center + (entity.second - center) * factor;
    entity.third = center + (entity.third - center) * factor;
}
QPointF reflectedPoint(const QPointF& source, const SketchEntity& axis) {
    const QPointF direction = axis.second - axis.first;
    const double squaredLength = QPointF::dotProduct(direction, direction);
    if (squaredLength <= 1.0e-20) return source;
    const QPointF projection = axis.first + direction
        * (QPointF::dotProduct(source - axis.first, direction) / squaredLength);
    return projection * 2.0 - source;
}
double symmetryResidual(const SketchEntity& source, const SketchEntity& target, const SketchEntity& axis) {
    double result = QLineF(reflectedPoint(source.first, axis), target.first).length();
    if (source.type == SketchEntityType::Circle)
        return std::max(result, std::abs(source.radius - target.radius));
    result = std::max(result, QLineF(reflectedPoint(source.second, axis), target.second).length());
    if (source.type == SketchEntityType::Arc)
        result = std::max(result, QLineF(reflectedPoint(source.third, axis), target.third).length());
    return result;
}
void mirrorEntity(const SketchEntity& source, SketchEntity& target, const SketchEntity& axis) {
    target.first = reflectedPoint(source.first, axis);
    target.second = reflectedPoint(source.second, axis);
    if (source.type == SketchEntityType::Arc) target.third = reflectedPoint(source.third, axis);
    if (source.type == SketchEntityType::Circle) target.radius = source.radius;
}
}

QString SketchModel::addLine(const QPointF& start, const QPointF& end, bool construction) {
    const QString id = QStringLiteral("line-%1").arg(m_nextEntity++);
    m_entities.push_back({id, SketchEntityType::Line, start, end, {}, 0.0, construction, false});
    return id;
}

QString SketchModel::addSymmetryConstraint(const QString& entityA, const QString& entityB, const QString& symmetryAxis) {
    const auto* a = findEntity(entityA);
    const auto* b = findEntity(entityB);
    const auto* axis = findEntity(symmetryAxis);
    if (!a || !b || !axis || entityA == entityB || entityA == symmetryAxis || entityB == symmetryAxis
        || a->type != b->type || axis->type != SketchEntityType::Line || QLineF(axis->first, axis->second).length() <= 1.0e-10)
        return {};
    for (const auto& existing : m_constraints)
        if (existing.type == SketchConstraintType::Symmetric && existing.entityA == entityA
            && existing.entityB == entityB && existing.entityC == symmetryAxis) return existing.id;
    const QString id = QStringLiteral("constraint-%1").arg(m_nextConstraint++);
    SketchConstraint constraint{id, SketchConstraintType::Symmetric, entityA, entityB};
    constraint.entityC = symmetryAxis;
    m_constraints.push_back(constraint);
    return id;
}

QString SketchModel::addCircle(const QPointF& center, double radius, bool construction) {
    const QString id = QStringLiteral("circle-%1").arg(m_nextEntity++);
    m_entities.push_back({id, SketchEntityType::Circle, center, center, {}, std::max(radius, 1.0e-9), construction, false});
    return id;
}

QString SketchModel::addArc(const QPointF& start, const QPointF& pointOnArc, const QPointF& end, bool construction) {
    const QString id = QStringLiteral("arc-%1").arg(m_nextEntity++);
    m_entities.push_back({id, SketchEntityType::Arc, start, pointOnArc, end, 0.0, construction, false});
    return id;
}

QString SketchModel::addConstraint(SketchConstraintType type, const QString& entityA,
    const QString& entityB, int pointA, int pointB, double value) {
    if (!findEntity(entityA) || (!entityB.isEmpty() && !findEntity(entityB))) return {};
    for (const auto& existing : m_constraints)
        if (existing.type == type && existing.entityA == entityA && existing.entityB == entityB
            && existing.pointA == pointA && existing.pointB == pointB && std::abs(existing.value - value) < 1.0e-10)
            return existing.id;
    const auto* a = findEntity(entityA);
    const auto* b = entityB.isEmpty() ? nullptr : findEntity(entityB);
    const bool aCurve = a && (a->type == SketchEntityType::Circle || a->type == SketchEntityType::Arc);
    const bool bCurve = b && (b->type == SketchEntityType::Circle || b->type == SketchEntityType::Arc);
    if ((type == SketchConstraintType::Radius && !aCurve)
        || (type == SketchConstraintType::Concentric && (!aCurve || !bCurve))
        || (type == SketchConstraintType::Tangent && (!b || (!aCurve && !bCurve)))
        || (type == SketchConstraintType::InternalTangent && (!aCurve || !bCurve))) return {};
    if (type == SketchConstraintType::InternalTangent) {
        QPointF centerA, centerB;
        double radiusA = 0.0, radiusB = 0.0;
        // Equal supporting circles coincide rather than touch internally at one point.
        if (entityA == entityB || !circleData(*a, &centerA, &radiusA) || !circleData(*b, &centerB, &radiusB)
            || std::abs(radiusA - radiusB) <= 1.0e-7) return {};
    }
    const QString id = QStringLiteral("constraint-%1").arg(m_nextConstraint++);
    m_constraints.push_back({id, type, entityA, entityB, pointA, pointB, value});
    if (type == SketchConstraintType::Fixed) findEntity(entityA)->fixed = true;
    return id;
}

bool SketchModel::removeEntity(const QString& id) {
    const auto before = m_entities.size();
    m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(), [&](const auto& item) { return item.id == id; }), m_entities.end());
    if (before == m_entities.size()) return false;
    m_constraints.erase(std::remove_if(m_constraints.begin(), m_constraints.end(), [&](const auto& item) { return item.entityA == id || item.entityB == id || item.entityC == id; }), m_constraints.end());
    return true;
}

bool SketchModel::removeConstraint(const QString& id) {
    const auto before = m_constraints.size();
    m_constraints.erase(std::remove_if(m_constraints.begin(), m_constraints.end(), [&](const auto& item) { return item.id == id; }), m_constraints.end());
    return before != m_constraints.size();
}

SketchSolveResult SketchModel::solve(int maximumIterations, double tolerance) {
    double residual = 0.0;
    for (int iteration = 0; iteration < maximumIterations; ++iteration) {
        residual = 0.0;
        for (const auto& constraint : m_constraints) {
            auto* a = findEntity(constraint.entityA);
            auto* b = findEntity(constraint.entityB);
            auto* c = findEntity(constraint.entityC);
            if (!a) continue;
            switch (constraint.type) {
            case SketchConstraintType::Coincident:
                if (b) {
                    QPointF& pa = point(*a, constraint.pointA);
                    QPointF& pb = point(*b, constraint.pointB);
                    residual = std::max(residual, std::hypot(pa.x() - pb.x(), pa.y() - pb.y()));
                    const QPointF middle = (pa + pb) / 2.0;
                    if (!a->fixed) pa = middle;
                    if (!b->fixed) pb = middle;
                }
                break;
            case SketchConstraintType::Horizontal:
                residual = std::max(residual, std::abs(a->second.y() - a->first.y()));
                if (!a->fixed) a->second.setY(a->first.y());
                break;
            case SketchConstraintType::Vertical:
                residual = std::max(residual, std::abs(a->second.x() - a->first.x()));
                if (!a->fixed) a->second.setX(a->first.x());
                break;
            case SketchConstraintType::Parallel:
            case SketchConstraintType::Perpendicular:
                if (b) {
                    double target = angle(*a) + (constraint.type == SketchConstraintType::Perpendicular ? M_PI_2 : 0.0);
                    residual = std::max(residual, std::abs(std::sin(angle(*b) - target)));
                    if (!b->fixed) setLineAngle(*b, target);
                }
                break;
            case SketchConstraintType::Equal:
                if (b && a->type == SketchEntityType::Line && b->type == SketchEntityType::Line) {
                    residual = std::max(residual, std::abs(length(*a) - length(*b)));
                    if (!b->fixed) { const double old = length(*b); if (old > tolerance) b->second = b->first + (b->second - b->first) * (length(*a) / old); }
                } else if (b && a->type == SketchEntityType::Circle && b->type == SketchEntityType::Circle) {
                    residual = std::max(residual, std::abs(a->radius - b->radius));
                    if (!b->fixed) b->radius = a->radius;
                }
                break;
            case SketchConstraintType::Concentric:
                if (b) {
                    QPointF centerA, centerB; double radiusA = 0.0, radiusB = 0.0;
                    if (circleData(*a, &centerA, &radiusA) && circleData(*b, &centerB, &radiusB)) {
                        residual = std::max(residual, QLineF(centerA, centerB).length());
                        if (!b->fixed) translateCurve(*b, centerA - centerB);
                    }
                }
                break;
            case SketchConstraintType::Tangent:
            case SketchConstraintType::InternalTangent:
                if (b) {
                    SketchEntity* line = a->type == SketchEntityType::Line ? a : b->type == SketchEntityType::Line ? b : nullptr;
                    SketchEntity* curve = line == a ? b : line == b ? a : nullptr;
                    if (line && curve && constraint.type == SketchConstraintType::Tangent) {
                        QPointF center; double radius = 0.0;
                        if (circleData(*curve, &center, &radius)) {
                            const QPointF direction = line->second - line->first;
                            const double lineLength = std::hypot(direction.x(), direction.y());
                            if (lineLength > tolerance) {
                                const QPointF normal(-direction.y() / lineLength, direction.x() / lineLength);
                                const double signedDistance = QPointF::dotProduct(center - line->first, normal);
                                residual = std::max(residual, std::abs(std::abs(signedDistance) - radius));
                                if (!curve->fixed) translateCurve(*curve, normal * ((signedDistance >= 0.0 ? radius : -radius) - signedDistance));
                            }
                        }
                    } else if (!line) {
                        QPointF centerA, centerB; double radiusA = 0.0, radiusB = 0.0;
                        if (circleData(*a, &centerA, &radiusA) && circleData(*b, &centerB, &radiusB)) {
                            QPointF delta = centerB - centerA; double current = std::hypot(delta.x(), delta.y());
                            const double target = constraint.type == SketchConstraintType::InternalTangent
                                ? std::abs(radiusA - radiusB) : radiusA + radiusB;
                            residual = std::max(residual, std::abs(current - target));
                            if (current <= tolerance) { delta = {1, 0}; current = 1.0; }
                            if (!b->fixed) translateCurve(*b, centerA + delta * (target / current) - centerB);
                            else if (!a->fixed) translateCurve(*a, centerB - delta * (target / current) - centerA);
                        }
                    }
                }
                break;
            case SketchConstraintType::Symmetric:
                if (b && c) {
                    residual = std::max(residual, symmetryResidual(*a, *b, *c));
                    if (!b->fixed) mirrorEntity(*a, *b, *c);
                    else if (!a->fixed) mirrorEntity(*b, *a, *c);
                }
                break;
            case SketchConstraintType::Distance:
                if (a->type == SketchEntityType::Line) {
                    residual = std::max(residual, std::abs(length(*a) - constraint.value));
                    if (!a->fixed && length(*a) > tolerance) a->second = a->first + (a->second - a->first) * (constraint.value / length(*a));
                }
                break;
            case SketchConstraintType::PointDistance:
                if (b) {
                    const QPointF pa = point(*a, constraint.pointA);
                    QPointF& pb = point(*b, constraint.pointB);
                    QPointF delta = pb - pa;
                    double current = std::hypot(delta.x(), delta.y());
                    residual = std::max(residual, std::abs(current - constraint.value));
                    if (!b->fixed) {
                        if (current <= tolerance) { delta = QPointF(1, 0); current = 1.0; }
                        pb = pa + delta * (constraint.value / current);
                    }
                }
                break;
            case SketchConstraintType::Angle:
                if (b) {
                    const double target = angle(*a) + qDegreesToRadians(constraint.value);
                    residual = std::max(residual, std::abs(std::sin(angle(*b) - target)));
                    if (!b->fixed) setLineAngle(*b, target);
                }
                break;
            case SketchConstraintType::Radius:
                { QPointF center; double current = 0.0;
                if (circleData(*a, &center, &current)) {
                    residual = std::max(residual, std::abs(current - constraint.value));
                    if (!a->fixed) setCurveRadius(*a, constraint.value);
                } }
                break;
            case SketchConstraintType::Fixed: break;
            }
        }
        if (residual <= tolerance) break;
    }
    const int dof = estimateDegreesOfFreedom();
    SketchSolveResult result;
    result.maximumResidual = residual;
    result.degreesOfFreedom = std::max(0, dof);
    if (residual > tolerance) { result.state = SketchSolveState::OverConstrained; result.message = QStringLiteral("Conflicting constraints"); }
    else if (dof <= 0) { result.state = SketchSolveState::FullyConstrained; result.message = QStringLiteral("Fully constrained"); }
    else { result.state = SketchSolveState::UnderConstrained; result.message = QStringLiteral("%1 degrees of freedom").arg(dof); }
    return result;
}

int SketchModel::estimateDegreesOfFreedom() const {
    int result = 0;
    for (const auto& entity : m_entities)
        result += entity.fixed ? 0 : (entity.type == SketchEntityType::Line ? 4 : entity.type == SketchEntityType::Circle ? 3 : 6);
    for (const auto& constraint : m_constraints) {
        if (constraint.type == SketchConstraintType::Fixed) continue;
        if (constraint.type == SketchConstraintType::Coincident) result -= 2;
        else if (constraint.type == SketchConstraintType::Symmetric) {
            const auto* entity = findEntity(constraint.entityA);
            result -= entity && entity->type == SketchEntityType::Circle ? 3 : entity && entity->type == SketchEntityType::Arc ? 6 : 4;
        } else --result;
    }
    return result;
}

bool SketchModel::isClosedProfile(double tolerance) const {
    QVector<QPointF> endpoints;
    for (const auto& entity : m_entities) if (!entity.construction) {
        if (entity.type == SketchEntityType::Line) endpoints << entity.first << entity.second;
        else if (entity.type == SketchEntityType::Arc) endpoints << entity.first << entity.third;
    }
    if (endpoints.size() < 4 || endpoints.size() % 2 != 0) return false;
    for (int i = 0; i < endpoints.size(); ++i) {
        int matches = 0;
        for (int j = 0; j < endpoints.size(); ++j)
            if (i != j && std::hypot(endpoints[i].x() - endpoints[j].x(), endpoints[i].y() - endpoints[j].y()) <= tolerance) ++matches;
        if (matches != 1) return false;
    }
    return true;
}

const QVector<SketchEntity>& SketchModel::entities() const { return m_entities; }
const QVector<SketchConstraint>& SketchModel::constraints() const { return m_constraints; }
SketchEntity* SketchModel::findEntity(const QString& id) { for (auto& item : m_entities) if (item.id == id) return &item; return nullptr; }
const SketchEntity* SketchModel::findEntity(const QString& id) const { for (const auto& item : m_entities) if (item.id == id) return &item; return nullptr; }

QJsonObject SketchModel::toJson() const {
    QJsonArray entities;
    for (const auto& e : m_entities) entities.append(QJsonObject{{QStringLiteral("id"), e.id}, {QStringLiteral("type"), entityTypeName(e.type)},
        {QStringLiteral("x1"), e.first.x()}, {QStringLiteral("y1"), e.first.y()}, {QStringLiteral("x2"), e.second.x()},
        {QStringLiteral("y2"), e.second.y()}, {QStringLiteral("x3"), e.third.x()}, {QStringLiteral("y3"), e.third.y()},
        {QStringLiteral("radius"), e.radius}, {QStringLiteral("construction"), e.construction}});
    QJsonArray constraints;
    for (const auto& c : m_constraints) constraints.append(QJsonObject{{QStringLiteral("id"), c.id}, {QStringLiteral("type"), constraintTypeName(c.type)},
        {QStringLiteral("entityA"), c.entityA}, {QStringLiteral("entityB"), c.entityB}, {QStringLiteral("pointA"), c.pointA},
        {QStringLiteral("pointB"), c.pointB}, {QStringLiteral("value"), c.value}, {QStringLiteral("entityC"), c.entityC}});
    return {{QStringLiteral("entities"), entities}, {QStringLiteral("constraints"), constraints}};
}

SketchModel SketchModel::fromJson(const QJsonObject& object, QString* error) {
    SketchModel model;
    for (const auto& value : object.value(QStringLiteral("entities")).toArray()) {
        const auto item = value.toObject();
        const auto type = entityType(item.value(QStringLiteral("type")).toString());
        const QPointF first{item.value(QStringLiteral("x1")).toDouble(), item.value(QStringLiteral("y1")).toDouble()};
        const QPointF second{item.value(QStringLiteral("x2")).toDouble(), item.value(QStringLiteral("y2")).toDouble()};
        const QString id = type == SketchEntityType::Line ? model.addLine(first, second, item.value(QStringLiteral("construction")).toBool())
            : type == SketchEntityType::Arc ? model.addArc(first, second,
                {item.value(QStringLiteral("x3")).toDouble(), item.value(QStringLiteral("y3")).toDouble()}, item.value(QStringLiteral("construction")).toBool())
            : model.addCircle(first,
                item.value(QStringLiteral("radius")).toDouble(), item.value(QStringLiteral("construction")).toBool());
        model.m_entities.last().id = item.value(QStringLiteral("id")).toString(id);
    }
    for (const auto& value : object.value(QStringLiteral("constraints")).toArray()) {
        const auto item = value.toObject();
        const auto type = constraintType(item.value(QStringLiteral("type")).toString());
        const QString added = type == SketchConstraintType::Symmetric
            ? model.addSymmetryConstraint(item.value(QStringLiteral("entityA")).toString(), item.value(QStringLiteral("entityB")).toString(),
                item.value(QStringLiteral("entityC")).toString())
            : model.addConstraint(type, item.value(QStringLiteral("entityA")).toString(), item.value(QStringLiteral("entityB")).toString(),
                item.value(QStringLiteral("pointA")).toInt(), item.value(QStringLiteral("pointB")).toInt(), item.value(QStringLiteral("value")).toDouble());
        if (added.isEmpty()) {
            if (error) *error = QStringLiteral("Constraint references a missing entity.");
            return {};
        }
    }
    return model;
}

