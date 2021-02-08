// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "BaseMessage.hpp"
#include <json/json11.hpp>

namespace app::manager
{
    class DOMRequest : public BaseMessage
    {
      public:
        DOMRequest(ApplicationName sender);
    };

    class DOMResponse : public BaseMessage
    {
        json11::Json json;

      public:
        DOMResponse(ApplicationName sender, json11::Json &&json);
        [[nodiscard]] auto get() -> json11::Json &&;
    };
} // namespace app::manager
