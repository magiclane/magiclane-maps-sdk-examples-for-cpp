get_filename_component(_lvgl_prefix "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

if(NOT TARGET lvgl::thorvg)
    add_library(lvgl::thorvg STATIC IMPORTED)
    find_library(_lvgl_thorvg_lib NAMES lvgl_thorvg PATHS "${_lvgl_prefix}/lib" NO_DEFAULT_PATH)
    if(_lvgl_thorvg_lib)
        set_target_properties(lvgl::thorvg PROPERTIES
            IMPORTED_LOCATION "${_lvgl_thorvg_lib}")
    endif()
    unset(_lvgl_thorvg_lib CACHE)
endif()

if(NOT TARGET lvgl::lvgl)
    add_library(lvgl::lvgl STATIC IMPORTED)
    find_library(_lvgl_lib NAMES lvgl PATHS "${_lvgl_prefix}/lib" NO_DEFAULT_PATH)
    set_target_properties(lvgl::lvgl PROPERTIES
        IMPORTED_LOCATION "${_lvgl_lib}"
        INTERFACE_INCLUDE_DIRECTORIES "${_lvgl_prefix}/include;${_lvgl_prefix}/include/lvgl"
        INTERFACE_LINK_LIBRARIES "lvgl::thorvg"
        INTERFACE_COMPILE_DEFINITIONS "LV_CONF_INCLUDE_SIMPLE;LV_LVGL_H_INCLUDE_SIMPLE"
    )
    unset(_lvgl_lib CACHE)
endif()

unset(_lvgl_prefix)
