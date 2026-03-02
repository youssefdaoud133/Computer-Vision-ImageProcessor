# AI Agent Instructions for Computer-Vision-Task1

This repository is a small C++ GUI application built using **wxWidgets**. It follows strict OOP principles with clear separation between interface (`include/`) and implementation (`src/`). The intent of these instructions is to help any AI coding agent understand the project's structure, build expectations, and common coding patterns so it can be productive immediately.

---

## 🚀 Big Picture Architecture

- **Entry point** is `App` (`include/App.h` `src/App.cpp`). `wxIMPLEMENT_APP(App)` creates the application.
- **MainFrame** (`include/MainFrame.h`/`src/MainFrame.cpp`) is the introductory window. It contains a static text intro and an "Upload Images" button. Clicking the button opens a `wxFileDialog` configured for multiple image selection and spawns one `ImageFrame` per selected file.
- **ImageFrame** (`include/ImageFrame.h`/`src/ImageFrame.cpp`) is a top‑level window for a single image. It hosts a toolbar with filter buttons and contains an `ImagePanel` child that actually draws the image.
- **ImagePanel** (`include/ImagePanel.h`/`src/ImagePanel.cpp`) stores two `wxImage` objects (`m_originalImage` and `m_currentImage`) plus a cached `wxBitmap` scaled for the panel size. It handles `EVT_PAINT` and `EVT_SIZE` to preserve aspect ratio when resizing.

Data flow:
1. Load image file in `ImagePanel` constructor. On errors, `wxLogError` is used.
2. Filters in `ImageFrame` may operate on either the original or current image then call `ImagePanel::SetImage` followed by `Refresh()`.
3. `ImagePanel::OnSize` recalculates `m_scaledBitmap` and triggers repaint.

Memory/ownership:
- Most UI objects are created with `new` and a parent pointer; wxWidgets autoreleases them. Avoid raw heap objects without a parent.

---

## 🛠 Build & Run Workflow

**Requirements**
- C++17 or newer
- CMake ≥3.10
- wxWidgets 3.0+ (on Windows you can install via vcpkg or the MSI installer; on Linux use `libwxgtk3.0-dev`)

**Steps** (run in project root):
```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```
Executable target is `ImageProcessor`.

No automated tests are present; manual testing consists of running the binary and exercising the UI.

> 💡 On Windows with Visual Studio, open the generated sln from the `build` folder.

---

## ✅ Project-Specific Conventions

- **Header/Source pairing** – every class has a `.h` and `.cpp` file with identical names.
- **Member naming**: prefix with `m_` (e.g. `m_uploadBtn`, `m_currentImage`).
- **Event handlers**: methods start with `On` (e.g. `OnUploadClicked`, `OnFilterBlur`).
- **Event tables**: use `wxBEGIN_EVENT_TABLE/ wxEND_EVENT_TABLE` macros in the `.cpp`; IDs are defined as enums in the header.
- **IDs**: start from 1 for custom controls; filters use `ID_FILTER_*` macros. When adding a new toolbar button or control, extend the enum and the event table accordingly.
- **UI layout**: wxBoxSizer (vertical) is used almost everywhere. Fonts for important text are modified inline.
- **Image operations**: use `wxImage` helpers (`ConvertToGreyscale`, `Blur`, `Scale`, etc.). Manual pixel loops appear only in `OnFilterInvert`.

> ⚠️ When adding new filters, decide whether they should apply to `m_originalImage` or `m_currentImage` and update the corresponding handler.

---

## 🔗 Integration Points & Dependencies

- The only external dependency is wxWidgets; linkage is handled in `CMakeLists.txt` via `find_package(wxWidgets COMPONENTS core base adv REQUIRED)`.
- No networking, databases, or third‑party libraries are used.
- The `CMakeLists.txt` collects all `src/*.cpp`; add new source files there or update the glob if necessary.

---

## 📂 Key Files to Reference

- `README.md` – contains user‑level overview and build instructions (often kept in sync).
- `include/MainFrame.h` / `src/MainFrame.cpp` – shows file dialog usage and frame creation.
- `include/ImageFrame.h` / `src/ImageFrame.cpp` – illustrates toolbar creation, event wiring, filter handlers.
- `include/ImagePanel.h` / `src/ImagePanel.cpp` – resizing logic and paint handling.
- `CMakeLists.txt` – simple CMake configuration; add new library dependencies here.

---

## 📝 When Writing Code or Answers

- For GUI changes, prefer modifying existing sizer layouts rather than hardcoding positions.
- Preserve aspect ratio logic when manipulating images.
- Keep all wxWidgets calls on the main thread—there is no multithreading support in the current design.
- Use `wxLogError`/`SetStatusText` for user‑visible error / status messages.
- Maintain the style of comments present in headers and implementations (Doxygen‑style for public methods).
- Do not introduce STL containers for UI objects; stick with wxWidgets types unless internal computation demands vectors/strings.

---

## ℹ️ Notes for Future Maintenance

- Tests can be added later with a GUI testing framework or by refactoring filter logic into testable utility functions.
- Additional image formats require nothing special beyond what `wxInitAllImageHandlers()` provides in `App::OnInit()`.
- New windows should follow the `Frame + Panel` pattern used by `ImageFrame` and `ImagePanel`.

> Feedback welcome—if any step or pattern isn't clear, ask for more details so this document can be improved.

---

Happy coding! 👩‍💻👨‍💻 Should you need clarification on any part of the application, let me know. 
