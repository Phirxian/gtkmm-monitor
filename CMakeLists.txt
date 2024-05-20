cmake_minimum_required(VERSION 3.16)
project(XtmProcessMonitorDemo)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
pkg_check_modules(CAIROMM REQUIRED cairomm-1.16) 
pkg_check_modules(GTK4 REQUIRED gtk4)

link_directories(
    ${GTK4_LIBRARY_DIRS}
    ${CAIROMM_LIBRARY_DIRS}
    ${GTKMM_LIBRARY_DIRS}
)

include_directories(
    ${GTK4_INCLUDE_DIRS}
    ${CAIROMM_INCLUDE_DIRS}
    ${GTKMM_INCLUDE_DIRS}
)

add_executable(SystemMonitorDemo
    main.cpp
    DynamicGraphDisplay.cpp
    System.cpp
)

target_link_libraries(
	SystemMonitorDemo
	${GTK4_LIBRARIES}
	${GTKMM_LIBRARIES}
	${CAIROMM_LIBRARIES}
)