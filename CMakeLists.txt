cmake_minimum_required(VERSION 3.16)

project(RadarVisualizer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -Wpedantic)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_PREFIX_PATH "/home/theo/Qt/6.9.1/gcc_64" CACHE PATH "Qt6 installation root")

find_package(Qt6 REQUIRED COMPONENTS Core Gui)
qt_standard_project_setup()

set(RADAR_DATA_DIR "/home/theo/Downloads/radar-oxford/radar/" CACHE STRING "Radar dataset directory")

# Radar processing library
add_library(radar_processing STATIC
    processing/RadarParser.cpp
    processing/Metrics.cpp
)
target_include_directories(radar_processing PUBLIC processing)

# Main application
add_executable(RadarVisualizer processing/main.cpp)
target_compile_definitions(RadarVisualizer PRIVATE RADAR_DATA_DIR="${RADAR_DATA_DIR}")
target_link_libraries(RadarVisualizer PRIVATE radar_processing Qt6::Core Qt6::Gui)

# Tests
enable_testing()
add_subdirectory(processing/tests)
