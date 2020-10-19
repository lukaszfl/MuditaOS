#pragma once

#include "Audio.hpp"
#include "AudioCommon.hpp"

namespace audio
{
    class Playback
    {
      public:
        Playback(PlaybackType playbackType, Token token, Position position, std::string fileName)
            : playbackType(playbackType), token(token), position(position), fileName(fileName)
        {}

        PlaybackType playbackType;
        Token token;
        Position position;
        std::string fileName;
    };

    class AudioManager
    {
      public:
        AudioManager(AsyncCallback asyncCallback, DbCallback dbCallback)
            : audio(asyncCallback, dbCallback), dbCallback(dbCallback)
        {}

        auto startPlayback(const std::string fileName, const audio::PlaybackType &playbackType)
            -> std::pair<Token, RetCode>;
        auto startRecorder(const std::string fileName) -> std::pair<Token, RetCode>;
        auto startRouter() -> std::pair<Token, RetCode>;
        auto stop(const std::vector<Token> &tokens) -> std::vector<std::pair<Token, RetCode>>;
        auto stop(const std::vector<PlaybackType> &types) -> std::vector<std::pair<Token, RetCode>>;
        auto stopAll() -> std::vector<std::pair<Token, RetCode>>;
        auto pause(const Token &token) -> audio::RetCode;
        auto resume(const Token &token) -> audio::RetCode;
        auto getFileTags(const std::string &fileName) -> std::pair<Tags, RetCode>;
        auto sendEvent(std::shared_ptr<Event> evt) -> audio::RetCode;

        auto getAudio() -> Audio &
        {
            return audio;
        }

      private:
        enum class VibrationType
        {
            None,
            OneShot,
            Continuous
        };
        enum class VibrationStatus
        {
            On,
            Off
        };

        void VibrationUpdate(const PlaybackType &type);
        auto GetVibrationType(const audio::PlaybackType &type) -> VibrationType;

        auto IsVibrationMotorOn()
        {
            return vibrationMotorStatus == VibrationStatus::On;
        }

        auto isRouting() -> bool;

        auto getPlaybackPriority(const audio::PlaybackType &type) const -> uint8_t;
        constexpr auto isMergable(const audio::PlaybackType &type) const -> bool;
        constexpr auto isResumable(const audio::PlaybackType &type) const -> bool;
        auto IsVibrationEnabled(const audio::PlaybackType &type) -> bool;
        auto IsPlaybackEnabled(const audio::PlaybackType &type) -> bool;

        void pushCurrentToStack();

        Audio audio;
        VibrationStatus vibrationMotorStatus = VibrationStatus::Off;
        std::vector<Playback> playbackStack;
        audio::Token refToken;
        DbCallback dbCallback;
    };
} // namespace audio
