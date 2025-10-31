#pragma once
#include "lvgl.h"

/* ---------- LVGL v8/v9 对齐常量兼容 ---------- */
/* 现在项目实际在用 v8，所以把 v9 的名字映射回 v8 的枚举 */
#if LVGL_VERSION_MAJOR == 8

/* v9: LV_ALIGN_LEFT  → v8: LV_ALIGN_LEFT_MID */
#ifndef LV_ALIGN_LEFT
#define LV_ALIGN_LEFT        LV_ALIGN_LEFT_MID
#endif

/* v9: LV_ALIGN_RIGHT → v8: LV_ALIGN_RIGHT_MID */
#ifndef LV_ALIGN_RIGHT
#define LV_ALIGN_RIGHT       LV_ALIGN_RIGHT_MID
#endif

/* v9: LV_ALIGN_TOP   → v8: LV_ALIGN_TOP_MID */
#ifndef LV_ALIGN_TOP
#define LV_ALIGN_TOP         LV_ALIGN_TOP_MID
#endif

/* v9: LV_ALIGN_BOTTOM → v8: LV_ALIGN_BOTTOM_MID */
#ifndef LV_ALIGN_BOTTOM
#define LV_ALIGN_BOTTOM      LV_ALIGN_BOTTOM_MID
#endif

/* v9: LV_ALIGN_CENTER 与 v8 同名，这里只兜底 */
#ifndef LV_ALIGN_CENTER
#define LV_ALIGN_CENTER      LV_ALIGN_CENTER
#endif

/* 一些代码会用到 “默认对齐”，在 v8 里用 CENTER 兜底 */
#ifndef LV_ALIGN_DEFAULT
#define LV_ALIGN_DEFAULT     LV_ALIGN_CENTER
#endif

#endif /* LVGL_VERSION_MAJOR == 8 */

/* ---------- label 文本 printf 兼容 ---------- */
/* 让旧代码里的 lv_label_set_text(...) 自动走到 lv_label_set_text_fmt(...) */
#ifndef LV_COMPAT_REMAP_LABEL_TEXT
#define LV_COMPAT_REMAP_LABEL_TEXT 1
#endif

#if LV_COMPAT_REMAP_LABEL_TEXT
#undef  lv_label_set_text
#define lv_label_set_text(_obj, ...) lv_label_set_text_fmt((_obj), __VA_ARGS__)
#endif
