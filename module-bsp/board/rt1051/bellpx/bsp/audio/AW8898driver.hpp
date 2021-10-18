// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "AW8898regs.hpp"
#include "drivers/i2c/DriverI2C.hpp"
#include "drivers/gpio/DriverGPIO.hpp"

extern "C"
{
#include "fsl_common.h"
}

namespace bsp::bell_audio
{
    extern std::shared_ptr<drivers::DriverI2C> i2c;
    extern std::shared_ptr<drivers::DriverGPIO> gpio;
    extern drivers::I2CAddress i2cAddr;

    constexpr auto LogicLow                 = 0;
    constexpr auto LogicHigh                = 1;
    constexpr auto TwoMilisecondWait        = 2;

    constexpr auto AW8898_CHIP_ID           = 0x1702;

    typedef enum
    {
        CHSEL_LEFT  = 0,
        CHSEL_RIGHT = 1,
        CHSEL_MONO  = 2,
    } aw_i2s_channel_t;

    typedef enum
    {
        FREQUENCY_08K = 0,
        FREQUENCY_11K = 1,
        FREQUENCY_16K = 2,
        FREQUENCY_22K = 3,
        FREQUENCY_24K = 4,
        FREQUENCY_32K = 5,
        FREQUENCY_44K = 6,
        FREQUENCY_48K = 7,

    } aw_i2s_frequency_t;

    typedef enum
    {
        WIDTH_16BITS = 0,
        WIDTH_24BITS = 1,
        WIDTH_32BITS = 2,

    } aw_i2s_width_t;

    typedef enum
    {
        I2SBCK_32FS = 0,
        I2SBCK_48FS = 1,
        I2SBCK_64FS = 2,
    } aw_i2s_fs_t;

    typedef enum
    {
        OFF_MODE   = 0,
        SPK_MODE   = 1,
        VOICE_MODE = 2,
        MUSIC_MODE = 3,
    } aw_sel_mode_t;

    typedef enum
    {
        AW_ENCODE       = 0,
        AW_DECODE       = 1,
        AW_MODE_BOTH    = 2,
        AW_MODE_LINE_IN = 3,
    } aw_codec_mode_t;

    typedef enum
    {
        STOP  = 0,
        START = 1,
    } aw_ctrl_t;

    status_t AW8898_WriteReg(uint8_t reg, uint16_t val);
    status_t AW8898_ReadReg(uint8_t reg, uint16_t *val);
    status_t AW8898_ModifyReg(uint8_t reg, uint16_t mask, uint16_t val);

    status_t AW8898_HwReset(void);
    void AW8898_SoftReset(void);

    status_t AW8898_ReadChipid(void);
    status_t AW8898_Init(const aw_i2s_channel_t chsel, aw_i2s_frequency_t rate);

    status_t AW8898_HwParams(aw_i2s_channel_t chsel, aw_i2s_frequency_t rate, aw_i2s_width_t width, aw_i2s_fs_t fs);
    status_t AW8898_RunPwd(bool pwd);
    status_t AW8898_RunMute(bool mute);
    void AW8898_LoadRegCfg(void);

    void AW8898_ColdStart(void);

    status_t AW8898_Stop(void);

    status_t AW8898_SmartpaCfg(bool play_flag);
    status_t AW8898_CtrlState(aw_codec_mode_t mode, aw_ctrl_t aw_ctrl);

    int AW8898_SetMode(aw_sel_mode_t mode);
    bool AW8898_CheckPllStatus(void);

    void AW8898_ReadAllReg(void);
    void HAL_Delay(uint32_t count);

    status_t AW8898_SetVolume(uint8_t gain);
    status_t AW8898_GetVolume(uint8_t *gain);

    status_t AW8898_Start(void);
};
