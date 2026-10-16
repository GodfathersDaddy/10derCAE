# 10derCAE

10derCAE is a native Windows CAD/CAE shell built with C++20, Qt 6, and Open CASCADE. Milestone 1 provides an Inventor-familiar light UI without Autodesk assets: eight workspaces, ribbon, model/simulation browser, properties, status area, DE/EN switching, persisted layouts, a real selectable OCCT demo body, camera navigation, and versioned `.10cae` documents.

The current CAD prototype additionally provides an interactive 2D sketch canvas (lines, rectangles, circles, three-point arcs, snapping and geometric/dimensional constraints), sketch extrusion, and initial fillet/chamfer features. Domain commands that are not implemented remain clickable and report their planned status. Numerical solvers remain behind documented adapter boundaries.

Development status and the next steps are maintained in [the roadmap](docs/IMPLEMENTATION_ROADMAP.md). Technical findings are recorded in [the dissertation journal](docs/DISSERTATION_LOG.md). This is a prototype; full Inventor feature parity is not yet achieved.

Internal tangency applies to the supporting circles of circles/arcs. It preserves radii and arc sweeps while moving an unfixed curve. It does not yet ensure that the contact point lies inside both finite arc spans. Equal-radius internal tangency is rejected because coincident circles do not have a unique contact point.

## Build on Windows 11 x64

Prerequisites are Visual Studio 2022 Build Tools with “Desktop development with C++”, CMake, Git, and PowerShell 7 or Windows PowerShell 5.1. Qt and OCCT do not need to be installed manually.

```powershell
cd 10derCAE
.\scripts\build.ps1
```

The bootstrap script uses an existing `VCPKG_ROOT` or installs the pinned vcpkg release `2026.07.29` below `.deps`. Create a tested portable ZIP with:

```powershell
.\scripts\package.ps1
```

Capture all workspace reference images with `.\scripts\capture.ps1 -Language de`.

## Interaction

- Middle drag: orbit
- Shift + middle drag: pan
- Mouse wheel: zoom
- Hover and left click: preselection and selection
- View cube buttons: six standard directions and isometric home

## Licensing

Original 10derCAE code: GPL-3.0-or-later. The v2 icon set is separate CC BY-NC-SA 4.0 content, so the bundled distribution is noncommercial. Its complete attribution, license, notice, and provenance are shipped under `resources/icons/v2`. Qt and OCCT retain their upstream licenses. No Autodesk artwork, product icons, logos, or fonts are included.
