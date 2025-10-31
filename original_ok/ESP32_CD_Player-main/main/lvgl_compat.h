#pragma once
#include "lvgl.h"

/* ---------------- LVGL v9 对齐枚举在 v8 的兼容映射 ---------------- */
#ifndef LV_ALIGN_LEFT_MID
  #define LV_ALIGN_LEFT_MID   LV_ALIGN_LEFT
#endif
#ifndef LV_ALIGN_RIGHT_MID
  #define LV_ALIGN_RIGHT_MID  LV_ALIGN_RIGHT
#endif
#ifndef LV_ALIGN_TOP_MID
  #define LV_ALIGN_TOP_MID    LV_ALIGN_TOP
#endif
#ifndef LV_ALIGN_BOTTOM_MID
  #define LV_ALIGN_BOTTOM_MID LV_ALIGN_BOTTOM
#endif
#ifndef LV_ALIGN_CENTER
  #define LV_ALIGN_CENTER     LV_ALIGN_CENTER   /* v8 已存在；为了防护，保留 */
#endif
#ifndef LV_ALIGN_OFF
  #define LV_ALIGN_OFF        LV_ALIGN_DEFAULT
#endif

/* ---------------- LVGL 符号名兼容（v9→v8 或兜底） ---------------- */
#ifndef LV_SYMBOL_VOLUME_MAX
  #ifdef LV_SYMBOL_VOLUME
    #define LV_SYMBOL_VOLUME_MAX LV_SYMBOL_VOLUME
  #else
    /* 兜底字形（FontAwesome 的喇叭图标）；若字体不含该码位会显示方块 */
    #define LV_SYMBOL_VOLUME_MAX "\xEF\x80\xAA"
  #endif
#endif

/* ---------------- 统一把 lv_label_set_text(...) 映射为 *_fmt ----------------
   你的工程里很多是 lv_label_set_text(obj, "xxx %d", val) 这种“带格式”的写法。
   v8 需要用 lv_label_set_text_fmt 才能传可变参数，这里做一个宏重定向。
*/
#ifndef LVGL_COMPAT_REMAP_LABEL_TEXT
  #define LVGL_COMPAT_REMAP_LABEL_TEXT 1
  #define lv_label_set_text(_obj_, ...) lv_label_set_text_fmt((_obj_), __VA_ARGS__)
#endif
