// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <string>

#include <Application.hpp>

namespace style
{
    namespace window
    {
        namespace name
        {
            inline constexpr auto event_reminder_window = "EventReminderWindow";
            inline constexpr auto volume_window = "VolumeWindow";
            inline constexpr auto music_volume_window = "MusicVolumeWindow";
            inline constexpr auto call_volume_window = "CallVolumeWindow";
            inline constexpr auto brightness_window = "BrightnessWindow";
            inline constexpr auto home_modes_window = "HomeModesWindow";
        }
    }
    // namespace window
} // namespace style

namespace timer
{
    namespace duration
    {
        constexpr static const int calendar_reminder = 10000;
        constexpr static const int volume_window = 1000;
        inline constexpr auto music_volume_window = 1000;
        inline constexpr auto call_volume_window = 1000;
    }

    namespace name
    {
        inline constexpr auto calendar_reminder = "CalendarReminderTimer";
        inline constexpr auto volume_window = "VolumeWindowTimer";
        inline constexpr auto music_volume_window = "MusicVolumeWindowTimer";
        inline constexpr auto call_volume_window = "CallVolumeWindowTimer";
        inline constexpr auto brightness_window = "BrightnessWindowTimer";
        inline constexpr auto home_modes_window = "HomeModesWindowTimer";
    }

}

namespace app
{
    inline constexpr auto name_popup = "ApplicationPopup";

    class ApplicationPopup : public Application
    {
      public:
        explicit ApplicationPopup(std::string name                    = name_popup,
                                  std::string parent                  = {},
                                  StartInBackground startInBackground = {false});

        auto InitHandler() -> sys::ReturnCodes override;
        auto DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp) -> sys::MessagePointer override;
        auto SwitchPowerModeHandler(sys::ServicePowerMode mode) -> sys::ReturnCodes final;

        void createUserInterface() override;
        void destroyUserInterface() override;
    };

    template <> struct ManifestTraits<ApplicationPopup>
    {
        static auto GetManifest() -> manager::ApplicationManifest
        {
            return {{
                     manager::actions::Launch,
                     manager::actions::ShowReminder,
                     manager::actions::ShowVolume
            }};
        }
    };

} // namespace app
