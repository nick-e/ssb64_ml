cmake_minimum_required (VERSION 3.10.2)

set(CMAKE_CXX_STANDARD 11)
project(SSBML)

file(GLOB CORE_SRC "core/*.cc")
file(GLOB GTKMM_SRC "gtkmm/*.cc")

find_package(X11 REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(OPENCV REQUIRED opencv)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

add_executable(SSBML ${CORE_SRC} ${GTKMM_SRC})
include_directories(${X11_INCLUDE_DIR} ${GTKMM_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS} "/opt/cuda/include" "/usr/local/include/tensorflow")
target_link_libraries(SSBML ${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${GTKMM_LIBRARIES} ${OPENCV_LDFLAGS})
add_definitions(${GTKMM_CFLAGS_OTHER})
link_directories(${GTKMM_LIBRARY_DIRS})
