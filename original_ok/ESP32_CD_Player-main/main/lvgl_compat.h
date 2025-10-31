#pragma once
#include "lvgl.h"

/* lvgl_compat.h — minimal compatibility helpers for this project.
 * Do NOT redefine LV_ALIGN_* macros. v8/v9 already have the *_MID enums.
 * We only remap lv_label_set_text to the printf-like version so that
 * calls with % (format strings) continue to work.
 */

#ifndef LV_COMPAT_REMAP_LABEL_TEXT
#define LV_COMPAT_REMAP_LABEL_TEXT 1
#endif

#if LV_COMPAT_REMAP_LABEL_TEXT
#undef  lv_label_set_text
#define lv_label_set_text(obj, ...) lv_label_set_text_fmt((obj), __VA_ARGS__)
#endif
