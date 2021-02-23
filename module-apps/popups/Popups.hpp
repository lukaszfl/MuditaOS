// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <module-gui/gui/SwitchData.hpp>
#include "VolumeWindow.hpp"
#include "BaseAppMessage.hpp"

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

        class PopupRequest : public app::AppMessage
        {
          public:
            PopupRequest(app::manager::actions::ActionId _actionId, app::manager::actions::ActionParamsPtr _data)
                : AppMessage(MessageType::TestPopupMessage), actionId{_actionId}, data{std::move(_data)}
            {}

            app::manager::actions::ActionId getAction() const noexcept
            {
                return actionId;
            }

            app::manager::actions::ActionParamsPtr &getData() noexcept
            {
                return data;
            }

          private:
            app::manager::actions::ActionId actionId;
            app::manager::actions::ActionParamsPtr data;
        };

        namespace window
        {
            inline constexpr auto VolumePopup      = "VolumePopup";
            inline constexpr auto desktop_pin_lock = "PinLockWindow";

        }; // namespace window

        //
    } // namespace popups
} // namespace gui
