// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ScreenLightControl.hpp"

#include <Service/Service.hpp>
#include <Timers/TimerFactory.hpp>
#include <Utils.hpp>

namespace bell::screen_light_control
{
    ScreenLightController::ScreenLightController(sys::Service *parent)
    {
        controlTimer = sys::TimerFactory::createPeriodicTimer(parent,
                                                              "LightControlTimer",
                                                              std::chrono::milliseconds{CONTROL_TIMER_MS},
                                                              [this](sys::Timer &) { controlTimerCallback(); });
        automaticMode = ScreenLightMode::Automatic;
    }

    ScreenLightController::~ScreenLightController()
    {
        disableTimers();
    }

    void ScreenLightController::processRequest(Action action)
    {
        processRequest(action, Parameters{});
    }

    void ScreenLightController::processRequest(Action action, const Parameters &params)
    {
        switch (action) {
        case Action::turnOff:
            turnOff();
            break;
        case Action::turnOn:
            turnOn(params.hasManualModeParams() ? std::optional<ManualModeParameters>(params.getManualModeParams())
                                                : std::nullopt);
            break;
        case Action::setAutomaticModeParameters:
            if (params.hasLinearProgressModeParams()) {
                setParameters(params.getLinearProgressModeParams());
            }
            else if (params.hasConstLinearProgressModeParams()) {
                setParameters(params.getConstLinearProgressModeParams());
            }
            break;
        default:
            break;
        }
    }

    void ScreenLightController::controlTimerCallback()
    {
        auto newBrightness = ::screen_light_control::functions::brightnessRampOut();

        bsp::eink_frontlight::setBrightness(newBrightness);
        if (::screen_light_control::functions::isRampTargetReached()) {
            if (!automaticModeFunctions.empty()) {
                setUpAutomaticFunction(getNextAutomaticFunction(), newBrightness);
            }
            else {
                if (newBrightness == 0) {
                    bsp::eink_frontlight::turnOff();
                }
                disableTimers();
            }
        }
    }

    auto ScreenLightController::isLightOn() const noexcept -> bool
    {
        return lightOn;
    }

    bool ScreenLightController::isAutoModeOn() const noexcept
    {
        return automaticMode == ScreenLightMode::Automatic;
    }

    auto ScreenLightController::getBrightnessValue() const noexcept -> bsp::eink_frontlight::BrightnessPercentage
    {
        return brightnessWhenOn;
    }

    void ScreenLightController::enableTimers()
    {
        controlTimer.start();
    }

    void ScreenLightController::disableTimers()
    {
        controlTimer.stop();
    }

    void ScreenLightController::setParameters(const LinearProgressModeParameters &params)
    {
        if (params.functions.empty()) {
            disableTimers();
            return;
        }
        setAutomaticModeFunctions(params.functions);

        if (lightOn && automaticMode == ScreenLightMode::Automatic) {
            disableTimers();
        }

        ::screen_light_control::functions::setHysteresis(params.brightnessHysteresis);
        setUpAutomaticFunction(getNextAutomaticFunction(), ::screen_light_control::functions::getRampState());
        setManualBrightnessLevel(params.startBrightnessValue);

        if (lightOn && automaticMode == ScreenLightMode::Automatic) {
            enableTimers();
        }
    }

    void ScreenLightController::setAutomaticModeFunctions(
        const LinearProgressModeParameters::LinearFunctions &functions)
    {
        automaticModeFunctions = LinearProgressModeParameters::LinearFunctions{functions.rbegin(), functions.rend()};
    }

    ::screen_light_control::functions::LinearProgressFunction ScreenLightController::getNextAutomaticFunction()
    {
        auto function = automaticModeFunctions.back();
        automaticModeFunctions.pop_back();
        return function;
    }

    void ScreenLightController::setUpAutomaticFunction(
        ::screen_light_control::functions::LinearProgressFunction function,
        bsp::eink_frontlight::BrightnessPercentage currentBrightness)
    {
        ::screen_light_control::functions::setRampStep(
            std::abs(function.target - currentBrightness) /
            (static_cast<float>(function.duration.count()) / CONTROL_TIMER_MS));
        ::screen_light_control::functions::setRampTarget(function.target);
    }

    void ScreenLightController::setParameters(const ConstLinearProgressModeParameters &params)
    {
        setManualBrightnessLevel(params.targetBrightness);
    }

    void ScreenLightController::turnOn(const std::optional<ManualModeParameters> &params)
    {
        if (params.has_value()) {
            brightnessWhenOn = params->manualModeBrightness;
        }

        ::screen_light_control::functions::setRampTarget(brightnessWhenOn);
        bsp::eink_frontlight::turnOn();
        enableTimers();
        lightOn = true;
    }

    void ScreenLightController::setManualBrightnessLevel(bsp::eink_frontlight::BrightnessPercentage brightness)
    {
        brightnessWhenOn = brightness;
        ::screen_light_control::functions::setRampTarget(brightnessWhenOn);
        ::screen_light_control::functions::setRampStep(RAMP_STEP_PER_MS * CONTROL_TIMER_MS);
    }

    void ScreenLightController::turnOff()
    {
        lightOn = false;
        ::screen_light_control::functions::setRampTarget(0);
        enableTimers();
    }
} // namespace screen_light_control
