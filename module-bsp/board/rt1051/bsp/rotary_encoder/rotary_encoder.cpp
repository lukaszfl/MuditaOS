// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "bsp/rotary_encoder/rotary_encoder.hpp"

#include <module-utils/Utils.hpp> // for byte conversion functions. it is included first because of magic enum define

#include "bsp/BoardDefinitions.hpp"

#include <fsl_common.h>
#include <timers.h>

using namespace drivers;
using namespace utils;

static xQueueHandle qHandleIrq = NULL;

namespace bsp
{
    namespace rotary_encoder
    {
        std::shared_ptr<DriverGPIO> gpio;

        bsp::KeyCodes current_parsed = bsp::KeyCodes::Undefined;

        static TimerHandle_t timerHandle;
//        static constexpr uint16_t MAGNETOMETER_POLL_INTERVAL_MS = 500;

        static void TimerHandler(TimerHandle_t xTimer)
        {
            if (qHandleIrq != nullptr) {
                uint8_t val = 0x01;
                xQueueSend(qHandleIrq, &val, 0);
            }
        }

        int32_t init(xQueueHandle qHandle)
        {
            qHandleIrq = qHandle;
            const enc_config_t encConfig;

            ENC_GetDefaultConfig(&encConfig);
            ENC_Init(SWITCHES_ENC_A_PERIPHERAL, &encConfig);
            
            ENC_DoSoftwareLoadInitialPositionValue(SWITCHES_ENC_A_PERIPHERAL);

            return kStatus_Success;
        }

        bool setActive(als31300::PWR_REG_SLEEP_MODE sleep_mode)
        {
            // POWER register
            als31300::whole_reg_t read_reg;

            if (!i2cRead(als31300::PWR_REG, read_reg)) {
                return false;
            }
            als31300::pwr_reg reg_pwr = read_reg;
            reg_pwr.sleep             = sleep_mode;

            if (!i2cWrite(als31300::PWR_REG, reg_pwr)) {
                return false;
            }
            if (sleep_mode == als31300::PWR_REG_SLEEP_MODE::active ||
                sleep_mode == als31300::PWR_REG_SLEEP_MODE::periodic_active) {
                vTaskDelay(pdMS_TO_TICKS(als31300::PWR_ON_DELAY_MS)); // give it some time to wake up
            }
            return true;
        }

        bool isPresent(void)
        {
            return true;
        }

        std::optional<bsp::KeyCodes> WorkerEventHandler()
        {
            uint16_t encDiffValue = ENC_GetHoldPositionDifferenceValue(SWITCHES_ENC_A_PERIPHERAL);

            // currently only single left/right keys are returned. TBD returning multiple "keypresses"

            // try to get new data from active magneto
            setActive(als31300::PWR_REG_SLEEP_MODE::active);
            auto [new_data, measurement] = getMeasurement();
            setActive(als31300::PWR_REG_SLEEP_MODE::sleep);
            if (new_data) {
                auto incoming_parsed = parse(measurement);
                if (incoming_parsed != bsp::KeyCodes::Undefined and incoming_parsed != current_parsed) {
                    current_parsed = incoming_parsed;
                    return current_parsed;
                }
            }
            return std::nullopt;
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

        void enableIRQ()
        {
            gpio->EnableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_IRQ));
        }
    } // namespace rotary-encoder
} // namespace bsp
