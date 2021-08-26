// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <module-bsp/hal/battery_charger/BatteryCharger.hpp>
#include <hal/GenericFactory.hpp>

namespace hal::battery
{
    class BatteryCharger : public AbstractBatteryCharger
    {
      public:
        void init(xQueueHandle, xQueueHandle) final
        {}

        void deinit() final
        {}

        void processStateChangeNotification(std::uint8_t notification) final
        {}

        void setChargingCurrentLimit(batteryChargerType) final
        {}
    };

    std::shared_ptr<AbstractBatteryCharger> AbstractBatteryCharger::Factory::create()
    {
        return hal::impl::factory<BatteryCharger, AbstractBatteryCharger>();
    }
} // namespace hal::battery
