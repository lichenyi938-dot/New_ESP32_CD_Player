#pragma once
#include "lvgl.h"

/* ===================== LVGL v8 <-> v9 对齐枚举兼容 ===================== */
#if !defined(LVGL_VERSION_MAJOR) || (LVGL_VERSION_MAJOR == 8)

#ifndef LV_ALIGN_LEFT
#define LV_ALIGN_LEFT            LV_ALIGN_LEFT_MID
#endif
#ifndef LV_ALIGN_RIGHT
#define LV_ALIGN_RIGHT           LV_ALIGN_RIGHT_MID
#endif
#ifndef LV_ALIGN_TOP
#define LV_ALIGN_TOP             LV_ALIGN_TOP_MID
#endif
#ifndef LV_ALIGN_BOTTOM
#define LV_ALIGN_BOTTOM          LV_ALIGN_BOTTOM_MID
#endif
#ifndef LV_ALIGN_DEFAULT
#define LV_ALIGN_DEFAULT         LV_ALIGN_CENTER
#endif

#endif /* v8 compatibility */

/* ===================== 符号兼容（示例：音量最大） ===================== */
#ifndef LV_SYMBOL_VOLUME_MAX
#define LV_SYMBOL_VOLUME_MAX  LV_SYMBOL_VOLUME
#endif

/* ===================== 字体回退（避免链接阶段的未定义） =====================
 * 某些工程用的是 ESP-IDF 自带的 lvgl 组件，启用哪些字体由 Kconfig 决定。
 * 如果某个字号未启用，这里把它映射到 14 号（通常默认启用），保证链接不报错。
 * 说明：
 *   这些宏只做“同名字号之间”的别名替换，因此你代码里写的 &lv_font_montserrat_XX
 *   会被预处理替换成 &lv_font_montserrat_14 / 24 等，类型也完全匹配。
 */
#if !(LV_FONT_MONTSERRAT_12+0)
#define lv_font_montserrat_12 lv_font_montserrat_14
#endif

#if !(LV_FONT_MONTSERRAT_16+0)
#define lv_font_montserrat_16 lv_font_montserrat_14
#endif

#if !(LV_FONT_MONTSERRAT_24+0)
#define lv_font_montserrat_24 lv_font_montserrat_14
#endif

#if !(LV_FONT_MONTSERRAT_26+0)
#define lv_font_montserrat_26 lv_font_montserrat_24
#endif

#if !(LV_FONT_MONTSERRAT_28+0)
#define lv_font_montserrat_28 lv_font_montserrat_24
#endif

/* 重要：不要在这里把 lv_label_set_text 重定向到 lv_label_set_text_fmt。
 * 代码里需要格式化（带 %）的地方，保持用 lv_label_set_text_fmt；
 * 普通字符串（包括 "" 空串）继续用 lv_label_set_text，避免空格式串告警。*/
