# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(CD_Player)

# —— 关键：让 LVGL 使用本地 components/lvgl/lv_conf.h ——
set(LV_CONF_PATH "${CMAKE_CURRENT_LIST_DIR}/components/lvgl/lv_conf.h")
idf_build_set_property(
  COMPILE_DEFINITIONS
  "LV_CONF_INCLUDE_SIMPLE=1"
  "LV_CONF_PATH=\"${LV_CONF_PATH}\""
  APPEND
)
