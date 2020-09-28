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

        std::shared_ptr<DriverGPIO> gpio;

        int32_t init(xQueueHandle qHandle)
        {
            i2c = DriverI2C::Create(
                static_cast<I2CInstances>(BoardDefinitions::MAGNETOMETER_I2C),
                DriverI2CParams{.baudrate = static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_I2C_BAUDRATE)});

            qHandleIrq = qHandle;

            // init device
            uint8_t buf[4];

            // GET WRITE ACCESS
            addr.subAddress = ALS31300_CUSTOMER_ACCESS_REG;
            USB_LONG_TO_BIG_ENDIAN_ADDRESS(ALS31300_CUSTOMER_ACCESS_REG_code, buf);
            assert(i2c->Write(addr, buf, 4) == 4);

            // CONFIGURATION register read
            addr.subAddress = ALS31300_CONF_REG;
            i2c->Read(addr, buf, 4);
            als31300_conf_reg reg_conf(USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf));
            LOG_DEBUG("conf pre: %" PRIu32, static_cast<uint32_t>(reg_conf));

            // CONFIGURATION register write
            addr.subAddress           = ALS31300_CONF_REG;
            reg_conf.I2C_threshold    = ALS31300_CONF_REG_I2C_THRES_1V8;
//            reg_conf.int_latch_enable = ALS31300_CONF_REG_LATCH_DISABLED; // we want to detect stable positions
            reg_conf.int_latch_enable = ALS31300_CONF_REG_LATCH_DISABLED;
            reg_conf.channel_X_en     = ALS31300_CONF_REG_CHANNEL_ENABLED;
            reg_conf.channel_Y_en     = ALS31300_CONF_REG_CHANNEL_DISABLED;
            reg_conf.channel_Z_en     = ALS31300_CONF_REG_CHANNEL_DISABLED;
            reg_conf.bandwidth        = 0; // longest unit measurement
            USB_LONG_TO_BIG_ENDIAN_ADDRESS(reg_conf, buf);
            assert(i2c->Write(addr, buf, 4) == 4);

            //             check conf reg
            addr.subAddress = ALS31300_CONF_REG;
            i2c->Read(addr, buf, 4);
            auto reg_c = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf);

            LOG_DEBUG("conf wrote: %" PRIu32 ", conf post: %" PRIu32,
                      static_cast<uint32_t>(reg_conf),
                      static_cast<uint32_t>(reg_c));

            //             check pwr reg
            addr.subAddress = ALS31300_PWR_REG;
            i2c->Read(addr, buf, 4);
            auto reg = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf);
            LOG_DEBUG("power pre: %" PRIu32, static_cast<uint32_t>(reg));

            // POWER register
            addr.subAddress = ALS31300_PWR_REG;
            als31300_pwr_reg reg_pwr;
            reg_pwr.I2C_loop_mode     = ALS31300_PWR_REG_LOOP_MODE_single; // we don't want constant data flow
            reg_pwr.sleep             = ALS31300_PWR_REG_SLEEP_MODE_LPDCM;
            reg_pwr.count_max_LP_mode = 6U; // get an update every 500 ms
            USB_LONG_TO_BIG_ENDIAN_ADDRESS(reg_pwr, buf);
            assert(i2c->Write(addr, buf, 4) == 4);
            //             check
            i2c->Read(addr, buf, 4);
            als31300_pwr_reg reg_2 = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf);
            LOG_DEBUG(
                "wrote: %" PRIu32 ", post: %" PRIu32, static_cast<uint32_t>(reg_pwr), static_cast<uint32_t>(reg_2));

            // INTERRUPTS register
            addr.subAddress              = ALS31300_INT_REG;
            als31300_int_reg reg_int     = 0;
            reg_int.int_mode             = ALS31300_INT_REG_INT_MODE_threshold;
            reg_int.int_threshold_signed = ALS31300_INT_REG_THRESHOLD_unSIGNED;
            reg_int.int_X_en             = ALS31300_INT_REG_INT_CHANNEL_ENABLED;
            reg_int.int_Y_en             = ALS31300_INT_REG_INT_CHANNEL_DISABLED;
            reg_int.int_Z_en             = ALS31300_INT_REG_INT_CHANNEL_DISABLED;
            reg_int.int_X_threshold      = 0;
            reg_int.int_Y_threshold      = 0b111111;
            reg_int.int_Z_threshold      = 0;
            USB_LONG_TO_BIG_ENDIAN_ADDRESS(reg_int, buf);
            vTaskDelay(10);
            assert(i2c->Write(addr, buf, 4) == 4);
            vTaskDelay(10);
            i2c->Read(addr, buf, 4);
            als31300_int_reg reg_3 = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf);
            LOG_DEBUG("INT REG wrote: %" PRIu32 ", post: %" PRIu32,
                      static_cast<uint32_t>(reg_int),
                      static_cast<uint32_t>(reg_3));

            // INTERRUPT PIN
            gpio =
                DriverGPIO::Create(static_cast<GPIOInstances>(BoardDefinitions::MAGNETOMETER_GPIO), DriverGPIOParams{});

            // INTERRUPT PIN
            gpio->ClearPortInterrupts(1 << static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_IRQ));
            gpio->ConfPin(DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Input,
                                              .irqMode  = DriverGPIOPinParams::InterruptMode::IntFallingEdge,
                                              .defLogic = 0,
                                              .pin      = static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_IRQ)});

            // CLEAR IRQ SOURCE
            addr.subAddress = ALS31300_MEASUREMENTS_MSB_REG;
            als31300_measurements_MSB_reg reg_msb;
            do {
                i2c->Read(addr, buf, 4);
                // is there anything new ?
                reg_msb = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf); // it arrives as big-endian, but we are little-endian
            } while (reg_msb.int_flag == true);

            // EN IRQ
            gpio->EnableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_IRQ));

            return kStatus_Success;
        }

        std::pair<bool, Measurements> getMeasurements()
        {
            addr.subAddress = ALS31300_MEASUREMENTS_MSB_REG;
            uint8_t buf[4];
            i2c->Read(addr, buf, 4);
            // is there anything new ?
            als31300_measurements_MSB_reg reg_msb =
                USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf); // it arrives as big-endian, but we are little-endian

            if (reg_msb.int_flag == true) {
                LOG_DEBUG("INT magneto");
            }

            if (reg_msb.new_data_flag != ALS31300_MEAS_REG_NEW_DATA_available) {
                return std::make_pair(false, Measurements());
            }
            else {
                Measurements meas;

                addr.subAddress = ALS31300_MEASUREMENTS_LSB_REG;
                i2c->Read(addr, buf, 4);

                als31300_measurements_LSB_reg reg_lsb = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(buf);

                meas.X     = als31300_measurement_sign_convert(reg_msb.X_MSB << 4 | reg_lsb.X_LSB);
                meas.Y     = als31300_measurement_sign_convert(reg_msb.Y_MSB << 4 | reg_lsb.Z_LSB);
                meas.Z     = als31300_measurement_sign_convert(reg_msb.Z_MSB << 4 | reg_lsb.Z_LSB);
                meas.tempC = als31300_temperature_convert((reg_msb.temperature_MSB << 6) | (reg_lsb.temperature_LSB));

                return std::make_pair(true, meas);
            }
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

        bsp::KeyCodes getDiscrete(const Measurements &measurements)
        {
            if (measurements.X < -150 || measurements.X > 150) {
                return bsp::KeyCodes::Undefined;
            }
            else {
                if (measurements.X < -65) {
                    return bsp::KeyCodes::SSwitchDown;
                }
                if (measurements.X > 60) {
                    return bsp::KeyCodes::SSwitchUp;
                }
                else
                    return bsp::KeyCodes::SSwitchMid;
            }
        }

        BaseType_t IRQHandler()
        {
            gpio->DisableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_IRQ));
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            if (qHandleIrq != NULL) {
                uint8_t val = 0x01;
                xQueueSendFromISR(qHandleIrq, &val, &xHigherPriorityTaskWoken);
            }
            gpio->ClearPortInterrupts(1 << static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_IRQ));
            gpio->EnableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_IRQ));
            return xHigherPriorityTaskWoken;
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
