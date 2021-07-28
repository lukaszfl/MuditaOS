// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <apps-common/Application.hpp>
#include <Audio/decoder/Decoder.hpp>
#include <cstddef>

namespace app::music_player
{
    class AbstractSongsRepository
    {
      public:
        virtual ~AbstractSongsRepository() noexcept = default;

        virtual void scanMusicFilesList() = 0;
        virtual std::vector<audio::Tags> getMusicFilesList() const = 0;
        virtual std::optional<audio::Tags> getFileTags(const std::string &filePath) = 0;
        virtual std::size_t getFileIndex(const std::string &filePath) = 0;
    };

    class SongsRepository : public AbstractSongsRepository
    {
      public:
        explicit SongsRepository(Application *application);

        void scanMusicFilesList() override;
        std::vector<audio::Tags> getMusicFilesList() const override;
        std::optional<audio::Tags> getFileTags(const std::string &filePath) override;
        std::size_t getFileIndex(const std::string &filePath) override;

      private:
        Application *application = nullptr;
        std::vector<audio::Tags> musicFiles; 
    };
} // namespace app::music_player
