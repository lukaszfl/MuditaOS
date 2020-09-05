/*
 *  @file ServiceTime.cpp
 *  @author
 *  @date 08.05.20
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#include "ServiceTime.hpp"
#include "messages/TimeMessage.hpp"

TimeEvents::TimeEvents()
{}

TimeEvents::~TimeEvents()
{}

void TimeEvents::StartTimer()
{
    if (timerActive)
        return;

    // timerId = CreateTimer(1000, false, "EventsTimer");
}

sys::Message_t TimeEvents::OnDataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
{
    return nullptr;
}

void TimeEvents::OnTickHandler(uint32_t id)
{}

bool TimeEvents::Reload()
{
    return false;
}

//************************************************************************************************

CalendarTimeEvents::CalendarTimeEvents()
{}

CalendarTimeEvents::~CalendarTimeEvents()
{}

void CalendarTimeEvents::DeleteTimer()
{}

void CalendarTimeEvents::FireReminder()
{}

sys::Message_t CalendarTimeEvents::OnDataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
{
    TimeEvents::OnDataReceivedHandler(msgl, resp);

    // Notifications:
    // - DateTime changed
    // - Timezone changed
    // - PurePhone ON or service started
    // - Midnight day switch

    // - DB add
    // - DB del
    // - DB change
    // - TickHandler

    return nullptr;
}

void CalendarTimeEvents::OnTickHandler(uint32_t id)
{
    TimeEvents::OnTickHandler(id);

    if (id != timerId) {
        return;
    }

    DeleteTimer();

    FireReminder();

    // Sprawdzenie, czy id to identyfikator naszego timera
    // Jeśli nie to return
}

bool CalendarTimeEvents::Reload()
{
    return false;
}

//************************************************************************************************
//************************************************************************************************
//************************************************************************************************

const char *ServiceTime::serviceName = "ServiceTime";

ServiceTime::ServiceTime() : sys::Service(serviceName, "", 4096 * 2, sys::ServicePriority::Idle)
{
    LOG_INFO("[ServiceTime] Initializing");
}

ServiceTime::~ServiceTime()
{
    LOG_INFO("[ServiceTime] Cleaning resources");
}

sys::ReturnCodes ServiceTime::InitHandler()
{
    return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceTime::DeinitHandler()
{
    return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceTime::SwitchPowerModeHandler(const sys::ServicePowerMode mode)
{
    LOG_FATAL("[ServiceTime] PowerModeHandler: %s", c_str(mode));
    return sys::ReturnCodes::Success;
}

sys::Message_t ServiceTime::DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
{
    std::shared_ptr<sys::ResponseMessage> responseMsg = nullptr;

    auto msgType = static_cast<int>(msgl->messageType);
    LOG_DEBUG("msgType %d", msgType);

    calendarEvents.OnDataReceivedHandler(msgl, resp);

    return responseMsg;
}

void ServiceTime::TickHandler(uint32_t id)
{

    calendarEvents.OnTickHandler(id);
}
