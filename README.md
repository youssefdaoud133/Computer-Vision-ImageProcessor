# Image Processor Application

## Overview

This is a C++ application built primarily with the `wxWidgets` library. It complies with the Best Object-Oriented Programming (OOP) design patterns and focuses on multi-window application management, filtering functionalities, and dynamic UI updates across discrete components.

## Requirements Specification

1. **Introductory Page:** A main window acts as an introductory frame. It contains instructions and a prominent button to upload multiple images.
2. **Multiple Uploads:** Clicking the "Upload Images" button prompts a multi-selection file dialog allowing users to pick several images at once.
3. **Dedicated Image Windows:** Every selected image natively spawns its own respective, standalone window (`ImageFrame`).
4. **Top Bar with Filters:** Each instantiated `ImageFrame` encapsulates a toolbar comprising tools to process the selected image natively:
   - **Reset**: Rapidly reverts all applied modifications and restores the original image.
   - **Grayscale**: Employs a filter algorithm reducing the color space to purely grey tints constraint.
   - **Blur**: Performs an area-blur transformation directly blurring intricate artifacts.
   - **Invert Colors**: Triggers an inversion matrix that strictly inverts standard RGB parameters linearly.
5. **Architectural Code Base (OOP):** Strictly segmented into header (`include`) and internal (`src`) definition architectures. Avoiding monolithic anti-patterns cleanly.
6. **Robust Display**: Ensures dynamic window resizing while gracefully mitigating scaling aberrations (maintaining aspect ratio).

## Project Structure

- `CMakeLists.txt` - Build automation recipe explicitly binding wxWidgets components appropriately.
- `include/App.h` & `src/App.cpp` - Entry point bootstrapping wxWidgets GUI logic iteratively.
- `include/MainFrame.h` & `src/MainFrame.cpp` - UI representation for the application's introspective launch point and file dispatcher.
- `include/ImageFrame.h` & `src/ImageFrame.cpp` - MDI architecture mapping window-to-image correlation directly mapping toolbar actions to the underlying panel.
- `include/ImagePanel.h` & `src/ImagePanel.cpp` - Dedicated UI Canvas that explicitly handles raw byte-data manipulation implicitly mapping `wxImage` and `wxBitmap` data into the GUI pipeline.

## Build and Run Instructions

### Prerequisites

- Compiler supporting **C++17** or higher.
- **CMake** (version 3.10+).
- **wxWidgets** (version 3.0+ is thoroughly recommended) installed optimally with configurations mapped to system paths.

_Ubuntu/Debian installation command:_

```bash
sudo apt-get update
sudo apt-get install libwxgtk3.0-gtk3-dev
```

### Build Steps

You can utilize standard CMake routines. With the terminal operating in the project's root folder:

```bash
# Optional: Create a separate directory (e.g., `mkdir build && cd build`)
cmake .
cmake --build .
```

### Execution

Run the compiled target executable natively in your CLI:

```bash
./ImageProcessor
```

## How It Works Under The Hood

The core paradigm follows Model-View-Controller conceptually mapped through UI elements.

1. `wxIMPLEMENT_APP` instantiates `App` which constructs `MainFrame`.
2. `MainFrame::OnUploadClicked` triggers `wxFileDialog(..., wxFD_MULTIPLE)`.
3. Paths retrieved populate independent iterations of `ImageFrame`.
4. `ImageFrame` maps its display canvas to `ImagePanel`.
5. Filters executed within `ImageFrame` explicitly call `ImagePanel::SetImage` invoking an automated asynchronous `Refresh()`, generating discrete `wxPaintEvent` sequences updating UI matrices.
