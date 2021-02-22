// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <module-gui/gui/SwitchData.hpp>
#include "VolumeWindow.hpp"

#include <memory>
#include <string>
#include <vector>

namespace gui
{
    namespace popups
    {
        using PopupId = int;

        enum Popup : PopupId
        {
            Volume,
            Brightness,
            PhoneModes
        };

        class PopupMessage : public sys::DataMessage
        {
          public:
            PopupMessage() : sys::DataMessage(MessageType::TestPopupMessage)
            {}
        };

        namespace window
        {
            inline constexpr auto VolumePopup = "VolumePopup";

        }; // namespace window

        //
    } // namespace popups
} // namespace gui
