# SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
# SPDX-License-Identifier: Apache-2.0
#
# Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

cmake_minimum_required(VERSION 3.29...4.1)

project(MapsSDK_CppExamples LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/SDK/lib/cmake/GEM")

if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/SDK/lib")
    message(FATAL_ERROR "SDK binaries missing")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# RPATH settings for Unix systems
if(UNIX)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    # Add origin-relative RPATH for finding shared libraries next to executables
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

get_property(__isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(__isMultiConfig)
  if(NOT CMAKE_CONFIGURATION_TYPES)
    if(MSVC)
      # For Windows/MSVC, Debug and Release SDKs are shipped
      set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
      "Choose the type of builds. Windows SDK supports: Debug Release." FORCE)
    else()
      set(CMAKE_CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo" CACHE STRING
      "Choose the type of builds, options are: Debug Release RelWithDebInfo MinSizeRel. (default: Release;Debug;RelWithDebInfo)" FORCE)
    endif()
  endif()
  message(STATUS "Configuration types: ${CMAKE_CONFIGURATION_TYPES}")
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel. (default: Release)" FORCE)
  endif()
  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

include(GNUInstallDirs)

# Set output directories
set(OUTPUT_BASE ${CMAKE_BINARY_DIR})
if(UNIX)
    set(LIB_SUBDIR ${CMAKE_INSTALL_LIBDIR})
    set(BIN_SUBDIR ${CMAKE_INSTALL_BINDIR})
else()
    set(LIB_SUBDIR ${CMAKE_INSTALL_BINDIR})
    set(BIN_SUBDIR ${CMAKE_INSTALL_BINDIR})
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BASE}/${LIB_SUBDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_BASE}/${LIB_SUBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BASE}/${BIN_SUBDIR})

foreach(OutputConfig IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER ${OutputConfig} OUTPUTCONFIG)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_BASE}/${OutputConfig}/${LIB_SUBDIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_BASE}/${OutputConfig}/${LIB_SUBDIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_BASE}/${OutputConfig}/${BIN_SUBDIR})
endforeach()

set(API_TOKEN "" CACHE STRING "Get new project API token from: https://developer.magiclane.com/api/projects")
set(MAPS_SDK_DATA_PATH "" CACHE STRING "Path to Data folder of the SDK")
set(MAPS_SDK_CACHE_PATH "" CACHE STRING "Path to the location where SDK temporaries can be stored")
set(MAPS_SDK_PLUGIN_PATH "" CACHE STRING "Path to the location where shared library of the SDK can be stored")

find_package(GEM REQUIRED CONFIG)

add_subdirectory(Examples)
