// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SongsPresenter.hpp"

#include <service-audio/AudioMessage.hpp>

namespace app::music_player
{
    SongsPresenter::SongsPresenter(std::shared_ptr<app::music_player::SongsModelInterface> songsModelInterface,
                                   std::unique_ptr<AbstractAudioOperations> &&audioOperations)
        : songsModelInterface{std::move(songsModelInterface)}, audioOperations{std::move(audioOperations)}
    {}

    std::shared_ptr<app::music_player::SongsListItemProvider> SongsPresenter::getMusicPlayerItemProvider() const
    {
        return songsModelInterface;
    }

    void SongsPresenter::createData(std::function<bool(const std::string &fileName)> fn)
    {
        songsModelInterface->createData(fn);
    }

    bool SongsPresenter::play(const std::string &filePath)
    {
        return audioOperations->play(filePath, [this, filePath](audio::RetCode retCode, audio::Token token) {
            if (retCode != audio::RetCode::Success || !token.IsValid()) {
                LOG_ERROR("Playback audio operation failed, retcode = %s, token validity = %d",
                          str(retCode).c_str(),
                          token.IsValid());
                return;
            }
            LOG_FATAL("path = %s", filePath.c_str());
            SongContext songToken{SongState::Playing, token, filePath};
            songsModelInterface->setCurrentSongContext(songToken);
        });
    }

    bool SongsPresenter::pause()
    {
        auto currentFileToken = songsModelInterface->getCurrentFileToken();
        if (currentFileToken) { 
            return audioOperations->pause(currentFileToken.value(), [this](audio::RetCode retCode, audio::Token token) {
                if (retCode != audio::RetCode::Success || !token.IsValid()) {
                    LOG_ERROR("Pause audio operation failed, retcode = %s, token validity = %d",
                              str(retCode).c_str(),
                              token.IsValid());
                    return;
                }
                if (token != songsModelInterface->getCurrentFileToken()) {
                    LOG_ERROR("Pause audio operation failed, wrong token");
                }
                songsModelInterface->setCurrentSongState(SongState::NotPlaying);
            });
        }
        return false;
    }

    bool SongsPresenter::resume()
    {
        auto currentFileToken = songsModelInterface->getCurrentFileToken();
        if (currentFileToken) {
            return audioOperations->resume(currentFileToken.value(), [this](audio::RetCode retCode, audio::Token token) {
                if (retCode != audio::RetCode::Success || !token.IsValid()) {
                    LOG_ERROR("Resume audio operation failed, retcode = %s, token validity = %d",
                              str(retCode).c_str(),
                              token.IsValid());
                    return;
                }
                if (token != songsModelInterface->getCurrentFileToken()) {
                    LOG_ERROR("Resume audio operation failed, wrong token");
                }
                songsModelInterface->setCurrentSongState(SongState::Playing);
            });
        }
        return false;
    }

    bool SongsPresenter::stop()
    {
        auto currentFileToken = songsModelInterface->getCurrentFileToken();
        if (currentFileToken) {
            return audioOperations->stop(currentFileToken.value(), [this](audio::RetCode retCode, audio::Token token) {
                if (retCode != audio::RetCode::Success || !token.IsValid()) {
                    LOG_ERROR("Stop audio operation failed, retcode = %s, token validity = %d",
                              str(retCode).c_str(),
                              token.IsValid());
                    return;
                }
                if (token != songsModelInterface->getCurrentFileToken()) {
                    LOG_ERROR("Playback audio operation failed, wrong token");
                }
                songsModelInterface->clearCurrentSongContext();
            });
        }
        return false;
    }

    void SongsPresenter::togglePlaying()
    {
        if (songsModelInterface->isSongPlaying()) {
            pause();
        }
        else {
            resume();
        }
    }

    sys::MessagePointer SongsPresenter::handleAudioNotification(const AudioNotificationMessage *notification)
    {
        if(notification == nullptr)
        {
            return sys::msgNotHandled();
        }
        if(notification->type == AudioNotificationMessage::Type::Stop && notification->token == songsModelInterface->getCurrentFileToken())
        {
            LOG_FATAL("handleAudioNotification stop");
            songsModelInterface->clearCurrentSongContext();
            return sys::msgHandled();
        }
        return sys::msgNotHandled();
    }

} // namespace app::music_player
