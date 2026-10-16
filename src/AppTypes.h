#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

enum class WorkspaceId { Start, Part, Sketch, Assembly, Drawing, Fem, Dynamics, SystemModel };

enum class CommandId {
    NewDocument, OpenDocument, SaveDocument, Undo, Redo, NewPart, NewAssembly, NewDrawing,
    CreateSketch, Line, Rectangle, Circle, Arc, Spline, Dimension, Constraints, Trim, FinishSketch,
    Extrude, Revolve, Sweep, Loft, Hole, Fillet, Chamfer, Shell, Mirror, RectangularPattern,
    CircularPattern, WorkPlane, WorkAxis, InsertComponent, Joint, Mate, Ground, BillOfMaterials,
    BaseView, ProjectedView, SectionView, DetailView, DrawingDimension, Annotation, NewSimulation,
    Material, FixedConstraint, Force, Pressure, Gravity, Contact, Mesh, Solve, StressResult,
    DisplacementResult, NewMechanism, DynamicJoint, Motor, Spring, DynamicForce, Trace, RunDynamics,
    BlockLibrary, AddPort, ConnectPorts, Parameters, RunSystem, Plot, ViewHome, ViewFit, ToggleProjection
};

struct CommandDefinition {
    CommandId id;
    QString stableId;
    QString panelKey;
    QString labelKey;
    QString descriptionKey;
    QString iconName;
    QString shortcut;
    bool implemented = false;
};

struct WorkspaceDefinition {
    WorkspaceId id;
    QString stableId;
    QString labelKey;
    bool contextual = false;
    QVector<CommandDefinition> commands;
    QStringList browserKeys;
    QStringList propertyKeys;
};

QString workspaceStableId(WorkspaceId id);
WorkspaceId workspaceFromStableId(const QString& value);

