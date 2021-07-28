// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SongsRepository.hpp"

#include <algorithm>
#include <log.hpp>
#include <service-audio/AudioServiceAPI.hpp>
#include <service-audio/AudioServiceName.hpp>
#include <time/ScopedTime.hpp>
#include <purefs/filesystem_paths.hpp>
#include <service-audio/AudioMessage.hpp>

#include <filesystem>

namespace app::music_player
{
    ServiceAudioTagsFetcher::ServiceAudioTagsFetcher(Application *application) : application(application)
    {}

    std::optional<audio::Tags> ServiceAudioTagsFetcher::getFileTags(const std::string &filePath)
    {
        return AudioServiceAPI::GetFileTags(application, filePath);
    }

    SongsRepository::SongsRepository(std::unique_ptr<AbstractTagsFetcher> tagsFetcher)
        : tagsFetcher(std::move(tagsFetcher))
    {}

    void SongsRepository::scanMusicFilesList()
    {
        const auto musicFolder = purefs::dir::getUserDiskPath() / "music";
        musicFiles.clear();

        LOG_INFO("Scanning music folder: %s", musicFolder.c_str());
        {
            auto time = utils::time::Scoped("fetch tags time");
            for (const auto &entry : std::filesystem::directory_iterator(musicFolder)) {
                if (!std::filesystem::is_directory(entry)) {
                    const auto &filePath = entry.path();
                    const auto fileTags  = tagsFetcher->getFileTags(filePath);
                    if (fileTags) {
                        musicFiles.push_back(*fileTags);
                        LOG_DEBUG(" - file %s found", entry.path().c_str());
                    }
                    else {
                        LOG_ERROR("Not an audio file %s", entry.path().c_str());
                    }
                }
            }
        }
        LOG_INFO("Total number of music files found: %u", static_cast<unsigned int>(musicFiles.size()));
    }

    std::vector<audio::Tags> SongsRepository::getMusicFilesList() const
    {
        return musicFiles;
    }

    std::size_t SongsRepository::getFileIndex(const std::string &filePath)
    {
        auto it = std::find_if(musicFiles.begin(), musicFiles.end(), [filePath](const auto &musicFile) {
            return musicFile.filePath == filePath;
        });

        if (it != musicFiles.end()) {
            return std::distance(musicFiles.begin(), it);
        }

        return static_cast<size_t>(-1);
    }
} // namespace app::music_player
