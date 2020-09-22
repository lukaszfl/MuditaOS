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
            uint8_t buf[4];

            // CONFIGURATION register
            addr.subAddress = ALS31300_CONF_REG;
            i2c->Read(addr, buf, 4);
            als31300_conf_reg reg_conf(USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf));
            reg_conf.int_latch_enable = ALS31300_CONF_REG_LATCH_DISABLED;  // we want to detect stable positions
            reg_conf.channel_X_en     = ALS31300_CONF_REG_CHANNEL_ENABLED; // enable only X axis
            reg_conf.channel_Y_en     = ALS31300_CONF_REG_CHANNEL_DISABLED;
            reg_conf.channel_Z_en     = ALS31300_CONF_REG_CHANNEL_DISABLED;
            reg_conf.bandwidth        = 0; // longest unit measurement
            USB_LONG_TO_LITTLE_ENDIAN_ADDRESS(reg_conf, buf);
            i2c->Write(addr, buf, 4);

            //             check
            //            i2c->Read(addr, buf, 4);
            //            reg = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf);
            //            LOG_DEBUG("conf reg: %" PRIu32, static_cast<uint32_t>(reg));

            // POWER register

            addr.subAddress = ALS31300_PWR_REG;
            als31300_pwr_reg reg_pwr;
            reg_pwr.I2C_loop_mode     = ALS31300_PWR_REG_LOOP_MODE_single; // we don't want constant data flow
            reg_pwr.sleep             = ALS31300_PWR_REG_SLEEP_MODE_LPDCM;
            reg_pwr.count_max_LP_mode = 6U; // update every 500 ms
            USB_LONG_TO_LITTLE_ENDIAN_ADDRESS(reg_pwr, buf);
            i2c->Write(addr, buf, 4);

            while (true) {
                LOG_DEBUG("pozycja: %d", static_cast<uint16_t>(getDiscrete(getAxisX())));
            }

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
        int16_t getAxisX()
        {
            addr.subAddress = ALS31300_MEASUREMENTS_MSB_REG;
            uint8_t buf[4];
            i2c->Read(addr, buf, 4);
            uint8_t xMSB = (USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf) >> 24) &
                           0xFF; // it arrives as big-endian, but we are little-endian
            addr.subAddress = ALS31300_MEASUREMENTS_LSB_REG;
            i2c->Read(addr, buf, 4);

            uint8_t xLSB = (USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf) >> 16) & 0xF;

            return als31300_measurement_sign_convert(xMSB << 4 | xLSB);
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

        bsp::KeyCodes getDiscrete(int16_t raw_measurement)
        {
            if (raw_measurement < -150 || raw_measurement > 150) {
                return bsp::KeyCodes::Undefined;
            }
            else {
                if (raw_measurement < -65) {
                    return bsp::KeyCodes::SSwitchDown;
                }
                if (raw_measurement > 60) {
                    return bsp::KeyCodes::SSwitchUp;
                }
                else
                    return bsp::KeyCodes::SSwitchMid;
            }
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
