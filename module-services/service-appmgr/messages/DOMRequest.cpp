// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <service-appmgr/messages/DOMRequest.hpp>

namespace app::manager
{
    DOMRequest::DOMRequest(ApplicationName sender) : BaseMessage(sender)
    {}
    DOMResponse::DOMResponse(ApplicationName sender, json11::Json &&json) : BaseMessage(sender), json(json)
    {}
} // namespace app::manager
