#include "WorkspaceCatalog.h"

#include <stdexcept>

namespace {
CommandDefinition cmd(CommandId id, const char* stable, const char* panel, const char* label,
                      const char* description, const char* icon, const char* shortcut = "",
                      bool implemented = false) {
    return {id, QString::fromLatin1(stable), QString::fromLatin1(panel),
            QString::fromLatin1(label), QString::fromLatin1(description),
            QString::fromLatin1(icon), QString::fromLatin1(shortcut), implemented};
}

QVector<WorkspaceDefinition> createCatalog() {
    using C = CommandId;
    return {
        {WorkspaceId::Start, "start", "workspace.start", false,
         {cmd(C::NewPart, "new-part", "panel.create", "command.newPart", "tip.newPart", "sketch-create.svg", "Ctrl+N", true),
          cmd(C::NewAssembly, "new-assembly", "panel.create", "command.newAssembly", "tip.newAssembly", "assembly.svg"),
          cmd(C::NewDrawing, "new-drawing", "panel.create", "command.newDrawing", "tip.newDrawing", "sketch-create.svg"),
          cmd(C::OpenDocument, "open", "panel.file", "command.open", "tip.open", "file-open.svg", "Ctrl+O", true)},
         {"browser.recent", "browser.templates", "browser.learn"},
         {"property.application", "property.version", "property.license"}},
        {WorkspaceId::Part, "part", "workspace.part", false,
         {cmd(C::CreateSketch, "create-sketch", "panel.sketch", "command.createSketch", "tip.createSketch", "sketch-create.svg", "S"),
          cmd(C::Extrude, "extrude", "panel.create", "command.extrude", "tip.extrude", "extrude.svg", "E"),
          cmd(C::Revolve, "revolve", "panel.create", "command.revolve", "tip.revolve", "revolve.svg"),
          cmd(C::Sweep, "sweep", "panel.create", "command.sweep", "tip.sweep", "sweep.svg"),
          cmd(C::Loft, "loft", "panel.create", "command.loft", "tip.loft", "loft.svg"),
          cmd(C::Hole, "hole", "panel.modify", "command.hole", "tip.hole", "hole.svg"),
          cmd(C::Fillet, "fillet", "panel.modify", "command.fillet", "tip.fillet", "fillet.svg"),
          cmd(C::Chamfer, "chamfer", "panel.modify", "command.chamfer", "tip.chamfer", "chamfer.svg"),
          cmd(C::Shell, "shell", "panel.modify", "command.shell", "tip.shell", "surface-boundary.svg"),
          cmd(C::Mirror, "mirror", "panel.pattern", "command.mirror", "tip.mirror", "mirror.svg"),
          cmd(C::RectangularPattern, "rect-pattern", "panel.pattern", "command.rectPattern", "tip.rectPattern", "pattern-rectangular.svg"),
          cmd(C::CircularPattern, "circular-pattern", "panel.pattern", "command.circularPattern", "tip.circularPattern", "pattern-circular.svg"),
          cmd(C::WorkPlane, "work-plane", "panel.work", "command.workPlane", "tip.workPlane", "plane.svg"),
          cmd(C::WorkAxis, "work-axis", "panel.work", "command.workAxis", "tip.workAxis", "axis.svg"),
          cmd(C::ViewHome, "view-home", "panel.view", "command.home", "tip.home", "view-home.svg", "F6", true),
          cmd(C::ViewFit, "view-fit", "panel.view", "command.fit", "tip.fit", "view-home.svg", "F", true),
          cmd(C::ToggleProjection, "projection", "panel.view", "command.projection", "tip.projection", "view-home.svg", "", true)},
         {"browser.origin", "browser.solids", "browser.sketches", "browser.features", "browser.endOfPart"},
         {"property.document", "property.material", "property.mass", "property.units"}},
        {WorkspaceId::Sketch, "sketch", "workspace.sketch", true,
         {cmd(C::Line, "line", "panel.draw", "command.line", "tip.line", "line.svg", "L"),
          cmd(C::Rectangle, "rectangle", "panel.draw", "command.rectangle", "tip.rectangle", "rectangle.svg", "R"),
          cmd(C::Circle, "circle", "panel.draw", "command.circle", "tip.circle", "circle.svg", "C"),
          cmd(C::Arc, "arc", "panel.draw", "command.arc", "tip.arc", "arc.svg"),
          cmd(C::Spline, "spline", "panel.draw", "command.spline", "tip.spline", "spline.svg"),
          cmd(C::Dimension, "dimension", "panel.constrain", "command.dimension", "tip.dimension", "dimension.svg", "D"),
          cmd(C::Constraints, "constraints", "panel.constrain", "command.constraints", "tip.constraints", "mate.svg"),
          cmd(C::Trim, "trim", "panel.modify", "command.trim", "tip.trim", "trim.svg", "T"),
          cmd(C::Mirror, "sketch-mirror", "panel.pattern", "command.mirror", "tip.mirror", "mirror.svg"),
          cmd(C::FinishSketch, "finish-sketch", "panel.finish", "command.finishSketch", "tip.finishSketch", "status-success.svg")},
         {"browser.origin", "browser.sketchGeometry", "browser.constraints", "browser.dimensions"},
         {"property.sketch", "property.plane", "property.constraintState", "property.units"}},
        {WorkspaceId::Assembly, "assembly", "workspace.assembly", false,
         {cmd(C::InsertComponent, "insert-component", "panel.component", "command.insertComponent", "tip.insertComponent", "component-insert.svg"),
          cmd(C::Joint, "joint", "panel.relationship", "command.joint", "tip.joint", "mate.svg"),
          cmd(C::Mate, "mate", "panel.relationship", "command.mate", "tip.mate", "mate.svg"),
          cmd(C::Ground, "ground", "panel.position", "command.ground", "tip.ground", "component-ground.svg"),
          cmd(C::RectangularPattern, "component-pattern", "panel.pattern", "command.rectPattern", "tip.rectPattern", "pattern-rectangular.svg"),
          cmd(C::BillOfMaterials, "bom", "panel.manage", "command.bom", "tip.bom", "assembly.svg"),
          cmd(C::ViewHome, "assembly-home", "panel.view", "command.home", "tip.home", "view-home.svg", "F6", true)},
         {"browser.representations", "browser.origin", "browser.components", "browser.relationships"},
         {"property.assembly", "property.occurrences", "property.dof", "property.units"}},
        {WorkspaceId::Drawing, "drawing", "workspace.drawing", false,
         {cmd(C::BaseView, "base-view", "panel.views", "command.baseView", "tip.baseView", "view-home.svg"),
          cmd(C::ProjectedView, "projected-view", "panel.views", "command.projectedView", "tip.projectedView", "view-home.svg"),
          cmd(C::SectionView, "section-view", "panel.views", "command.sectionView", "tip.sectionView", "surface-boundary.svg"),
          cmd(C::DetailView, "detail-view", "panel.views", "command.detailView", "tip.detailView", "circle.svg"),
          cmd(C::DrawingDimension, "drawing-dimension", "panel.annotate", "command.dimension", "tip.dimension", "dimension.svg"),
          cmd(C::Annotation, "annotation", "panel.annotate", "command.annotation", "tip.annotation", "text.svg")},
         {"browser.sheet", "browser.views", "browser.annotations", "browser.resources"},
         {"property.sheet", "property.standard", "property.scale", "property.units"}},
        {WorkspaceId::Fem, "fem", "workspace.fem", true,
         {cmd(C::NewSimulation, "new-simulation", "panel.manage", "command.newSimulation", "tip.newSimulation", "sketch-create.svg"),
          cmd(C::Material, "material", "panel.material", "command.material", "tip.material", "component-insert.svg"),
          cmd(C::FixedConstraint, "fixed", "panel.constraints", "command.fixed", "tip.fixed", "component-ground.svg"),
          cmd(C::Force, "force", "panel.loads", "command.force", "tip.force", "move.svg"),
          cmd(C::Pressure, "pressure", "panel.loads", "command.pressure", "tip.pressure", "surface-boundary.svg"),
          cmd(C::Gravity, "gravity", "panel.loads", "command.gravity", "tip.gravity", "move.svg"),
          cmd(C::Contact, "contact", "panel.contacts", "command.contact", "tip.contact", "mate.svg"),
          cmd(C::Mesh, "mesh", "panel.mesh", "command.mesh", "tip.mesh", "display-grid.svg"),
          cmd(C::Solve, "solve", "panel.solve", "command.solve", "tip.solve", "status-success.svg"),
          cmd(C::StressResult, "stress", "panel.results", "command.stress", "tip.stress", "surface-boundary.svg"),
          cmd(C::DisplacementResult, "displacement", "panel.results", "command.displacement", "tip.displacement", "move.svg")},
         {"browser.simulations", "browser.materials", "browser.constraints", "browser.loads", "browser.contacts", "browser.mesh", "browser.results"},
         {"property.analysisType", "property.solver", "property.meshState", "property.resultState"}},
        {WorkspaceId::Dynamics, "dynamics", "workspace.dynamics", true,
         {cmd(C::NewMechanism, "new-mechanism", "panel.manage", "command.newMechanism", "tip.newMechanism", "assembly.svg"),
          cmd(C::DynamicJoint, "dynamic-joint", "panel.joints", "command.joint", "tip.joint", "mate.svg"),
          cmd(C::Motor, "motor", "panel.forces", "command.motor", "tip.motor", "rotate.svg"),
          cmd(C::Spring, "spring", "panel.forces", "command.spring", "tip.spring", "spline.svg"),
          cmd(C::DynamicForce, "dynamic-force", "panel.forces", "command.force", "tip.force", "move.svg"),
          cmd(C::Trace, "trace", "panel.results", "command.trace", "tip.trace", "spline.svg"),
          cmd(C::RunDynamics, "run-dynamics", "panel.simulate", "command.run", "tip.run", "status-success.svg")},
         {"browser.mechanism", "browser.bodies", "browser.joints", "browser.forces", "browser.traces"},
         {"property.time", "property.step", "property.gravity", "property.engine"}},
        {WorkspaceId::SystemModel, "system", "workspace.system", false,
         {cmd(C::BlockLibrary, "block-library", "panel.blocks", "command.blockLibrary", "tip.blockLibrary", "component-insert.svg"),
          cmd(C::AddPort, "add-port", "panel.connect", "command.addPort", "tip.addPort", "point.svg"),
          cmd(C::ConnectPorts, "connect-ports", "panel.connect", "command.connect", "tip.connect", "mate.svg"),
          cmd(C::Parameters, "parameters", "panel.model", "command.parameters", "tip.parameters", "dimension.svg"),
          cmd(C::RunSystem, "run-system", "panel.simulate", "command.run", "tip.run", "status-success.svg"),
          cmd(C::Plot, "plot", "panel.results", "command.plot", "tip.plot", "spline.svg")},
         {"browser.system", "browser.blocks", "browser.connections", "browser.experiments", "browser.results"},
         {"property.stopTime", "property.tolerance", "property.integrator", "property.fmi"}},
    };
}
}

QString workspaceStableId(WorkspaceId id) { return WorkspaceCatalog::get(id).stableId; }

WorkspaceId workspaceFromStableId(const QString& value) {
    for (const auto& workspace : WorkspaceCatalog::all()) if (workspace.stableId == value) return workspace.id;
    return WorkspaceId::Part;
}

const QVector<WorkspaceDefinition>& WorkspaceCatalog::all() {
    static const QVector<WorkspaceDefinition> catalog = createCatalog();
    return catalog;
}

const WorkspaceDefinition& WorkspaceCatalog::get(WorkspaceId id) {
    for (const auto& workspace : all()) if (workspace.id == id) return workspace;
    throw std::logic_error("Unknown workspace");
}

