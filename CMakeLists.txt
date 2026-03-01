cmake_minimum_required(VERSION 3.10)
project(ImageProcessor VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Manual wxWidgets configuration (using the local build in Downloads)
set(WX_ROOT "C:/Users/lenovo/Downloads/wxWidgets-3.3.1")
set(WX_LIB_DIR "${WX_ROOT}/build_cmake/lib/gcc_x64_dll")

# Include directories
include_directories(include)
include_directories("${WX_ROOT}/include")
include_directories("${WX_LIB_DIR}/mswu") # For setup.h

# Essential wxWidgets preprocessor definitions
add_definitions(-DUNICODE -D_UNICODE -D__WXMSW__ -DWXUSINGDLL)

# Link directories
link_directories("${WX_LIB_DIR}")

# Source files
file(GLOB SOURCES "src/*.cpp")

# Executable
add_executable(ImageProcessor ${SOURCES})

# Link libraries
target_link_libraries(ImageProcessor 
    wxmsw33u_core
    wxbase33u
    # System libraries required for wxWidgets
    kernel32 user32 gdi32 comdlg32 winspool winmm shell32 comctl32 ole32 oleaut32 uuid rpcrt4 advapi32 version wsock32 gdiplus msimg32 shlwapi uxtheme oleacc
)

# Copy DLLs to build directory for runtime
add_custom_command(TARGET ImageProcessor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WX_LIB_DIR}/wxbase331u_gcc_custom.dll"
        "${WX_LIB_DIR}/wxmsw331u_core_gcc_custom.dll"
        $<TARGET_FILE_DIR:ImageProcessor>
)
