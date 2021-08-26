// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "BatteryCharger.hpp"

#include <hal/GenericFactory.hpp>

namespace hal::battery
{
    std::shared_ptr<AbstractBatteryCharger> AbstractBatteryCharger::Factory::create(sys::Service *service)
    {
        return hal::impl::factory<BatteryCharger, AbstractBatteryCharger>(service);
    }

    BatteryCharger::BatteryCharger(sys::Service *)
    {}

    void BatteryCharger::init(xQueueHandle queueBatteryHandle, xQueueHandle)
    {}

    void BatteryCharger::BatteryCharger::deinit()
    {}

    void BatteryCharger::processStateChangeNotification(std::uint8_t notification)
    {}

    void BatteryCharger::setChargingCurrentLimit(std::uint8_t)
    {}

} // namespace hal::battery
