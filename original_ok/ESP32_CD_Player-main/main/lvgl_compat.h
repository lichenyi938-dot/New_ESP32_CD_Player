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

/* ===================== 符号兼容（例：音量最大） ===================== */
#ifndef LV_SYMBOL_VOLUME_MAX
#define LV_SYMBOL_VOLUME_MAX  LV_SYMBOL_VOLUME
#endif

/* ===================== 字体名回退（源码层面的替换） =====================
 * 如果工程里没真正编进这些字号，下面把它们映射到已存在的字号，
 * 以减少链接缺符号的概率（仍建议配合 ② 的“弱别名”，更稳）。
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
