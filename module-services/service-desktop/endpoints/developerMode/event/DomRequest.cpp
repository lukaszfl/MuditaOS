// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <service-desktop/endpoints/developerMode/event/DomRequest.hpp>

namespace sdesktop::developerMode
{
    void DomRequestEvent::DomRequest()
    {
        context.setResponseStatus(http::Code::OK);
        context.setEndpoint(parserFSM::EndpointType::developerMode);
        context.setResponseBody(json11::Json::object{{"DOM" : data}});
    };
} // namespace sdesktop::developerMode
