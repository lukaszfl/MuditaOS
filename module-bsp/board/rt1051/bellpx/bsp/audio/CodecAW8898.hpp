// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "AW8898.hpp"
#include <bsp/audio/Codec.hpp>
#include "drivers/i2c/DriverI2C.hpp"
#include "drivers/gpio/DriverGPIO.hpp"

extern "C"
{
#include "fsl_common.h"
}

class CodecParamsAW8898 : public CodecParams
{
  public:
    enum class Cmd
    {
        SetOutVolume,
        SetInGain,
        SetMute,
        SetOutput,
        SetInput,
        Reset,
        MicBiasCtrl,
        None
    };

    enum class SampleRate
    {
        Rate8KHz   = 8000,
        Rate16KHz  = 16000,
        Rate44K1Hz = 44100,
        Rate48KHz  = 48000,
        Rate32KHz  = 32000,
        Rate96KHz  = 96000,
        Invalid
    };

    enum class MonoStereo
    {
        Mono,
        Stereoq
    };

    static SampleRate ValToSampleRate(uint32_t rate)
    {
        switch (rate) {
        case 8000:
            return SampleRate ::Rate8KHz;
        case 16000:
            return SampleRate::Rate16KHz;
        case 32000:
            return SampleRate::Rate32KHz;
        case 44100:
            return SampleRate::Rate44K1Hz;
        case 48000:
            return SampleRate::Rate48KHz;
        case 96000:
            return SampleRate::Rate96KHz;
        default:
            return SampleRate ::Invalid;
        }
    }

    uint32_t GetSampleRateVal()
    {
        switch (sampleRate) {
        case SampleRate::Rate8KHz:
            return 8000;
        case SampleRate::Rate16KHz:
            return 16000;
        case SampleRate::Rate32KHz:
            return 32000;
        case SampleRate::Rate44K1Hz:
            return 44100;
        case SampleRate::Rate48KHz:
            return 48000;
        case SampleRate::Rate96KHz:
            return 96000;
        default:
            return 0;
        }
    }

    Cmd opCmd             = Cmd::None;
    float outVolume       = 0;
    bool muteEnable       = false;
    bool resetEnable      = false;
    SampleRate sampleRate = SampleRate ::Rate44K1Hz;
};

class CodecAW8898 : public Codec
{
  public:
    CodecAW8898();
    ~CodecAW8898();

    std::optional<uint32_t> Probe() override final;

    CodecRetCode Start(const CodecParams &param) override final;

    CodecRetCode Pause() override final;

    CodecRetCode Resume() override final;

    CodecRetCode Stop() override final;

    CodecRetCode Ioctrl(const CodecParams &param) override final;

  private:
    std::shared_ptr<drivers::DriverI2C> i2c;
    std::shared_ptr<drivers::DriverGPIO> gpio;
    drivers::I2CAddress i2cAddr;
    CodecParamsAW8898 currentParams;

    CodecRetCode SetOutputVolume(const float vol);
    CodecRetCode SetMute(const bool enable);
    CodecRetCode Reset();

    //internal typedefs & variables
    typedef struct aw8898_reg_cfg_type
    {
        uint8_t  addr;
        uint16_t data;
    }aw8898_reg_cfg_t;

    typedef enum
    {
        AW8898_INIT_ST = 0,
        AW8898_INIT_OK = 1,
        AW8898_INIT_NG = 2,
    }aw8898_init_t;

    typedef struct _codec_config
    {
        uint32_t codecDevType; /*!< codec type */
        void *codecDevConfig;  /*!< Codec device specific configuration */
    } codec_config_t;

    struct aw8898
    {
    //	aw_hw_irq_handle_t irq_handle;
        aw8898_init_t init;
        aw_sel_mode_t mode;
    };

    struct aw8898 g_aw8898 =
    {
        .init =  AW8898_INIT_ST,
    //    .irq_handle = AW_HW_IRQ_HANDLE_OFF,
        .mode = OFF_MODE,
    };

    static constexpr const aw8898_reg_cfg_t  aw8898_reg_cfg[] =
    {
        {0x05, 0x0c03},
        {0x06, 0x0330},
        {0x09, 0x424a},
        {0x0a, 0x03c2},
        {0x0b, 0x03c2},
        {0x0c, 0x3007},
        {0x0d, 0x011b},
        {0x0e, 0x0329},
        {0x20, 0x0001},
        {0x60, 0x1cbc},
        {0x61, 0x0f0e},
        {0x62, 0xf5b6},
        {0x63, 0x307f},
        {0x67, 0x007c},
        {0x69, 0x0245},
        {0x04, 0x0044},
    };


    static constexpr uint8_t reg[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0f, 0x21, 0x60, 0x61, 0x62};

    //internal functions
    status_t AW8898_WriteReg(uint8_t reg, uint16_t val);
    status_t AW8898_ReadReg(uint8_t reg, uint16_t *val);
    status_t AW8898_ModifyReg(uint8_t reg, uint16_t mask, uint16_t val);

    status_t AW8898_HwReset(void);
    void AW8898_SoftReset(void);

    status_t AW8898_ReadChipid(void);
    status_t AW8898_Init(const aw8898_config_t *config);

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

    status_t AW8898_SetVolume(aw8898_module_t module, uint8_t gain);
    status_t AW8898_GetVolume(aw8898_module_t module, uint8_t *gain);

    uint8_t aw8898_init(void);
    uint8_t aw8898_deinit(void);
    bool aw8898_check_pll_status(void);
    int  aw8898_set_voice_volume(int volume);
    int  aw8898_get_voice_volume(int *volume);
    status_t AW8898_Start(void);

    status_t HAL_CODEC_Init(void *config);
    status_t HAL_CODEC_Deinit(void);
};
