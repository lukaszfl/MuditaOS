/*
 *  @file RouterOperation.cpp
 *  @author Mateusz Piesta (mateusz.piesta@mudita.com)
 *  @date 25.07.19
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#include "RouterOperation.hpp"

#include <Audio/Profiles/ProfileRoutingEarspeaker.hpp>
#include <Audio/Profiles/ProfileRoutingHeadset.hpp>
#include <Audio/Profiles/ProfileRoutingSpeakerphone.hpp>

#include <log/log.hpp>
#include <semaphore.hpp>

namespace audio
{

    RouterOperation::RouterOperation(
        [[maybe_unused]] const char *file,
        std::function<uint32_t(const std::string &path, const uint32_t &defaultValue)> dbCallback)
    {

        audioDeviceCallback =
            [this](const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer) -> int32_t {
            if (inputBuffer != nullptr) {
                if (framesPerBuffer > audioDeviceBuffer.size()) {
                    audioDeviceBuffer.resize(framesPerBuffer, 0);
                }

                if (muteEnable) {
                    memset(&audioDeviceBuffer[0], 0, framesPerBuffer * sizeof(int16_t));
                }
                else {
                    memcpy(&audioDeviceBuffer[0], inputBuffer, framesPerBuffer * sizeof(int16_t));
                }
            }

            if (outputBuffer != nullptr) {
                if (framesPerBuffer > audioDeviceCellularBuffer.size()) {
                    audioDeviceCellularBuffer.resize(framesPerBuffer, 0);
                }

                memcpy(outputBuffer, &audioDeviceCellularBuffer[0], framesPerBuffer * sizeof(int16_t));
            }
            return framesPerBuffer;
        };

        audioDeviceCellularCallback =
            [this](const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer) -> int32_t {
            if (inputBuffer != nullptr) {
                if (framesPerBuffer > audioDeviceCellularBuffer.size()) {
                    audioDeviceCellularBuffer.resize(framesPerBuffer, 0);
                }

                memcpy(&audioDeviceCellularBuffer[0], inputBuffer, framesPerBuffer * sizeof(int16_t));
            }

            if (outputBuffer != nullptr) {
                if (framesPerBuffer > audioDeviceBuffer.size()) {
                    audioDeviceBuffer.resize(framesPerBuffer, 0);
                }

                memcpy(outputBuffer, &audioDeviceBuffer[0], framesPerBuffer * sizeof(int16_t));
            }
            return framesPerBuffer;
        };

        audioDeviceBuffer.resize(1024, 0);
        audioDeviceCellularBuffer.resize(1024, 0);

        constexpr audio::Gain defaultRoutingEarspeakerGain       = 20;
        constexpr audio::Volume defaultRoutingEarspeakerVolume   = 10;
        constexpr audio::Gain defaultRoutingSpeakerphoneGain     = 20;
        constexpr audio::Volume defaultRoutingSpeakerphoneVolume = 10;
        constexpr audio::Gain defaultRoutingHeadsetGain          = 50;
        constexpr audio::Volume defaultRoutingHeadsetVolume      = 10;

        const auto dbRoutingEarspeakerGainPath =
            audio::str(audio::Profile::Type::RoutingEarspeaker, audio::Setting::Gain);
        const auto routingEarspeakerGain = dbCallback(dbRoutingEarspeakerGainPath, defaultRoutingEarspeakerGain);
        const auto dbRoutingEarspeakerVolumePath =
            audio::str(audio::Profile::Type::RoutingEarspeaker, audio::Setting::Volume);
        const auto routingEarspeakerVolume = dbCallback(dbRoutingEarspeakerVolumePath, defaultRoutingEarspeakerVolume);
        const auto dbRoutingSpeakerphoneGainPath =
            audio::str(audio::Profile::Type::RoutingSpeakerphone, audio::Setting::Gain);
        const auto routingSpeakerphoneGain = dbCallback(dbRoutingSpeakerphoneGainPath, defaultRoutingSpeakerphoneGain);
        const auto dbRoutingSpeakerphoneVolumePath =
            audio::str(audio::Profile::Type::RoutingSpeakerphone, audio::Setting::Volume);
        const auto routingSpeakerphoneVolume =
            dbCallback(dbRoutingSpeakerphoneVolumePath, defaultRoutingSpeakerphoneVolume);
        const auto dbRoutingHeadsetGainPath = audio::str(audio::Profile::Type::RoutingHeadset, audio::Setting::Gain);
        const auto routingHeadsetGain       = dbCallback(dbRoutingHeadsetGainPath, defaultRoutingHeadsetGain);
        const auto dbRoutingHeadsetVolumePath =
            audio::str(audio::Profile::Type::RoutingHeadset, audio::Setting::Volume);
        const auto routingHeadsetVolume = dbCallback(dbRoutingHeadsetVolumePath, defaultRoutingHeadsetVolume);

        availableProfiles.push_back(
            Profile::Create(Profile::Type::RoutingEarspeaker, nullptr, routingEarspeakerVolume, routingEarspeakerGain));
        availableProfiles.push_back(Profile::Create(
            Profile::Type::RoutingSpeakerphone, nullptr, routingSpeakerphoneVolume, routingSpeakerphoneGain));
        availableProfiles.push_back(
            Profile::Create(Profile::Type::RoutingHeadset, nullptr, routingHeadsetVolume, routingHeadsetGain));

        currentProfile = availableProfiles[0];

        audioDevice =
            bsp::AudioDevice::Create(currentProfile->GetAudioDeviceType(), audioDeviceCallback).value_or(nullptr);
        if (audioDevice == nullptr) {
            LOG_ERROR("Error creating AudioDevice");
            return;
        }

        audioDeviceCellular =
            bsp::AudioDevice::Create(bsp::AudioDevice::Type::Cellular, audioDeviceCellularCallback).value_or(nullptr);
        if (audioDeviceCellular == nullptr) {
            LOG_ERROR("Error creating AudioDeviceCellular");
            return;
        }

        isInitialized = true;
    }

    audio::RetCode RouterOperation::SetOutputVolume(float vol)
    {
        currentProfile->SetOutputVolume(vol);
        auto ret = audioDevice->OutputVolumeCtrl(vol);
        return GetDeviceError(ret);
    }

    audio::RetCode RouterOperation::SetInputGain(float gain)
    {
        currentProfile->SetInputGain(gain);
        auto ret = audioDevice->InputGainCtrl(gain);
        return GetDeviceError(ret);
    }

    audio::RetCode RouterOperation::Start(audio::AsyncCallback callback, audio::Token token)
    {
        if (state == State::Paused || state == State::Active) {
            return RetCode::InvokedInIncorrectState;
        }
        operationToken = token;
        eventCallback  = callback;
        state          = State::Active;

        if (audioDevice->IsFormatSupported(currentProfile->GetAudioFormat())) {
            auto ret = audioDevice->Start(currentProfile->GetAudioFormat());
            if (ret != bsp::AudioDevice::RetCode::Success) {
                LOG_ERROR("Start error: %s", audio::c_str(audio::RetCode::DeviceFailure));
            }
        }
        else {
            return RetCode::InvalidFormat;
        }

        if (audioDeviceCellular->IsFormatSupported(currentProfile->GetAudioFormat())) {
            auto ret = audioDeviceCellular->Start(currentProfile->GetAudioFormat());
            return GetDeviceError(ret);
        }
        else {
            return RetCode::InvalidFormat;
        }
    }

    audio::RetCode RouterOperation::Stop()
    {
        if (state == State::Paused || state == State::Idle) {
            return RetCode::InvokedInIncorrectState;
        }

        state = State::Idle;
        audioDevice->Stop();
        audioDeviceCellular->Stop();

        return RetCode::Success;
    }

    audio::RetCode RouterOperation::Pause()
    {
        if (state == State::Paused || state == State::Idle) {
            return RetCode::InvokedInIncorrectState;
        }

        state = State::Paused;
        audioDevice->Stop();
        audioDeviceCellular->Stop();
        return RetCode::Success;
    }

    audio::RetCode RouterOperation::Resume()
    {
        if (state == State::Active || state == State::Idle) {
            return RetCode::InvokedInIncorrectState;
        }

        state = State::Active;
        audioDevice->Start(currentProfile->GetAudioFormat());
        audioDeviceCellular->Start(currentProfile->GetAudioFormat());
        return RetCode::Success;
    }

    audio::RetCode RouterOperation::SendEvent(const audio::Operation::Event evt, const audio::EventData *data)
    {
        switch (evt) {
        case Event::HeadphonesPlugin:
            SwitchProfile(Profile::Type::RoutingHeadset);
            break;
        case Event::HeadphonesUnplug:
            SwitchProfile(Profile::Type::RoutingEarspeaker);
            break;
        case Event::BTHeadsetOn:
            break;
        case Event::BTHeadsetOff:
            break;
        case Event ::CallMute:
            Mute(true);
            break;
        case Event ::CallUnmute:
            Mute(false);
            break;
        case Event ::CallSpeakerphoneOn:
            SwitchProfile(Profile::Type::RoutingSpeakerphone);
            break;
        case Event ::CallSpeakerphoneOff:
            SwitchProfile(Profile::Type::RoutingEarspeaker);
            break;
        default:
            return RetCode::UnsupportedEvent;
        }

        return RetCode::Success;
    }

    audio::RetCode RouterOperation::SwitchProfile(const audio::Profile::Type type)
    {
        auto ret = GetProfile(type);
        if (ret) {
            currentProfile = ret.value();
        }
        else {
            return RetCode::UnsupportedProfile;
        }

        audioDevice =
            bsp::AudioDevice::Create(currentProfile->GetAudioDeviceType(), audioDeviceCallback).value_or(nullptr);
        audioDeviceCellular =
            bsp::AudioDevice::Create(bsp::AudioDevice::Type::Cellular, audioDeviceCellularCallback).value_or(nullptr);

        switch (state) {
        case State::Idle:
        case State::Paused:
            break;

        case State::Active:
            state = State::Idle;
            Start(eventCallback, operationToken);
            break;
        }

        return RetCode::Success;
    }

    bool RouterOperation::Mute(bool enable)
    {
        muteEnable = enable;
        return true;
    }

} // namespace audio
