cmake_minimum_required (VERSION 2.8)
project (wave-gui)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wno-long-long -pedantic")

#
## Dependencies
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(SYSTEM ${GLFW_INCLUDE_DIRS})

find_package(FFTW)
if (FFTW_FOUND)
    message(STATUS "FFTW support enabled.")
    include_directories(SYSTEM ${FFTW_INCLUDE_DIRS})
else()
    message(WARNING "Cannot find FFTW. Not using/building FFTW stuff")
endif()

#
## Logging parameters
if (CG_LOGGING) # Verbosity
    add_definitions(-DCG_LOGGING=${CG_LOGGING})
else (CG_LOGGING)
    add_definitions(-DCG_LOGGING=10)
endif (CG_LOGGING)

option(CG_OUTPUT "Use cg-output" ON)
if (CG_OUTPUT)
    add_definitions(-DCG_OUTPUT -DCOLOR_OUTPUT)
endif()

add_definitions(-DCG_COLOR_OUTPUT) # Colored log messages

## Libs
set(CG_CORE_LIB cg_core)
set(CG_IMGUI_LIB ImGui)

include_directories(src)
include_directories(external)

add_subdirectory(src)
add_subdirectory(external)
add_subdirectory(main)
