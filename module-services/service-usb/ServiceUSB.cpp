// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "Constants.hpp"
#include "service-usb/ServiceUSB.hpp"

#include <module-sys/Service/Bus.hpp>

#include <Service/Service.hpp>
#include <Service/Message.hpp>

#include <log/log.hpp>

ServiceUSB::ServiceUSB() : sys::Service(service::name::service_usb)
{
    LOG_INFO("[ServiceUSB] Initializing");
}

ServiceUSB::~ServiceUSB()
{
    LOG_INFO("[ServiceUSB] Cleaning resources");
}

sys::ReturnCodes ServiceUSB::InitHandler()
{
    LOG_ERROR("ServiceUSB experimental!");

    return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceUSB::DeinitHandler()
{

    return sys::ReturnCodes::Success;
}

sys::MessagePointer ServiceUSB::DataReceivedHandler(sys::DataMessage *msg, sys::ResponseMessage *resp)
{

    return std::make_shared<sys::ResponseMessage>();
}

sys::ReturnCodes ServiceUSB::SwitchPowerModeHandler(const sys::ServicePowerMode mode)
{
    LOG_ERROR("TODO ServiceUSB");
    return sys::ReturnCodes::Success;
}
