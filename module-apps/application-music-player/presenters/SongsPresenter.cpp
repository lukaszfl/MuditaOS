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
            getView()->updateSongsState();
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
                    return;
                }
                songsModelInterface->setCurrentSongState(SongState::NotPlaying);
                getView()->updateSongsState();
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
                    return;
                }
                songsModelInterface->setCurrentSongState(SongState::Playing);
                getView()->updateSongsState();
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
                if (token == songsModelInterface->getCurrentFileToken()) { // TODO: alek: need to be !=
                    LOG_ERROR("Playback audio operation failed, wrong token");
                    return;
                }
                songsModelInterface->clearCurrentSongContext();
                getView()->updateSongsState();
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

    bool SongsPresenter::handleAudioStopNotifiaction(audio::Token token)
    {
        if(token == songsModelInterface->getCurrentFileToken())
        {
            songsModelInterface->clearCurrentSongContext();
            getView()->updateSongsState();
            getView()->refreshWindow();
            return true;
        }
        return false;
    }

} // namespace app::music_player
