# MergeQt Lifecycle Management

This file describes the lifecycle discipline for a QWidget frontend and a UI-agnostic lower layer.

## Ownership

- every object must have one clear owner
- valid owners: C++ static lifetime, `QObject` parent, or a widget object tree
- do not mix ownership models casually

## C++ Rules

- do not combine static singleton lifetime with a `QObject` parent
- `app`, `core`, and `platform` must not store direct pointers to `QWidget`, `QMainWindow`, or page instances
- do not let framework APIs delete global singleton services

## Frontend Rules

- UI state belongs in the chosen frontend adapter layer
- page-specific state should stay in QWidget compare pages or future per-page stores
- lower layers should notify the UI through signals and service APIs, not direct widget pointers

## QWidget Rules

- widgets own widget hierarchy state
- do not let application services cache concrete widget pointers
- connect widget refresh logic to service signals instead of pushing UI updates from `app`
- `MainWindow` should own only window-level state, not the internal state of every compare tab
- each compare tab should own its own compare session where practical
- text tabs and folder tabs should each keep their own service instance and teardown with the page

## Multi-Tab Rules

- each tab must be independently closable
- each tab must be able to keep its own compare inputs and results
- opening a new compare type should create a new page object, not mutate one shared page into another type
- main-window actions should query page capabilities instead of assuming every tab is a text compare page

## Async Rules

- never capture raw `QObject *` across async boundaries
- use `QPointer` or another liveness guard
- ignore stale async results with request ids or generation counters

## Pre-Change Checklist

Before changing lifecycle-sensitive code, check:

1. who owns this object
2. whether Qt may think it also owns this object
3. whether any async callback can outlive this object
4. whether a raw pointer crosses an async boundary
5. whether the state is global, page-local, frontend-specific, or core-domain state
