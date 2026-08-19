// SPDX-FileCopyrightText: 2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include <lvgl.h>
#else
#include <lvgl/lvgl.h>
#endif

extern "C" const lv_font_t font_montserrat_semi_bold_18;
extern "C" const lv_font_t font_montserrat_semi_bold_22;

namespace LvglTheme
{
    // Magic Lane styled dark palette
    const lv_color_t kPanelBg = lv_color_hex( 0x161B22 );
    const lv_color_t kWidgetBg = lv_color_hex( 0x252C37 );
    const lv_color_t kBorder = lv_color_hex( 0x323B48 );
    const lv_color_t kAccent = lv_color_hex( 0x9A78FC );
    const lv_color_t kText = lv_color_hex( 0xFFFFFF );

    inline void Apply()
    {
        lv_theme_default_init( lv_display_get_default(), kAccent, kAccent, true /*dark*/, LV_FONT_DEFAULT );
    }

    inline void StylePanel( lv_obj_t* panel )
    {
        lv_obj_set_style_bg_color( panel, kPanelBg, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_bg_opa( panel, LV_OPA_90, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_radius( panel, 12, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_border_width( panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_border_color( panel, kBorder, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_pad_all( panel, 12, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_text_color( panel, kText, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_text_font( panel, &font_montserrat_semi_bold_18, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_remove_flag( panel, LV_OBJ_FLAG_SCROLLABLE ); // don't steal map drags
    }

    inline void StyleOverlayPanel( lv_obj_t* panel )
    {
        StylePanel( panel );
        lv_obj_set_style_bg_opa( panel, LV_OPA_60, LV_PART_MAIN | LV_STATE_DEFAULT );
    }

    inline void StyleWidget( lv_obj_t* obj )
    {
        lv_obj_set_style_bg_color( obj, kWidgetBg, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_bg_opa( obj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_bg_grad_dir( obj, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT ); // theme buttons have a light gradient
        lv_obj_set_style_radius( obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_border_width( obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_border_color( obj, kBorder, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_shadow_width( obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_text_color( obj, kText, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_text_font( obj, &font_montserrat_semi_bold_18, LV_PART_MAIN | LV_STATE_DEFAULT );
    }

    inline void StylePrimaryWidget( lv_obj_t* obj )
    {
        StyleWidget( obj );
        lv_obj_set_style_bg_color( obj, kAccent, LV_PART_MAIN | LV_STATE_DEFAULT );
    }

    inline void StyleSlider( lv_obj_t* slider )
    {
        lv_obj_set_style_bg_color( slider, kWidgetBg, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_bg_opa( slider, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_bg_color( slider, kAccent, LV_PART_INDICATOR | LV_STATE_DEFAULT );
        lv_obj_set_style_bg_color( slider, lv_color_white(), LV_PART_KNOB | LV_STATE_DEFAULT );
        lv_obj_set_style_pad_all( slider, 3, LV_PART_KNOB | LV_STATE_DEFAULT );
    }

    inline void StyleTitle( lv_obj_t* label )
    {
        lv_obj_set_style_text_color( label, kText, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_text_font( label, &font_montserrat_semi_bold_22, LV_PART_MAIN | LV_STATE_DEFAULT );
    }
}
