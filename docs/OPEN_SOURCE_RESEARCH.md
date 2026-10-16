# Open-source research and recommendation

Research date: 15 October 2026.

## Clear recommendation

Build an independent C++20/Qt desktop host and integrate focused open-source libraries behind adapters. Do not fork FreeCAD or SALOME for the product shell. C++ is the shortest path because every recommended geometry, meshing, dynamics, and visualization component has a native C/C++ API; Rust would add wrappers at almost every core boundary.

| Area | Recommendation | License | Milestone |
|---|---|---|---|
| Exact CAD/B-Rep and STEP | [Open CASCADE Technology](https://dev.opencascade.org/doc/overview/html/) | LGPL-2.1 with OCCT exception | UI shell, now |
| Desktop UI | [Qt 6 Widgets](https://doc.qt.io/qt-6/qtwidgets-index.html) | LGPL-3.0/GPL/commercial | UI shell, now |
| Tetrahedral meshing | [Netgen](https://github.com/NGSolve/netgen) | LGPL-2.1 | FEM milestone |
| FEM framework | [MFEM](https://mfem.org/features/) | BSD-3-Clause | FEM milestone |
| Multibody dynamics | [Project Chrono](https://www.projectchrono.org/about/) | BSD-3-Clause | dynamics milestone |
| ODE/DAE integration | [SUNDIALS](https://github.com/LLNL/sundials) | BSD-3-Clause | system milestone |
| Model exchange/co-simulation | [FMI 3](https://github.com/modelica/fmi-standard) | BSD-2-Clause specification code | system milestone |
| Scientific results | [VTK](https://docs.vtk.org/en/latest/about.html) | BSD-3-Clause | FEM/results milestone |
| Multiphysics coupling | [preCICE](https://precice.org/) | LGPL-3.0 | later |

## Alternatives

- [FreeCAD](https://github.com/FreeCAD/FreeCAD) is valuable as a behavioral reference and interoperability oracle, but its large existing architecture would make an independent, Inventor-familiar shell harder to control.
- [SALOME](https://www.salome-platform.org/) demonstrates mature CAD/mesh/solver workflows but is too broad and distribution-heavy for this Windows-native shell.
- Gmsh is technically excellent, but GPL embedding constraints make Netgen the cleaner default for this GPL application’s modular roadmap; Gmsh can remain an optional external adapter.
- CalculiX, Code_Aster, Elmer, OpenFOAM, and MBDyn are useful optional process backends. Their individual GPL/license mixes and platform workflows should be audited per adapter.
- OpenModelica is best treated initially as an external compiler that exports FMUs. The main process should consume FMI models rather than embed a complete Modelica compiler.

## Integration order

1. Qt + OCCT UI shell and versioned document.
2. Parametric sketch and B-Rep feature history.
3. Netgen + MFEM + VTK linear statics.
4. Project Chrono joints, drives, contacts, and histories.
5. FMI + SUNDIALS block/system simulation.
6. Optional external solvers and preCICE coupling after benchmark and license audits.

This is a technical recommendation, not legal advice. Every shipped dependency and transitive runtime must be audited at packaging time.
