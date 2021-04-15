// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <module-gui/gui/widgets/Style.hpp>

namespace style::calculator
{
    inline constexpr auto grid_cells        = 9;
    inline constexpr auto equals            = "app_calculator_equals";
    inline constexpr auto decimal_separator = "app_calculator_decimal_separator";
    inline constexpr auto ellipsis_img      = "calculator_ellipsis_W_M";

    namespace window
    {
        inline constexpr auto math_box_height      = 240;
        inline constexpr auto math_box_offset_top  = style::header::height + 200;
        inline constexpr auto math_box_cell_height = 80;
        inline constexpr auto math_box_cell_width  = style::window::default_body_width / 3;

        inline constexpr auto input_box_offset_top = style::header::height + 20;
        inline constexpr auto input_box_height     = 100;
        inline constexpr auto input_box_width      = 380;
        inline constexpr auto input_box_margin     = 50;

        inline constexpr auto ellipsis_width  = 29;
        inline constexpr auto ellipsis_height = 5;
        //        inline constexpr auto ellipsis_margin = 0;

        namespace ellipsis_hidden
        {
            //            inline constexpr auto input_offset_top = 0;
            //            inline constexpr auto input_margin     = 0;
            inline constexpr auto input_width  = input_box_width;
            inline constexpr auto input_height = input_box_height;
        } // namespace ellipsis_hidden

        namespace ellipsis_visible
        {
            //            inline constexpr auto input_offset_top = ellipsis_hidden::input_offset_top;
            //            inline constexpr auto input_margin     = ellipsis_hidden::input_margin + ellipsis_width;
            inline constexpr auto input_width  = input_box_width - ellipsis_width;
            inline constexpr auto input_height = input_box_height;
        } // namespace ellipsis_visible

        //        inline constexpr auto ellipsis_offset_top = (ellipsis_visible::input_height - ellipsis_height) / 2;
    } // namespace window

    namespace symbols
    {
        namespace codes
        {
            inline constexpr auto plus           = 0x002B;
            inline constexpr auto minus          = 0x002D;
            inline constexpr auto division       = 0x00F7;
            inline constexpr auto multiplication = 0x00D7;
            inline constexpr auto full_stop      = 0x002E;
            inline constexpr auto comma          = 0x002C;
            inline constexpr auto equals         = 0x003D;
            inline constexpr auto zero           = 0x0030;
        } // namespace codes

        namespace strings
        {
            inline constexpr auto plus           = "\u002B";
            inline constexpr auto minus          = "\u002D";
            inline constexpr auto division       = "\u00F7";
            inline constexpr auto multiplication = "\u00D7";
            inline constexpr auto equals         = "\u003D";
            inline constexpr auto full_stop      = "\u002E";
            inline constexpr auto comma          = "\u002C";
            inline constexpr auto asterisk       = "\u002A";
            inline constexpr auto solidus        = "\u002F";
        } // namespace strings
    }     // namespace symbols
} // namespace style::calculator
