// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md
#pragma once

#include <Audio/decoder/Decoder.hpp>

namespace app::music_player
{

    enum class SongState
    {
        Playing,
        NotPlaying
    };

    class SongContext
    {
      public:
        SongState currentSongState = SongState::NotPlaying;
        std::optional<audio::Token> currentFileToken;
        std::string filePath;

        void clear();
    };

} // namespace app::music_player
