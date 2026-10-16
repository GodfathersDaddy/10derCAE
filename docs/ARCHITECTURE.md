# Architecture

## Current milestone

`MainWindow` only composes the shell. `RibbonWidget` renders the command catalog, `WorkspaceCatalog` is the stable workspace/command API, `Document` owns serializable state, `GeometryKernel` owns B-Rep creation and exchange, and `OcctViewport` owns all OCCT presentation and input handling. Visible labels come from the JSON DE/EN catalogs.

The `.10cae` file is JSON schema version 2 with application marker, name, document type, active workspace, units, and a provisional feature list. `QSaveFile` provides atomic replacement. A reader rejects unknown versions without modifying the current document.

## Stable boundaries

- `WorkspaceId`: Start, Part, Sketch, Assembly, Drawing, FEM, Dynamics, System Model.
- `CommandDefinition`: stable ID, panel, translation keys, v2 icon, shortcut, implementation state.
- `OcctViewport`: shape display, selection/highlight, home/fit, standard views, projection and mouse navigation.
- Future solver adapters: Netgen/MFEM, Project Chrono, FMI/SUNDIALS, and VTK. None is linked into the UI-shell milestone.

## Next implementation sequence

1. Constrained sketch model and real extrusion/revolution history commands.
2. Robust topological naming, fillet/chamfer/hole, STEP import, and command transactions.
3. Assembly occurrences and constraints; derived drawing views.
4. Netgen/MFEM/VTK linear-static adapter with process isolation and reference benchmarks.
5. Chrono dynamics and FMI/SUNDIALS system simulation adapters.

The UI follows familiar engineering-software information architecture, but branding, source, text, icon assets, styling, and layout implementation are original.
