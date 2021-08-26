// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

namespace hal::battery
{
    enum class batteryChargerType
    {
        DcdTimeOut = 0x00U, /*!< Dcd detect result is timeout */
        DcdUnknownType,     /*!< Dcd detect result is unknown type */
        DcdError,           /*!< Dcd detect result is error*/
        DcdSDP,             /*!< The SDP facility is detected */
        DcdCDP,             /*!< The CDP facility is detected */
        DcdDCP,             /*!< The DCP facility is detected */
    };
} // namespace hal::battery
