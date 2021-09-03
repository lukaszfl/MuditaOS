// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "AlarmHandlerActions.hpp"

#include <BellAlarmHandler.hpp>
#include <application-bell-main/ApplicationBellMain.hpp>
#include <application-bell-main/data/AlarmRingingSwitchData.hpp>
#include <apps-common/windows/AppWindow.hpp>
#include <service-appmgr/include/service-appmgr/Constants.hpp>
#include <service-appmgr/messages/SwitchRequest.hpp>
#include <service-time/ServiceTime.hpp>
#include <service-evtmgr/Constants.hpp>
#include <service-evtmgr/EventManagerServiceAPI.hpp>
#include <service-evtmgr/ScreenLightControlMessage.hpp>

namespace alarms
{
    auto playAlarmSound(const std::string &soundPath) -> bool
    {
        // playAlarmSound after it will be implemented [BH-660]
        return true;
    }

    auto displayAlarmPopup(stm::ServiceTime *serviceTime) -> bool
    {
        auto msg = std::make_shared<app::manager::SwitchRequest>(BellAlarmClockHandler::name,
                                                                 app::applicationBellName,
                                                                 gui::name::window::main_window,
                                                                 std::make_unique<AlarmRingingSwitchData>());
        serviceTime->bus.sendUnicast(std::move(msg), service::name::appmgr);
        return true;
    }

    auto turnOnFrontlight(stm::ServiceTime *serviceTime) -> bool
    {
        // turnOnFrontlight after it will be implemented [BH-756]
        serviceTime->bus.sendUnicast(std::make_shared<sevm::ScreenLightControlMessage>(screen_light_control::Action::turnOn),
                        service::name::evt_manager);
        return true;
    }
} // namespace alarms
