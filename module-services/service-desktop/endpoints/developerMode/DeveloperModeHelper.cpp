// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "DeveloperModeEndpoint.hpp"
#include "DeveloperModeHelper.hpp"
#include "SystemManager/SystemManager.hpp"
#include "DesktopMessages.hpp"
#include "bsp/watchdog/watchdog.hpp"

using namespace parserFSM;

auto DeveloperModeHelper::processGetRequest(Context &context) -> sys::ReturnCodes
{
    sys::ReturnCodes returnCode = sys::ReturnCodes::Failure;
    context.setResponseBody(json11::Json::object( {{json::status, std::to_string(static_cast<int>(sys::ReturnCodes::Failure))}}));

    if (context.getBody()[parserFSM::json::filesystem::command].string_value() == "reboot"){
        LOG_DEBUG("processGetRequest reboot");
        sys::SystemManager::Reboot(ownerServicePtr);
    }
    if (context.getBody()[parserFSM::json::filesystem::command].string_value() == "reset"){
        LOG_DEBUG("processGetRequest reset");
        bsp::watchdog::system_reset();
    }

    if (context.getBody()[parserFSM::json::filesystem::command].string_value() == "logs"){
        LOG_DEBUG("processGetRequest logs");
        std::unique_ptr<char[]> buf(new char[4096]);
        //log_copyBuffer(buf.get(), 4096);

        context.setResponseBody(json11::Json::object(
            {{json::status, std::to_string(static_cast<int>(sys::ReturnCodes::Success)) },
             {"logs", buf.get()}}
        ));
    }

    return returnCode;
}
