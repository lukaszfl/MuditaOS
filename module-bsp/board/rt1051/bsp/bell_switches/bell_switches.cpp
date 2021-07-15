// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "bsp/bell_switches/bell_switches.hpp"

#include <module-utils/Utils.hpp> // for byte conversion functions. it is included first because of magic enum define

#include "bsp/BoardDefinitions.hpp"

#include <fsl_common.h>
#include <timers.h>

using namespace drivers;
using namespace utils;

namespace bsp
{
    namespace bell_switches
    {
        #define CONTACT_OSCILLATION_TIMEOUT_MS 20

        enum class DEBOUNCE_TIMERS
        {
            Bell_SW_Left = 0,
            Bell_SW_Right,
            Bell_SW_Center,
            Bell_SW_Latch,
            Bell_SW_Wakeup,
            COUNT
        };

        static TimerHandle_t debounce_timer[DEBOUNCE_TIMERS::COUNT]   = {NULL};
        static volatile uint8_t debounce_last_state[DEBOUNCE_TIMERS::COUNT] = {0};

        static xQueueHandle qHandleIrq = NULL;
        std::shared_ptr<DriverGPIO> gpio_sw;
        std::shared_ptr<DriverGPIO> gpio_wakeup;

        std::function <void(TimerHandle_t)> debounce_timer_callbacks[DEBOUNCE_TIMERS::COUNT] = {
            debounce_timer_callback(debounce_timer[DEBOUNCE_TIMERS::Bell_SW_Left], debounce_last_state[DEBOUNCE_TIMERS::Bell_SW_Left], gpio_sw, BELL_SWITCHES_LEFT),
            debounce_timer_callback(debounce_timer[DEBOUNCE_TIMERS::Bell_SW_Right], debounce_last_state[DEBOUNCE_TIMERS::Bell_SW_Right], gpio_sw, BELL_SWITCHES_RIGHT),
            debounce_timer_callback(debounce_timer[DEBOUNCE_TIMERS::Bell_SW_Center], debounce_last_state[DEBOUNCE_TIMERS::Bell_SW_Center], gpio_sw, BELL_SWITCHES_CENTER),
            debounce_timer_callback(debounce_timer[DEBOUNCE_TIMERS::Bell_SW_Latch], debounce_last_state[DEBOUNCE_TIMERS::Bell_SW_Latch], gpio_sw, BELL_SWITCHES_LATCH),
            debounce_timer_callback(debounce_timer[DEBOUNCE_TIMERS::Bell_SW_Wakeup], debounce_last_state[DEBOUNCE_TIMERS::Bell_SW_Wakeup], gpio_sw, BELL_WAKEUP),
        };

        void debounce_timer_callback(TimerHandle_t timer, uint8_t &last_state, DriverGPIO &gpio, uint32_t pin)
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            gpio->ClearPortInterrupts(1U << pin);

            if (s_right_functional_check_timer != NULL) {
                gpio->DisableInterrupt(1U << pin);

                if (gpio->ReadPin(pin) == 0) {
                    last_state = 0;
                }
                else {
                    last_state = 1;
                }

                xTimerResetFromISR(timer, &xHigherPriorityTaskWoken);
            }

            return xHigherPriorityTaskWoken;
        }

        int32_t init(xQueueHandle qHandle)
        {
            qHandleIrq = qHandle;

            // Switches
            gpio_sw = DriverGPIO::Create(static_cast<GPIOInstances>(BoardDefinitions::BELL_SWITCHES_GPIO), DriverGPIOParams{});
            //wakeup
            gpio_wakup = DriverGPIO::Create(static_cast<GPIOInstances>(BoardDefinitions::BELL_WAKEUP_GPIO), DriverGPIOParams{});

            // Center switch
            gpio_sw->ClearPortInterrupts(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_CENTER));
            gpio_sw->ConfPin(DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Input,
                                              .irqMode  = DriverGPIOPinParams::InterruptMode::IntFallingEdge,
                                              .defLogic = 0,
                                              .pin      = static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_CENTER)});

            // Left switch
            gpio_sw->ClearPortInterrupts(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_LEFT));
            gpio_sw->ConfPin(DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Input,
                                              .irqMode  = DriverGPIOPinParams::InterruptMode::IntFallingEdge,
                                              .defLogic = 0,
                                              .pin      = static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_LEFT)});

            // Right switch
            gpio_sw->ClearPortInterrupts(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_RIGHT));
            gpio_sw->ConfPin(DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Input,
                                              .irqMode  = DriverGPIOPinParams::InterruptMode::IntFallingEdge,
                                              .defLogic = 0,
                                              .pin      = static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_RIGHT)});

            // Latch
            gpio_sw->ClearPortInterrupts(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_LATCH));
            gpio_sw->ConfPin(DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Input,
                                              .irqMode  = DriverGPIOPinParams::InterruptMode::IntRisingOrFallingEdge,
                                              .defLogic = 0,
                                              .pin      = static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_LATCH)});       

            // Wakeup
            gpio_wakeup->ClearPortInterrupts(1 << static_cast<uint32_t>(BoardDefinitions::BELL_WAKEUP));
            gpio_wakeup->ConfPin(DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Input,
                                              .irqMode  = DriverGPIOPinParams::InterruptMode::IntFallingEdge,
                                              .defLogic = 0,
                                              .pin      = static_cast<uint32_t>(BoardDefinitions::BELL_WAKEUP)});  

            if (debounce_timer[DEBOUNCE_TIMERS::Bell_SW_Left] == NULL) {
                debounce_timer[DEBOUNCE_TIMERS::Bell_SW_Left] = xTimerCreate("Bell_SW_Left",
                                                          pdMS_TO_TICKS(CONTACT_OSCILLATION_TIMEOUT_MS),
                                                          false,
                                                          NULL,
                                                          debounce_timer_callbacks[DEBOUNCE_TIMERS::Bell_SW_Left]);
            if (s_right_functional_check_timer == NULL) {
                return kStatus_Fail;
            }
        }    

            enableIRQ();           

            return kStatus_Success;
        }

        void deinit()
        {
            qHandleIrq = nullptr;
            disableIRQ();
        }

        BaseType_t IRQHandler()
        {
            gpio->DisableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_IRQ));
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            if (qHandleIrq != NULL) {
                uint8_t val = 0x01;
                xQueueSendFromISR(qHandleIrq, &val, &xHigherPriorityTaskWoken);
            }
            return xHigherPriorityTaskWoken;
        }

        BaseType_t bell_switches_Cent_Right_Latch_IRQHandler(uint32_t mask)
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            gpio->ClearPortInterrupts(mask);

            if (s_right_functional_check_timer != NULL) {
                gpio->DisableInterrupt(1U << static_cast<uint32_t>(BoardDefinitions::KEYBOARD_RF_BUTTON));

                if (gpio->ReadPin(static_cast<uint32_t>(BoardDefinitions::KEYBOARD_RF_BUTTON)) == 0) {
                    s_rigth_functional_last_state = 0;
                }
                else {
                    s_rigth_functional_last_state = 1;
                }

                xTimerResetFromISR(s_right_functional_check_timer, &xHigherPriorityTaskWoken);
            }

            return xHigherPriorityTaskWoken;
        }

        BaseType_t bell_switches_Left_IRQHandler() 
        {
            return xHigherPriorityTaskWoken;
        }

        BaseType_t bell_wakeup_IRQHandler()
        {
            return xHigherPriorityTaskWoken;
        }

        void enableIRQ()
        {
            gpio_sw->EnableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_CENTER));
            gpio_sw->EnableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_LEFT));
            gpio_sw->EnableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_RIGHT));
            gpio_sw->EnableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_LATCH));
            gpio_wakeup->EnableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_WAKEUP));
        }

        void disableIRQ()
        {
            gpio_sw->DisableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_CENTER));
            gpio_sw->DisableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_LEFT));
            gpio_sw->DisableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_RIGHT));
            gpio_sw->DisableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_LATCH));
            gpio_wakeup->DisableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_WAKEUP));
        }

        // debounce timers
        
    } // namespace bell_switches
} // namespace bsp
