// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <log.hpp>

#include "PowerManager.hpp"
#include "bsp/BoardDefinitions.hpp"
#include <module-bsp/drivers/gpio/DriverGPIO.hpp>
#include "fsl_common.h"

namespace sys
{
    namespace
    {
        std::shared_ptr<drivers::DriverGPIO> gpio;
    }

    PowerManager::PowerManager()
    {
        lowPowerControl = bsp::LowPowerMode::Create().value_or(nullptr);
        driverSEMC      = drivers::DriverSEMC::Create("ExternalRAM");
        cpuGovernor     = std::make_unique<CpuGovernor>();
    }

    void PowerManager::init()
    {
        using namespace drivers;
        gpio = DriverGPIO::Create(static_cast<GPIOInstances>(BoardDefinitions::KEYPAD_BACKLIGHT_DRIVER_GPIO),
                                  DriverGPIOParams{});

        gpio->ConfPin(DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Output,
                                          .irqMode  = DriverGPIOPinParams::InterruptMode::NoIntmode,
                                          .defLogic = 0,
                                          .pin      = static_cast<uint32_t>(BoardDefinitions::EINK_FRONTLIGHT_GPIO)});

        gpio->ConfPin(
            DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Output,
                                .irqMode  = DriverGPIOPinParams::InterruptMode::NoIntmode,
                                .defLogic = 0,
                                .pin      = static_cast<uint32_t>(BoardDefinitions::KEYPAD_BACKLIGHT_DRIVER_NRST)});

        gpio->ConfPin(DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Output,
                                          .irqMode  = DriverGPIOPinParams::InterruptMode::NoIntmode,
                                          .defLogic = 0,
                                          .pin      = static_cast<uint32_t>(BoardDefinitions::TORCH_DRIVER_EN)});
        setGpio(0xff);
    }

    void PowerManager::setGpio(std::uint8_t encoded) const
    {
        gpio->WritePin(static_cast<uint32_t>(BoardDefinitions::EINK_FRONTLIGHT_GPIO), encoded & 0x01);
        gpio->WritePin(static_cast<uint32_t>(BoardDefinitions::KEYPAD_BACKLIGHT_DRIVER_NRST), encoded & 0x02);
        gpio->WritePin(static_cast<uint32_t>(BoardDefinitions::TORCH_DRIVER_EN), encoded & 0x04);
    }

    PowerManager::~PowerManager()
    {}

    int32_t PowerManager::PowerOff()
    {
        return lowPowerControl->PowerOff();
    }

    int32_t PowerManager::Reboot()
    {
        return lowPowerControl->Reboot();
    }

    void PowerManager::UpdateCpuFrequency(uint32_t cpuLoad)
    {
        const auto currentCpuFreq        = lowPowerControl->GetCurrentFrequencyLevel();

        if (currentCpuFreq > bsp::CpuFrequencyHz::Level_1) {
            DecreaseCpuFrequency();
        }
        else {
            IncreaseCpuFrequency();
        }

        /*        const auto minFrequencyRequested = cpuGovernor->GetMinimumFrequencyRequested();

                if (cpuLoad > frequencyShiftUpperThreshold && currentCpuFreq < bsp::CpuFrequencyHz::Level_6) {
                    aboveThresholdCounter++;
                    belowThresholdCounter = 0;
                }
                else if (cpuLoad < frequencyShiftLowerThreshold && currentCpuFreq > bsp::CpuFrequencyHz::Level_1) {
                    belowThresholdCounter++;
                    aboveThresholdCounter = 0;
                }
                else {
                    ResetFrequencyShiftCounter();
                }

                if (aboveThresholdCounter >= maxAboveThresholdCount || minFrequencyRequested > currentCpuFreq) {
                    ResetFrequencyShiftCounter();
                    IncreaseCpuFrequency();
                }
                else {
                    if (belowThresholdCounter >= maxBelowThresholdCount && currentCpuFreq > minFrequencyRequested) {
                        ResetFrequencyShiftCounter();
                        DecreaseCpuFrequency();
                    }
                }*/
    }

    void PowerManager::IncreaseCpuFrequency() const
    {
        const auto freq      = lowPowerControl->GetCurrentFrequencyLevel();

        if (freq == bsp::CpuFrequencyHz::Level_1) {
            // switch osc source first
            lowPowerControl->SwitchOscillatorSource(bsp::LowPowerMode::OscillatorSource::External);

            // then switch external RAM clock source
            if (driverSEMC) {
                driverSEMC->SwitchToPLL2ClockSource();
            }
        }

        // and increase frequency
        if (freq < bsp::CpuFrequencyHz::Level_6) {
            SetCpuFrequency(bsp::CpuFrequencyHz::Level_6);
            setGpio(6);
        }
    }

    void PowerManager::DecreaseCpuFrequency() const
    {
        const auto freq = lowPowerControl->GetCurrentFrequencyLevel();
        auto level      = bsp::CpuFrequencyHz::Level_1;

        switch (freq) {
        case bsp::CpuFrequencyHz::Level_6:
            level = bsp::CpuFrequencyHz::Level_5;
            setGpio(6);
            break;
        case bsp::CpuFrequencyHz::Level_5:
            level = bsp::CpuFrequencyHz::Level_4;
            setGpio(5);
            break;
        case bsp::CpuFrequencyHz::Level_4:
            level = bsp::CpuFrequencyHz::Level_3;
            setGpio(4);
            break;
        case bsp::CpuFrequencyHz::Level_3:
            level = bsp::CpuFrequencyHz::Level_2;
            setGpio(3);
            break;
        case bsp::CpuFrequencyHz::Level_2:
            level = bsp::CpuFrequencyHz::Level_1;
            setGpio(2);
            break;
        case bsp::CpuFrequencyHz::Level_1:
            setGpio(1);
            break;
        }

        // decrease frequency first
        if (level != freq) {
            SetCpuFrequency(level);
        }

        if (level == bsp::CpuFrequencyHz::Level_1) {
            // then switch osc source
            lowPowerControl->SwitchOscillatorSource(bsp::LowPowerMode::OscillatorSource::Internal);

            // and switch external RAM clock source
            if (driverSEMC) {
                driverSEMC->SwitchToPeripheralClockSource();
            }
        }
    }

    void PowerManager::RegisterNewSentinel(std::shared_ptr<CpuSentinel> newSentinel) const
    {
        cpuGovernor->RegisterNewSentinel(newSentinel);
    }

    void PowerManager::SetCpuFrequencyRequest(std::string sentinelName, bsp::CpuFrequencyHz request)
    {
        cpuGovernor->SetCpuFrequencyRequest(std::move(sentinelName), request);
    }

    void PowerManager::ResetCpuFrequencyRequest(std::string sentinelName)
    {
        cpuGovernor->ResetCpuFrequencyRequest(std::move(sentinelName));
    }

    void PowerManager::SetCpuFrequency(bsp::CpuFrequencyHz freq) const
    {
        lowPowerControl->SetCpuFrequency(freq);
        cpuGovernor->InformSentinelsAboutCpuFrequencyChange(freq);
    }

    void PowerManager::ResetFrequencyShiftCounter()
    {
        aboveThresholdCounter = 0;
        belowThresholdCounter = 0;
    }

    [[nodiscard]] auto PowerManager::getExternalRamDevice() const noexcept -> std::shared_ptr<devices::Device>
    {
        return driverSEMC;
    }

} // namespace sys
