// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "FreeRTOSConfig.h"
#include "SystemReturnCodes.hpp"

namespace sys
{

    enum class BusChannels
    {
        System,
        SystemManagerRequests,
        PowerManagerRequests,
        ServiceCellularNotifications,
        Test2CustomBusChannel,
        ServiceDBNotifications,
        ServiceAudioNotifications,
        AppManagerNotifications,
        ServiceFotaNotifications,
        AntennaNotifications,
        ServiceEvtmgrNotifications,
        CalendarNotifications
    };

    enum class ServicePriority
    {
        Idle     = 0, ///< priority: idle (lowest)
        Low      = 1, ///< priority: low
        Normal   = 2, ///< priority: normal
        High     = 3, ///< priority: high
        Realtime = 4, ///< priority: realtime (highest)
    };

    enum class ServicePowerMode
    {
        Active,
        SuspendToRAM,
        SuspendToNVM
    };
} // namespace sys

inline const char *c_str(sys::ReturnCodes code)
{
    switch (code) {
    case sys::ReturnCodes::Success:
        return "Success";
    case sys::ReturnCodes::Failure:
        return "Failure";
    case sys::ReturnCodes::Timeout:
        return "Timeout";
    case sys::ReturnCodes::ServiceDoesntExist:
        return "ServiceDoesntExist";
    case sys::ReturnCodes::Unresolved:
        return "Unresolved";
    }
    return "Undefined";
}

inline const char *c_str(sys::ServicePowerMode code)
{
    switch (code) {
    case sys::ServicePowerMode::Active:
        return "Active";
    case sys::ServicePowerMode::SuspendToRAM:
        return "SuspendToRAM";
    case sys::ServicePowerMode::SuspendToNVM:
        return "SuspendToNVM";
    }
    return "";
}

inline const char *c_str(sys::BusChannels channel)
{
    switch (channel) {
    case sys::BusChannels::System:
        return "System";
    case sys::BusChannels::SystemManagerRequests:
        return "SystemManagerRequests";
    case sys::BusChannels::PowerManagerRequests:
        return "PowerManagerRequests";
    case sys::BusChannels::ServiceCellularNotifications:
        return "ServiceCellularNotifications,";
    case sys::BusChannels::Test2CustomBusChannel:
        return "Test2CustomBusChannel,";
    case sys::BusChannels::ServiceDBNotifications:
        return "ServiceDBNotifications,";
    case sys::BusChannels::ServiceAudioNotifications:
        return "ServiceAudioNotifications";
    case sys::BusChannels::AppManagerNotifications:
        return "AppManagerNotifications,";
    case sys::BusChannels::ServiceFotaNotifications:
        return "ServiceFotaNotifications";
    case sys::BusChannels::AntennaNotifications:
        return "AntennaNotifications";
    case sys::BusChannels::ServiceEvtmgrNotifications:
        return "ServiceEvtmgrNotifications";
    case sys::BusChannels::CalendarNotifications:
        return "CalendarNotifications";
    }
    return "";
}
