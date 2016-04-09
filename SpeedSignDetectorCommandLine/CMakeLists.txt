cmake_minimum_required(VERSION 2.8)

project(SpeedSignDetectorCommandLine)

add_subdirectory(../Detector ${CMAKE_CURRENT_BINARY_DIR}/Detector)
include_directories(../Detector)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)

# Tell CMake to create the SpeedSignDetectorCommandLine executable
add_executable(SpeedSignDetectorCommandLine
	main.cpp
  detectortask.cpp
)

# Use the Widgets module from Qt 5.
target_link_libraries(SpeedSignDetectorCommandLine Qt5::Widgets)
target_link_libraries(SpeedSignDetectorCommandLine Detector)
