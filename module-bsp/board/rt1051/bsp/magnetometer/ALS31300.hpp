#pragma once

// ALS31300 magnetometer driver

/*
 * Note: this device returns 32-bit register values in MSB order
 */

// base ALS31300 register struct. it is 32-bits long
typedef struct als31300_reg
{
    operator uint32_t() const
    {
        return *((uint32_t *)this);
    };

    als31300_reg &operator=(uint32_t whole_reg)
    {
        *this = static_cast<als31300_reg>(whole_reg); // == als31300_reg(whole_reg)
        return *this;
    };

    als31300_reg(uint32_t whole_reg)
    {
        memcpy(this, (uint8_t *)&whole_reg, 4);
    };
    als31300_reg() = default;

} als31300_reg;

// REGISTER DEFINITIONS

// there is no 0x01 register
constexpr auto ALS31300_CONF_REG = 0x02;
typedef struct : als31300_reg
{
    using als31300_reg::als31300_reg;
    using als31300_reg::operator=;
    uint8_t user_eeprom : 5;
    bool int_latch_enable : 1;
    bool channel_X_en : 1;
    bool channel_Y_en : 1;
    bool channel_Z_en : 1;
    bool I2C_threshold : 1;
    uint8_t slave_addr : 7;
    bool disable_slave_ADC : 1;
    bool I2C_CRC_en : 1;
    uint8_t hall_mode : 2;
    uint8_t bandwidth : 3;
    uint8_t RESERVED : 8;
} als31300_conf_reg;

constexpr auto ALS31300_CONF_REG_LATCH_ENABLED  = 0b1;
constexpr auto ALS31300_CONF_REG_LATCH_DISABLED = 0b0;

constexpr auto ALS31300_CONF_REG_CHANNEL_ENABLED  = 0b1;
constexpr auto ALS31300_CONF_REG_CHANNEL_DISABLED = 0b0;

// --------
constexpr auto ALS31300_INT_REG = 0x03;

typedef struct : als31300_reg
{
    using als31300_reg::als31300_reg;
    using als31300_reg::operator=;
    uint8_t int_X_threshold : 6;
    uint8_t int_Y_threshold : 6;
    uint8_t int_Z_threshold : 6;
    bool int_X_en : 1;
    bool int_Y_en : 1;
    bool int_Z_en : 1;
    bool int_eeprom_en : 1;
    bool int_eeprom_status : 1;
    bool int_mode : 1;
    bool int_signed_en : 1;

} als31300_int_reg;

// --------
constexpr auto ALS31300_PWR_REG = 0x27;

typedef struct : als31300_reg
{
    using als31300_reg::als31300_reg;
    using als31300_reg::operator=;
    uint8_t sleep : 2;
    uint8_t I2C_loop_mode : 2;
    uint8_t count_max_LP_mode : 3;
} als31300_pwr_reg;

constexpr auto ALS31300_PWR_REG_SLEEP_MODE_active = 0;
constexpr auto ALS31300_PWR_REG_SLEEP_MODE_sleep  = 1;
constexpr auto ALS31300_PWR_REG_SLEEP_MODE_LPDCM  = 2; // Low-Power Duty Cycle Mode

constexpr auto ALS31300_PWR_REG_LOOP_MODE_single    = 0b00;
constexpr auto ALS31300_PWR_REG_LOOP_MODE_fast_loop = 0b01;
constexpr auto ALS31300_PWR_REG_LOOP_MODE_full_loop = 0b10;

// --------
constexpr auto ALS31300_MEASUREMENTS_MSB_REG = 0x28;

typedef struct : als31300_reg
{
    using als31300_reg::als31300_reg;
    using als31300_reg::operator=;
    uint8_t temperature_MSB : 6;
    bool int_flag : 1;
    bool new_data_flag : 1;
    uint8_t Z_MSB : 8;
    uint8_t Y_MSB : 8;
    uint8_t X_MSB : 8;
} als31300_measurements_MSB_reg;

constexpr auto ALS31300_MEAS_REG_NEW_DATA_available = 0b1;
constexpr auto ALS31300_MEAS_REG_NEW_DATA_not_avail = 0b0;

// --------
constexpr auto ALS31300_MEASUREMENTS_LSB_REG = 0x29;

typedef struct : als31300_reg
{
    using als31300_reg::als31300_reg;
    using als31300_reg::operator=;
    uint8_t temperature_LSB : 6;
    bool hall_mode_status : 2;
    uint8_t Z_LSB : 4;
    uint8_t Y_LSB : 4;
    uint8_t X_LSB : 4;
    bool int_eeprom_write_pending : 1;
} als31300_measurements_LSB_reg;

float als31300_temperature_convert(uint16_t raw_temperature)
{
    const int32_t intermediate = raw_temperature - 1708;
    return intermediate * 0.0737;
}

// NOTE: device sensitivity HW fixed at 4 LSB/Gauss == 0.4 LSB/mT
// All measurements are supposed to be raw 4 LSB/Gauss. No need to introduce fractions
constexpr auto ALS31300_MEAS_REG_bit_length_full_scale = 12;
inline int16_t als31300_measurement_sign_convert(uint16_t raw_measurement,
                                                 uint8_t bit_length = ALS31300_MEAS_REG_bit_length_full_scale)
{
    // via: https://stackoverflow.com/questions/16946801/n-bit-2s-binary-to-decimal-in-c
    const auto sign_flag = 1 << (bit_length - 1);
    if (raw_measurement & sign_flag) {
        raw_measurement |= -(1 << bit_length);
    }
    return raw_measurement;
}

// --------
constexpr auto ALS31300_CUSTOMER_ACCESS_REG          = 0x35;
constexpr uint32_t ALS31300_CUSTOMER_ACCESS_REG_code = 0x2C413534;

/////////////////////
