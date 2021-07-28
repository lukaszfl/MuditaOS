// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SongContext.hpp"
#include <optional>

namespace app::music_player
{
    void SongContext::clear()
    {
        currentSongState = SongState::NotPlaying;
        currentFileToken = std::nullopt;
        filePath = "";
    }
}
