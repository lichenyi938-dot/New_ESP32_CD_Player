#pragma once
#include "lvgl.h"

/* ---- LVGL v8 <-> v9 对齐枚举兼容 ----
 * v8 没有 LV_ALIGN_LEFT/RIGHT/TOP/BOTTOM 这些“短名字”，
 * 只有 *_MID。这里把“短名字”映射回 v8 的 *_MID。
 */
#if !defined(LVGL_VERSION_MAJOR) || (LVGL_VERSION_MAJOR == 8)

/* plain names -> v8 names */
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

/* 某些项目里会直接用到这些（已经是 v8 名称，保持不动）
 * LV_ALIGN_LEFT_MID / RIGHT_MID / TOP_MID / BOTTOM_MID 本来就存在于 v8
 */

#endif /* v8 compatibility */

/* ---- 兼容 printf 风格文本设置 ----
 * 统一把 lv_label_set_text(...) 重定向到 lv_label_set_text_fmt(...)
 * 这样带格式串（含 %）和普通字符串都能过编译。
 */
#ifndef LV_COMPAT_WRAP_LABEL_TEXT
#define LV_COMPAT_WRAP_LABEL_TEXT 1
#endif

#if LV_COMPAT_WRAP_LABEL_TEXT
#undef  lv_label_set_text
#define lv_label_set_text(_obj, ...) lv_label_set_text_fmt((_obj), __VA_ARGS__)
#endif
