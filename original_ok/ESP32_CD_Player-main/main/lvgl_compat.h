#pragma once
#include "lvgl.h"

/* ---------- Align compatibility: map both ways ---------- */
/* 如果只有不带 _MID 的旧名，就映射出 *_MID；如果只有 *_MID，就映射出不带 _MID 的旧名。 */

#ifndef LV_ALIGN_LEFT_MID
# ifdef LV_ALIGN_LEFT
#  define LV_ALIGN_LEFT_MID LV_ALIGN_LEFT
# endif
#endif

#ifndef LV_ALIGN_RIGHT_MID
# ifdef LV_ALIGN_RIGHT
#  define LV_ALIGN_RIGHT_MID LV_ALIGN_RIGHT
# endif
#endif

#ifndef LV_ALIGN_TOP_MID
# ifdef LV_ALIGN_TOP
#  define LV_ALIGN_TOP_MID LV_ALIGN_TOP
# endif
#endif

#ifndef LV_ALIGN_BOTTOM_MID
# ifdef LV_ALIGN_BOTTOM
#  define LV_ALIGN_BOTTOM_MID LV_ALIGN_BOTTOM
# endif
#endif

#ifndef LV_ALIGN_CENTER
# ifdef LV_ALIGN_CENTER_MID
#  define LV_ALIGN_CENTER LV_ALIGN_CENTER_MID
# endif
#endif

/* 反向：如果只存在 *_MID，则补齐不带 _MID 的名字，避免宏展开到不存在的标识符 */
#ifndef LV_ALIGN_LEFT
# ifdef LV_ALIGN_LEFT_MID
#  define LV_ALIGN_LEFT LV_ALIGN_LEFT_MID
# endif
#endif

#ifndef LV_ALIGN_RIGHT
# ifdef LV_ALIGN_RIGHT_MID
#  define LV_ALIGN_RIGHT LV_ALIGN_RIGHT_MID
# endif
#endif

#ifndef LV_ALIGN_TOP
# ifdef LV_ALIGN_TOP_MID
#  define LV_ALIGN_TOP LV_ALIGN_TOP_MID
# endif
#endif

#ifndef LV_ALIGN_BOTTOM
# ifdef LV_ALIGN_BOTTOM_MID
#  define LV_ALIGN_BOTTOM LV_ALIGN_BOTTOM_MID
# endif
#endif

#ifndef LV_ALIGN_CENTER_MID
# ifdef LV_ALIGN_CENTER
#  define LV_ALIGN_CENTER_MID LV_ALIGN_CENTER
# endif
#endif

/* ---------- Label set_text compatibility ---------- */
/* 允许把没有 % 的调用通过重定向走 fmt 版本；有 % 的本来就用 lv_label_set_text_fmt */
#ifndef LVGL_COMPAT_REMAP_LABEL_TEXT
#define LVGL_COMPAT_REMAP_LABEL_TEXT 1
# if LVGL_COMPAT_REMAP_LABEL_TEXT
#  undef lv_label_set_text
#  define lv_label_set_text(_obj, ...) lv_label_set_text_fmt((_obj), __VA_ARGS__)
# endif
#endif

/* ---------- Volume symbol fallback ---------- */
#ifndef LV_SYMBOL_VOLUME_MAX
# define LV_SYMBOL_VOLUME_MAX "\xEF\x80\xA8" /* 随便给个可见占位符，必要时改成你想要的符号 */
#endif
