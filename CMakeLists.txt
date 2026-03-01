cmake_minimum_required(VERSION 3.10)
project(ImageProcessor VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ── wxWidgets ──────────────────────────────────────────────────
if(NOT wxWidgets_LIBRARIES)
    find_package(wxWidgets COMPONENTS core base adv REQUIRED)
    include(${wxWidgets_USE_FILE})
endif()

# ── OpenCV ─────────────────────────────────────────────────────
set(OpenCV_DIR "C:/Users/ahmed/Downloads/OpenCV-MinGW-Build-OpenCV-4.5.5-x64/OpenCV-MinGW-Build-OpenCV-4.5.5-x64/x64/mingw/lib")   # <-- change if you extracted to a different path
find_package(OpenCV REQUIRED)

# ── Include directories ────────────────────────────────────────
include_directories(include ${wxWidgets_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
add_definitions(-D__WXMSW__ -DUNICODE -D_UNICODE)

# ── Source files ───────────────────────────────────────────────
file(GLOB SOURCES "src/*.cpp")

# ── Executable ─────────────────────────────────────────────────
add_executable(ImageProcessor ${SOURCES})

# ── Link libraries ─────────────────────────────────────────────
target_link_libraries(ImageProcessor ${wxWidgets_LIBRARIES} ${OpenCV_LIBS})