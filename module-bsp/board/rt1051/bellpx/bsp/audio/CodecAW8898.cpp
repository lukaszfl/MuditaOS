// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "CodecAW8898.hpp"
//#include "AW8898.hpp"
#include "board/BoardDefinitions.hpp"

#include <log/log.hpp>

extern "C"
{
#include "FreeRTOS.h"
#include "task.h"
}

using namespace drivers;
//using namespace bsp::bell_audio;

constexpr auto ReadStatusRetries        = 5;
constexpr auto OneByteAddressing        = 1;
constexpr auto PositiveLogic            = 0;
constexpr auto LogicLow                 = 0;
constexpr auto LogicHigh                = 1;
constexpr auto ReadWriteTwoBytes        = 2;
constexpr auto TwoMilisecondWait        = 2;
constexpr auto TenMilisecondWait        = 10;


    inline void FlipBytes(uint16_t *val)
    {
        uint16_t tmp = *val << 8;
        *val         = (*val >> 8) | tmp;
    }

    /*******************************************************************************
     * Definitations
     ******************************************************************************/
    #define AW8898_CHECK_RET(x, status)      \
        do                                   \
        {                                    \
            (status) = (x);                  \
            if (kStatus_Success != (status)) \
            {                                \
                return (status);             \
            }                                \
        } while (false)

    #define AW8898_CHIP_ID			0x1702
    #define AW8898_SOFT_RESET		0x55aa
    #define AW_TAG                  "aw8898"
    #define AW8898_VERSION 	        "v1.1.0"
    #define AW_I2C_RETRIES			5
    #define AW_I2C_RETRY_DELAY		2
    #define AW_READ_CHIPID_RETRIES	5
    #define MAX_RAM_WRITE_BYTE_SIZE 128
    #define AW_LOG_ENABLE           1
    #if AW_LOG_ENABLE
        #define AW_LOGI(...)  LOG_DEBUG(__VA_ARGS__)
    #else
        #define AW_LOGI(...)  ((void)0)
    #endif

CodecAW8898::CodecAW8898() : i2cAddr{}
{
    LOG_INFO("Initializing AW8898 audio codec");
    i2cAddr.deviceAddress  = AW8898_I2C_ADDR;
    i2cAddr.subAddressSize = OneByteAddressing; // AW8898 uses 1byte addressing
    i2c                    = DriverI2C::Create(
        static_cast<I2CInstances>(BoardDefinitions::AUDIOCODEC_I2C),
        DriverI2CParams{.baudrate = static_cast<uint32_t>(BoardDefinitions::AUDIOCODEC_I2C_BAUDRATE)});

    gpio = DriverGPIO::Create(static_cast<GPIOInstances>(BoardDefinitions::BELL_AUDIOCODEC_GPIO), DriverGPIOParams{});

    gpio->ConfPin(DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Output,
                                      .irqMode  = DriverGPIOPinParams::InterruptMode::NoIntmode,
                                      .defLogic = PositiveLogic,
                                      .pin = static_cast<uint32_t>(BoardDefinitions::BELL_AUDIOCODEC_RSTN_PA_PIN)});

    gpio->ConfPin(DriverGPIOPinParams{.dir      = DriverGPIOPinParams::Direction::Input,
                                      .irqMode  = DriverGPIOPinParams::InterruptMode::IntFallingEdge,
                                      .defLogic = PositiveLogic,
                                      .pin = static_cast<uint32_t>(BoardDefinitions::BELL_AUDIOCODEC_INTN_PA_PIN)});

    gpio->ClearPortInterrupts(1 << static_cast<uint32_t>(BoardDefinitions::BELL_AUDIOCODEC_INTN_PA_PIN));
    gpio->DisableInterrupt(1 << static_cast<uint32_t>(BoardDefinitions::BELL_AUDIOCODEC_INTN_PA_PIN));

    gpio->WritePin(static_cast<std::uint32_t>(BoardDefinitions::BELL_AUDIOCODEC_RSTN_PA_PIN), LogicLow); // reset chip
    vTaskDelay(pdMS_TO_TICKS(TwoMilisecondWait));
    gpio->WritePin(static_cast<std::uint32_t>(BoardDefinitions::BELL_AUDIOCODEC_RSTN_PA_PIN), LogicHigh); // clear reset
    vTaskDelay(pdMS_TO_TICKS(TwoMilisecondWait));

    LOG_DEBUG("Probing AW8898 ...");
    auto ret = Probe();
    LOG_DEBUG("AW8898 Probe: 0x%04lX", ret.value());
}

CodecAW8898::~CodecAW8898()
{
    Reset();
}

CodecRetCode CodecAW8898::Start(const CodecParams &param)
{
    CodecParamsAW8898 params;
    params.opCmd = static_cast<CodecParamsAW8898::Cmd>(param.opCmd);
    params.outVolume = param.outVolume;
    params.sampleRate = static_cast<CodecParamsAW8898::SampleRate>(param.sampleRate);

    AW8898_Init(params);

    // Store param configuration
    currentParams = params;

    auto currVol = currentParams.outVolume;
    currVol = 2.0;
    SetOutputVolume(currVol);

    AW8898_ReadAllReg();

    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::Pause()
{
    AW8898_Stop();
    
    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::Resume()
{
    AW8898_Start();

    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::Stop()
{
    AW8898_Stop();
    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::Ioctrl(const CodecParams &param)
{

    const CodecParamsAW8898 &params = static_cast<const CodecParamsAW8898 &>(param);

    CodecRetCode ret = CodecRetCode::Success;

    switch (params.opCmd) {
    case CodecParamsAW8898::Cmd::SetOutVolume:
        ret = SetOutputVolume(params.outVolume);
        break;

    case CodecParamsAW8898::Cmd::SetInGain:
        ret = CodecRetCode::Success;
        break;
    case CodecParamsAW8898::Cmd::SetInput:
        ret = CodecRetCode::Success;
        break;
    case CodecParamsAW8898::Cmd::SetOutput:
        ret = CodecRetCode::Success;
        break;
    case CodecParamsAW8898::Cmd::MicBiasCtrl:
        ret = CodecRetCode::Success;
        break;
    case CodecParamsAW8898::Cmd::Reset:
        ret = Reset();
        break;
    case CodecParamsAW8898::Cmd::SetMute:
        ret = SetMute(true);
        break;
    default:
        break;
    }

    return ret;
}

CodecRetCode CodecAW8898::SetOutputVolume(const float vol)
{
    uint8_t mute = 0;

    // If volume set to 0 then mute output
     if (vol == 0)
         AW8898_RunMute(true); //(PWMCTRL.HMUTE=1) - enable mute
     else
         AW8898_RunMute(false); //(PWMCTRL.HMUTE=0) - disable mute


    AW8898_SetVolume(static_cast<uint8_t>(25.5 * vol));
    currentParams.outVolume = vol;
    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::Reset()
{
    AW8898_HwReset();
    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::SetMute(const bool enable)
{
    AW8898_RunMute(enable);
    return CodecRetCode::Success;
}

std::optional<uint32_t> CodecAW8898::Probe()
{
    if (AW8898_ReadChipid() == kStatus_Success)
        return AW8898_CHIP_ID;
    else
        return 0;
}

/*******************************************************************************
     * Code
     ******************************************************************************/
    status_t CodecAW8898::AW8898_WriteReg(uint8_t reg, uint16_t val)
    {
        i2cAddr.subAddress           = reg;
        uint16_t tval = val;
        FlipBytes(reinterpret_cast<uint16_t *>(&tval));
        auto sent = i2c->Write(i2cAddr, reinterpret_cast<uint8_t *>(&tval), ReadWriteTwoBytes);
        if (sent == ReadWriteTwoBytes)
            return kStatus_Success;
        else
            return kStatus_Fail;
    }

    status_t CodecAW8898::AW8898_ReadReg(uint8_t reg, uint16_t *val)
    {
        uint16_t tval;

        i2cAddr.subAddress           = reg;

        auto received = i2c->Read(i2cAddr, reinterpret_cast<uint8_t *>(&tval), ReadWriteTwoBytes);
        if (received != ReadWriteTwoBytes)
            return kStatus_Fail;

        FlipBytes(reinterpret_cast<uint16_t *>(&tval));
        *val = tval;

        return kStatus_Success;
    }

    status_t CodecAW8898::AW8898_ModifyReg(uint8_t reg, uint16_t mask, uint16_t val)
    {
        status_t retval  = 0;
        uint16_t reg_val = 0;
        retval           = AW8898_ReadReg(reg, &reg_val);
        if (retval != kStatus_Success)
        {
            return kStatus_Fail;
        }
        reg_val &= (uint16_t)mask;
        reg_val |= val;
        retval = AW8898_WriteReg(reg, reg_val);
        if (retval != kStatus_Success)
        {
            return kStatus_Fail;
        }
        return kStatus_Success;
    }

    status_t CodecAW8898::AW8898_HwReset(void)
    {
        AW_LOGI("%s enter ", __func__);

        gpio->WritePin(static_cast<std::uint32_t>(BoardDefinitions::BELL_AUDIOCODEC_RSTN_PA_PIN), LogicLow); // reset chip
        vTaskDelay(pdMS_TO_TICKS(TwoMilisecondWait));
        gpio->WritePin(static_cast<std::uint32_t>(BoardDefinitions::BELL_AUDIOCODEC_RSTN_PA_PIN), LogicHigh); // clear reset
        vTaskDelay(pdMS_TO_TICKS(TwoMilisecondWait));

        return kStatus_Success;
    }

    void CodecAW8898::AW8898_SoftReset(void)
    {
        AW_LOGI("enter %s start", __func__);

        AW8898_WriteReg(AW8898_REG_ID, AW8898_SOFT_RESET);

        AW_LOGI("enter %s end", __func__);

    }

    status_t CodecAW8898::AW8898_ReadChipid(void)
    {
        uint8_t cnt = 0;
        uint16_t reg_val = 0;

        while (cnt < AW_READ_CHIPID_RETRIES)
        {
        AW8898_ReadReg(AW8898_REG_ID, &reg_val);
        if (reg_val == AW8898_CHIP_ID)
        {
            AW_LOGI("this chip is Aw8898 chipid=0x%x",reg_val);
            return kStatus_Success;
        }
        AW_LOGI("%s: aw8898 chipid=0x%x error\n",__func__, reg_val);
        cnt++;
        HAL_Delay(2);
        }

        return kStatus_Fail;
    }

    status_t CodecAW8898::AW8898_Init(const CodecParamsAW8898 &params)
    {
        status_t ret = kStatus_Success;

        AW_LOGI("enter %s start", __func__);

        AW8898_HwReset();
	    HAL_Delay(2);

        ret = AW8898_ReadChipid();
        if (ret == kStatus_Fail)
        {
            AW_LOGI("please check hardward ad_pin && i2c config!!!");
            return ret;
        }

        AW8898_RunPwd(false);
        HAL_Delay(2);

        AW8898_SetMode(SPK_MODE);

        AW8898_HwParams(params.MonoStereoToCodecChsel(), params.SampleRateToCodecFreq(), WIDTH_16BITS, I2SBCK_32FS);
        HAL_Delay(2);

        AW8898_CtrlState(AW_DECODE, START);

        AW8898_Start();

        AW8898_ReadAllReg();

        AW_LOGI("enter %s end", __func__);

        return ret;
    }

    status_t CodecAW8898::AW8898_HwParams(aw_i2s_channel_t chsel, aw_i2s_frequency_t rate, aw_i2s_width_t width, aw_i2s_fs_t fs)
    {
        int reg_value = 0;
        AW_LOGI("enter %s start", __func__);

        AW_LOGI("[chsel]=%i, [rate]=%i", static_cast<int>(chsel), static_cast<int>(rate));
        
        switch (chsel)
        {
        case CHSEL_LEFT:
        reg_value = AW8898_BIT_I2SCTRL_CHS_LEFT;
        break;
        case CHSEL_RIGHT:
        reg_value = AW8898_BIT_I2SCTRL_CHS_RIGHT;
        break;
        case CHSEL_MONO:
        reg_value = AW8898_BIT_I2SCTRL_CHS_MONO;
        break;
        default:
        reg_value = AW8898_BIT_I2SCTRL_CHS_MONO;
        AW_LOGI("%s: chsel can not support ", __func__);
        break;
        }
        //set chsel
        AW8898_ModifyReg(AW8898_REG_I2SCTRL, AW8898_BIT_I2SCTRL_CHS_MASK, reg_value);

        switch (rate)
        {
        case FREQUENCY_08K:
            reg_value =  AW8898_BIT_I2SCTRL_SR_8K;
            break;
        case FREQUENCY_11K:
            reg_value =  AW8898_BIT_I2SCTRL_SR_11K;
            break;
        case FREQUENCY_16K:
            reg_value = AW8898_BIT_I2SCTRL_SR_16K;
            break;
        case FREQUENCY_22K:
            reg_value = AW8898_BIT_I2SCTRL_SR_22K;
            break;
        case FREQUENCY_24K:
            reg_value = AW8898_BIT_I2SCTRL_SR_24K;
            break;
        case FREQUENCY_32K:
            reg_value = AW8898_BIT_I2SCTRL_SR_32K;
            break;
        case FREQUENCY_44K:
            reg_value = AW8898_BIT_I2SCTRL_SR_44P1K;
            break;
        case FREQUENCY_48K:
            reg_value = AW8898_BIT_I2SCTRL_SR_48K;
            break;
        default:
            reg_value = AW8898_BIT_I2SCTRL_SR_48K;
            AW_LOGI("%s: rate can not support", __func__);
            break;
        }
        //set rate
        AW8898_ModifyReg(AW8898_REG_I2SCTRL, AW8898_BIT_I2SCTRL_SR_MASK, reg_value);

        switch (width)
        {
        case WIDTH_16BITS:
            reg_value = AW8898_BIT_I2SCTRL_FMS_16BIT;
            break;
        case WIDTH_24BITS:
            reg_value = AW8898_BIT_I2SCTRL_FMS_24BIT;
            break;
        case WIDTH_32BITS:
            reg_value = AW8898_BIT_I2SCTRL_FMS_32BIT;
            break;
        default:
            reg_value = AW8898_BIT_I2SCTRL_FMS_16BIT;
            AW_LOGI("%s: width can not support ", __func__);
            break;
        }
        //set width
        AW8898_ModifyReg(AW8898_REG_I2SCTRL, AW8898_BIT_I2SCTRL_FMS_MASK, reg_value);

        switch (fs)
        {
        case I2SBCK_32FS:
            reg_value = AW8898_BIT_I2SCTRL_BCK_32FS;
            break;
        case I2SBCK_48FS:
            reg_value = AW8898_BIT_I2SCTRL_BCK_48FS;
            break;
        case I2SBCK_64FS:
            reg_value = AW8898_BIT_I2SCTRL_BCK_64FS;
            break;
        default:
            reg_value = AW8898_BIT_I2SCTRL_BCK_64FS;
            AW_LOGI("%s: fs can not support ", __func__);
            break;
        }
        //set fs
        AW8898_ModifyReg(AW8898_REG_I2SCTRL, AW8898_BIT_I2SCTRL_BCK_MASK, reg_value);

        return kStatus_Success;
    }

    void CodecAW8898::AW8898_LoadRegCfg(void)
    {
        uint16_t i = 0;
        uint16_t data = 0;

        AW_LOGI("enter %s start", __func__);

        for (i = 0; i < sizeof(aw8898_reg_cfg) / sizeof(aw8898_reg_cfg[0]); i ++)
        {
            AW8898_WriteReg(aw8898_reg_cfg[i].addr, aw8898_reg_cfg[i].data);
        }

        AW_LOGI("enter %s end", __func__);
    }

    status_t CodecAW8898::AW8898_RunPwd(bool pwd)
    {
        AW_LOGI("enter %s start [%s]", __func__, pwd == true ? "TRUE" : "FALSE");

        if (pwd)
        {
            AW8898_ModifyReg(AW8898_REG_SYSCTRL, AW8898_BIT_SYSCTRL_PW_MASK, AW8898_BIT_SYSCTRL_PW_PDN);
        }
        else
        {
            AW8898_ModifyReg(AW8898_REG_SYSCTRL, AW8898_BIT_SYSCTRL_PW_MASK, AW8898_BIT_SYSCTRL_PW_ACTIVE);
            AW8898_ModifyReg(AW8898_REG_SYSCTRL, AW8898_BIT_SYSCTRL_I2SEN_MASK, AW8898_BIT_SYSCTRL_I2S_ENABLE);
        }

        AW_LOGI("enter %s end", __func__);
        return kStatus_Success;
    }

    status_t CodecAW8898::AW8898_RunMute(bool mute)
    {
        AW_LOGI("enter %s start [%s]", __func__, mute == true ? "TRUE" : "FALSE");

        if (mute)
        {
            AW8898_ModifyReg(AW8898_REG_PWMCTRL, AW8898_BIT_PWMCTRL_HMUTE_MASK, AW8898_BIT_PWMCTRL_HMUTE_ENABLE);
        }
        else
        {
            AW8898_ModifyReg(AW8898_REG_PWMCTRL, AW8898_BIT_PWMCTRL_HMUTE_MASK, AW8898_BIT_PWMCTRL_HMUTE_DISABLE);
        }

        AW_LOGI("enter %s end", __func__);
        return kStatus_Success;
    }

    void CodecAW8898::AW8898_ColdStart(void)
    {
        AW_LOGI("%s: enter", __func__);

        AW8898_LoadRegCfg();
        g_aw8898.init = AW8898_INIT_OK;

        if ((g_aw8898.mode == SPK_MODE) || (g_aw8898.mode == MUSIC_MODE))
        {
            AW_LOGI("SPK MODE");
            AW8898_ModifyReg(AW8898_REG_SYSCTRL, AW8898_BIT_SYSCTRL_MODE_MASK, AW8898_BIT_SYSCTRL_SPK_MODE);
        }
        else
        {
            AW_LOGI("RCV MODE");
            AW8898_ModifyReg(AW8898_REG_SYSCTRL, AW8898_BIT_SYSCTRL_MODE_MASK, AW8898_BIT_SYSCTRL_RCV_MODE);
        }
    }

    status_t CodecAW8898::AW8898_Start(void)
    {
        uint16_t reg_val = 0;
        unsigned int i = 0;

        AW_LOGI("enter %s start", __func__);

        AW8898_RunPwd(false);
        HAL_Delay(2);

        for (i = 0; i < 5; i ++)
        {
            AW8898_ReadReg(AW8898_REG_SYSST, &reg_val);
            if ((reg_val & AW8898_BIT_SYSST_PLLS))
            {
                AW8898_RunMute(false);
                AW_LOGI("%s iis signal check pass!", __func__);
                return kStatus_Success;
            }

            HAL_Delay(2);
        }
        AW8898_RunPwd(true);

        AW_LOGI("%s: iis signal check error[0x%04X]", __func__, reg_val);

        return kStatus_Fail;
    }

    status_t CodecAW8898::AW8898_Stop(void)
    {
        AW_LOGI("enter %s start", __func__);
        AW8898_RunMute(true);

        AW_LOGI("enter %s end", __func__);
        AW8898_RunPwd(true);

        return kStatus_Success;
    }

    status_t CodecAW8898::AW8898_SmartpaCfg(bool play_flag)
    {
        int ret = -1;

        AW_LOGI("%s: flag = %d", __func__, play_flag);

        if (play_flag == true && g_aw8898.mode != OFF_MODE)
        {
        if ((g_aw8898.init == AW8898_INIT_ST) || (g_aw8898.init == AW8898_INIT_NG))
        {
            AW_LOGI("%s: init = %d", __func__, g_aw8898.init);
            AW8898_ColdStart();
        }
        else
        {
            ret = AW8898_Start();
            AW_LOGI("%s: init = %d", __func__, g_aw8898.init);
            if (ret < 0)
                AW_LOGI("%s: start fail, ret=%d\n", __func__, ret);
            else
                AW_LOGI("%s: start success", __func__);
        }
        }
        else
        {
        AW8898_Stop();
        }

        return kStatus_Success;
    }

    status_t CodecAW8898::AW8898_CtrlState(aw_codec_mode_t mode, aw_ctrl_t aw_ctrl)
    {
        AW_LOGI("enter %s  start", __func__);

        switch (mode)
        {
            case AW_ENCODE:
            break;
            case AW_DECODE:
            if (aw_ctrl == STOP)
            {
                AW8898_SmartpaCfg(false);
            }
            else
            {
                AW8898_SmartpaCfg(true);
            }
            break;
            case AW_MODE_BOTH:
            break;
            case AW_MODE_LINE_IN:
            break;
            default:
            break;
        }

        AW_LOGI("enter %s end", __func__);

        return kStatus_Success;
    }

    status_t CodecAW8898::AW8898_SetVolume(uint8_t gain)
    {
        status_t res = kStatus_Success;
        uint16_t reg = 0, reg_val = 0;

        AW_LOGI("enter %s start", __func__);

        res = AW8898_ReadReg(AW8898_REG_HAGCCFG7, &reg);
        if (res != 0)
        {
            AW_LOGI("reg read err(%ld)", res);
            return res;
        }

        reg_val = (gain << 8) | (reg & 0x00ff);
        res = AW8898_WriteReg(AW8898_REG_HAGCCFG7, reg_val);
        LOG_DEBUG("Vol 0x%04X", reg_val);

        AW_LOGI("enter %s end", __func__);
        return res;
    }

    status_t CodecAW8898::AW8898_GetVolume(uint8_t *gian)
    {
        status_t res = kStatus_Success;
        uint16_t reg = 0;

        AW_LOGI("enter %s start", __func__);

        res = AW8898_ReadReg(AW8898_REG_HAGCCFG7, &reg);
        if (res != kStatus_Success)
        {
            AW_LOGI("reg read err(%ld)", res);
            return res;
        }

        *gian = reg >> 8;

        AW_LOGI("enter %s end", __func__);
        return res;
    }

    int CodecAW8898::AW8898_SetMode(aw_sel_mode_t mode)
    {
        AW_LOGI("enter %s start", __func__);

        if ((mode == SPK_MODE) && (g_aw8898.mode != mode))
        {
            g_aw8898.mode = SPK_MODE;
            g_aw8898.init =  AW8898_INIT_ST;
        }
        else if((mode == VOICE_MODE) && (g_aw8898.mode != mode))
        {
            g_aw8898.mode = VOICE_MODE;
            g_aw8898.init =  AW8898_INIT_ST;
        }
        else if((mode == MUSIC_MODE) && (g_aw8898.mode != mode))
        {
            g_aw8898.mode = MUSIC_MODE;
            g_aw8898.init =  AW8898_INIT_ST;
        }
        else if (mode == OFF_MODE)
        {
            g_aw8898.mode = OFF_MODE;
        }
        else
        {
            AW_LOGI("enter %s mode not change!", __func__);
        }
        return 0;
    }



    bool CodecAW8898::AW8898_CheckPllStatus(void)
    {
        uint16_t reg_val = 0;

        AW8898_ReadReg(AW8898_REG_SYSST, &reg_val);

        if ((reg_val & AW8898_BIT_SYSST_PLLS))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void CodecAW8898::HAL_Delay(uint32_t count)
    {
        extern uint32_t SystemCoreClock;

        //SDK_DelayAtLeastUs(count * 1000, SystemCoreClock);
        vTaskDelay(pdMS_TO_TICKS(count));
    }

    void CodecAW8898::AW8898_ReadAllReg(void)
    {
        uint16_t val = 0;
        for(uint8_t i = 0; i< sizeof(reg); i++)
        {
            AW8898_ReadReg(reg[i], &val);
            AW_LOGI("reg[%x] = 0x%x", reg[i], val);
        }
    }
