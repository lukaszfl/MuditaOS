#include "AudioManager.hpp"

#include <Service/Bus.hpp>

using namespace audio;

// below static methods will be replaced by final vibration API
static void ExtVibrateOnce()
{
    LOG_ERROR("[Vibration] - Unimplemented - vibration one shot");
}
static void ExtVibrationStart()
{
    LOG_ERROR("[Vibration] - Unimplemented - vibration start");
}
static void ExtVibrationStop()
{
    LOG_ERROR("[Vibration] - Unimplemented - vibration stop");
}

std::pair<Token, RetCode> AudioManager::startPlayback(const std::string fileName, const PlaybackType &playbackType)
{
    // TODO vibration
    if (isRouting()) {
        return std::make_pair(Token::MakeBadToken(), RetCode::Ignored);
    }

    // auto newToken = refToken.IncrementToken();

    if (!IsPlaybackEnabled(playbackType)) {
        return std::make_pair(Token::MakeBadToken(), RetCode::Ignored);
    }

    if (audio.GetCurrentState() != Audio::State::Idle) {
        auto currentPlaybackType = audio.GetCurrentOperationPlaybackType();
        // handle priorities
        if (getPlaybackPriority(playbackType) > getPlaybackPriority(currentPlaybackType)) {
            return std::make_pair(Token::MakeBadToken(), RetCode::Ignored);
        }
        else if (getPlaybackPriority(playbackType) <= getPlaybackPriority(currentPlaybackType)) {
            if (currentPlaybackType == playbackType && isMergable(currentPlaybackType)) {
                return std::make_pair(Token::MakeBadToken(), RetCode::Merged);
            }
            else if (isResumable(currentPlaybackType)) {
                pushCurrentToStack();
                audio.Stop();
            }
        }
    }
    auto token   = refToken.IncrementToken();
    auto retCode = audio.Start(Operation::Type::Playback, token, fileName.c_str(), playbackType);

    return std::make_pair(retCode == RetCode::Success ? token : Token::MakeBadToken(), retCode);
}

std::pair<Token, RetCode> AudioManager::startRecorder(const std::string fileName)
{
    if (audio.GetCurrentState() != Audio::State::Idle) {
        return std::make_pair(Token::MakeBadToken(), RetCode::Ignored);
    }

    auto token   = refToken.IncrementToken();
    auto retCode = audio.Start(Operation::Type::Recorder, token, fileName.c_str());

    return std::make_pair(retCode == RetCode::Success ? token : Token::MakeBadToken(), retCode);
}

std::pair<Token, RetCode> AudioManager::startRouter()
{
    if (isRouting()) {
        return std::make_pair(Token::MakeBadToken(), RetCode::Ignored);
    }

    if (audio.GetCurrentState() != Audio::State::Idle) {
        auto currentPlaybackType = audio.GetCurrentOperationPlaybackType();
        if (isResumable(currentPlaybackType)) {
            pushCurrentToStack();
            audio.Stop();
        }
    }

    auto token   = refToken.IncrementToken();
    auto retCode = audio.Start(Operation::Type::Router, token);

    return std::make_pair(retCode == RetCode::Success ? token : Token::MakeBadToken(), retCode);
}

std::vector<std::pair<Token, RetCode>> AudioManager::stop(const std::vector<Token> &tokens)
{
    std::vector<std::pair<Token, RetCode>> stopped;

    for (auto &token : tokens) {
        if (!token.IsValid()) {
            stopped.emplace_back(std::make_pair(token, RetCode::TokenNotFound));
            continue;
        }

        auto currentToken = audio.GetCurrentOperation().GetToken();
        if (currentToken == token) {
            stopped.emplace_back(std::make_pair(token, audio.Stop()));
        }
        else {
            auto it = std::find_if(
                playbackStack.begin(), playbackStack.end(), [&token](auto &e) { return e.token == token; });

            if (it != playbackStack.end()) {
                playbackStack.erase(it);
                stopped.emplace_back(std::make_pair(token, RetCode::Success));
            }
            else {
                stopped.emplace_back(std::make_pair(token, RetCode::TokenNotFound));
            }
        }
    }

    /*for (auto &i : stopped) {
        auto broadMsg = std::make_shared<AudioStopResponse>(i.second, i.first);
        sys::Bus::SendMulticast(broadMsg, sys::BusChannels::ServiceAudioNotifications, &service);
    }*/
    return stopped;
}

std::vector<std::pair<Token, RetCode>> AudioManager::stop(const std::vector<PlaybackType> &types)
{
    std::vector<Token> tokens;

    auto currentPlayback = audio.GetCurrentOperationPlaybackType();
    for (auto &t : types) {
        if (t == currentPlayback) {
            tokens.push_back(audio.GetCurrentOperation().GetToken());
        }
    }

    auto it = playbackStack.begin();
    while (it != playbackStack.end()) {
        it = std::find_first_of(
            it, playbackStack.end(), types.begin(), types.end(), [](auto &p, auto &t) { return p.playbackType == t; });
        if (it != playbackStack.end()) {
            tokens.push_back(it->token);
            it++;
        }
    }

    return stop(tokens);
}

std::vector<std::pair<Token, RetCode>> AudioManager::stopAll()
{
    std::vector<Token> tokens;
    tokens.push_back(audio.GetCurrentOperation().GetToken());
    for (auto &p : playbackStack) {
        tokens.push_back(p.token);
    }

    return stop(tokens);
}

RetCode AudioManager::pause(const Token &token)
{
    auto currentToken = audio.GetCurrentOperation().GetToken();
    if (currentToken == token) {
        return audio.Pause();
    }

    for (auto &p : playbackStack) {
        if (p.token == token) {
            return RetCode::InvokedInIncorrectState;
        }
    }

    return RetCode::TokenNotFound;
}

RetCode AudioManager::resume(const Token &token)
{
    auto currentToken = audio.GetCurrentOperation().GetToken();
    if (currentToken == token) {
        return audio.Resume();
    }

    for (auto &p : playbackStack) {
        if (p.token == token) {
            return RetCode::InvokedInIncorrectState;
        }
    }

    return RetCode::TokenNotFound;
}

std::pair<Tags, RetCode> AudioManager::getFileTags(const std::string &fileName)
{
    if (auto tag = Audio::GetFileTags(fileName.c_str())) {
        // tag.value()
        return std::make_pair(tag.value(), RetCode::Success);
    }
    return std::make_pair(Tags(), RetCode::FileDoesntExist);
}

RetCode AudioManager::sendEvent(std::shared_ptr<Event> evt)
{
    return audio.SendEvent(evt);
}

AudioManager::VibrationType AudioManager::GetVibrationType(const PlaybackType &type)
{
    if (!IsVibrationEnabled(type)) {
        return VibrationType::None;
    }

    if (type == PlaybackType::CallRingtone) {
        return VibrationType::Continuous;
    }
    else if (type == PlaybackType::Notifications || type == PlaybackType::TextMessageRingtone) {
        return VibrationType::OneShot;
    }
    return VibrationType::None;
}

void AudioManager::VibrationUpdate(const PlaybackType &type)
{
    auto vibrationType = GetVibrationType(type);
    if (vibrationType == VibrationType::OneShot && !IsVibrationMotorOn()) {
        ExtVibrateOnce();
    }
    else if (vibrationType == VibrationType::Continuous) {
        if (IsVibrationMotorOn()) {
            ExtVibrationStart();
            // vibrationMotorStatus = AudioMux::VibrationStatus::On;
        }
        else {
            ExtVibrationStop();
            // vibrationMotorStatus = AudioMux::VibrationStatus::Off;
        }
    }
}

void AudioManager::pushCurrentToStack()
{
    playbackStack.push_back(Playback(audio.GetCurrentOperationPlaybackType(),
                                     audio.GetCurrentOperation().GetToken(),
                                     audio.GetPosition(),
                                     audio.GetCurrentOperation().GetFilePath()));
}

bool AudioManager::isRouting()
{
    return audio.GetCurrentState() == Audio::State::Routing;
}

constexpr bool AudioManager::isResumable(const PlaybackType &type) const
{
    return type == PlaybackType::Multimedia;
}

constexpr bool AudioManager::isMergable(const PlaybackType &type) const
{
    return !(type == PlaybackType::Multimedia);
}

bool AudioManager::IsVibrationEnabled(const PlaybackType &type)
{
    const auto dbLoudspeakerVolumePath = dbPath(Setting::EnableVibration, type, Profile::Type::Idle);
    return dbCallback(dbLoudspeakerVolumePath, false);
}

bool AudioManager::IsPlaybackEnabled(const PlaybackType &type)
{
    const auto dbLoudspeakerVolumePath = dbPath(Setting::EnableSound, type, Profile::Type::Idle);
    return dbCallback(dbLoudspeakerVolumePath, true);
}

uint8_t AudioManager::getPlaybackPriority(const PlaybackType &type) const
{
    const auto &pmap = PlaybackTypePriority;
    if (pmap.find(type) != pmap.end()) {
        return pmap.at(type);
    }
    return static_cast<uint8_t>(PlaybackType::Last);
}
