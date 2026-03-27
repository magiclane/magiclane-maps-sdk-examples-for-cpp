/**
 * LVGL 9.5 configuration for Maps SDK examples.
 *
 * Only the settings that differ from LVGL defaults are listed here;
 * everything else keeps the library default (see lv_conf_internal.h).
 */

#ifndef LV_CONF_H
#define LV_CONF_H

/* Guard against inclusion from assembly (.S) files — stdint.h contains
   C typedefs that the assembler cannot parse. */
#ifndef __ASSEMBLER__
#include <stdint.h>
#endif

/*====================
   COLOR SETTINGS
 *====================*/

/* 32-bit color depth. */
#define LV_COLOR_DEPTH 32

/*====================
   STDLIB SETTINGS
 *====================*/

/* Use C standard library for malloc/free, string and sprintf. */
#define LV_USE_STDLIB_MALLOC  LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING  LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

/*====================
   HAL SETTINGS
 *====================*/

/* Default display refresh period [ms]. */
#define LV_DEF_REFR_PERIOD 10

/* Input device read period [ms]. */
#define LV_DEF_INDEV_READ_PERIOD 10


/*====================
   FONT SETTINGS
 *====================*/

/* Required by the custom font_montserrat_semi_bold_*.c files.
   Must match the value used when compiling the font data. */
#define LV_FONT_FMT_TXT_LARGE 1

/*====================
   WIDGETS
 *====================*/

/* All widget types used by the examples are enabled by default
   in LVGL 9.5 (LV_USE_BUTTON, LV_USE_LABEL, LV_USE_IMAGE = 1).
   No overrides needed. */

/*====================
   LAYOUTS
 *====================*/

/* Flex layout is enabled by default (LV_USE_FLEX = 1).
   No override needed. */

#endif /* LV_CONF_H */
