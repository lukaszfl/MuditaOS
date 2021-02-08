// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "service-desktop/DesktopMessages.hpp"
#include "parser/MessageHandler.hpp"

namespace sdesktop::developerMode
{
    using namespace parserFSM;
    void Event::send()
    {
        MessageHandler::putToSendQueue(context.createSimpleResponse());
    }

    ATResponseEvent::ATResponseEvent(std::vector<std::string> resp)
    {
        context.setResponseStatus(http::Code::OK);
        context.setEndpoint(EndpointType::developerMode);
        context.setResponseBody(json11::Json::object{{json::developerMode::ATResponse, resp}});
    }

    AppFocusChangeEvent::AppFocusChangeEvent(std::string appName)
    {
        context.setResponseStatus(http::Code::OK);
        context.setEndpoint(EndpointType::developerMode);
        context.setResponseBody(json11::Json::object{{json::developerMode::focus, appName}});
    }

    ScreenlockCheckEvent::ScreenlockCheckEvent(bool isLocked)
    {
        context.setResponseStatus(http::Code::OK);
        context.setEndpoint(EndpointType::developerMode);
        context.setResponseBody(json11::Json::object{{json::developerMode::isLocked, isLocked}});
    }

    BluetoothStatusRequestEvent::BluetoothStatusRequestEvent(int state)
    {
        context.setResponseStatus(http::Code::OK);
        context.setEndpoint(EndpointType::developerMode);
        context.setResponseBody(json11::Json::object{{json::developerMode::btState, state}});
    }

} // namespace sdesktop::developerMode
