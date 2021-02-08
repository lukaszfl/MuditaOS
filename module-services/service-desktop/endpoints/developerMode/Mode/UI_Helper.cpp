// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "UI_Helper.hpp"
#include "log/log.hpp"
#include "service-desktop/Constants.hpp"
#include <variant>
#include <service-appmgr/model/ApplicationManager.hpp>
#include <service-appmgr/service-appmgr/messages/DOMRequest.hpp>

namespace parserFSM
{
    auto UI_Helper::processGet(Context &context) -> ProcessResult
    {
        auto response    = endpoint::ResponseContext{};
        const auto &body = context.getBody();
        if (body["getWindow"].is_bool()) {
            sys::Bus::SendUnicast(std::make_shared<app::manager::DOMRequest>(service::name::service_desktop),
                                  app::manager::ApplicationManager::ServiceName,
                                  owner);
        }
        return {sent::no, response};
    }
}; // namespace parserFSM
