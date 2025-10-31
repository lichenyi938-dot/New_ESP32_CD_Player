# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# 指向 ESP-IDF 的 cmake
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# ---- 强制 LVGL 使用项目里的 lv_conf.h ----
set(LV_CONF_PATH "${CMAKE_CURRENT_LIST_DIR}/components/lvgl/lv_conf.h")

# 让 LVGL 按 “SIMPLE” 规则寻找 lv_conf.h
idf_build_set_property(COMPILE_DEFINITIONS "LV_CONF_INCLUDE_SIMPLE=1" APPEND)
# 明确传入 lv_conf.h 的绝对路径（双保险）
idf_build_set_property(COMPILE_DEFINITIONS "LV_CONF_PATH=\"${LV_CONF_PATH}\"" APPEND)

# 你的工程名
project(CD_Player)
