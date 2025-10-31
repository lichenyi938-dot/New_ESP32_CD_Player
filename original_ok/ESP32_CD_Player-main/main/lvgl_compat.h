#pragma once
#include "lvgl.h"

/* ---------- Align compatibility (v8 names that don't exist in v9) ---------- */
#ifndef LV_ALIGN_LEFT_MID
#define LV_ALIGN_LEFT_MID    LV_ALIGN_LEFT
#endif
#ifndef LV_ALIGN_RIGHT_MID
#define LV_ALIGN_RIGHT_MID   LV_ALIGN_RIGHT
#endif
#ifndef LV_ALIGN_TOP_MID
#define LV_ALIGN_TOP_MID     LV_ALIGN_TOP
#endif
#ifndef LV_ALIGN_BOTTOM_MID
#define LV_ALIGN_BOTTOM_MID  LV_ALIGN_BOTTOM
#endif
#ifndef LV_ALIGN_CENTER
/* v8 里也叫 CENTER，这里只是兜底 */
#define LV_ALIGN_CENTER      LV_ALIGN_CENTER
#endif
#ifndef LV_ALIGN_DEFAULT
#define LV_ALIGN_DEFAULT     LV_ALIGN_CENTER
#endif

/* ---------- Symbols compatibility ---------- */
#ifndef LV_SYMBOL_VOLUME_MAX
#define LV_SYMBOL_VOLUME_MAX LV_SYMBOL_VOLUME
#endif

/* ---------- Redirect old lv_label_set_text(...) to fmt variant ---------- */
/* 这条只做“把 text 当作格式串”的重定向；配合步骤A避免传入 "" */
#ifndef LVGL_COMPAT_REMAP_LABEL_TEXT
#define LVGL_COMPAT_REMAP_LABEL_TEXT 1
#endif
#if LVGL_COMPAT_REMAP_LABEL_TEXT
#undef  lv_label_set_text
#define lv_label_set_text(_obj, ...) lv_label_set_text_fmt((_obj), __VA_ARGS__)
#endif
