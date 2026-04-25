# MergeQt Architecture Overview

This document describes the current `MergeQt` layering and the requirement that the lower layers remain independent from concrete widget classes.

## Layers

Top to bottom:

1. `ui`
2. `app`
3. `core`
4. `platform`

The build graph now mirrors those boundaries with separate static libraries:

- `MergeQtUi`
- `MergeQtApp`
- `MergeQtCore`
- `MergeQtPlatform`

The `MergeQt` executable is intentionally thin and mainly owns the process entry point, app bundle metadata, translations, and final linkage.

## ui

Files under `src/ui/`.

Responsibilities:

- render the QWidget frontend workbench
- host tabs, windows, and page selection
- adapt app services into QWidget presentation
- own page-local interaction state inside compare pages
- avoid direct diff or filesystem logic

Do not:

- place comparison engine logic in QWidget classes
- let the frontend talk to `core` directly
- let QWidget-specific APIs leak into `app`

### UI Structure

The current UI is intentionally split into two layers:

- `UiFrontend`: frontend entry, window-host boundary, and top-level lifetime coordination
- `MainWindow`: workspace shell, tabs, menus, toolbars, window-level actions
- `CompareTabPage`: per-tab content interface

Current page implementations:

- `TextCompareTabPage`: real text compare workflow
- `FolderCompareTabPage`: real folder compare workflow
- `BinaryCompareTabPage`: binary metadata and byte-identity compare shell
- `ImageCompareTabPage`: image metadata, preview, and pixel-identity compare shell
- `CompareInputHeader`: shared compare-page location/path header for left and right inputs
- `TextDiffOverviewBar`: shared text-diff overview strip for block-level navigation

This keeps the main window stable while new compare types are added one page at a time.
`MainWindow` now routes descriptor-driven page creation through a dedicated page factory so the workspace shell does not keep accumulating per-type construction logic.
The application entry point now also resolves the concrete frontend through a small frontend factory, so `main.cpp` no longer depends directly on the QWidget implementation type.
Initial launch intent is now parsed in the app layer and then handed to the frontend as an optional descriptor-driven open request, which keeps command-line startup logic out of `MainWindow`.

The workspace shell now also exposes a unified `Open Compare` dialog so new compare tabs can be created from a single entry point instead of relying only on empty per-type tabs.
It now starts on a dedicated workspace start page instead of auto-opening an empty text compare tab.
That start page is a persistent workspace entry surface rather than a disposable splash screen:

- quick actions for text, folder, binary, and image compare tabs
- application-level entry points such as settings and about
- explicit shell navigation between the start page and the currently open tab workspace

### Compare Page Contract

Every compare tab page should follow the same contract:

- page-local UI state stays inside the page widget
- page-local compare orchestration stays inside a page-owned app service
- window creation and multi-window hosting stay in the frontend layer rather than inside compare pages
- `MainWindow` only dispatches generic actions such as open, compare, reload, swap, and navigate
- descriptor-driven tab creation may carry full inputs or partial inputs, such as file-vs-empty compare requests
- if one compare page wants to open another compare type, it should emit a descriptor request instead of instantiating tabs directly
- result-item actions such as copy-path and reveal-in-file-manager should stay page-local unless they become shared app commands
- adding a new compare type should require a new page implementation, not type checks scattered across the workspace shell
- page-level location bars, path headers, and overview strips should be implemented as reusable UI components instead of duplicated layout code inside every compare page

## app

Files under `src/app/`.

Responsibilities:

- application lifecycle
- settings persistence
- command/session orchestration
- logging
- expose UI-agnostic services and state

Do not:

- hide platform-specific behavior all over the codebase
- let app services become concrete widget presenters
- depend on `QWidget`, `QMainWindow`, or other concrete UI classes

### Session Rule

Compare services should be owned by the compare page that uses them, not by the main window as global mutable state.

Current direction:

- one text compare tab owns one `CompareSessionService`
- one folder compare tab owns one `FolderCompareSessionService`
- one binary compare tab owns one `BinaryCompareSessionService`
- one image compare tab owns one `ImageCompareSessionService`
- folder compare report formatting now lives in app-level formatter code so export logic does not depend on widget classes
- shared workspace-level state should stay in app services and settings, not in page widgets

## core

Files under `src/core/`.

Responsibilities:

- comparison domain models
- diff engine interfaces
- compare-type-specific result models
- filtering rules
- encoding and content abstractions

Do not:

- depend on Qt Widgets
- depend on OS-specific APIs
- couple core logic to widget object lifetime

### Current Compare Pipelines

Text compare is already split into:

1. document loading
2. text normalization
3. line diff
4. diff block aggregation

The current QWidget presentation now adds a lightweight overview layer on top of the core result model:

- the text page renders a compact difference overview bar from `TextCompareResult.blocks`
- overview navigation still maps back to the same block/line data instead of inventing a second UI-only diff model

Future compare types should mirror that idea:

- folder compare: scan -> classify -> aggregate
- binary compare: load -> compare -> summarize
- image compare: decode -> compare -> visualize

### Current Binary And Image Compare Skeletons

Binary compare currently provides:

- path-level input selection
- readable file validation
- size comparison
- full-file byte identity check

Image compare currently provides:

- path-level input selection
- image decode and format metadata
- dimension comparison
- exact pixel equality counting
- side-by-side preview rendering in the UI layer

### Current Folder Compare Pipeline

Folder compare is now split into:

1. recursive folder scan
2. relative-path union
3. type/content classification
4. result aggregation for the page table

Folder merge/sync preparation is now split into:

1. row selection in the folder result table
2. action-specific operation planning in `core`
3. UI preview rendering from the plan model
4. guarded execution through an app service
5. batch-ready execution reports that can later feed history, undo hooks, or queue UIs

Current execution scope is intentionally narrow:

- safe single-file one-way copy is enabled
- safe missing-directory creation is enabled
- delete-related actions remain preview-only until command policy and safety rules are formalized
- the app layer now also keeps a last batch report so the current single-item UI can evolve toward multi-item commands without rewriting the core contract
- the folder compare page can now select multiple rows and route safe actions through the batch planning/execution path
- folder compare also supports page-local filtering and app-backed report export without moving folder comparison rules into the UI layer

## platform

Files under `src/platform/`.

Responsibilities:

- filesystem paths
- platform conventions
- OS integration points gathered behind narrow interfaces

Do not:

- leak scattered `#ifdef` usage into core or UI
- make platform services own business rules

## Dependency Direction

Allowed direction:

- `ui -> app -> core`
- `app -> platform`
- `app -> core`

Avoid:

- `core -> ui`
- `core -> platform` unless the dependency is abstracted
- `ui -> platform` for anything beyond trivial display needs

## QWidget Rule

The current application frontend is fully QWidget-based.

- `src/ui/widgets/`: widget windows, compare tabs, and UI binding code
- `app`, `core`, and `platform` must not include concrete widget headers unless they are inside `src/ui`
- localization refresh, settings, and session state should flow through signals and services, not direct widget pointers

## Multi-Document Rule

`MergeQt` now follows a Qt-style multi-document workbench:

- multiple windows are allowed
- each window can host multiple compare tabs
- each tab owns its own compare session and page state
- adding a new compare type should mean adding a new `CompareTabPage` implementation, not rewriting `MainWindow`
- text and folder compare are the first concrete implementations of this rule
- folder compare can now open a selected file pair into a new text compare tab through the shared descriptor flow
- folder compare can also derive one-sided text compare tabs for left-only and right-only files
- folder compare now also owns row-level utility actions such as copying paths and opening selected paths in the file manager
- folder compare now includes operation-plan previews backed by core/app models before any real merge command is executed
- folder compare can now execute a limited safe subset of planned operations after explicit confirmation
- folder compare now supports multi-row selection for safe batch copy/create-directory actions while keeping delete actions in preview-only mode
- compare pages now share a common path/location header with compare, reload, and swap entry points, which makes future compare types cheaper to add
- text compare now has a WinMerge-style compact difference overview strip for faster navigation across large documents
- folder compare now includes a filter bar and report export entry so the page can scale beyond small result sets
- the workspace shell can switch between a home/start surface and the open-tab surface without collapsing the tab/session model

## Design Ideas

The framework currently leans on these general design ideas:

- settings and path services should be reusable runtime infrastructure
- lifecycle rules should keep global services separate from widget instances
- UI shells should stay replaceable without dragging widget details into lower layers
