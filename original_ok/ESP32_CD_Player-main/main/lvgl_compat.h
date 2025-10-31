#pragma once
#include "lvgl.h"

/* ---------------- LVGL v8/v9 align compatibility ---------------- */
#ifndef LV_ALIGN_LEFT_MID
#  define LV_ALIGN_LEFT_MID   LV_ALIGN_LEFT
#endif
#ifndef LV_ALIGN_RIGHT_MID
#  define LV_ALIGN_RIGHT_MID  LV_ALIGN_RIGHT
#endif
#ifndef LV_ALIGN_TOP_MID
#  define LV_ALIGN_TOP_MID    LV_ALIGN_TOP
#endif
#ifndef LV_ALIGN_BOTTOM_MID
#  define LV_ALIGN_BOTTOM_MID LV_ALIGN_BOTTOM
#endif
#ifndef LV_ALIGN_CENTER
#  define LV_ALIGN_CENTER     LV_ALIGN_CENTER  /* v8 already has it */
#endif
#ifndef LV_ALIGN_DEFAULT
#  define LV_ALIGN_DEFAULT    LV_ALIGN_CENTER
#endif

/* ---------------- LVGL symbols compatibility ---------------- */
#ifndef LV_SYMBOL_VOLUME_MAX
#  ifdef LV_SYMBOL_VOLUME
#    define LV_SYMBOL_VOLUME_MAX  LV_SYMBOL_VOLUME
#  else
#    define LV_SYMBOL_VOLUME_MAX  "\xEF\x80\xA8" /* fallback glyph */
#  endif
#endif

/* ---------------- Most important part ----------------
 * Redirect ALL lv_label_set_text(...) calls to the printf-like
 * lv_label_set_text_fmt(...). This works both with and without %.
 * So you don't need to touch call sites.
 */
#if !defined(LVGL_COMPAT_REMAP_LABEL_TEXT)
#  define LVGL_COMPAT_REMAP_LABEL_TEXT 1
#  ifdef lv_label_set_text
#    undef lv_label_set_text
#  endif
#  define lv_label_set_text(obj, ...) lv_label_set_text_fmt((obj), __VA_ARGS__)
#endif
