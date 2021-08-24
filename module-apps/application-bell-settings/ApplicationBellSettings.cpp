// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ApplicationBellSettings.hpp"
#include "TimeUnitsPresenter.hpp"
#include "FrontlightPresenter.hpp"
#include "windows/BellSettingsAdvancedWindow.hpp"
#include "windows/BellSettingsFinishedWindow.hpp"
#include "windows/BellSettingsTimeUnitsWindow.hpp"
#include "windows/BellSettingsWindow.hpp"
#include "windows/BellSettingsFrontlight.hpp"

#include <apps-common/windows/Dialog.hpp>
#include <apps-common/AsyncTask.hpp>
#include <service-evtmgr/Constants.hpp>
#include <service-evtmgr/EventManagerServiceAPI.hpp>
#include <service-evtmgr/ScreenLightControlMessage.hpp>

namespace app
{
    static constexpr auto settingStackDepth = 1024 * 6; // 6Kb stack size

    ApplicationBellSettings::ApplicationBellSettings(std::string name,
                                                     std::string parent,
                                                     sys::phone_modes::PhoneMode mode,
                                                     StartInBackground startInBackground)
        : Application(std::move(name), std::move(parent), mode, startInBackground, settingStackDepth)
    {}

    sys::ReturnCodes ApplicationBellSettings::InitHandler()
    {
        auto ret = Application::InitHandler();
        if (ret != sys::ReturnCodes::Success) {
            return ret;
        }
        createUserInterface();

        return sys::ReturnCodes::Success;
    }

    void ApplicationBellSettings::createUserInterface()
    {
        windowsFactory.attach(gui::name::window::main_window, [](Application *app, const std::string &name) {
            return std::make_unique<gui::BellSettingsWindow>(app);
        });

        windowsFactory.attach(gui::window::name::bellSettingsAdvanced, [](Application *app, const std::string &name) {
            return std::make_unique<gui::BellSettingsAdvancedWindow>(app);
        });

        windowsFactory.attach(gui::window::name::bellSettingsTimeUnits, [](Application *app, const std::string &name) {
            auto temperatureUnitModel = std::make_unique<bell_settings::TemperatureUnitModel>(app);
            auto timeUnitsProvider    = std::make_shared<bell_settings::TimeUnitsModel>(app);
            auto presenter            = std::make_unique<bell_settings::TimeUnitsWindowPresenter>(timeUnitsProvider,
                                                                                       std::move(temperatureUnitModel));
            return std::make_unique<gui::BellSettingsTimeUnitsWindow>(app, std::move(presenter));
        });

        windowsFactory.attach(gui::window::name::bellSettingsFrontlight, [](Application *app, const std::string &name) {
            auto FrontlightProvider = std::make_shared<bell_settings::FrontlightModel>(app, static_cast<ApplicationBellSettings *>(app));
            auto presenter          = std::make_unique<bell_settings::FrontlightWindowPresenter>(FrontlightProvider);
            return std::make_unique<gui::BellSettingsFrontlightWindow>(app, std::move(presenter));
        });

        windowsFactory.attach(gui::window::name::bellSettingsFinished, [](Application *app, const std::string &name) {
            return std::make_unique<gui::BellSettingsFinishedWindow>(app);
        });
    }

    sys::MessagePointer ApplicationBellSettings::DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
    {
        auto retMsg = Application::DataReceivedHandler(msgl);
        if (dynamic_cast<sys::ResponseMessage *>(retMsg.get())->retCode == sys::ReturnCodes::Success) {
            return retMsg;
        }

        // handle database response
        if (resp != nullptr) {
            if (auto command = callbackStorage->getCallback(resp); command->execute()) {
                refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
            }
            return sys::msgHandled();
        }
        return std::make_shared<sys::ResponseMessage>();
    }

    auto ApplicationBellSettings::getCurrentValues() -> settingsInterface::BellScreenLightSettings::Values
    {
        constexpr int timeout = pdMS_TO_TICKS(1500);

        auto response = bus.sendUnicastSync(
            std::make_shared<sevm::ScreenLightControlRequestParameters>(), service::name::evt_manager, timeout);

        if (response.first == sys::ReturnCodes::Success) {
            auto msgState = dynamic_cast<sevm::ScreenLightControlParametersResponse *>(response.second.get());
            if (msgState == nullptr) {
                return {};
            }

            return {msgState->isLightOn(), msgState->getMode(), msgState->getParams()};
        }

        return {};
    }

    void ApplicationBellSettings::setBrightness(bsp::eink_frontlight::BrightnessPercentage value)
    {
        screen_light_control::ManualModeParameters parameters{value};
        bus.sendUnicast(std::make_shared<sevm::ScreenLightSetManualModeParams>(parameters), service::name::evt_manager);
    }

    void ApplicationBellSettings::setMode(bool isAutoLightSwitchOn)
    {
        bus.sendUnicast(std::make_shared<sevm::ScreenLightControlMessage>(
                            isAutoLightSwitchOn ? screen_light_control::Action::enableAutomaticMode
                                                : screen_light_control::Action::disableAutomaticMode),
                        service::name::evt_manager);
    }

    void ApplicationBellSettings::setStatus(bool isDisplayLightSwitchOn)
    {
        bus.sendUnicast(std::make_shared<sevm::ScreenLightControlMessage>(isDisplayLightSwitchOn
                                                                              ? screen_light_control::Action::turnOn
                                                                              : screen_light_control::Action::turnOff),
                        service::name::evt_manager);
    }
} // namespace app
