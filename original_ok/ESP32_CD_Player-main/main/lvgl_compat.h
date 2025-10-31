#pragma once
#include "lvgl.h"

/* ---------- Align aliases (v8 <-> v9) ---------- */
/* 如果只有 _MID 版本，则补一个无 _MID 的别名；反之亦然。*/

#if !defined(LV_ALIGN_LEFT) && defined(LV_ALIGN_LEFT_MID)
#  define LV_ALIGN_LEFT LV_ALIGN_LEFT_MID
#endif
#if !defined(LV_ALIGN_RIGHT) && defined(LV_ALIGN_RIGHT_MID)
#  define LV_ALIGN_RIGHT LV_ALIGN_RIGHT_MID
#endif
#if !defined(LV_ALIGN_TOP) && defined(LV_ALIGN_TOP_MID)
#  define LV_ALIGN_TOP LV_ALIGN_TOP_MID
#endif
#if !defined(LV_ALIGN_BOTTOM) && defined(LV_ALIGN_BOTTOM_MID)
#  define LV_ALIGN_BOTTOM LV_ALIGN_BOTTOM_MID
#endif
#if !defined(LV_ALIGN_CENTER) && defined(LV_ALIGN_CENTER) /* 占位，保持对称性 */
#endif

#if !defined(LV_ALIGN_LEFT_MID) && defined(LV_ALIGN_LEFT)
#  define LV_ALIGN_LEFT_MID LV_ALIGN_LEFT
#endif
#if !defined(LV_ALIGN_RIGHT_MID) && defined(LV_ALIGN_RIGHT)
#  define LV_ALIGN_RIGHT_MID LV_ALIGN_RIGHT
#endif
#if !defined(LV_ALIGN_TOP_MID) && defined(LV_ALIGN_TOP)
#  define LV_ALIGN_TOP_MID LV_ALIGN_TOP
#endif
#if !defined(LV_ALIGN_BOTTOM_MID) && defined(LV_ALIGN_BOTTOM)
#  define LV_ALIGN_BOTTOM_MID LV_ALIGN_BOTTOM
#endif

/* ---------- Label text compatibility ----------
 * 允许把老代码里没有 % 的 lv_label_set_text(...) 透明地走到 _fmt 版本。
 * 如果 fmt 字符串不含 %，这在 v8/v9 都是安全的。
 */
#ifndef LVGL_COMPAT_REMAP_LABEL_TEXT
#  define LVGL_COMPAT_REMAP_LABEL_TEXT 1
#  define lv_label_set_text(_obj, ...) lv_label_set_text_fmt((_obj), __VA_ARGS__)
#endif

/* ---------- Symbols fallback ---------- */
#ifndef LV_SYMBOL_VOLUME_MAX
/* 一个通用的“音量”图标（FontAwesome/内置符号缺失时的 fallback） */
#  define LV_SYMBOL_VOLUME_MAX  "\xEF\x8B\xA8"
#endif
