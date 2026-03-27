vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO lvgl/lvgl
    REF v9.5.0
    SHA512 fde9ebd6663e9fea532e49cb586fa947d562f1c4bc22d65ede8e58ed4a63fe2fbc4186d0bbc10fe32e2b6388f0ccf527d043ddaba5013b0f1406ac170d1751ba
    HEAD_REF master
)

# Place the project-specific lv_conf.h in the LVGL source root so that
# both the build (via LV_BUILD_CONF_PATH) and the install step pick it up.
file(COPY "${CMAKE_CURRENT_LIST_DIR}/lv_conf.h" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        "-DLV_BUILD_CONF_PATH=${SOURCE_PATH}/lv_conf.h"
        -DCONFIG_LV_BUILD_DEMOS=OFF
        -DCONFIG_LV_BUILD_EXAMPLES=OFF
)

vcpkg_cmake_install()

# LVGL does not generate CMake config files, so install our own.
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/lvgl-config.cmake"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")

# Ensure lv_conf.h is installed alongside the LVGL headers.
if(NOT EXISTS "${CURRENT_PACKAGES_DIR}/include/lvgl/lv_conf.h")
    file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/lv_conf.h"
         DESTINATION "${CURRENT_PACKAGES_DIR}/include/lvgl")
endif()

# Fix pkgconfig: move to lib/pkgconfig and fix absolute paths.
if(EXISTS "${CURRENT_PACKAGES_DIR}/share/pkgconfig/lvgl.pc")
    file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/lib/pkgconfig")
    file(RENAME "${CURRENT_PACKAGES_DIR}/share/pkgconfig/lvgl.pc"
                "${CURRENT_PACKAGES_DIR}/lib/pkgconfig/lvgl.pc")
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/pkgconfig")
endif()
if(EXISTS "${CURRENT_PACKAGES_DIR}/debug/share/pkgconfig/lvgl.pc")
    file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig")
    file(RENAME "${CURRENT_PACKAGES_DIR}/debug/share/pkgconfig/lvgl.pc"
                "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig/lvgl.pc")
endif()
vcpkg_fixup_pkgconfig()

file(INSTALL "${SOURCE_PATH}/LICENCE.txt"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include"
                     "${CURRENT_PACKAGES_DIR}/debug/share")

# Remove empty directories left by the header install.
file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/include/lvgl/src/drivers/opengles/glad/src"
    "${CURRENT_PACKAGES_DIR}/include/lvgl/src/libs/gltf/fastgltf"
    "${CURRENT_PACKAGES_DIR}/include/lvgl/src/stdlib/clib"
    "${CURRENT_PACKAGES_DIR}/include/lvgl/src/stdlib/micropython"
    "${CURRENT_PACKAGES_DIR}/include/lvgl/src/stdlib/rtthread"
    "${CURRENT_PACKAGES_DIR}/include/lvgl/src/stdlib/uefi"
)
