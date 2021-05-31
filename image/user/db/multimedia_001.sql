-- Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
-- For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

CREATE TABLE IF NOT EXISTS audio_files
(
    _id         INTEGER PRIMARY KEY,
    media_type  TEXT,
    title       TEXT,
    artist      TEXT,
    album       TEXT,
    speed       INTEGER,
    genre       TEXT,
    start_time  INTEGER,
    end_time    INTEGER,
    song_length INTEGER,
    file_length INTEGER,
    bitrate     INTEGER,
    channels    INTEGER
);

