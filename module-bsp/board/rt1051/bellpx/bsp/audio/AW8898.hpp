// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <memory>

extern "C"
{
#include "fsl_common.h"
}


/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/********************************************
 * Register List
 *******************************************/
#define AW8898_REG_ID           0x00
#define AW8898_REG_SYSST        0x01
#define AW8898_REG_SYSINT       0x02
#define AW8898_REG_SYSINTM      0x03
#define AW8898_REG_SYSCTRL      0x04
#define AW8898_REG_I2SCTRL      0x05
#define AW8898_REG_I2STXCFG     0x06
#define AW8898_REG_PWMCTRL      0x08
#define AW8898_REG_HAGCCFG1     0x09
#define AW8898_REG_HAGCCFG2     0x0A
#define AW8898_REG_HAGCCFG3     0x0B
#define AW8898_REG_HAGCCFG4     0x0C
#define AW8898_REG_HAGCCFG5     0x0D
#define AW8898_REG_HAGCCFG6     0x0E
#define AW8898_REG_HAGCCFG7     0x0F
#define AW8898_REG_HAGCST       0x10
#define AW8898_REG_DBGCTRL      0x20
#define AW8898_REG_I2SCFG       0x21
#define AW8898_REG_I2SSTAT      0x22
#define AW8898_REG_I2SCAPCNT    0x23
#define AW8898_REG_GENCTRL      0x60
#define AW8898_REG_BSTCTRL1     0x61
#define AW8898_REG_BSTCTRL2     0x62
#define AW8898_REG_PLLCTRL1     0x63
#define AW8898_REG_PLLCTRL2     0x64
#define AW8898_REG_TESTCTRL     0x65
#define AW8898_REG_AMPDBG1      0x66
#define AW8898_REG_AMPDBG2      0x67
#define AW8898_REG_BSTDBG1      0x68
#define AW8898_REG_CDACTRL1     0x69
#define AW8898_REG_CDACTRL2     0x6A
#define AW8898_REG_TESTCTRL2    0x6B

#define AW8898_REG_MAX          0x6F

/********************************************
 * Register Access
 *******************************************/
#define REG_NONE_ACCESS 0
#define REG_RD_ACCESS  1 << 0
#define REG_WR_ACCESS  1 << 1

/******************************************************
 * Register Detail
 *****************************************************/
// SYSST
#define AW8898_BIT_SYSST_UVLOS                      ( 1<<14)
#define AW8898_BIT_SYSST_ADPS                       ( 1<<13)
#define AW8898_BIT_SYSST_DSPS                       ( 1<<12)
#define AW8898_BIT_SYSST_BSTOCS                     ( 1<<11)
#define AW8898_BIT_SYSST_OVPS                       ( 1<<10)
#define AW8898_BIT_SYSST_BSTS                       ( 1<< 9)
#define AW8898_BIT_SYSST_SWS                        ( 1<< 8)
#define AW8898_BIT_SYSST_CLIPS                      ( 1<< 7)
#define AW8898_BIT_SYSST_WDS                        ( 1<< 6)
#define AW8898_BIT_SYSST_NOCLKS                     ( 1<< 5)
#define AW8898_BIT_SYSST_CLKS                       ( 1<< 4)
#define AW8898_BIT_SYSST_OCDS                       ( 1<< 3)
#define AW8898_BIT_SYSST_OTLS                       ( 1<< 2)
#define AW8898_BIT_SYSST_OTHS                       ( 1<< 1)
#define AW8898_BIT_SYSST_PLLS                       ( 1<< 0)

// SYSINT
#define AW8898_BIT_SYSINT_UVLOI                     ( 1<<14)
#define AW8898_BIT_SYSINT_ADPI                      ( 1<<13)
#define AW8898_BIT_SYSINT_DSPI                      ( 1<<12)
#define AW8898_BIT_SYSINT_BSTOCI                    ( 1<<11)
#define AW8898_BIT_SYSINT_OVPI                      ( 1<<10)
#define AW8898_BIT_SYSINT_BSTI                      ( 1<< 9)
#define AW8898_BIT_SYSINT_SWI                       ( 1<< 8)
#define AW8898_BIT_SYSINT_CLIPI                     ( 1<< 7)
#define AW8898_BIT_SYSINT_WDI                       ( 1<< 6)
#define AW8898_BIT_SYSINT_NOCLKI                    ( 1<< 5)
#define AW8898_BIT_SYSINT_CLKI                      ( 1<< 4)
#define AW8898_BIT_SYSINT_OCDI                      ( 1<< 3)
#define AW8898_BIT_SYSINT_OTLI                      ( 1<< 2)
#define AW8898_BIT_SYSINT_OTHI                      ( 1<< 1)
#define AW8898_BIT_SYSINT_PLLI                      ( 1<< 0)

// SYSINTM
#define AW8898_BIT_SYSINTM_UVLOM                    ( 1<<14)
#define AW8898_BIT_SYSINTM_ADPM                     ( 1<<13)
#define AW8898_BIT_SYSINTM_DSPM                     ( 1<<12)
#define AW8898_BIT_SYSINTM_BSTOCM                   ( 1<<11)
#define AW8898_BIT_SYSINTM_OVPM                     ( 1<<10)
#define AW8898_BIT_SYSINTM_BSTM                     ( 1<< 9)
#define AW8898_BIT_SYSINTM_SWM                      ( 1<< 8)
#define AW8898_BIT_SYSINTM_CLIPM                    ( 1<< 7)
#define AW8898_BIT_SYSINTM_WDM                      ( 1<< 6)
#define AW8898_BIT_SYSINTM_NOCLKM                   ( 1<< 5)
#define AW8898_BIT_SYSINTM_CLKM                     ( 1<< 4)
#define AW8898_BIT_SYSINTM_OCDM                     ( 1<< 3)
#define AW8898_BIT_SYSINTM_OTLM                     ( 1<< 2)
#define AW8898_BIT_SYSINTM_OTHM                     ( 1<< 1)
#define AW8898_BIT_SYSINTM_PLLM                     ( 1<< 0)

// SYSCTRL
#define AW8898_BIT_SYSCTRL_INTMODE_MASK             (~( 3<< 8))
#define AW8898_BIT_SYSCTRL_INT_HIGH_PP              ( 3<< 8)
#define AW8898_BIT_SYSCTRL_INT_LOW_PP               ( 2<< 8)
#define AW8898_BIT_SYSCTRL_INT_HIGH_OD              ( 1<< 8)
#define AW8898_BIT_SYSCTRL_INT_LOW_OD               ( 0<< 8)
#define AW8898_BIT_SYSCTRL_MODE_MASK                (~( 1<< 7))
#define AW8898_BIT_SYSCTRL_RCV_MODE                 ( 1<< 7)
#define AW8898_BIT_SYSCTRL_SPK_MODE                 ( 0<< 7)
#define AW8898_BIT_SYSCTRL_I2SEN_MASK               (~( 1<< 6))
#define AW8898_BIT_SYSCTRL_I2S_ENABLE               ( 1<< 6)
#define AW8898_BIT_SYSCTRL_I2S_DISABLE              ( 0<< 6)
#define AW8898_BIT_SYSCTRL_WSINV_MASK               (~( 1<< 5))
#define AW8898_BIT_SYSCTRL_WS_INVERT                ( 1<< 5)
#define AW8898_BIT_SYSCTRL_WS_NO_INVERT             ( 0<< 5)
#define AW8898_BIT_SYSCTRL_BCKINV_MASK              (~( 1<< 4))
#define AW8898_BIT_SYSCTRL_BCK_INVERT               ( 1<< 4)
#define AW8898_BIT_SYSCTRL_BCK_NO_INVERT            ( 0<< 4)
#define AW8898_BIT_SYSCTRL_IPLL_MASK                (~( 1<< 3))
#define AW8898_BIT_SYSCTRL_PLL_WORD                 ( 1<< 3)
#define AW8898_BIT_SYSCTRL_PLL_BIT                  ( 0<< 3)
#define AW8898_BIT_SYSCTRL_DSPBY_MASK               (~( 1<< 2))
#define AW8898_BIT_SYSCTRL_DSP_BYPASS               ( 1<< 2)
#define AW8898_BIT_SYSCTRL_DSP_WORK                 ( 0<< 2)
#define AW8898_BIT_SYSCTRL_CP_MASK                  (~( 1<< 1))
#define AW8898_BIT_SYSCTRL_CP_PDN                   ( 1<< 1)
#define AW8898_BIT_SYSCTRL_CP_ACTIVE                ( 0<< 1)
#define AW8898_BIT_SYSCTRL_PW_MASK                  (~( 1<< 0))
#define AW8898_BIT_SYSCTRL_PW_PDN                   ( 1<< 0)
#define AW8898_BIT_SYSCTRL_PW_ACTIVE                ( 0<< 0)

// I2SCTRL
#define AW8898_BIT_I2SCTRL_INPLEV_MASK              (~( 1<<13))
#define AW8898_BIT_I2SCTRL_INPLEV_0DB               ( 1<<13)
#define AW8898_BIT_I2SCTRL_INPLEV_NEG_6DB           ( 0<<13)
#define AW8898_BIT_I2SCTRL_STEREO_MASK              (~( 1<<12))
#define AW8898_BIT_I2SCTRL_STEREO_ENABLE            ( 1<<12)
#define AW8898_BIT_I2SCTRL_STEREO_DISABLE           ( 0<<12)
#define AW8898_BIT_I2SCTRL_CHS_MASK                 (~( 3<<10))
#define AW8898_BIT_I2SCTRL_CHS_MONO                 ( 3<<10)
#define AW8898_BIT_I2SCTRL_CHS_RIGHT                ( 2<<10)
#define AW8898_BIT_I2SCTRL_CHS_LEFT                 ( 1<<10)
#define AW8898_BIT_I2SCTRL_MD_MASK                  (~( 3<< 8))
#define AW8898_BIT_I2SCTRL_MD_LSB                   ( 2<< 8)
#define AW8898_BIT_I2SCTRL_MD_MSB                   ( 1<< 8)
#define AW8898_BIT_I2SCTRL_MD_STD                   ( 0<< 8)
#define AW8898_BIT_I2SCTRL_FMS_MASK                 (~( 3<< 6))
#define AW8898_BIT_I2SCTRL_FMS_32BIT                ( 3<< 6)
#define AW8898_BIT_I2SCTRL_FMS_24BIT                ( 2<< 6)
#define AW8898_BIT_I2SCTRL_FMS_20BIT                ( 1<< 6)
#define AW8898_BIT_I2SCTRL_FMS_16BIT                ( 0<< 6)
#define AW8898_BIT_I2SCTRL_BCK_MASK                 (~( 3<< 4))
#define AW8898_BIT_I2SCTRL_BCK_64FS                 ( 2<< 4)
#define AW8898_BIT_I2SCTRL_BCK_48FS                 ( 1<< 4)
#define AW8898_BIT_I2SCTRL_BCK_32FS                 ( 0<< 4)
#define AW8898_BIT_I2SCTRL_SR_MASK                  (~(15<< 0))
#define AW8898_BIT_I2SCTRL_SR_192K                  (10<< 0)
#define AW8898_BIT_I2SCTRL_SR_96K                   ( 9<< 0)
#define AW8898_BIT_I2SCTRL_SR_48K                   ( 8<< 0)
#define AW8898_BIT_I2SCTRL_SR_44P1K                 ( 7<< 0)
#define AW8898_BIT_I2SCTRL_SR_32K                   ( 6<< 0)
#define AW8898_BIT_I2SCTRL_SR_24K                   ( 5<< 0)
#define AW8898_BIT_I2SCTRL_SR_22K                   ( 4<< 0)
#define AW8898_BIT_I2SCTRL_SR_16K                   ( 3<< 0)
#define AW8898_BIT_I2SCTRL_SR_12K                   ( 2<< 0)
#define AW8898_BIT_I2SCTRL_SR_11K                   ( 1<< 0)
#define AW8898_BIT_I2SCTRL_SR_8K                    ( 0<< 0)


// I2STXCFG
#define AW8898_BIT_I2STXCFG_FSYNC_MASK              (~( 1<<15))
#define AW8898_BIT_I2STXCFG_FSYNC_BCK_CYCLE         ( 1<<15)
#define AW8898_BIT_I2STXCFG_FSYNC_ONE_SLOT          ( 0<<15)
#define AW8898_BIT_I2STXCFG_SLOT_NUM_MASK           (~( 1<<14))
#define AW8898_BIT_I2STXCFG_SLOT_NUM_4_TIMES        ( 1<<14)
#define AW8898_BIT_I2STXCFG_SLOT_NUM_2_TIMES        ( 0<<14)
#define AW8898_BIT_I2STXCFG_TX_SLOT_VLD_MASK        (~(15<<12))
#define AW8898_BIT_I2STXCFG_TX_SLOT_VLD_3           ( 3<<12)
#define AW8898_BIT_I2STXCFG_TX_SLOT_VLD_2           ( 2<<12)
#define AW8898_BIT_I2STXCFG_TX_SLOT_VLD_1           ( 1<<12)
#define AW8898_BIT_I2STXCFG_TX_SLOT_VLD_0           ( 0<<12)
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_MASK        (~(15<< 8))
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_3_2         (12<< 8)
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_3_1         (10<< 8)
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_3_0         ( 9<< 8)
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_2_1         ( 6<< 8)
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_2_0         ( 5<< 8)
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_1_0         ( 3<< 8)
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_3           ( 8<< 8)
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_2           ( 4<< 8)
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_1           ( 2<< 8)
#define AW8898_BIT_I2STXCFG_RX_SLOT_VLD_0           ( 1<< 8)
#define AW8898_BIT_I2STXCFG_DRVSTREN_MASK           (~( 1<< 5))
#define AW8898_BIT_I2STXCFG_DRVSTREN_8MA            ( 1<< 5)
#define AW8898_BIT_I2STXCFG_DRVSTREN_2MA            ( 0<< 5)
#define AW8898_BIT_I2STXCFG_DOHZ_MASK               (~( 1<< 4))
#define AW8898_BIT_I2STXCFG_DOHZ_HIZ                ( 1<< 4)
#define AW8898_BIT_I2STXCFG_DOHZ_GND                ( 0<< 4)
#define AW8898_BIT_I2STXCFG_DSEL_MASK               (~( 3<< 2))
#define AW8898_BIT_I2STXCFG_DSEL_DSP                ( 2<< 2)
#define AW8898_BIT_I2STXCFG_DSEL_GAIN               ( 1<< 2)
#define AW8898_BIT_I2STXCFG_DSEL_ZERO               ( 0<< 2)
#define AW8898_BIT_I2STXCFG_CHS_MASK                (~( 1<< 1))
#define AW8898_BIT_I2STXCFG_CHS_RIGHT               ( 1<< 1)
#define AW8898_BIT_I2STXCFG_CHS_LEFT                ( 0<< 1)
#define AW8898_BIT_I2STXCFG_TX_MASK                 (~( 1<< 0))
#define AW8898_BIT_I2STXCFG_TX_ENABLE               ( 1<< 0)
#define AW8898_BIT_I2STXCFG_TX_DISABLE              ( 0<< 0)

// PWMCTRL
#define AW8898_BIT_PWMCTRL_DSMZTH_MASK              (~(15<<12))
#define AW8898_BIT_PWMCTRL_DSMZTH_UNIT              ( 1<<12)
#define AW8898_BIT_PWMCTRL_PWMDELA_MASK             (~(15<< 8))
#define AW8898_BIT_PWMCTRL_PWMDELA_UNIT             ( 1<< 8)
#define AW8898_BIT_PWMCTRL_PWMDELB_MASK             (~(15<< 4))
#define AW8898_BIT_PWMCTRL_PWMDELB_UNIT             ( 1<< 4)
#define AW8898_BIT_PWMCTRL_PWMSH_MASK               (~( 1<< 3))
#define AW8898_BIT_PWMCTRL_PWMSH_TRIANGLE           ( 1<< 3)
#define AW8898_BIT_PWMCTRL_PWMSH_SAWTOOTH           ( 0<< 3)
#define AW8898_BIT_PWMCTRL_PWMRES_MASK              (~( 1<< 2))
#define AW8898_BIT_PWMCTRL_PWMRES_8BIT              ( 1<< 2)
#define AW8898_BIT_PWMCTRL_PWMRES_7BIT              ( 0<< 2)
#define AW8898_BIT_PWMCTRL_HDCCE_MASK               (~( 1<< 1))
#define AW8898_BIT_PWMCTRL_HDCCE_ENABLE             ( 1<< 1)
#define AW8898_BIT_PWMCTRL_HDCCE_DISABLE            ( 0<< 1)
#define AW8898_BIT_PWMCTRL_HMUTE_MASK               (~( 1<< 0))
#define AW8898_BIT_PWMCTRL_HMUTE_ENABLE             ( 1<< 0)
#define AW8898_BIT_PWMCTRL_HMUTE_DISABLE            ( 0<< 0)

// HAGCCFG1
#define AW8898_BIT_HAGCCFG1_RVTH_MASK               (~(255<<8))
#define AW8898_BIT_HAGCCFG1_RVTH_UNIT               ( 1<< 8)
#define AW8898_BIT_HAGCCFG1_AVTH_MASK               (~(255<<0))
#define AW8898_BIT_HAGCCFG1_AVTH_UNIT               ( 1<< 0)

// HAGCCFG2
#define AW8898_BIT_HAGCCFG2_ATTH_UNIT               ( 1<< 0)

// HAGCCFG3
#define AW8898_BIT_HAGCCFG3_RTTH_UNIT               ( 1<< 0)

// HAGCCFG4
#define AW8898_BIT_HAGCCFG4_MPD_MASK                (~( 1<<14))
#define AW8898_BIT_HAGCCFG4_MPD_ENABLE              ( 1<<14)
#define AW8898_BIT_HAGCCFG4_MPD_DISABLE             ( 0<<14)
#define AW8898_BIT_HAGCCFG4_MPD_TTH_MASK            (~( 3<<12))
#define AW8898_BIT_HAGCCFG4_MPD_TTH_0P047           ( 3<<12)
#define AW8898_BIT_HAGCCFG4_MPD_TTH_0P032           ( 2<<12)
#define AW8898_BIT_HAGCCFG4_MPD_TTH_0P016           ( 1<<12)
#define AW8898_BIT_HAGCCFG4_MPD_TTH_0P008           ( 0<<12)
#define AW8898_BIT_HAGCCFG4_MPD_RTH_MASK            (~( 3<<10))
#define AW8898_BIT_HAGCCFG4_MPD_RTH_0P047           ( 3<<10)
#define AW8898_BIT_HAGCCFG4_MPD_RTH_0P032           ( 2<<10)
#define AW8898_BIT_HAGCCFG4_MPD_RTH_0P016           ( 1<<10)
#define AW8898_BIT_HAGCCFG4_MPD_RTH_0P008           ( 0<<10)
#define AW8898_BIT_HAGCCFG4_MPD_ATH_MASK            (~( 3<< 8))
#define AW8898_BIT_HAGCCFG4_MPD_ATH_0P047           ( 3<< 8)
#define AW8898_BIT_HAGCCFG4_MPD_ATH_0P032           ( 2<< 8)
#define AW8898_BIT_HAGCCFG4_MPD_ATH_0P016           ( 1<< 8)
#define AW8898_BIT_HAGCCFG4_MPD_ATH_0P008           ( 0<< 8)
#define AW8898_BIT_HAGCCFG4_HOLDTH_MASK             (~(255<< 0))

// HAGCCFG5

// HAGCCFG6

// HAGCCFG7
#define AW8898_BIT_HAGCCFG7_VOL_MASK                (~(255< 8))
#define AW8898_VOLUME_MAX                           (0)
#define AW8898_VOLUME_MIN                           (-255)
#define AW8898_VOL_REG_SHIFT                        (8)

// HAGCST
#define AW8898_BIT_BSTVOUT_ST_10P25V                (15<< 0)
#define AW8898_BIT_BSTVOUT_ST_10V                   (14<< 0)
#define AW8898_BIT_BSTVOUT_ST_9P75V                 (13<< 0)
#define AW8898_BIT_BSTVOUT_ST_9P5V                  (12<< 0)
#define AW8898_BIT_BSTVOUT_ST_9P25V                 (11<< 0)
#define AW8898_BIT_BSTVOUT_ST_9V                    (10<< 0)
#define AW8898_BIT_BSTVOUT_ST_8P75V                 ( 9<< 0)
#define AW8898_BIT_BSTVOUT_ST_8P5V                  ( 8<< 0)
#define AW8898_BIT_BSTVOUT_ST_8P25V                 ( 7<< 0)
#define AW8898_BIT_BSTVOUT_ST_8V                    ( 6<< 0)
#define AW8898_BIT_BSTVOUT_ST_7P75V                 ( 5<< 0)
#define AW8898_BIT_BSTVOUT_ST_7P5V                  ( 4<< 0)
#define AW8898_BIT_BSTVOUT_ST_7P25V                 ( 3<< 0)
#define AW8898_BIT_BSTVOUT_ST_7V                    ( 2<< 0)
#define AW8898_BIT_BSTVOUT_ST_6P75V                 ( 1<< 0)
#define AW8898_BIT_BSTVOUT_ST_6P5V                  ( 0<< 0)

// DBGCTRL
#define AW8898_BIT_DBGCTRL_LPBK_FAR_MASK            (~( 1<<15))
#define AW8898_BIT_DBGCTRL_LPBK_FAR_ENABLE          ( 1<<15)
#define AW8898_BIT_DBGCTRL_LPBK_FAR_DISABLE         ( 0<<15)
#define AW8898_BIT_DBGCTRL_LPBK_NEAR_MASK           (~( 1<<14))
#define AW8898_BIT_DBGCTRL_LPBK_NEAR_ENABLE         ( 1<<14)
#define AW8898_BIT_DBGCTRL_LPBK_NEAR_DISABLE        ( 0<<14)
#define AW8898_BIT_DBGCTRL_PDUVL_MASK               (~( 1<<13))
#define AW8898_BIT_DBGCTRL_PDUVL_DISABLE            ( 1<<13)
#define AW8898_BIT_DBGCTRL_PDUVL_ENABLE             ( 0<<13)
#define AW8898_BIT_DBGCTRL_MUTE_MASK                (~( 1<<12))
#define AW8898_BIT_DBGCTRL_MUTE_NO_AUTO             ( 1<<12)
#define AW8898_BIT_DBGCTRL_MUTE_AUTO                ( 0<<12)
#define AW8898_BIT_DBGCTRL_NOCLK_RESET_MASK         (~( 1<<11))
#define AW8898_BIT_DBGCTRL_NOCLK_NO_RESET           ( 1<<11)
#define AW8898_BIT_DBGCTRL_NOCLK_RESET              ( 0<<11)
#define AW8898_BIT_DBGCTRL_PLL_UNLOCK_RESET_MASK    (~( 1<<10))
#define AW8898_BIT_DBGCTRL_PLL_UNLOCK_NO_RESET      ( 1<<10)
#define AW8898_BIT_DBGCTRL_PLL_UNLOCK_RESET         ( 0<<10)
#define AW8898_BIT_DBGCTRL_CLKMD_MASK               (~( 1<< 9))
#define AW8898_BIT_DBGCTRL_CLKMD_HALF               ( 1<< 9)
#define AW8898_BIT_DBGCTRL_CLKMD_NORMAL             ( 0<< 9)
#define AW8898_BIT_DBGCTRL_OSCPD_MASK               (~( 1<< 8))
#define AW8898_BIT_DBGCTRL_OSCPD_ENABLE             ( 1<< 8)
#define AW8898_BIT_DBGCTRL_OSCPD_DISABLE            ( 0<< 8)
#define AW8898_BIT_DBGCTRL_AMPPD_MASK               (~( 1<< 7))
#define AW8898_BIT_DBGCTRL_AMPPD_PDN                ( 1<< 7)
#define AW8898_BIT_DBGCTRL_AMPPD_ACTIVE             ( 0<< 7)
#define AW8898_BIT_DBGCTRL_PLLPD_MASK               (~( 1<< 6))
#define AW8898_BIT_DBGCTRL_PLLPD_PDN                ( 1<< 6)
#define AW8898_BIT_DBGCTRL_PLLPD_ACTIVE             ( 0<< 6)
#define AW8898_BIT_DBGCTRL_I2SRST_MASK              (~( 1<< 5))
#define AW8898_BIT_DBGCTRL_I2SRST_RESET             ( 1<< 5)
#define AW8898_BIT_DBGCTRL_I2SRST_WORK              ( 0<< 5)
#define AW8898_BIT_DBGCTRL_SYSRST_MASK              (~( 1<< 4))
#define AW8898_BIT_DBGCTRL_SYSRST_RESET             ( 1<< 4)
#define AW8898_BIT_DBGCTRL_SYSRST_WORK              ( 0<< 4)
#define AW8898_BIT_DBGCTRL_SYSCE_MASK               (~( 1<< 0))
#define AW8898_BIT_DBGCTRL_SYSCE_ENABLE             ( 1<< 0)
#define AW8898_BIT_DBGCTRL_SYSCE_DISABLE            ( 0<< 0)


// I2SCFG
#define AW8898_BIT_I2SCFG_I2SRX_MASK                (~( 1<< 0))
#define AW8898_BIT_I2SCFG_I2SRX_ENABLE              ( 1<< 0)
#define AW8898_BIT_I2SCFG_I2SRX_DISABLE             ( 0<< 0)

// I2SSAT
#define AW8898_BIT_I2SSAT_DPSTAT                    ( 1<< 2)
#define AW8898_BIT_I2SSAT_I2SROVS                   ( 1<< 1)
#define AW8898_BIT_I2SSAT_I2STOVS                   ( 1<< 0)

// GENCTRL
#define AW8898_BIT_GENCTRL_BURST_PEAK_MASK          (~( 3<<14))
#define AW8898_BIT_GENCTRL_BURST_PEAK_200MA         ( 3<<14)
#define AW8898_BIT_GENCTRL_BURST_PEAK_160MA         ( 2<<14)
#define AW8898_BIT_GENCTRL_BURST_PEAK_100MA         ( 1<<14)
#define AW8898_BIT_GENCTRL_BURST_PEAK_130MA         ( 0<<14)
#define AW8898_BIT_GENCTRL_BST_TDEG2_MASK           (~( 7<< 9))
#define AW8898_BIT_GENCTRL_BST_TDEG2_2P7S           ( 7<< 9)
#define AW8898_BIT_GENCTRL_BST_TDEG2_1P3S           ( 6<< 9)
#define AW8898_BIT_GENCTRL_BST_TDEG2_672MS          ( 5<< 9)
#define AW8898_BIT_GENCTRL_BST_TDEG2_336MS          ( 4<< 9)
#define AW8898_BIT_GENCTRL_BST_TDEG2_168MS          ( 3<< 9)
#define AW8898_BIT_GENCTRL_BST_TDEG2_84MS           ( 2<< 9)
#define AW8898_BIT_GENCTRL_BST_TDEG2_42MS           ( 1<< 9)
#define AW8898_BIT_GENCTRL_BST_TDEG2_21MS           ( 0<< 9)
#define AW8898_BIT_GENCTRL_BST_OCAP_MASK            (~( 1<< 8))
#define AW8898_BIT_GENCTRL_BST_OCAP_SLOW            ( 1<< 8)
#define AW8898_BIT_GENCTRL_BST_OCAP_FAST            ( 0<< 8)
#define AW8898_BIT_GENCTRL_BST_EN_MASK              (~( 1<< 7))
#define AW8898_BIT_GENCTRL_BST_ENABLE               ( 1<< 7)
#define AW8898_BIT_GENCTRL_BST_DISABLE              ( 0<< 7)
#define AW8898_BIT_GENCTRL_BST_ILIMIT_MASK          (~( 7<< 4))
#define AW8898_BIT_GENCTRL_BST_ILIMIT_4P5A          ( 7<< 4)
#define AW8898_BIT_GENCTRL_BST_ILIMIT_4P25A         ( 6<< 4)
#define AW8898_BIT_GENCTRL_BST_ILIMIT_4A            ( 5<< 4)
#define AW8898_BIT_GENCTRL_BST_ILIMIT_3P75A         ( 4<< 4)
#define AW8898_BIT_GENCTRL_BST_ILIMIT_3P5A          ( 3<< 4)
#define AW8898_BIT_GENCTRL_BST_ILIMIT_3P25A         ( 2<< 4)
#define AW8898_BIT_GENCTRL_BST_ILIMIT_3A            ( 1<< 4)
#define AW8898_BIT_GENCTRL_BST_ILIMIT_2P75A         ( 0<< 4)
#define AW8898_BIT_GENCTRL_BST_VOUT_MASK            (~(15<< 0))
#define AW8898_BIT_GENCTRL_BST_VOUT_10P25V          (15<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_10V             (14<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_9P75V           (13<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_9P5V            (12<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_9P25V           (11<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_9V              (10<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_8P75V           ( 9<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_8P5V            ( 8<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_8P25V           ( 7<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_8V              ( 6<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_7P75V           ( 5<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_7P5V            ( 4<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_7P25V           ( 3<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_7V              ( 2<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_6P75V           ( 1<< 0)
#define AW8898_BIT_GENCTRL_BST_VOUT_6P5V            ( 0<< 0)

// BSTCTRL1
#define AW8898_BIT_BSTCTRL1_RTH_MASK                (~(64<< 8))
#define AW8898_BIT_BSTCTRL1_ATH_MASK                (~(64<< 0))

// BSTCTRL2
#define AW8898_BIT_BST_MODE_MASK                    (~( 7<< 3))
#define AW8898_BIT_BST_MODE_SMART_BOOST             ( 6<< 3)
#define AW8898_BIT_BST_MODE_ADAPT_BOOST             ( 5<< 3)
#define AW8898_BIT_BST_MODE_FORCE_BOOST             ( 1<< 3)
#define AW8898_BIT_BST_MODE_TRANSP_BOOST            ( 0<< 3)
#define AW8898_BIT_BST_TDEG_MASK                    (~( 7<< 0))
#define AW8898_BIT_BST_TDEG_2P7S                    ( 7<< 0)
#define AW8898_BIT_BST_TDEG_1P3S                    ( 6<< 0)
#define AW8898_BIT_BST_TDEG_672MS                   ( 5<< 0)
#define AW8898_BIT_BST_TDEG_336MS                   ( 4<< 0)
#define AW8898_BIT_BST_TDEG_168MS                   ( 3<< 0)
#define AW8898_BIT_BST_TDEG_84MS                    ( 2<< 0)
#define AW8898_BIT_BST_TDEG_42MS                    ( 1<< 0)
#define AW8898_BIT_BST_TDEG_21MS                    ( 0<< 0)



typedef enum
{
	CHSEL_LEFT = 0,
	CHSEL_RIGHT = 1,
	CHSEL_MONO = 2,
}aw_i2s_channel_t;


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

}aw_i2s_frequency_t;

typedef enum
{
	WIDTH_16BITS = 0,
	WIDTH_24BITS = 1,
	WIDTH_32BITS = 2,

}aw_i2s_width_t;

typedef enum
{
	I2SBCK_32FS = 0,
	I2SBCK_48FS = 1,
	I2SBCK_64FS = 2,
}aw_i2s_fs_t;

typedef enum
{
	OFF_MODE = 0,
	SPK_MODE = 1,
	VOICE_MODE = 2,
	MUSIC_MODE = 3,
}aw_sel_mode_t;

typedef enum
{
	AW_ENCODE = 0,
	AW_DECODE = 1,
	AW_MODE_BOTH = 2,
	AW_MODE_LINE_IN = 3,
}aw_codec_mode_t;


typedef enum
{
	STOP = 0,
	START = 1,
}aw_ctrl_t;


/*!
 * @addtogroup aw8898
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @name Driver version */
/*@{*/
/*! @brief CLOCK driver version 2.1.3 */
#define FSL_AW8898_DRIVER_VERSION (MAKE_VERSION(2, 1, 3))
/*@}*/

/*! @brief aw8898 handle size */
#ifndef AW8898_I2C_HANDLER_SIZE
#define AW8898_I2C_HANDLER_SIZE (80U)
#endif

/*! @brief Define the register address of AW8898. */
#define AW8898_LINVOL  0x0U
#define AW8898_RINVOL  0x1U
#define AW8898_LOUT1   0x2U
#define AW8898_ROUT1   0x3U
#define AW8898_CLOCK1  0x4U
#define AW8898_DACCTL1 0x5U
#define AW8898_DACCTL2 0x6U
#define AW8898_IFACE1  0x7U
#define AW8898_CLOCK2  0x8U
#define AW8898_IFACE2  0x9U
#define AW8898_LDAC    0xaU
#define AW8898_RDAC    0xbU

#define AW8898_RESET   0xfU
#define AW8898_3D      0x10U
#define AW8898_ALC1    0x11U
#define AW8898_ALC2    0x12U
#define AW8898_ALC3    0x13U
#define AW8898_NOISEG  0x14U
#define AW8898_LADC    0x15U
#define AW8898_RADC    0x16U
#define AW8898_ADDCTL1 0x17U
#define AW8898_ADDCTL2 0x18U
#define AW8898_POWER1  0x19U
#define AW8898_POWER2  0x1aU
#define AW8898_ADDCTL3 0x1bU
#define AW8898_APOP1   0x1cU
#define AW8898_APOP2   0x1dU

#define AW8898_LINPATH 0x20U
#define AW8898_RINPATH 0x21U
#define AW8898_LOUTMIX 0x22U

#define AW8898_ROUTMIX  0x25U
#define AW8898_MONOMIX1 0x26U
#define AW8898_MONOMIX2 0x27U
#define AW8898_LOUT2    0x28U
#define AW8898_ROUT2    0x29U
#define AW8898_MONO     0x2aU
#define AW8898_INBMIX1  0x2bU
#define AW8898_INBMIX2  0x2cU
#define AW8898_BYPASS1  0x2dU
#define AW8898_BYPASS2  0x2eU
#define AW8898_POWER3   0x2fU
#define AW8898_ADDCTL4  0x30U
#define AW8898_CLASSD1  0x31U

#define AW8898_CLASSD3 0x33U
#define AW8898_PLL1    0x34U
#define AW8898_PLL2    0x35U
#define AW8898_PLL3    0x36U
#define AW8898_PLL4    0x37U

/*! @brief Cache register number */
#define AW8898_CACHEREGNUM 56U

/*! @brief AW8898_IFACE1 FORMAT bits */
#define AW8898_IFACE1_FORMAT_MASK  0x03U
#define AW8898_IFACE1_FORMAT_SHIFT 0x00U
#define AW8898_IFACE1_FORMAT_RJ    0x00U
#define AW8898_IFACE1_FORMAT_LJ    0x01U
#define AW8898_IFACE1_FORMAT_I2S   0x02U
#define AW8898_IFACE1_FORMAT_DSP   0x03U
#define AW8898_IFACE1_FORMAT(x)    (((x) << AW8898_IFACE1_FORMAT_SHIFT) & AW8898_IFACE1_FORMAT_MASK)

/*! @brief AW8898_IFACE1 WL bits */
#define AW8898_IFACE1_WL_MASK   0x0CU
#define AW8898_IFACE1_WL_SHIFT  0x02U
#define AW8898_IFACE1_WL_16BITS 0x00U
#define AW8898_IFACE1_WL_20BITS 0x01U
#define AW8898_IFACE1_WL_24BITS 0x02U
#define AW8898_IFACE1_WL_32BITS 0x03U
#define AW8898_IFACE1_WL(x)     (((x) << AW8898_IFACE1_WL_SHIFT) & AW8898_IFACE1_WL_MASK)

/*! @brief AW8898_IFACE1 LRP bit */
#define AW8898_IFACE1_LRP_MASK         0x10U
#define AW8898_IFACE1_LRP_SHIFT        0x04U
#define AW8898_IFACE1_LRCLK_NORMAL_POL 0x00U
#define AW8898_IFACE1_LRCLK_INVERT_POL 0x01U
#define AW8898_IFACE1_DSP_MODEA        0x00U
#define AW8898_IFACE1_DSP_MODEB        0x01U
#define AW8898_IFACE1_LRP(x)           (((x) << AW8898_IFACE1_LRP_SHIFT) & AW8898_IFACE1_LRP_MASK)

/*! @brief AW8898_IFACE1 DLRSWAP bit */
#define AW8898_IFACE1_DLRSWAP_MASK  0x20U
#define AW8898_IFACE1_DLRSWAP_SHIFT 0x05U
#define AW8898_IFACE1_DACCH_NORMAL  0x00U
#define AW8898_IFACE1_DACCH_SWAP    0x01U
#define AW8898_IFACE1_DLRSWAP(x)    (((x) << AW8898_IFACE1_DLRSWAP_SHIFT) & AW8898_IFACE1_DLRSWAP_MASK)

/*! @brief AW8898_IFACE1 MS bit */
#define AW8898_IFACE1_MS_MASK  0x40U
#define AW8898_IFACE1_MS_SHIFT 0x06U
#define AW8898_IFACE1_SLAVE    0x00U
#define AW8898_IFACE1_MASTER   0x01U
#define AW8898_IFACE1_MS(x)    (((x) << AW8898_IFACE1_MS_SHIFT) & AW8898_IFACE1_MS_MASK)

/*! @brief AW8898_IFACE1 BCLKINV bit */
#define AW8898_IFACE1_BCLKINV_MASK   0x80U
#define AW8898_IFACE1_BCLKINV_SHIFT  0x07U
#define AW8898_IFACE1_BCLK_NONINVERT 0x00U
#define AW8898_IFACE1_BCLK_INVERT    0x01U
#define AW8898_IFACE1_BCLKINV(x)     (((x) << AW8898_IFACE1_BCLKINV_SHIFT) & AW8898_IFACE1_BCLKINV_MASK)

/*! @brief AW8898_IFACE1 ALRSWAP bit */
#define AW8898_IFACE1_ALRSWAP_MASK  0x100U
#define AW8898_IFACE1_ALRSWAP_SHIFT 0x08U
#define AW8898_IFACE1_ADCCH_NORMAL  0x00U
#define AW8898_IFACE1_ADCCH_SWAP    0x01U
#define AW8898_IFACE1_ALRSWAP(x)    (((x) << AW8898_IFACE1_ALRSWAP_SHIFT) & AW8898_IFACE1_ALRSWAP_MASK)

/*! @brief AW8898_POWER1 */
#define AW8898_POWER1_VREF_MASK  0x40U
#define AW8898_POWER1_VREF_SHIFT 0x06U

#define AW8898_POWER1_AINL_MASK  0x20U
#define AW8898_POWER1_AINL_SHIFT 0x05U

#define AW8898_POWER1_AINR_MASK  0x10U
#define AW8898_POWER1_AINR_SHIFT 0x04U

#define AW8898_POWER1_ADCL_MASK  0x08U
#define AW8898_POWER1_ADCL_SHIFT 0x03U

#define AW8898_POWER1_ADCR_MASK  0x0U
#define AW8898_POWER1_ADCR_SHIFT 0x02U

#define AW8898_POWER1_MICB_MASK  0x02U
#define AW8898_POWER1_MICB_SHIFT 0x01U

#define AW8898_POWER1_DIGENB_MASK  0x01U
#define AW8898_POWER1_DIGENB_SHIFT 0x00U

/*! @brief AW8898_POWER2 */
#define AW8898_POWER2_DACL_MASK  0x100U
#define AW8898_POWER2_DACL_SHIFT 0x08U

#define AW8898_POWER2_DACR_MASK  0x80U
#define AW8898_POWER2_DACR_SHIFT 0x07U

#define AW8898_POWER2_LOUT1_MASK  0x40U
#define AW8898_POWER2_LOUT1_SHIFT 0x06U

#define AW8898_POWER2_ROUT1_MASK  0x20U
#define AW8898_POWER2_ROUT1_SHIFT 0x05U

#define AW8898_POWER2_SPKL_MASK  0x10U
#define AW8898_POWER2_SPKL_SHIFT 0x04U

#define AW8898_POWER2_SPKR_MASK  0x08U
#define AW8898_POWER2_SPKR_SHIFT 0x03U

#define AW8898_POWER3_LMIC_MASK   0x20U
#define AW8898_POWER3_LMIC_SHIFT  0x05U
#define AW8898_POWER3_RMIC_MASK   0x10U
#define AW8898_POWER3_RMIC_SHIFT  0x04U
#define AW8898_POWER3_LOMIX_MASK  0x08U
#define AW8898_POWER3_LOMIX_SHIFT 0x03U
#define AW8898_POWER3_ROMIX_MASK  0x04U
#define AW8898_POWER3_ROMIX_SHIFT 0x02U
/*! @brief AW8898 I2C address. */
#define AW8898_I2C_ADDR 0x34
/*! @brief AW8898 I2C baudrate */
#define AW8898_I2C_BAUDRATE (100000U)
/*! @brief AW8898 maximum volume value */
#define AW8898_ADC_MAX_VOLUME_vALUE       0xFFU
#define AW8898_DAC_MAX_VOLUME_vALUE       0xFFU
#define AW8898_HEADPHONE_MAX_VOLUME_vALUE 0xFFU
#define AW8898_HEADPHONE_MIN_VOLUME_vALUE 0x0U
#define AW8898_LINEIN_MAX_VOLUME_vALUE    0x3FU
#define AW8898_SPEAKER_MAX_VOLUME_vALUE   0x7FU
#define AW8898_SPEAKER_MIN_VOLUME_vALUE   0x30U

/*! @brief Modules in AW8898 board. */
typedef enum _aw8898_module
{
    kAW8898_ModuleADC     = 0, /*!< ADC module in AW8898 */
    kAW8898_ModuleDAC     = 1, /*!< DAC module in AW8898 */
    kAW8898_ModuleVREF    = 2, /*!< VREF module */
    kAW8898_ModuleHP      = 3, /*!< Headphone */
    kAW8898_ModuleMICB    = 4, /*!< Mic bias */
    kAW8898_ModuleMIC     = 5, /*!< Input Mic */
    kAW8898_ModuleLineIn  = 6, /*!< Analog in PGA  */
    kAW8898_ModuleLineOut = 7, /*!< Line out module */
    kAW8898_ModuleSpeaker = 8, /*!< Speaker module */
    kAW8898_ModuleOMIX    = 9, /*!< Output mixer */
} aw8898_module_t;

/*! @brief aw8898 play channel
 * @anchor _aw8898_play_channel
 */
enum
{
    kAW8898_HeadphoneLeft  = 1, /*!< aw8898 headphone left channel */
    kAW8898_HeadphoneRight = 2, /*!< aw8898 headphone right channel */
    kAW8898_SpeakerLeft    = 4, /*!< aw8898 speaker left channel */
    kAW8898_SpeakerRight   = 8, /*!< aw8898 speaker right channel */
};

/*! @brief aw8898 play source */
typedef enum _aw8898_play_source
{
    kAW8898_PlaySourcePGA   = 1, /*!< aw8898 play source PGA */
    kAW8898_PlaySourceInput = 2, /*!< aw8898 play source Input */
    kAW8898_PlaySourceDAC   = 4, /*!< aw8898 play source DAC */
} aw8898_play_source_t;

/*!
 * @brief AW8898 data route.
 * Only provide some typical data route, not all route listed.
 * Note: Users cannot combine any routes, once a new route is set, the previous one would be replaced.
 */
typedef enum _aw8898_route
{
    kAW8898_RouteBypass            = 0, /*!< LINEIN->Headphone. */
    kAW8898_RoutePlayback          = 1, /*!<  I2SIN->DAC->Headphone. */
    kAW8898_RoutePlaybackandRecord = 2, /*!< I2SIN->DAC->Headphone, LINEIN->ADC->I2SOUT. */
    kAW8898_RouteRecord            = 5  /*!< LINEIN->ADC->I2SOUT. */
} aw8898_route_t;

/*!
 * @brief The audio data transfer protocol choice.
 * AW8898 only supports I2S format and PCM format.
 */
typedef enum _aw8898_protocol
{
    kAW8898_BusI2S            = 2,           /*!< I2S type */
    kAW8898_BusLeftJustified  = 1,           /*!< Left justified mode */
    kAW8898_BusRightJustified = 0,           /*!< Right justified mode */
    kAW8898_BusPCMA           = 3,           /*!< PCM A mode */
    kAW8898_BusPCMB           = 3 | (1 << 4) /*!< PCM B mode */
} aw8898_protocol_t;

/*! @brief aw8898 input source */
typedef enum _aw8898_input
{
    kAW8898_InputClosed                = 0, /*!< Input device is closed */
    kAW8898_InputSingleEndedMic        = 1, /*!< Input as single ended mic, only use L/RINPUT1 */
    kAW8898_InputDifferentialMicInput2 = 2, /*!< Input as differential mic, use L/RINPUT1 and L/RINPUT2 */
    kAW8898_InputDifferentialMicInput3 = 3, /*!< Input as differential mic, use L/RINPUT1 and L/RINPUT3*/
    kAW8898_InputLineINPUT2            = 4, /*!< Input as line input, only use L/RINPUT2 */
    kAW8898_InputLineINPUT3            = 5  /*!< Input as line input, only use L/RINPUT3 */
} aw8898_input_t;

/*! @brief audio sample rate definition
 * @anchor _aw8898_sample_rate
 */
enum
{
    kAW8898_AudioSampleRate8KHz    = 8000U,   /*!< Sample rate 8000 Hz */
    kAW8898_AudioSampleRate11025Hz = 11025U,  /*!< Sample rate 11025 Hz */
    kAW8898_AudioSampleRate12KHz   = 12000U,  /*!< Sample rate 12000 Hz */
    kAW8898_AudioSampleRate16KHz   = 16000U,  /*!< Sample rate 16000 Hz */
    kAW8898_AudioSampleRate22050Hz = 22050U,  /*!< Sample rate 22050 Hz */
    kAW8898_AudioSampleRate24KHz   = 24000U,  /*!< Sample rate 24000 Hz */
    kAW8898_AudioSampleRate32KHz   = 32000U,  /*!< Sample rate 32000 Hz */
    kAW8898_AudioSampleRate44100Hz = 44100U,  /*!< Sample rate 44100 Hz */
    kAW8898_AudioSampleRate48KHz   = 48000U,  /*!< Sample rate 48000 Hz */
    kAW8898_AudioSampleRate96KHz   = 96000U,  /*!< Sample rate 96000 Hz */
    kAW8898_AudioSampleRate192KHz  = 192000U, /*!< Sample rate 192000 Hz */
    kAW8898_AudioSampleRate384KHz  = 384000U, /*!< Sample rate 384000 Hz */
};

/*! @brief audio bit width
 * @anchor _aw8898_audio_bit_width
 */
enum
{
    kAW8898_AudioBitWidth16bit = 16U, /*!< audio bit width 16 */
    kAW8898_AudioBitWidth20bit = 20U, /*!< audio bit width 20 */
    kAW8898_AudioBitWidth24bit = 24U, /*!< audio bit width 24 */
    kAW8898_AudioBitWidth32bit = 32U, /*!< audio bit width 32 */
};

/*! @brief aw8898 audio format */
typedef struct _aw8898_audio_format
{
    uint32_t mclk_HZ;    /*!< master clock frequency */
    uint32_t sampleRate; /*!< sample rate */
    uint32_t bitWidth;   /*!< bit width */
} aw8898_audio_format_t;

/*! @brief CODEC I2C configurations structure */
typedef struct _codec_i2c_config
{
    uint32_t codecI2CInstance;    /*!< i2c bus instance */
    uint32_t codecI2CSourceClock; /*!< i2c bus source clock frequency */
} codec_i2c_config_t;

/*! @brief Initialize structure of AW8898 */
typedef struct aw8898_config
{
    aw8898_route_t route;            /*!< Audio data route.*/
    aw8898_protocol_t bus;           /*!< Audio transfer protocol */
    aw8898_audio_format_t format;    /*!< Audio format */
    bool master_slave;               /*!< Master or slave. */
    bool enableSpeaker;              /*!< True means enable class D speaker as output, false means no */
    aw8898_input_t leftInputSource;  /*!< Left input source for AW8898 */
    aw8898_input_t rightInputSource; /*!< Right input source for aw8898 */
    aw8898_play_source_t playSource; /*!< play source */
    uint8_t slaveAddress;            /*!< aw8898 device address */
    codec_i2c_config_t i2cConfig;    /*!< i2c configuration */
} aw8898_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief AW8898 initialize function.
 *
 * The second parameter is NULL to AW8898 in this version. If users want
 * to change the settings, they have to use aw8898_write_reg() or aw8898_modify_reg()
 * to set the register value of AW8898.
 * Note: If the codec_config is NULL, it would initialize AW8898 using default settings.
 * The default setting:
 * codec_config->route = kAW8898_RoutePlaybackandRecord
 * codec_config->bus = kAW8898_BusI2S
 * codec_config->master = slave
 *
 * @param handle AW8898 handle structure.
 * @param config AW8898 configuration structure.
 */
status_t AW8898_Init(const aw8898_config_t *config);

/*!
 * @brief Deinit the AW8898 codec.
 *
 * This function close all modules in AW8898 to save power.
 *
 * @param handle AW8898 handle structure pointer.
 */
status_t AW8898_Deinit(void);

/*!
 * @brief Set audio data route in AW8898.
 *
 * This function would set the data route according to route. The route cannot be combined,
 * as all route would enable different modules.
 * Note: If a new route is set, the previous route would not work.
 *
 * @param handle AW8898 handle structure.
 * @param route Audio data route in AW8898.
 */
status_t AW8898_SetDataRoute(aw8898_route_t route);

/*!
 * @brief Set left audio input source in AW8898.
 *
 * @param handle AW8898 handle structure.
 * @param input Audio input source.
 */
status_t AW8898_SetLeftInput(aw8898_input_t input);

/*!
 * @brief Set right audio input source in AW8898.
 *
 * @param handle AW8898 handle structure.
 * @param input Audio input source.
 */
status_t AW8898_SetRightInput(aw8898_input_t input);

/*!
 * @brief Set the audio transfer protocol.
 *
 * AW8898 only supports I2S, left justified, right justified, PCM A, PCM B format.
 *
 * @param handle AW8898 handle structure.
 * @param protocol Audio data transfer protocol.
 */
status_t AW8898_SetProtocol(aw8898_protocol_t protocol);

/*!
 * @brief Set AW8898 as master or slave.
 *
 * @param handle AW8898 handle structure.
 * @param master 1 represent master, 0 represent slave.
 */
void AW8898_SetMasterSlave(bool master);

/*!
 * @brief Set the volume of different modules in AW8898.
 *
 * This function would set the volume of AW8898 modules. Uses need to appoint the module.
 * The function assume that left channel and right channel has the same volume.
 *
 * Module:kAW8898_ModuleADC, volume range value: 0 is mute, 1-255 is -97db to 30db
 * Module:kAW8898_ModuleDAC, volume range value: 0 is mute, 1-255 is -127db to 0db
 * Module:kAW8898_ModuleHP, volume range value: 0 - 2F is mute, 0x30 - 0x7F is -73db to 6db
 * Module:kAW8898_ModuleLineIn, volume range value: 0 - 0x3F is -17.25db to 30db
 * Module:kAW8898_ModuleSpeaker, volume range value: 0 - 2F is mute, 0x30 - 0x7F is -73db to 6db
 *
 *
 * @param handle AW8898 handle structure.
 * @param module Module to set volume, it can be ADC, DAC, Headphone and so on.
 * @param volume Volume value need to be set.
 */
status_t AW8898_SetVolume(aw8898_module_t module, uint8_t gain);

/*!
 * @brief Get the volume of different modules in AW8898.
 *
 * This function gets the volume of AW8898 modules. Uses need to appoint the module.
 * The function assume that left channel and right channel has the same volume.
 *
 * @param handle AW8898 handle structure.
 * @param module Module to set volume, it can be ADC, DAC, Headphone and so on.
 * @return Volume value of the module.
 */
status_t AW8898_GetVolume(aw8898_module_t module, uint8_t *gain);

/*!
 * @brief Mute modules in AW8898.
 *
 * @param handle AW8898 handle structure.
 * @param module Modules need to be mute.
 * @param isEnabled Mute or unmute, 1 represent mute.
 */
status_t AW8898_SetMute(aw8898_module_t module, bool isEnabled);

/*!
 * @brief Enable/disable expected devices.
 *
 * @param handle AW8898 handle structure.
 * @param module Module expected to enable.
 * @param isEnabled Enable or disable moudles.
 */
status_t AW8898_SetModule(aw8898_module_t module, bool isEnabled);

/*!
 * @brief SET the AW8898 play source.
 *
 * @param handle AW8898 handle structure.
 * @param playSource play source , can be a value combine of kAW8898_ModuleHeadphoneSourcePGA,
 * kAW8898_ModuleHeadphoneSourceDAC, kAW8898_ModulePlaySourceInput, kAW8898_ModulePlayMonoRight,
 * kAW8898_ModulePlayMonoLeft.
 *
 * @return kStatus_WM8904_Success if successful, different code otherwise..
 */
status_t AW8898_SetPlay(uint32_t playSource);

/*!
 * @brief Configure the data format of audio data.
 *
 * This function would configure the registers about the sample rate, bit depths.
 *
 * @param handle AW8898 handle structure pointer.
 * @param sysclk system clock of the codec which can be generated by MCLK or PLL output.
 * @param sample_rate Sample rate of audio file running in AW8898. AW8898 now
 * supports 8k, 11.025k, 12k, 16k, 22.05k, 24k, 32k, 44.1k, 48k and 96k sample rate.
 * @param bits Bit depth of audio file (AW8898 only supports 16bit, 20bit, 24bit
 * and 32 bit in HW).
 */
status_t AW8898_ConfigDataFormat(uint32_t sysclk, uint32_t sample_rate, uint32_t bits);

/*!
 * @brief Enable/disable jack detect feature.
 *
 * @param handle AW8898 handle structure.
 * @param isEnabled Enable or disable moudles.
 */
status_t AW8898_SetJackDetect(bool isEnabled);

/*!
 * @brief Write register to AW8898 using I2C.
 *
 * @param handle AW8898 handle structure.
 * @param reg The register address in AW8898.
 * @param val Value needs to write into the register.
 */
status_t AW8898_WriteReg(uint8_t reg, uint16_t val);

/*!
 * @brief Read register from AW8898 using I2C.
 * @param reg The register address in AW8898.
 * @param val Value written to.
 */
status_t AW8898_ReadReg(uint8_t reg, uint16_t *val);

/*!
 * @brief Modify some bits in the register using I2C.
 * @param handle AW8898 handle structure.
 * @param reg The register address in AW8898.
 * @param mask The mask code for the bits want to write. The bit you want to write should be 0.
 * @param val Value needs to write into the register.
 */
status_t AW8898_ModifyReg(uint8_t reg, uint16_t mask, uint16_t val);



uint8_t aw8898_init(void);
uint8_t aw8898_deinit(void);
bool aw8898_check_pll_status(void);
int  aw8898_set_voice_volume(int volume);
int  aw8898_get_voice_volume(int *volume);
status_t AW8898_Start(void);

#if defined(__cplusplus)
}
#endif

/*! @} */


/*******************************************************************************
 * API
 ******************************************************************************/
