// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "CodecAW8898.hpp"
#include "AW8898regs.hpp"
#include "AW8898driver.hpp"
#include "board/BoardDefinitions.hpp"

#include <log/log.hpp>

extern "C"
{
#include "FreeRTOS.h"
#include "task.h"
}

using namespace drivers;
using namespace bsp::bell_audio;

constexpr auto ReadStatusRetries        = 5;
constexpr auto OneByteAddressing        = 1;
constexpr auto PositiveLogic            = 0;

CodecAW8898::CodecAW8898()
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

    AW8898_Init(params.MonoStereoToCodecChsel(), params.SampleRateToCodecFreq());

    // Store param configuration
    currentParams = params;

    auto currVol = currentParams.outVolume;
    // remove this once volume control in application is working
    currVol = 3.0;
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
