# MergeQt

MergeQt is a cross-platform desktop diff and merge workbench built with Qt Widgets.

It targets Windows, macOS, and Linux with the same QWidget-based architecture, and is currently focused on a clean layered design, a multi-tab/multi-window workspace, and an extensible compare framework that can grow from text comparison into folder, binary, and image workflows.

## Features

- Qt Widgets desktop UI with clear separation between `ui`, `app`, `core`, and `platform`
- cross-platform desktop direction for Windows, macOS, and Linux
- text compare with aligned rows, original line-number rendering, and synchronized vertical scrolling
- workspace shell that supports multiple tabs and multiple windows
- compare-type expansion path for folder, binary, and image comparison
- CMake-based build with test targets

## Requirements and dependencies

- [Qt 6.8+](https://www.qt.io/download-qt-installer)
- CMake 3.24+
- A C++20 compiler supported by your Qt toolchain

## Build

Replace `"/path/to/install"` with the location where you want to install the application.

```bash
git clone https://github.com/s12mmm3/MergeQt.git
cd MergeQt
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build
```

## Translation workflow

When UI text changes, update Qt translation files with:

```bash
python3 scripts/translate_ts.py
```

You can also target specific files:

```bash
python3 scripts/translate_ts.py --files translations/MergeQt_ja_JP.ts
```

## Status

This project is under active development. The current implementation already includes the foundational architecture and an initial QWidget-based compare experience, with more compare and merge capabilities planned.
