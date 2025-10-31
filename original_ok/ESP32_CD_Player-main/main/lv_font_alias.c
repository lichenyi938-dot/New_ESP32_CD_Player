// main/lv_font_alias.c
#include "lvgl.h"

/* 说明：
 * - 这些 __attribute__((weak, alias("..."))) 会在“目标字体不存在”时，
 *   让缺失的符号自动指向一个已存在的字号（14 或 24），保证链接通过。
 * - 需要 GNU 工具链（ESP-IDF 默认就是）。
 * - 请确保 lv_font_montserrat_14 至少是可用的；24 若不存在会继续回退到 14。
 */

// 至少要有 14（ESP-IDF 默认一般会有；若没有，请在 sdkconfig 或你的 lv_conf.h 里启用一下）
extern const lv_font_t lv_font_montserrat_14;

// 如果 24 没有，就让它落回 14
#ifdef LV_FONT_MONTSERRAT_24
extern const lv_font_t lv_font_montserrat_24;
#else
#define lv_font_montserrat_24 lv_font_montserrat_14
#endif

// 给 12/16/26/28 做弱别名回退
const lv_font_t lv_font_montserrat_12 __attribute__((weak, alias("lv_font_montserrat_14")));
const lv_font_t lv_font_montserrat_16 __attribute__((weak, alias("lv_font_montserrat_14")));
const lv_font_t lv_font_montserrat_26 __attribute__((weak, alias("lv_font_montserrat_24")));
const lv_font_t lv_font_montserrat_28 __attribute__((weak, alias("lv_font_montserrat_24")));
