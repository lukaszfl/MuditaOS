// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "Popups.hpp"
#include <type_traits>
namespace gui
{
    namespace popup
    {
        struct Disposition
        {
          public:
            /// from lowest to highest, order matters
            enum class Priority
            {
                Low,
                Normal,
                High,
                Highest
            };

            Priority priority = Priority::Normal;

            enum class WindowType
            {
                Normal,
                Popup,
            } windowtype = WindowType::Normal;

            gui::popup::ID id = gui::popup::ID::Invalid;
        };
    }; // namespace popup
} // namespace gui

static_assert(std::is_compound<gui::popup::Disposition>(), "This class is designed to carry trivial compound data");
