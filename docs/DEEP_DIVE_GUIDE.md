# ImageProcessor — Comprehensive Technical Deep-Dive

## 1. Project Mission

This application is a specialized image processing workbench designed for educational and professional exploration of Computer Vision algorithms. Unlike general-purpose editors, it emphasizes **transparency**: almost every filter is implemented from mathematical first principles in raw C++, allowing developers to see exactly how pixel manipulation occurs without the "black box" of heavy libraries.

---

## 2. Core Architecture

The project follows a **Modified Model-View-Controller (MVC)** pattern tailored for the wxWidgets event-driven loops.

### Component & Class Breakdown

The project operates through a set of tightly coupled components:

- **`App` (App.h / App.cpp):** The main application entry point (`wxApp`). It initializes wxWidgets and launches the `MainFrame`.
- **`MainFrame` (MainFrame.h / MainFrame.cpp):** The initial launcher window. It provides the interface to select/upload images and handles launching the Hybrid Image generation flow (`OnHybridClicked()`).
- **`ImageFrame` (ImageFrame.h / ImageFrame.cpp):** The core image editing UI. It orchestrates the GUI by tying together the toolbar, an `ImagePanel`, a live `HistogramPanel`, and a scrolling History sidebar. It routes filter events to the engine and uses `PushHistory()` to save snapshots for undo.
- **`ImagePanel` (ImagePanel.h / ImagePanel.cpp):** A custom viewport (via `wxPanel`) dedicated to rendering `wxImage`s. It automatically scales the viewing bitmap (`m_scaledBitmap`) while preserving the aspect ratio within `OnSize()`.
- **`HistogramPanel` (HistogramPanel.h / HistogramPanel.cpp):** A live-rendering component that calculates per-channel (RGB) intensity frequencies and Cumulative Distribution Functions (CDFs).
- **`HistogramFrame` (HistogramFrame.h / HistogramFrame.cpp):** A standalone resizable wrapper frame used when the histogram needs to be spawned as an independent popup instead of a sidebar.
- **`Filtering` (Filtering.h / Filtering.cpp):** The stateless, functional algorithm engine of the application. It receives a `const wxImage&` and returns a newly processed `wxImage`. It contains static mathematical implementations for:
  - **Edge Detection:** Custom convolution routines (Sobel, Roberts, Prewitt) returning magnitude/X/Y.
  - **Histograms:** Density scaling via Equalization and Normalization.
  - **Noise Synthesis:** Uniform, Gaussian, and Salt & Pepper functions (utilizing standard math/random libs where appropriate).
  - **Convolution & Spectral Approximations:** Low-pass (Average, Gaussian, Median) and frequency combinations (High-pass, Hybrid Image composition).

---

## 3. Data Structures & Memory Management

### The Pixel Buffer

We primarily use `wxImage` for processing. Internally, a `wxImage` holds a raw `unsigned char*` buffer in **RGBRGBRGB...** format (8 bits per channel).

- **Accessing Data:** `unsigned char* data = image.GetData();`
- **Memory Safety:** We use `image.Copy()` to ensure that when we modify pixels, we aren't accidentally changing a reference-counted shared buffer from a previous history state.

### Coordinate Mapping

Since the image is displayed inside a panel that might be a different size than the image itself, the `ImagePanel` performs a **Linear Transform** to map screen coordinates to pixel coordinates during rendering.

---

## 4. Algorithm Deep-Dive

### Spatial Domain Filtering (Convolution)

Most filters (Blur, Sobel, Gaussian) use **2D Convolution**.

1.  **Padding:** To handle edges, we use "Clamp Padding" (replicating the edge pixels).
2.  **Kernel Application:** A sliding window moves across the image. Each pixel in the result is the weighted sum of its neighbors defined by a kernel matrix.
3.  **Clamping:** After math operations (especially subtractions in High-Pass filters), we use `Filtering::Clamp()` to ensure values stay within the 0-255 range.

### Histogram Manipulation

- **Equalization:** We compute the **Cumulative Distribution Function (CDF)** of the image brightness. We then use this CDF as a lookup table (LUT) to remap pixel values, effectively "stretching" the density to be as uniform as possible.
- **Normalization:** A simpler linear stretch that maps the darkest pixel to 0 and the brightest to 255.

### Frequency Domain (Spatial Approximation)

The "Hybrid Image" feature uses spatial approximations of frequency filters:

- **Low-Pass:** Gaussian blur removes high-frequency detail (edges).
- **High-Pass:** Computed as `Original Image - Low-Pass Image`. This isolates the fine details.
- **Hybrid:** `Low-Pass(Image A) + High-Pass(Image B)`. At close distances, the human eye sees the high-pass details; from far away, the low-pass structure dominates.

---

## 5. The History & Versioning System

The application maintains a **Branchless Undo/Redo Stack**.

- Every time a filter is applied, `PushHistory()` takes a "Snapshot" (deep copy) of the current image.
- If you select an item from the middle of the history and apply a _new_ filter, the "future" branch is discarded (truncated), and a new timeline begins from that point. This prevents logical inconsistencies in the image state.

---

## 6. Real-time Analysis (Live Histogram)

The sidebar histogram is not just a static image. It is a live-rendering component that:

1.  Counts occurrences of every R, G, and B value [0-255].
2.  Normalizes the vertical height based on the most frequent value (so the graph always fits the window).
3.  Calculates the CDF on-the-fly to show the color density distribution.

---

## 7. Developer's "Golden Rules"

1.  **Strictly ASCII:** Never use Unicode symbols (like σ) in string formats; it causes library-level crashes on certain Linux locales.
2.  **Stateless Filtering:** Keep algorithm logic in `Filtering.cpp` and UI logic in `ImageFrame.cpp`.
3.  **Deep Copies:** Always `Copy()` an image before modifying its pixel buffer.
4.  **UI Updates:** After changing the image, call `RefreshHistogram()` and `PushHistory()` to keep the sidebar in sync.

---

## 8. Troubleshooting the Build

If the build fails:

- Verify `libwxgtk3.0-gtk3-dev` is installed.
- Ensure OpenCV is found via `pkg-config` or `find_package`.
- Check that you are using C++17 (required for `std::clamp` and filesystem features).

---

_Created for developers, by developers._
