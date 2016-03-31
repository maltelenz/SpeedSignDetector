cmake_minimum_required(VERSION 2.8)

project(Detector)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)

# Tell CMake to create the Detector library
add_library(Detector STATIC
	detector.cpp
  math_utilities.cpp
  array2d.cpp
)

# Use the Widgets module from Qt 5.
target_link_libraries(Detector Qt5::Widgets)
