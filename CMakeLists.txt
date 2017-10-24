#  Copyright (c) 2017 Alireza Kheirkhahang
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.4)
project(launch_porcess)

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
link_directories(${CMAKE_BINARY_DIR}/lib)
include_directories(${PROJECT_SOURCE_DIR})
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find and Include HPX
IF(HPX_ROOT AND NOT HPX_DIR)
    SET(HPX_DIR "${HPX_ROOT}/lib/cmake/HPX")
ENDIF()

# Instruct cmake to find the HPX settings, it looks for HPXConfig.cmake in ${HPX_DIR}
find_package(HPX)

include_directories(${HPX_INCLUDE_DIR})
link_directories(${HPX_LIBRARY_DIR})

# Adjust auxiliary build settings for HPX to work properly
IF(NOT MSVC)
    set(HPX_RPATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${HPX_RPATH})
ELSE()
    # enable solution folders for MSVC
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    IF(HPX_NO_INSTALL)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/$(Configuration)" CACHE PATH
                "Prefix prepended to install directories." FORCE)
    ENDIF()
ENDIF()

add_hpx_executable(hello_world
        SOURCES hello_world.cpp
        FOLDER
        COMPONENT_DEPENDENCIES iostreams)

add_subdirectory(components)

add_hpx_executable(launch_process_test
        SOURCES launch_process.cpp
        DEPENDENCIES iostreams_component process_component
        launch_process_test_server_component
        HPX_PREFIX ${HPX_BUILD_PREFIX}
        FOLDER)

add_hpx_executable(launch_process_parallel_test
        SOURCES launch_process_parallel.cpp
        DEPENDENCIES iostreams_component process_component
        launch_process_test_server_component
        HPX_PREFIX ${HPX_BUILD_PREFIX}
        FOLDER)

add_hpx_executable(launch_process_parallel2_test
        SOURCES launch_process_parallel2.cpp
        DEPENDENCIES iostreams_component process_component
        launch_process_test_server_component
        HPX_PREFIX ${HPX_BUILD_PREFIX}
        FOLDER)

add_hpx_executable(launch_process_parallel3_test
        SOURCES launch_process_parallel3.cpp
        DEPENDENCIES iostreams_component process_component
        launch_process_test_server_component
        HPX_PREFIX ${HPX_BUILD_PREFIX}
        FOLDER)

add_hpx_executable(launched_process_test
        SOURCES launched_process.cpp
        HPX_PREFIX ${HPX_BUILD_PREFIX}
        FOLDER
        COMPONENT_DEPENDENCIES launch_process_test_server)