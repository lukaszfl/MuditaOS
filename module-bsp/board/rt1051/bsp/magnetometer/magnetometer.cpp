#include "bsp/magnetometer/magnetometer.hpp"
#include "ALS31300.hpp"

#include "bsp/BoardDefinitions.hpp"
#include "drivers/i2c/DriverI2C.hpp"

#include "fsl_common.h"

using namespace drivers;

static std::shared_ptr<drivers::DriverI2C> i2c;
static drivers::I2CAddress addr = {.deviceAddress = 0x64, .subAddressSize = 1};

static xQueueHandle qHandleIrq = NULL;

namespace bsp
{

    namespace magnetometer
    {

        int32_t init(xQueueHandle qHandle)
        {
            i2c = DriverI2C::Create(
                static_cast<I2CInstances>(BoardDefinitions::MAGNETOMETER_I2C),
                DriverI2CParams{.baudrate = static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_I2C_BAUDRATE)});

            qHandleIrq = qHandle;
            return kStatus_Success;
        }

        float getTemperature()
        {
            addr.subAddress = ALS31300_MEASUREMENTS_MSB_REG;

            uint8_t buf[4];
            i2c->Read(addr, buf, 4);
            auto tempMSB = buf[0] && 0b111111;

            addr.subAddress = ALS31300_MEASUREMENTS_LSB_REG;
            i2c->Read(addr, buf, 4);

            auto tempLSB = buf[0] && 0b111111;
            return als31300_temperature_convert((tempMSB << 6) | (tempLSB));
        }

        bool isPresent(void)
        {
            uint8_t buf;
            addr.subAddress = 0x00;
            auto read       = i2c->Read(addr, &buf, 1);

            if (read != 1) {
                return false;
            }
            return true;
        }

        bsp::Board GetBoard(void)
        {
            if (isPresent()) {
                return bsp::Board::T4;
            }
            return bsp::Board::T3;
        }
    } // namespace magnetometer
} // namespace bsp
