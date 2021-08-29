// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <SwitchData.hpp>

namespace gui
{
    class PowerNapData : public gui::SwitchData
    {
        std::chrono::minutes napTime;

      public:
        explicit PowerNapData(std::chrono::minutes napTime) : napTime{napTime}
        {
            ignoreCurrentWindowOnStack = true;
        }

        [[nodiscard]] std::chrono::minutes getNapTime() const noexcept
        {
            return napTime;
        }
    };
} // namespace gui
