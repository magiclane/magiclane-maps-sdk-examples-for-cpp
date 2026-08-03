# SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
# SPDX-License-Identifier: Apache-2.0
#
# Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

function(setup_example_target exampleApp)
    add_executable(${exampleApp} 
		${${exampleApp}_SRCS} 
		${EXAMPLES_COMMON_HDRS} 
		${EXAMPLES_COMMON_SRCS} 
		${IMGUI_BACKEND_SOURCES})

    target_compile_definitions(${exampleApp} PRIVATE $<$<CONFIG:Debug>:_DEBUG>)

	if(MSVC)
		target_compile_definitions(${exampleApp} PRIVATE
			WIN32
			_WINDOWS
			_CRT_SECURE_NO_WARNINGS)

		target_compile_options(${exampleApp} PRIVATE
			/W3
			/MP
			/Gy
			/EHsc
			/FC
			/sdl
			/permissive-
			/Zc:inline)

		set_property(TARGET ${exampleApp} PROPERTY
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
	endif()

    foreach(def_name API_TOKEN MAPS_SDK_DATA_PATH MAPS_SDK_CACHE_PATH MAPS_SDK_PLUGIN_PATH)
        if(${def_name})
            target_compile_definitions(${exampleApp} PRIVATE ${def_name}="${${def_name}}")
        endif()
    endforeach()

    get_target_property(GEM_TARGET_TYPE GEM::GEM TYPE)
    if(NOT GEM_TARGET_TYPE STREQUAL "STATIC_LIBRARY")
        target_compile_definitions(${exampleApp} PRIVATE GEM_PLUGIN)
    endif()

    target_link_libraries(${exampleApp} PRIVATE GEM::GEM EXAMPLES_BUILD_OPTIONS)

    if(WIN32)
        set(THIRD_PARTY_TARGETS SDL2::SDL2 unofficial::angle::libEGL unofficial::angle::libGLESv2)
        foreach(lib_target ${THIRD_PARTY_TARGETS})
            if(TARGET ${lib_target})
                get_target_property(target_type ${lib_target} TYPE)
                if(NOT target_type STREQUAL "STATIC_LIBRARY")
                    add_custom_command(TARGET ${exampleApp} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            $<TARGET_FILE:${lib_target}>
                            $<TARGET_FILE_DIR:${exampleApp}>
                        VERBATIM
                    )
                endif()
            endif()
        endforeach()
    endif()

    if(NOT GEM_TARGET_TYPE STREQUAL "STATIC_LIBRARY")
        if(WIN32)
            add_custom_command(TARGET ${exampleApp} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_SOURCE_DIR}/SDK/bin/$<IF:$<CONFIG:Debug>,Debug,Release>/x64/$<IF:$<CONFIG:Debug>,GEM_d,GEM>.dll"
                    "$<TARGET_FILE_DIR:${exampleApp}>/"
                VERBATIM
            )
        elseif(UNIX AND NOT APPLE)
            file(GLOB _GEM_SHARED_LIBS "${GEM_SDK_LIB_DIR}/libGEM*.so*")
            if(_GEM_SHARED_LIBS)
                set(_gem_copy_commands)
                foreach(_lib ${_GEM_SHARED_LIBS})
                    get_filename_component(_lib_name "${_lib}" NAME)
                    if(IS_SYMLINK "${_lib}")
                        file(READ_SYMLINK "${_lib}" _link_target)
                        list(APPEND _gem_copy_commands
                            COMMAND ${CMAKE_COMMAND} -E create_symlink
                                "${_link_target}"
                                "$<TARGET_FILE_DIR:${exampleApp}>/${_lib_name}")
                    else()
                        list(APPEND _gem_copy_commands
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                "${_lib}"
                                "$<TARGET_FILE_DIR:${exampleApp}>/")
                    endif()
                endforeach()
                add_custom_command(TARGET ${exampleApp} POST_BUILD
                    ${_gem_copy_commands}
                    COMMENT "Copying libGEM shared libraries"
                    VERBATIM
                )
            endif()
        endif()
    endif()
endfunction()
