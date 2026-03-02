# ImageProcessor — Developer Deep-Dive Guide

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Technology Stack](#2-technology-stack)
3. [Architecture & Class Hierarchy](#3-architecture--class-hierarchy)
4. [Directory Structure](#4-directory-structure)
5. [Build System](#5-build-system)
6. [Application Lifecycle](#6-application-lifecycle)
7. [Class-by-Class Breakdown](#7-class-by-class-breakdown)
8. [How the History System Works](#8-how-the-history-system-works)
9. [How the Live Histogram Works](#9-how-the-live-histogram-works)
10. [How Image Filters Work](#10-how-image-filters-work)
11. [Adding a New Feature (Step-by-Step)](#11-adding-a-new-feature-step-by-step)
12. [Common Pitfalls & Gotchas](#12-common-pitfalls--gotchas)
13. [Coding Conventions](#13-coding-conventions)

---

## 1. Project Overview

ImageProcessor is a desktop image-editing application built for a Computer
Vision course. It lets users load one or more images, apply a variety of
filters (edge detection, histogram enhancement, noise injection, low-pass
smoothing), and inspect per-channel RGB histograms with CDF overlays in real
time. Every filter operation is recorded in a version-history sidebar so the
user can navigate back to any previous state.

---

## 2. Technology Stack

| Component       | Version / Details                               |
| --------------- | ----------------------------------------------- |
| Language        | C++17                                           |
| GUI Framework   | wxWidgets 3.0.5 (GTK3 build on Linux)           |
| CV Library      | OpenCV 4.5.4 (used only for Canny detection)    |
| Build System    | CMake 3.10+                                     |
| Compiler        | GCC 11.4.0 (Ubuntu 22.04 Jammy)                 |
| Target Platform | Linux x86_64 (easily portable to macOS/Windows) |

> **Important**: Most image filters (Sobel, Roberts, Prewitt, histogram
> equalization, noise, median/average/Gaussian smoothing) are implemented
> **entirely from scratch** without any OpenCV or wxWidgets filter helpers.
> OpenCV is used only for Canny edge detection and for the `wxImage <-> cv::Mat`
> conversion helpers that Canny requires.

---

## 3. Architecture & Class Hierarchy

```
wxApp
 └── App                          (entry point, initialises image handlers)
       └── creates MainFrame

wxFrame
 ├── MainFrame                    (intro window with "Upload" button)
 │     └── creates ImageFrame(s) via file dialog
 │
 ├── ImageFrame                   (main editing window, one per image)
 │     ├── owns ImagePanel        (displays the image, handles scaling)
 │     ├── owns HistogramPanel    (live RGB histogram in sidebar)
 │     ├── owns wxListBox         (version-history log in sidebar)
 │     └── owns wxToolBar         (all filter/noise/lowpass buttons)
 │
 └── HistogramFrame               (popup histogram window, on demand)
       └── owns HistogramPanel

wxPanel
 ├── ImagePanel                   (image display + aspect-ratio scaling)
 └── HistogramPanel               (RGB histogram + CDF drawing)

Filtering                         (static utility class, all algorithms)
```

**Data flow**: User clicks a toolbar button in `ImageFrame` → event handler
calls a `Filtering::*` static method with the current `wxImage` → result is
passed to `ImagePanel::SetImage()` → sidebar histogram auto-updates →
`PushHistory()` records a snapshot.

---

## 4. Directory Structure

```
Task1/
├── CMakeLists.txt          # Build configuration
├── README.md               # Project readme
├── docs/
│   └── DEVELOPER_GUIDE.md  # This file
├── include/                # All header files
│   ├── App.h
│   ├── MainFrame.h
│   ├── ImageFrame.h        # Main editing window (toolbar + sidebar)
│   ├── ImagePanel.h        # Image display panel
│   ├── HistogramPanel.h    # RGB histogram drawing panel
│   ├── HistogramFrame.h    # Popup histogram window
│   └── Filtering.h         # All filter algorithm declarations
├── src/                    # All implementation files
│   ├── App.cpp
│   ├── MainFrame.cpp
│   ├── ImageFrame.cpp      # Event handlers, toolbar, sidebar, dialogs
│   ├── ImagePanel.cpp      # Image display + scaling
│   ├── HistogramPanel.cpp  # Histogram computation + rendering
│   ├── HistogramFrame.cpp  # Thin wrapper around HistogramPanel
│   └── Filtering.cpp       # All filter/noise/edge algorithms
└── build/                  # Out-of-source build directory (git-ignored)
```

---

## 5. Build System

### Prerequisites (Ubuntu/Debian)

```bash
sudo apt install build-essential cmake \
    libwxgtk3.0-gtk3-dev \
    libopencv-dev
```

### Build Commands

```bash
cd Task1
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Run

```bash
./ImageProcessor
```

### CMakeLists.txt Explained

```cmake
cmake_minimum_required(VERSION 3.10)
project(ImageProcessor VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)              # Required for std::clamp, etc.
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(wxWidgets COMPONENTS core base adv REQUIRED)
include(${wxWidgets_USE_FILE})

find_package(OpenCV REQUIRED)

include_directories(include ${wxWidgets_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp")           # Auto-discovers all .cpp files
add_executable(ImageProcessor ${SOURCES})
target_link_libraries(ImageProcessor ${wxWidgets_LIBRARIES} ${OpenCV_LIBS})
```

> When you add a new `.cpp` file under `src/`, the `file(GLOB)` directive picks
> it up automatically on the next `cmake ..` invocation. You do **not** need to
> edit CMakeLists.txt.

---

## 6. Application Lifecycle

1. `wxIMPLEMENT_APP(App)` defines the platform-specific `main()`.
2. `App::OnInit()` calls `wxInitAllImageHandlers()` (so wxImage can load
   PNG/JPEG/BMP) and creates a `MainFrame`.
3. The user clicks **Upload Images** — a multi-file dialog opens.
4. For each selected file, a new `ImageFrame` is created and `Show(true)`.
5. Inside `ImageFrame`'s constructor:
   - A toolbar is built with all filter buttons.
   - A horizontal sizer places the `ImagePanel` (weight 3) and a sidebar
     panel (weight 1) side by side.
   - The sidebar contains a `HistogramPanel` and a `wxListBox` for history.
   - `PushHistory("Original")` records the initial image.
6. The user clicks toolbar buttons, each of which:
   - Reads the current image via `m_imagePanel->GetCurrentImage()`
   - Passes it to a `Filtering::*` method
   - Writes the result back via `m_imagePanel->SetImage(result)`
   - Calls `RefreshHistogram()` and `PushHistory("label")`
7. Clicking a history entry restores that version. Applying a new effect
   after navigating back truncates the forward branch.

---

## 7. Class-by-Class Breakdown

### App (`App.h` / `App.cpp`)

- Inherits `wxApp`. Single responsibility: initialise image format handlers
  and create the intro window.

### MainFrame (`MainFrame.h` / `MainFrame.cpp`)

- The intro/landing page. Has a centered "Upload Images" button.
- `OnUploadClicked()` opens a `wxFileDialog` with multi-select enabled.
- For each chosen file, spawns an independent `ImageFrame`.

### ImageFrame (`ImageFrame.h` / `ImageFrame.cpp`)

This is the **most complex class** in the project. It owns all the UI for a
single image editing session.

**Key members:**

| Member            | Type               | Purpose                          |
| ----------------- | ------------------ | -------------------------------- |
| `m_imagePanel`    | `ImagePanel*`      | Displays the image               |
| `m_toolbar`       | `wxToolBar*`       | Filter/Noise/LPF buttons         |
| `m_histPanel`     | `HistogramPanel*`  | Live RGB histogram in sidebar    |
| `m_historyList`   | `wxListBox*`       | Clickable version-history log    |
| `m_historyImages` | `vector<wxImage>`  | Deep copies at each history step |
| `m_historyLabels` | `vector<wxString>` | Human-readable label per step    |

**Event routing**: Each toolbar button has a unique `ID_*` constant (defined in
an `enum` at the bottom of `ImageFrame.h`). The `wxBEGIN_EVENT_TABLE` macro
maps each ID to its handler method.

### ImagePanel (`ImagePanel.h` / `ImagePanel.cpp`)

- Displays a single `wxImage` with aspect-ratio-preserving scaling.
- On resize (`OnSize`), creates a `wxBitmap` scaled to fit the panel and
  caches it in `m_scaledBitmap`.
- On paint (`OnPaint`), draws the cached bitmap centered in the panel.
- Stores both `m_originalImage` (never modified) and `m_currentImage` (the
  working copy).

### HistogramPanel (`HistogramPanel.h` / `HistogramPanel.cpp`)

- Computes per-channel (R, G, B) histograms (256 bins each) and cumulative
  distribution functions.
- Uses double-buffered painting (`wxBG_STYLE_PAINT` + `wxAutoBufferedPaintDC`)
  for flicker-free rendering.
- Draws solid colored lines for the frequency histogram and dotted lines for
  the CDF overlay.
- `Update(const wxImage&)` recomputes everything and calls `Refresh()`.

### HistogramFrame (`HistogramFrame.h` / `HistogramFrame.cpp`)

- A thin popup window wrapper around `HistogramPanel`. Created when the user
  clicks the "Histogram" toolbar button. Only 16 lines of code.

### Filtering (`Filtering.h` / `Filtering.cpp`)

**The algorithm engine.** All methods are `static` — the class has no state.
It is a pure utility class.

| Category         | Method                 | Implementation      |
| ---------------- | ---------------------- | ------------------- |
| Edge detection   | `ApplySobel`           | Manual convolution  |
|                  | `ApplyRoberts`         | Manual convolution  |
|                  | `ApplyPrewitt`         | Manual convolution  |
|                  | `ApplyCanny`           | OpenCV              |
| Histogram        | `GetHistogram`         | Manual counting     |
|                  | `GetDistributionCurve` | Sliding-window avg  |
|                  | `EqualizeHistogram`    | CDF-based LUT       |
|                  | `NormalizeHistogram`   | Linear stretch LUT  |
| Noise            | `AddUniformNoise`      | std::mt19937        |
|                  | `AddGaussianNoise`     | std::normal_distr   |
|                  | `AddSaltPepperNoise`   | Random pixel set    |
| Low-pass filters | `FilterAverage`        | Manual box kernel   |
|                  | `FilterGaussian`       | Manual Gauss kernel |
|                  | `FilterMedian`         | Manual sort-median  |

**Internal helpers (private):**

- `ApplyMatrix()` — generic 2D convolution for edge detectors. Takes two
  kernels (X direction and Y direction) and returns `{magnitude, gradX, gradY}`.
- `Clamp(int v)` — clamps a value to `[0, 255]`. Used across many functions.
- `ApplyKernel()` — file-static helper that convolves any float kernel over
  an RGB image with edge-clamped padding.

---

## 8. How the History System Works

The history system is a **linear branch model** (like `git` without branches):

```
[Original] → [Grayscale] → [Blur] → [Uniform Noise]
                                          ^
                                     current selection
```

### State storage

```cpp
std::vector<wxImage>   m_historyImages;   // deep copy at each step
std::vector<wxString>  m_historyLabels;   // "Grayscale", "Blur", etc.
wxListBox*             m_historyList;     // UI: shows numbered entries
```

### Recording a new step (`PushHistory`)

```
1. Read image from m_imagePanel->GetCurrentImage()
2. If user is viewing an older entry (not the last one):
   → Erase all entries AFTER the selected one (truncate future branch)
   → Rebuild the wxListBox to match
3. Deep-copy the image into m_historyImages
4. Append label to m_historyLabels and wxListBox
5. Auto-select the new (last) entry
```

### Navigating history (`OnHistorySelect`)

Clicking an entry in the list restores that snapshot. The entries are NOT
deleted. They are only truncated when you apply a **new** effect after
navigating back.

---

## 9. How the Live Histogram Works

Every toolbar handler follows this pattern:

```cpp
m_imagePanel->SetImage(result);    // update the displayed image
RefreshHistogram();                // recompute & redraw histogram
PushHistory("label");              // record the snapshot
```

`RefreshHistogram()` simply calls:

```cpp
m_histPanel->Update(m_imagePanel->GetCurrentImage());
```

Which in turn:

1. Resets all 6 vectors (3 histograms + 3 CDFs) to zero.
2. Iterates every pixel, incrementing `m_histR[ red_value ]`, etc.
3. Finds the global max frequency for Y-axis scaling.
4. Builds cumulative distributions for the CDF overlay.
5. Calls `Refresh()` → triggers `OnPaint()` → redraws the chart.

---

## 10. How Image Filters Work

All filters in `Filtering.cpp` follow a consistent pattern:

```cpp
wxImage Filtering::SomeFilter(const wxImage& image, /* params */) {
    // 1. Validate
    if (!image.IsOk()) return wxImage();

    // 2. Create a working copy
    wxImage result = image.Copy();          // deep copy
    unsigned char* data = result.GetData(); // raw RGB pointer

    // 3. Process pixels
    for (each pixel) {
        data[i] = Clamp(/* new value */);
    }

    // 4. Return
    return result;
}
```

For convolution-based filters (Average, Gaussian, Median), a file-static
helper `ApplyKernel()` handles the edge-clamped convolution loop. You only
need to build the kernel and pass it in.

---

## 11. Adding a New Feature (Step-by-Step)

Here is a concrete walkthrough for adding a **new filter** (e.g. "Sharpen").

### Step 1: Declare the algorithm in `Filtering.h`

```cpp
// In the public section of class Filtering:
static wxImage ApplySharpen(const wxImage& image);
```

### Step 2: Implement the algorithm in `Filtering.cpp`

```cpp
wxImage Filtering::ApplySharpen(const wxImage& image) {
    if (!image.IsOk()) return wxImage();

    // 3x3 Laplacian sharpening kernel
    int kSize = 3;
    std::vector<float> kernel = {
         0, -1,  0,
        -1,  5, -1,
         0, -1,  0
    };

    // Reuse the existing convolution helper
    return ApplyKernel(image, kernel, kSize);
}
```

> **Note**: `ApplyKernel` is a `static` (file-local) function in
> `Filtering.cpp`. Since your new method is also in `Filtering.cpp`,
> you can call it directly.

### Step 3: Add a toolbar ID in `ImageFrame.h`

```cpp
enum {
    // ... existing IDs ...
    ID_LOWPASS_MEDIAN,
    // ↓ Add yours here (before ID_HISTORY_SELECT)
    ID_FILTER_SHARPEN,
    // History
    ID_HISTORY_SELECT,
    ID_HISTORY_RESTORE
};
```

### Step 4: Declare the event handler in `ImageFrame.h`

```cpp
// In the private section, near the other filter handlers:
void OnFilterSharpen(wxCommandEvent& event);
```

### Step 5: Wire the event in the event table (`ImageFrame.cpp`)

```cpp
wxBEGIN_EVENT_TABLE(ImageFrame, wxFrame)
    // ... existing entries ...
    EVT_TOOL(ID_FILTER_SHARPEN,    ImageFrame::OnFilterSharpen)    // ← add
    // ...
wxEND_EVENT_TABLE()
```

### Step 6: Add the toolbar button (`CreateFilterToolbar` in `ImageFrame.cpp`)

```cpp
// In the appropriate section of CreateFilterToolbar():
m_toolbar->AddTool(ID_FILTER_SHARPEN, "Sharpen",
    wxArtProvider::GetBitmap(wxART_FIND, wxART_TOOLBAR),
    "Apply Sharpening Filter");
```

### Step 7: Implement the handler (`ImageFrame.cpp`)

```cpp
void ImageFrame::OnFilterSharpen(wxCommandEvent&) {
    if (!PanelReady()) return;

    wxImage result = Filtering::ApplySharpen(
        m_imagePanel->GetCurrentImage());

    m_imagePanel->SetImage(result);
    RefreshHistogram();
    PushHistory("Sharpen");
    SetStatusText("Applied Sharpening filter.");
}
```

### Step 8: Build and test

```bash
cd build && make -j$(nproc) && ./ImageProcessor
```

> If your filter needs **user parameters** (like kernel size), create a
> `wxDialog` with text controls before calling the filter — see
> `OnLowPassAverage` or `OnNoiseGaussian` in `ImageFrame.cpp` for examples.

### Summary checklist for any new feature

| File             | What to add                                  |
| ---------------- | -------------------------------------------- |
| `Filtering.h`    | `static wxImage YourFilter(...)` declaration |
| `Filtering.cpp`  | Implementation of the algorithm              |
| `ImageFrame.h`   | `ID_YOUR_FILTER` in the enum + handler decl  |
| `ImageFrame.cpp` | Event table entry + toolbar button + handler |

No changes needed to CMakeLists.txt, ImagePanel, HistogramPanel, or any
other file.

---

## 12. Common Pitfalls & Gotchas

### 1. Never use non-ASCII characters in `wxString::Format` calls

**This was the cause of a persistent SIGSEGV crash.** The Unicode character
`σ` (sigma) in format strings like:

```cpp
wxString::Format("Gaussian Noise (σ=%.1f)", stddev)  // CRASHES
```

causes a segfault inside `libwx_baseu-3.0.so.0` when the system locale is
not UTF-8 (e.g. `LC_CTYPE=C`). The UTF-8 multi-byte sequence confuses the
internal `wchar_t` conversion in `wxString::Format`.

**Rule**: Use only plain ASCII in any string that goes through wxWidgets
string formatting or `wxString::Format`:

```cpp
wxString::Format("Gaussian Noise (s=%.1f)", stddev)   // SAFE
```

Non-ASCII is safe in comments — just not in string literals passed to wx.

### 2. Always guard `wxImage` functions

```cpp
if (!image.IsOk()) return wxImage();         // before processing
unsigned char* data = result.GetData();
if (!data) return result;                     // before pixel access
```

### 3. `wxImage::Copy()` vs copy constructor

- `wxImage copy = original;` → **reference-counted** (shallow, fast)
- `wxImage copy = original.Copy();` → **deep copy** (new pixel buffer)

When you need to modify pixel data, **always** use `.Copy()` to get your own
buffer. Otherwise you'll corrupt the source image through shared data.

### 4. `wxImage::GetData()` returns non-const `unsigned char*`

It does NOT trigger a copy-on-write. If two wxImages share data (via the
copy constructor) and you call `GetData()` on one and write through the
pointer, you corrupt both.

### 5. Return `wxImage` by value — RVO is reliable

```cpp
wxImage Filtering::SomeFilter(const wxImage& image) {
    wxImage result = image.Copy();
    // ... modify result ...
    return result;   // RVO or move — efficient
}
```

### 6. Toolbar IDs must be unique and sequential

The `enum` at the bottom of `ImageFrame.h` auto-numbers them starting at
`ID_RESET = 100`. Add new IDs at the end (but before the History IDs) to
avoid renumbering existing ones.

### 7. Event table must match

Every `EVT_TOOL(ID, Handler)` entry requires both:

- The ID to exist in the enum
- The handler to be declared in the class header

A missing entry means the toolbar button does nothing (silent failure).

### 8. Sidebar proportions

The main sizer gives weight 3 to `ImagePanel` and weight 1 to the sidebar.
The sidebar's internal sizer gives weight 3 to the histogram and weight 2 to
the history list. Change these weights to adjust the layout ratio.

---

## 13. Coding Conventions

- **Headers**: one class per header, `#ifndef` include guards.
- **Naming**: PascalCase for class names and methods, `m_` prefix for member
  variables, `ID_UPPER_SNAKE` for enum constants.
- **Static methods**: `Filtering` is all-static — treat it like a namespace.
- **Comments**: section-separating comments use box-drawing characters
  (`────────`) in comments only (never in string literals).
- **Image format**: All internal processing uses `wxImage` (RGB, 8-bit per
  channel, no alpha). OpenCV's `cv::Mat` is only used transiently for Canny.
- **Parameter dialogs**: When a filter needs user input, create a local
  `wxDialog` on the stack with a `wxFlexGridSizer` for labels + text
  controls, plus `CreateButtonSizer(wxOK | wxCANCEL)`.

---

_Last updated: 2 March 2026_
