// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <service-appmgr/service-appmgr/Actions.hpp>

namespace app::manager::actions
{
    class CallParams : public ActionParams
    {
        utils::PhoneNumber::View number;

      public:
        CallParams(const utils::PhoneNumber::View &number) : number(number){};

        [[nodiscard]] const utils::PhoneNumber::View &getPhoneNumber() const noexcept
        {
            return number;
        }
    };
} // namespace app::manager::actions
