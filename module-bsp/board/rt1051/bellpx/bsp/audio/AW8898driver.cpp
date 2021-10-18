// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "AW8898driver.hpp"
#include "board/BoardDefinitions.hpp"
#include <log/log.hpp>

extern "C"
{
#include "FreeRTOS.h"
#include "task.h"
}

namespace bsp::bell_audio
{
    std::shared_ptr<drivers::DriverI2C> i2c;
    std::shared_ptr<drivers::DriverGPIO> gpio;
    drivers::I2CAddress i2cAddr;

    inline void FlipBytes(uint16_t *val)
    {
        uint16_t tmp = *val << 8;
        *val         = (*val >> 8) | tmp;
    }

    constexpr auto AW8898_SOFT_RESET        = 0x55aa;
    constexpr auto AW_READ_CHIPID_RETRIES   = 5;
    constexpr auto ReadWriteTwoBytes        = 2;

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

    struct aw8898
    {
        aw8898_init_t init;
        aw_sel_mode_t mode;
    };

    struct aw8898 g_aw8898 =
    {
        .init =  AW8898_INIT_ST,
        .mode = OFF_MODE,
    };

    static constexpr const aw8898_reg_cfg_t  aw8898_reg_cfg[] =
    {
        {0x06, 0x0330},
        {0x08, 0xa00e},
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

    /*******************************************************************************
     * Code
     ******************************************************************************/
    status_t AW8898_WriteReg(uint8_t reg, uint16_t val)
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

    status_t AW8898_ReadReg(uint8_t reg, uint16_t *val)
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

    status_t AW8898_ModifyReg(uint8_t reg, uint16_t mask, uint16_t val)
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

    status_t AW8898_HwReset(void)
    {
        LOG_DEBUG("%s enter ", __func__);

        gpio->WritePin(static_cast<std::uint32_t>(BoardDefinitions::BELL_AUDIOCODEC_RSTN_PA_PIN), LogicLow); // reset chip
        vTaskDelay(pdMS_TO_TICKS(TwoMilisecondWait));
        gpio->WritePin(static_cast<std::uint32_t>(BoardDefinitions::BELL_AUDIOCODEC_RSTN_PA_PIN), LogicHigh); // clear reset
        vTaskDelay(pdMS_TO_TICKS(TwoMilisecondWait));

        return kStatus_Success;
    }

    void AW8898_SoftReset(void)
    {
        LOG_DEBUG("enter %s start", __func__);

        AW8898_WriteReg(AW8898_REG_ID, AW8898_SOFT_RESET);

        LOG_DEBUG("enter %s end", __func__);

    }

    status_t AW8898_ReadChipid(void)
    {
        uint8_t cnt = 0;
        uint16_t reg_val = 0;

        while (cnt < AW_READ_CHIPID_RETRIES)
        {
        AW8898_ReadReg(AW8898_REG_ID, &reg_val);
        if (reg_val == AW8898_CHIP_ID)
        {
            LOG_DEBUG("this chip is Aw8898 chipid=0x%x",reg_val);
            return kStatus_Success;
        }
        LOG_DEBUG("%s: aw8898 chipid=0x%x error\n",__func__, reg_val);
        cnt++;
        HAL_Delay(2);
        }

        return kStatus_Fail;
    }

    status_t AW8898_Init(const aw_i2s_channel_t chsel, aw_i2s_frequency_t rate)
    {
        status_t ret = kStatus_Success;

        LOG_DEBUG("enter %s start", __func__);

        AW8898_HwReset();
	    HAL_Delay(2);

        ret = AW8898_ReadChipid();
        if (ret == kStatus_Fail)
        {
            LOG_DEBUG("please check hardward ad_pin && i2c config!!!");
            return ret;
        }

        AW8898_RunPwd(false);
        HAL_Delay(2);

        AW8898_SetMode(SPK_MODE);

        AW8898_HwParams(chsel, rate, WIDTH_16BITS, I2SBCK_32FS);
        HAL_Delay(2);

        AW8898_CtrlState(AW_DECODE, START);

        AW8898_Start();

        AW8898_ReadAllReg();

        LOG_DEBUG("enter %s end", __func__);

        return ret;
    }

    status_t AW8898_HwParams(aw_i2s_channel_t chsel, aw_i2s_frequency_t rate, aw_i2s_width_t width, aw_i2s_fs_t fs)
    {
        int reg_value = 0;
        LOG_DEBUG("enter %s start", __func__);

        LOG_DEBUG("[chsel]=%i, [rate]=%i", static_cast<int>(chsel), static_cast<int>(rate));
        
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
        LOG_DEBUG("%s: chsel can not support ", __func__);
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
            LOG_DEBUG("%s: rate can not support", __func__);
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
            LOG_DEBUG("%s: width can not support ", __func__);
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
            LOG_DEBUG("%s: fs can not support ", __func__);
            break;
        }
        //set fs
        AW8898_ModifyReg(AW8898_REG_I2SCTRL, AW8898_BIT_I2SCTRL_BCK_MASK, reg_value);

        return kStatus_Success;
    }

    void AW8898_LoadRegCfg(void)
    {
        uint16_t i = 0;
        uint16_t data = 0;

        LOG_DEBUG("enter %s start", __func__);

        for (i = 0; i < sizeof(aw8898_reg_cfg) / sizeof(aw8898_reg_cfg[0]); i ++)
        {
            AW8898_WriteReg(aw8898_reg_cfg[i].addr, aw8898_reg_cfg[i].data);
        }

        LOG_DEBUG("enter %s end", __func__);
    }

    status_t AW8898_RunPwd(bool pwd)
    {
        LOG_DEBUG("enter %s start [%s]", __func__, pwd == true ? "TRUE" : "FALSE");

        if (pwd)
        {
            AW8898_ModifyReg(AW8898_REG_SYSCTRL, AW8898_BIT_SYSCTRL_PW_MASK, AW8898_BIT_SYSCTRL_PW_PDN);
        }
        else
        {
            AW8898_ModifyReg(AW8898_REG_SYSCTRL, AW8898_BIT_SYSCTRL_PW_MASK, AW8898_BIT_SYSCTRL_PW_ACTIVE);
            AW8898_ModifyReg(AW8898_REG_SYSCTRL, AW8898_BIT_SYSCTRL_I2SEN_MASK, AW8898_BIT_SYSCTRL_I2S_ENABLE);
        }

        LOG_DEBUG("enter %s end", __func__);
        return kStatus_Success;
    }

    status_t AW8898_RunMute(bool mute)
    {
        LOG_DEBUG("enter %s start [%s]", __func__, mute == true ? "TRUE" : "FALSE");

        if (mute)
        {
            AW8898_ModifyReg(AW8898_REG_PWMCTRL, AW8898_BIT_PWMCTRL_HMUTE_MASK, AW8898_BIT_PWMCTRL_HMUTE_ENABLE);
        }
        else
        {
            AW8898_ModifyReg(AW8898_REG_PWMCTRL, AW8898_BIT_PWMCTRL_HMUTE_MASK, AW8898_BIT_PWMCTRL_HMUTE_DISABLE);
        }

        LOG_DEBUG("enter %s end", __func__);
        return kStatus_Success;
    }

    void AW8898_ColdStart(void)
    {
        LOG_DEBUG("%s: enter", __func__);

        AW8898_LoadRegCfg();
        g_aw8898.init = AW8898_INIT_OK;

        if ((g_aw8898.mode == SPK_MODE) || (g_aw8898.mode == MUSIC_MODE))
        {
            LOG_DEBUG("SPK MODE");
            AW8898_ModifyReg(AW8898_REG_SYSCTRL, AW8898_BIT_SYSCTRL_MODE_MASK, AW8898_BIT_SYSCTRL_SPK_MODE);
        }
        else
        {
            LOG_DEBUG("RCV MODE");
            AW8898_ModifyReg(AW8898_REG_SYSCTRL, AW8898_BIT_SYSCTRL_MODE_MASK, AW8898_BIT_SYSCTRL_RCV_MODE);
        }
    }

    status_t AW8898_Start(void)
    {
        uint16_t reg_val = 0;
        unsigned int i = 0;

        LOG_DEBUG("enter %s start", __func__);

        AW8898_RunPwd(false);
        HAL_Delay(2);

        for (i = 0; i < 5; i ++)
        {
            AW8898_ReadReg(AW8898_REG_SYSST, &reg_val);
            if ((reg_val & AW8898_BIT_SYSST_PLLS))
            {
                AW8898_RunMute(false);
                LOG_DEBUG("%s iis signal check pass!", __func__);
                return kStatus_Success;
            }

            HAL_Delay(2);
        }
        AW8898_RunPwd(true);

        LOG_DEBUG("%s: iis signal check error[0x%04X]", __func__, reg_val);

        return kStatus_Fail;
    }

    status_t AW8898_Stop(void)
    {
        LOG_DEBUG("enter %s start", __func__);
        AW8898_RunMute(true);

        LOG_DEBUG("enter %s end", __func__);
        AW8898_RunPwd(true);

        return kStatus_Success;
    }

    status_t AW8898_SmartpaCfg(bool play_flag)
    {
        int ret = -1;

        LOG_DEBUG("%s: flag = %d", __func__, play_flag);

        if (play_flag == true && g_aw8898.mode != OFF_MODE)
        {
        if ((g_aw8898.init == AW8898_INIT_ST) || (g_aw8898.init == AW8898_INIT_NG))
        {
            LOG_DEBUG("%s: init = %d", __func__, g_aw8898.init);
            AW8898_ColdStart();
        }
        else
        {
            ret = AW8898_Start();
            LOG_DEBUG("%s: init = %d", __func__, g_aw8898.init);
            if (ret < 0)
                LOG_DEBUG("%s: start fail, ret=%d\n", __func__, ret);
            else
                LOG_DEBUG("%s: start success", __func__);
        }
        }
        else
        {
        AW8898_Stop();
        }

        return kStatus_Success;
    }

    status_t AW8898_CtrlState(aw_codec_mode_t mode, aw_ctrl_t aw_ctrl)
    {
        LOG_DEBUG("enter %s  start", __func__);

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

        LOG_DEBUG("enter %s end", __func__);

        return kStatus_Success;
    }

    status_t AW8898_SetVolume(uint8_t gain)
    {
        status_t res = kStatus_Success;
        uint16_t reg = 0, reg_val = 0;

        LOG_DEBUG("enter %s start", __func__);

        res = AW8898_ReadReg(AW8898_REG_HAGCCFG7, &reg);
        if (res != 0)
        {
            LOG_DEBUG("reg read err(%ld)", res);
            return res;
        }

        reg_val = (gain << 8) | (reg & 0x00ff);
        res = AW8898_WriteReg(AW8898_REG_HAGCCFG7, reg_val);
        LOG_DEBUG("Vol 0x%04X", reg_val);

        LOG_DEBUG("enter %s end", __func__);
        return res;
    }

    status_t AW8898_GetVolume(uint8_t *gian)
    {
        status_t res = kStatus_Success;
        uint16_t reg = 0;

        LOG_DEBUG("enter %s start", __func__);

        res = AW8898_ReadReg(AW8898_REG_HAGCCFG7, &reg);
        if (res != kStatus_Success)
        {
            LOG_DEBUG("reg read err(%ld)", res);
            return res;
        }

        *gian = reg >> 8;

        LOG_DEBUG("enter %s end", __func__);
        return res;
    }

    int AW8898_SetMode(aw_sel_mode_t mode)
    {
        LOG_DEBUG("enter %s start", __func__);

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
            LOG_DEBUG("enter %s mode not change!", __func__);
        }
        return 0;
    }



    bool AW8898_CheckPllStatus(void)
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

    void HAL_Delay(uint32_t count)
    {
        extern uint32_t SystemCoreClock;

        //SDK_DelayAtLeastUs(count * 1000, SystemCoreClock);
        vTaskDelay(pdMS_TO_TICKS(count));
    }

    void AW8898_ReadAllReg(void)
    {
        uint16_t val = 0;
        for(uint8_t i = 0; i< sizeof(reg); i++)
        {
            AW8898_ReadReg(reg[i], &val);
            LOG_DEBUG("reg[%x] = 0x%x", reg[i], val);
        }
    }

};