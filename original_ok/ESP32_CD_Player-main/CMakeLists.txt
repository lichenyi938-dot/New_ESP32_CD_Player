# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(CD_Player)
# 让 LVGL 在包含 lvgl.h 时使用本地 lv_conf.h（简单包含模式）
idf_build_set_property(COMPILE_DEFINITIONS "LV_CONF_INCLUDE_SIMPLE=1" APPEND)
# 明确指定 lv_conf.h 的路径（强制使用你这份）
idf_build_set_property(
    COMPILE_DEFINITIONS
    "LV_CONF_PATH=\"${CMAKE_CURRENT_LIST_DIR}/components/lvgl/lv_conf.h\""
    APPEND
)
