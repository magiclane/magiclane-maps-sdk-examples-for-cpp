# SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
# SPDX-License-Identifier: Apache-2.0
#
# Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

function(get_all_targets result dir)
    get_property(targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(subdirs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir IN LISTS subdirs)
        get_all_targets(sub_targets ${subdir})
        list(APPEND targets ${sub_targets})
    endforeach()
    set(${result} ${targets} PARENT_SCOPE)
endfunction()

option(USE_GLFW "Use GLFW backend" ON)
if(EMSCRIPTEN)
    set(USE_GLFW OFF)
endif()

set(USE_SANITIZER "OFF" CACHE STRING "Sanitizer mode to enable")
set_property(CACHE USE_SANITIZER PROPERTY STRINGS OFF Address Thread Undefined)

add_library(EXAMPLES_BUILD_OPTIONS INTERFACE)
target_include_directories(EXAMPLES_BUILD_OPTIONS INTERFACE
    ${CMAKE_SOURCE_DIR}/SDK/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR})

string(TOLOWER "${USE_SANITIZER}" USE_SANITIZER_LOWERCASE)

if(NOT USE_SANITIZER_LOWERCASE MATCHES "off")
    if(NOT CMAKE_BUILD_TYPE MATCHES ".*Deb.*")
        message(FATAL_ERROR "Sanitizers are supported only with debug info enabled (Debug or RelWithDebInfo build type)")
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
        if(MSVC)
            message(STATUS "Replacing /O2 with /O1 for sanitizer")
            string(REPLACE "/O2" "/O1" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
            string(REPLACE "/O2" "/O1" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
        else()
            message(STATUS "Replacing -O2 with -O1 for sanitizer")
            string(REPLACE "-O2" "-O1" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
            string(REPLACE "-O2" "-O1" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
        endif()
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" CACHE STRING "" FORCE)
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}" CACHE STRING "" FORCE)
    endif()

    if(MSVC)
        if(USE_SANITIZER_LOWERCASE MATCHES "address")
            target_compile_options(EXAMPLES_BUILD_OPTIONS INTERFACE /fsanitize=address)
            target_link_options(EXAMPLES_BUILD_OPTIONS INTERFACE /INCREMENTAL:NO)
        else()
            message(FATAL_ERROR "MSVC only supports address sanitizer")
        endif()
    else()
        target_compile_options(EXAMPLES_BUILD_OPTIONS INTERFACE -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-recover=all)
        if(USE_SANITIZER_LOWERCASE MATCHES "address")
            target_compile_options(EXAMPLES_BUILD_OPTIONS INTERFACE -fsanitize=address)
            target_link_options(EXAMPLES_BUILD_OPTIONS INTERFACE -fsanitize=address)
        elseif(USE_SANITIZER_LOWERCASE MATCHES "thread")
            target_compile_options(EXAMPLES_BUILD_OPTIONS INTERFACE -fsanitize=thread)
            target_link_options(EXAMPLES_BUILD_OPTIONS INTERFACE -fsanitize=thread)
        elseif(USE_SANITIZER_LOWERCASE MATCHES "undefined")
            target_compile_options(EXAMPLES_BUILD_OPTIONS INTERFACE -fsanitize=undefined -fno-sanitize=vptr)
            target_link_options(EXAMPLES_BUILD_OPTIONS INTERFACE -fsanitize=undefined)
        else()
            message(FATAL_ERROR "Unsupported sanitizer '${USE_SANITIZER_LOWERCASE}'")
        endif()
    endif()

    include(CheckPIESupported OPTIONAL RESULT_VARIABLE CHECK_PIE_SUPPORTED)
    if(CHECK_PIE_SUPPORTED)
        check_pie_supported(LANGUAGES CXX)
        if(CMAKE_CXX_LINK_PIE_SUPPORTED)
            set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
        endif()
    endif()
endif()

if(USE_GLFW)
    find_package(glfw3 REQUIRED)
    target_link_libraries(EXAMPLES_BUILD_OPTIONS INTERFACE glfw)
    target_compile_definitions(EXAMPLES_BUILD_OPTIONS INTERFACE USE_GLFW)
else()
    find_package(SDL2 CONFIG REQUIRED)
    find_package(imgui CONFIG REQUIRED)
    target_link_libraries(EXAMPLES_BUILD_OPTIONS INTERFACE SDL2::SDL2main SDL2::SDL2 imgui::imgui)
endif()

find_package(Threads REQUIRED)
target_link_libraries(EXAMPLES_BUILD_OPTIONS INTERFACE ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})

if(MSVC)
    find_package(unofficial-angle CONFIG REQUIRED)
    target_link_libraries(EXAMPLES_BUILD_OPTIONS INTERFACE unofficial::angle::libEGL unofficial::angle::libGLESv2 shlwapi psapi)
    target_link_options(EXAMPLES_BUILD_OPTIONS INTERFACE /ignore:4099)
else()
    find_package(OpenGL COMPONENTS EGL GLES2 REQUIRED)
    if(TARGET OpenGL::GLES3)
        target_link_libraries(EXAMPLES_BUILD_OPTIONS INTERFACE OpenGL::GLES3)
    elseif(TARGET OpenGL::GLES2)
        target_link_libraries(EXAMPLES_BUILD_OPTIONS INTERFACE OpenGL::GLES2)
    endif()
    if(TARGET OpenGL::EGL)
        target_link_libraries(EXAMPLES_BUILD_OPTIONS INTERFACE OpenGL::EGL)
    endif()
endif()

if(NOT USE_GLFW)
    target_compile_definitions(EXAMPLES_BUILD_OPTIONS INTERFACE USE_IMGUI)
    if(MSVC OR TARGET OpenGL::GLES3)
        target_compile_definitions(EXAMPLES_BUILD_OPTIONS INTERFACE IMGUI_IMPL_OPENGL_ES3)
    elseif(TARGET OpenGL::GLES2)
        target_compile_definitions(EXAMPLES_BUILD_OPTIONS INTERFACE IMGUI_IMPL_OPENGL_ES2)
    endif()
endif()

set(IMGUI_BACKENDS_DIR "${CMAKE_CURRENT_BINARY_DIR}/imgui_backends")
file(MAKE_DIRECTORY "${IMGUI_BACKENDS_DIR}")

function(download_imgui_backends)
    find_package(imgui CONFIG REQUIRED)

    get_target_property(IMGUI_INCLUDE_DIRS imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)
    list(GET IMGUI_INCLUDE_DIRS 0 IMGUI_INCLUDE_DIR)

    file(READ "${IMGUI_INCLUDE_DIR}/imgui.h" IMGUI_HEADER_CONTENT)
    string(REGEX MATCH "// dear imgui, v([0-9]+\\.[0-9]+\\.[0-9]+)" IMGUI_VERSION_MATCH "${IMGUI_HEADER_CONTENT}")
    if(IMGUI_VERSION_MATCH)
        string(REGEX REPLACE "// dear imgui, v([0-9]+\\.[0-9]+\\.[0-9]+)" "\\1" IMGUI_VERSION_NUM "${IMGUI_VERSION_MATCH}")
        set(IMGUI_VERSION "v${IMGUI_VERSION_NUM}")
    else()
        message(FATAL_ERROR "Could not detect ImGui version from ${IMGUI_INCLUDE_DIR}/imgui.h")
    endif()
    message(STATUS "Detected ImGui version: ${IMGUI_VERSION}")

    set(IMGUI_BACKEND_FILES
        imgui_impl_sdl2.h
        imgui_impl_sdl2.cpp
        imgui_impl_opengl3.h
        imgui_impl_opengl3.cpp
        imgui_impl_opengl3_loader.h)

    set(IMGUI_BACKEND_FILE_PATHS)
    foreach(BACKEND_FILE IN LISTS IMGUI_BACKEND_FILES)
        set(BACKEND_FILE_PATH "${IMGUI_BACKENDS_DIR}/${BACKEND_FILE}")
        list(APPEND IMGUI_BACKEND_FILE_PATHS "${BACKEND_FILE_PATH}")

        if(NOT EXISTS "${BACKEND_FILE_PATH}")
            message(STATUS "Downloading ImGui backend: ${BACKEND_FILE}")
            file(DOWNLOAD
                "https://raw.githubusercontent.com/ocornut/imgui/${IMGUI_VERSION}/backends/${BACKEND_FILE}"
                "${BACKEND_FILE_PATH}"
                STATUS DOWNLOAD_STATUS
                SHOW_PROGRESS
                TIMEOUT 30
                TLS_VERIFY ON)

            list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
            if(NOT STATUS_CODE EQUAL 0)
                list(REMOVE_AT DOWNLOAD_STATUS 0)
                string(JOIN " " ERROR_MESSAGE ${DOWNLOAD_STATUS})
                message(FATAL_ERROR "Failed to download ${BACKEND_FILE}: ${ERROR_MESSAGE}")
            endif()
            message(STATUS "Successfully downloaded: ${BACKEND_FILE}")
        endif()

        add_custom_command(
            OUTPUT "${BACKEND_FILE_PATH}"
            COMMAND ${CMAKE_COMMAND} -E echo "Checked existence of ${BACKEND_FILE}"
            VERBATIM)
    endforeach()

    add_custom_target(imgui_backends_download ALL DEPENDS ${IMGUI_BACKEND_FILE_PATHS})
endfunction()

if(NOT USE_GLFW)
    download_imgui_backends()
    add_dependencies(EXAMPLES_BUILD_OPTIONS imgui_backends_download)
endif()
target_include_directories(EXAMPLES_BUILD_OPTIONS INTERFACE ${IMGUI_BACKENDS_DIR})

enable_language(C)
if(NOT USE_GLFW)
    find_package(lvgl CONFIG REQUIRED)
    target_link_libraries(EXAMPLES_BUILD_OPTIONS INTERFACE lvgl::lvgl)
    target_compile_definitions(EXAMPLES_BUILD_OPTIONS INTERFACE
        USE_LVGL LV_LVGL_H_INCLUDE_SIMPLE LV_CONF_INCLUDE_SIMPLE)
endif()

include(Examples.srcs.cmake)

set(IMGUI_BACKEND_SOURCES "")
if(NOT USE_GLFW)
    set(IMGUI_BACKEND_SOURCES
        ${IMGUI_BACKENDS_DIR}/imgui_impl_sdl2.cpp
        ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp)
endif()

set(BUILD_APPS
	CenterMap
	MapPerspective
	MapStyle
	MapView
	MultiMapView
	PointsMarker
	PolygonMarker
	PolylineMarker

	SearchAround
	SearchFreeText
	SearchFreeTextLimitByGeographicArea

	CalculateRouteMultiView
	FeedSensorData
	FingerRoute
	NavigationLowEndHW
	PlayRoute
	RouteDirectionArrows
	Simulation

	TimezoneCoordinates
	TimezoneId

    AddCircleTerritory
	AddCustomer
	AddFullOptimization
	AddOptimSingleVDiffDepartDest
	AddOptimWithFixedOrdersSequences
	AddOptimWithMultiVMultiDepart
	AddOptimWithMultiVMultiDest
	AddOptimWithMultiVSingleDepart
	AddOptimWithMultiVSingleVConstr
	AddOptimWithOrderFields
	AddOptimWithSequencePairs
	AddOptimWithSetMatrices
	AddOptimWithSingleVehicle
	AddOrdersToOptimization
	AddOrdersToOptimWithSetMatrices
	AddOrdersToRouteAtTheEnd
	AddOrdersToRouteAtTheOptimPos
	AddOrdersToRouteAtTheSpecPos
	AddOrdersToRouteWithSetMatrices
	AddPolygonTerritory
	AddRectangleTerritory
	AddVehicle
	DeleteCustomer
	DeleteOptimization
	DeleteOrderFromOptimization
	DeleteOrderFromRoute
	DeleteRoute
	DeleteTerritory
	DeleteVehicle
	GenerateTerritories
	GetAllCustomers
	GetAllOptimizations
	GetAllRoutes
	GetAllTerritories
	GetAllVehicles
	GetCustomer
	GetCustomerHistory
	GetFuelPrices
	GetOptimization
	GetRoute
	GetSolutionForOptimization
	GetTerritory
	GetVehicle
	MergeRoutes
	ReoptimizeOptimization
	ReoptimizeRoute
	SearchCustomers
	SearchFuelPrices
	SearchMiscLocations
	SearchOptimization
	SearchOrders
	SearchRoutes
	SearchTerritories
	SearchVehicles
	SetFuelPrices
	UnlinkRoute
	UpdateCustomer
	UpdateOptimization
	UpdateOrderFromOptimization
	UpdateRoute
	UpdateTerritory
	UpdateVehicle)

if(NOT USE_GLFW)
    list(APPEND BUILD_APPS
        CalculateRoute
        ChangeMapStyle
        ContentDownload
        ExportTrack
        GpxNmeaPlayback
        GpxRoutingSimulation
        Markers
        MultiSearch
        SaveGPX
        ReverseGeocoding
        SimulateNavigation
        SwitchMapPerspective
        VRPOptimization
        Weather)
endif()

if(UNIX AND NOT APPLE)
    set(_GEM_SDK_LIB_SEARCH_ORDER Release Debug)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(_GEM_SDK_LIB_SEARCH_ORDER Debug Release)
    endif()
    foreach(_variant IN LISTS _GEM_SDK_LIB_SEARCH_ORDER)
        if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/SDK/lib/${_variant}/x86_64")
            set(GEM_SDK_LIB_DIR "${CMAKE_SOURCE_DIR}/SDK/lib/${_variant}/x86_64")
            break()
        endif()
    endforeach()
    if(NOT GEM_SDK_LIB_DIR)
        message(WARNING "GEM SDK shared library directory not found under ${CMAKE_SOURCE_DIR}/SDK/lib/")
    endif()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/SetupExampleTarget.cmake)

foreach(exampleApp IN LISTS BUILD_APPS)
    setup_example_target(${exampleApp})
endforeach()

add_custom_target(AllExamples)
if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Internal")
    add_subdirectory(Internal)
endif()

get_all_targets(allTargets ${CMAKE_CURRENT_SOURCE_DIR})
list(REMOVE_ITEM allTargets "EXAMPLES_BUILD_OPTIONS" "imgui_backends_download")

foreach(targetName IN LISTS allTargets)
    get_target_property(targetType ${targetName} TYPE)
    if(targetType STREQUAL "EXECUTABLE")
        add_dependencies(AllExamples ${targetName})
    endif()
endforeach()

set(_CACHE_DIRS)
foreach(targetName IN LISTS allTargets)
    get_target_property(targetType ${targetName} TYPE)
    if(targetType STREQUAL "EXECUTABLE")
        list(APPEND _CACHE_DIRS "$ENV{HOME}/TmpData_${targetName}")
    endif()
endforeach()
add_custom_target(clean-cache
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${_CACHE_DIRS}
    COMMENT "Removing example SDK cache directories"
    VERBATIM)
