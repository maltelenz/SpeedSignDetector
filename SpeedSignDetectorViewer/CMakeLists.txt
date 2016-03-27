cmake_minimum_required(VERSION 2.8)

project(SpeedSignDetectorViewer)

add_subdirectory(../Detector ${CMAKE_CURRENT_BINARY_DIR}/Detector)
include_directories(../Detector)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)

qt5_wrap_ui(UI_HEADERS mainwindow.ui)

# Tell CMake to create the SpeedSignDetectorViewer executable
add_executable(SpeedSignDetectorViewer
	main.cpp
	mainwindow.cpp
	${UI_HEADERS}
)

# Use the Widgets module from Qt 5.
target_link_libraries(SpeedSignDetectorViewer Qt5::Widgets)
target_link_libraries(SpeedSignDetectorViewer Detector)
