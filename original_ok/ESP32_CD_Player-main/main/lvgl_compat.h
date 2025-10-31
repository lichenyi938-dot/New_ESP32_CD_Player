#pragma once
#include "lvgl.h"

/* --------------------------------------------------------------------------
 * LVGL v8 <-> v9 对齐枚举兼容
 * v8 使用 *_MID，v9 使用短名字（LEFT/RIGHT/TOP/BOTTOM/CENTER）
 * 两边都做映射，确保无论项目里写的是哪一套都能编过。
 * --------------------------------------------------------------------------*/
#if defined(LVGL_VERSION_MAJOR) && (LVGL_VERSION_MAJOR >= 9)
/* v9: 兼容老代码里的 *_MID */
#ifndef LV_ALIGN_LEFT_MID
#define LV_ALIGN_LEFT_MID     LV_ALIGN_LEFT
#endif
#ifndef LV_ALIGN_RIGHT_MID
#define LV_ALIGN_RIGHT_MID    LV_ALIGN_RIGHT
#endif
#ifndef LV_ALIGN_TOP_MID
#define LV_ALIGN_TOP_MID      LV_ALIGN_TOP
#endif
#ifndef LV_ALIGN_BOTTOM_MID
#define LV_ALIGN_BOTTOM_MID   LV_ALIGN_BOTTOM
#endif
#ifndef LV_ALIGN_CENTER
#define LV_ALIGN_CENTER       LV_ALIGN_DEFAULT
#endif
#else
/* v8: 兼容新代码里的短名字 */
#ifndef LV_ALIGN_LEFT
#define LV_ALIGN_LEFT         LV_ALIGN_LEFT_MID
#endif
#ifndef LV_ALIGN_RIGHT
#define LV_ALIGN_RIGHT        LV_ALIGN_RIGHT_MID
#endif
#ifndef LV_ALIGN_TOP
#define LV_ALIGN_TOP          LV_ALIGN_TOP_MID
#endif
#ifndef LV_ALIGN_BOTTOM
#define LV_ALIGN_BOTTOM       LV_ALIGN_BOTTOM_MID
#endif
#ifndef LV_ALIGN_DEFAULT
#define LV_ALIGN_DEFAULT      LV_ALIGN_CENTER
#endif
#endif /* version switch */

/* --------------------------------------------------------------------------
 * LVGL 符号兼容：有些版本没有 LV_SYMBOL_VOLUME_MAX
 * --------------------------------------------------------------------------*/
#ifndef LV_SYMBOL_VOLUME_MAX
#define LV_SYMBOL_VOLUME_MAX  LV_SYMBOL_VOLUME
#endif

/* --------------------------------------------------------------------------
 * 文本设置兼容：
 * 把 lv_label_set_text(obj, str) 包装为 lv_label_set_text_fmt(obj, "%s", str)
 * 这样传空串 "" 时不会触发 -Werror=format-zero-length。
 * 需要格式化（含 %）时，照常调用 lv_label_set_text_fmt(...)。
 * --------------------------------------------------------------------------*/
#ifndef LV_COMPAT_WRAP_LABEL_TEXT
#define LV_COMPAT_WRAP_LABEL_TEXT 1
#endif

#if LV_COMPAT_WRAP_LABEL_TEXT
/* 只处理 2 参数版本；带格式的请直接用 lv_label_set_text_fmt(...) */
#undef  lv_label_set_text
#define lv_label_set_text(_obj, _str) lv_label_set_text_fmt((_obj), "%s", (_str))
#endif
