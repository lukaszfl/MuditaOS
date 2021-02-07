// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <module-apps/application-popup/windows/EventReminderWindow.hpp>
#include <module-apps/application-popup/windows/VolumeWindow.hpp>
#include <module-apps/application-popup/windows/HomeModesWindow.hpp>
#include <module-apps/application-popup/windows/BrightnessWindow.hpp>
#include "ApplicationPopup.hpp"

namespace app
{

    ApplicationPopup::ApplicationPopup(std::string name, std::string parent, StartInBackground startInBackground)
        : Application{name, parent, startInBackground}
    {
        addActionReceiver(manager::actions::ShowReminder, [this](auto &&data) {
            switchWindow(style::window::name::event_reminder_window, std::move(data));
            return msgHandled();
        });

        addActionReceiver(manager::actions::ShowVolume, [this](auto &&data) {
            switchWindow(style::window::name::volume_window, std::move(data));
            return msgHandled();
        });
    }

    auto ApplicationPopup::InitHandler() -> sys::ReturnCodes
    {
        const auto ret = Application::InitHandler();
        if (ret != sys::ReturnCodes::Success)
            return ret;

        createUserInterface();

        setActiveWindow(style::window::name::volume_window);
        return ret;
    }

    auto ApplicationPopup::DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
        -> sys::MessagePointer
    {
        return Application::DataReceivedHandler(msgl);
    }

    auto ApplicationPopup::SwitchPowerModeHandler(sys::ServicePowerMode mode) -> sys::ReturnCodes
    {
        return sys::ReturnCodes::Success;
    }

    void ApplicationPopup::createUserInterface()
    {
        windowsFactory.attach(
            style::window::name::event_reminder_window, [](Application *app, const std::string &name) {
                return std::make_unique<gui::EventReminderWindow>(app, style::window::name::event_reminder_window);
            });
        windowsFactory.attach(style::window::name::volume_window, [](Application *app, const std::string &name) {
            return std::make_unique<gui::VolumeWindow>(app, style::window::name::volume_window);
        });
        windowsFactory.attach(style::window::name::brightness_window, [](Application *app, const std::string &name) {
            return std::make_unique<gui::BrightnessWindow>(app, style::window::name::brightness_window);
        });
        windowsFactory.attach(style::window::name::home_modes_window, [](Application *app, const std::string &name) {
            return std::make_unique<gui::HomeModesWindow>(app, style::window::name::home_modes_window);
        });
    }

    void ApplicationPopup::destroyUserInterface()
    {}
} // namespace app
