/*
 *  @file ServiceCalendar.cpp
 *  @author
 *  @date 08.05.20
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#include "ServiceCalendar.hpp"
#include "messages/CalendarMessage.hpp"

const char *ServiceCalendar::serviceName = "ServiceCalendar";

ServiceCalendar::ServiceCalendar() : sys::Service(serviceName, "", 4096 * 2, sys::ServicePriority::Idle)
{
    LOG_INFO("[ServiceCalendar] Initializing");
}

ServiceCalendar::~ServiceCalendar()
{
    LOG_INFO("[ServiceCalendar] Cleaning resources");
}

sys::ReturnCodes ServiceCalendar::InitHandler()
{
    return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceCalendar::DeinitHandler()
{
    return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceCalendar::SwitchPowerModeHandler(const sys::ServicePowerMode mode)
{
    LOG_FATAL("[ServiceCalendar] PowerModeHandler: %s", c_str(mode));
    return sys::ReturnCodes::Success;
}

void ServiceCalendar::TickHandler(uint32_t id)
{}

sys::Message_t ServiceCalendar::DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
{
    std::shared_ptr<sys::ResponseMessage> responseMsg = nullptr;

    auto msgType = static_cast<int>(msgl->messageType);
    LOG_DEBUG("msgType %d", msgType);

    return responseMsg;
}
