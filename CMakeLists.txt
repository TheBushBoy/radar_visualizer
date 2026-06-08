cmake_minimum_required(VERSION 3.16)

project(RadarVisualizer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -Wpedantic)

set(CMAKE_PREFIX_PATH "/home/theo/Qt/6.9.1/gcc_64" CACHE PATH "Qt6 installation root")

find_package(Qt6 REQUIRED COMPONENTS Core Gui)
qt_standard_project_setup()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    processing/*.cpp
)

add_executable(RadarVisualizer ${SOURCES})

target_include_directories(RadarVisualizer PRIVATE src)

target_link_libraries(RadarVisualizer PRIVATE
    Qt6::Core
    Qt6::Gui
)
