cmake_minimum_required(VERSION 3.10)
project(ImageProcessor VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT wxWidgets_LIBRARIES)
    # Find wxWidgets
    find_package(wxWidgets COMPONENTS core base adv REQUIRED)
    include(${wxWidgets_USE_FILE})
endif()

# Include directories
include_directories(include ${wxWidgets_INCLUDE_DIRS})
add_definitions(-D__WXMSW__ -DUNICODE -D_UNICODE)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Executable
add_executable(ImageProcessor ${SOURCES})

# Link libraries
target_link_libraries(ImageProcessor ${wxWidgets_LIBRARIES})
