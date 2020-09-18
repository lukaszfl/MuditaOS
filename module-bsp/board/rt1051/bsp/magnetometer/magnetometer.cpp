#include "bsp/magnetometer/magnetometer.hpp"
#include "ALS31300.hpp"
#include "bsp/BoardDefinitions.hpp"

#include "drivers/i2c/DriverI2C.hpp"

#include <board/rt1051/bsp/usb_cdc/driver/osa/usb_osa.h>                 // just for the define to populate
#include <module-bsp/board/rt1051/bsp/usb_cdc/driver/include/usb_misc.h> // for endian-swapping macro

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

            // init device
            // enable only X axis
            {
                als31300_conf_reg reg;

                addr.subAddress = ALS31300_CONF_REG;

                uint8_t buf[4];
                i2c->Read(addr, buf, 4);
                *((uint32_t *)&reg) = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf);
                LOG_DEBUG("conf reg: %" PRIu32, *((uint32_t*)&reg) );

                reg.channel_X_en = ALS31300_CONF_REG_CHANNEL_ENABLED;
            }

            // set loop mode to single - we don't want constant data flow
//            als31300_pwr_reg reg;
//            reg.I2C_loop_mode     = ALS31300_PWR_REG_LOOP_MODE_single;
//            reg.sleep             = ALS31300_PWR_REG_SLEEP_MODE_LPDCM;
//            reg.count_max_LP_mode = 6; // update every 500ms

            return kStatus_Success;
        }

        float getTemperature()
        {
            addr.subAddress = ALS31300_MEASUREMENTS_MSB_REG;

            uint8_t buf[4];
            i2c->Read(addr, buf, 4);
            auto tempMSB =
                USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf) & 0b111111; // it comes as big-endian, but we are little-endian

            addr.subAddress = ALS31300_MEASUREMENTS_LSB_REG;
            i2c->Read(addr, buf, 4);

            auto tempLSB = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf) & 0b111111;
            float temp   = als31300_temperature_convert((tempMSB << 6) | (tempLSB));
            return temp;
        }

        // read magneto - slider only moves in the X axis
//        uint32_t readAxis(Axis axis)
//        {
//
//        }

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
