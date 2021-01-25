// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "EinkMessage.hpp"

namespace service::eink
{
    class EinkPowerOffRequest : public EinkMessage
    {
      public:
        EinkPowerOffRequest()
        {}
    };
} // namespace service::eink
