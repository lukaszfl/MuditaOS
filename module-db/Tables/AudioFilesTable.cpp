// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "AudioFilesTable.hpp"

const char *fieldName(AudioFileField field)
{
    switch (field) {
    case AudioFileField::MediaType:
        return "media_type";
    case AudioFileField::Title:
        return "title";
    case AudioFileField::Artist:
        return "artist";
    case AudioFileField::Album:
        return "album";
    case AudioFileField::Speed:
        return "speed";
    case AudioFileField::Genre:
        return "genre";
    case AudioFileField::StartTime:
        return "start_time";
    case AudioFileField::EndTime:
        return "end_time";
    case AudioFileField::SongLength:
        return "song_length";
    case AudioFileField::FileLength:
        return "file_length";
    case AudioFileField::Bitrate:
        return "bitrate";
    case AudioFileField::Channels:
        return "channels";
    }

    return nullptr;
}

AudioFileRecord audioFileRecordFrom(const QueryResult &res)
{
    return AudioFileRecord{
        res[0].getUInt32(),
        res[1].getString(),
        res[2].getString(),
        res[3].getString(),
        res[4].getString(),
        static_cast<AudioSpeed>(res[5].getUInt32()),
        res[6].getString(),
        std::chrono::seconds{res[7].getUInt32()},
        std::chrono::seconds{res[8].getUInt32()},
        std::chrono::seconds{res[9].getUInt32()},
        res[10].getUInt32(),
        res[11].getUInt32(),
        static_cast<AudioChannels>(res[12].getUInt32()),
    };
}

std::vector<AudioFileRecord> audioFileRecordsFrom(QueryResult &res)
{
    std::vector<AudioFileRecord> v;
    v.reserve(res.getRowCount());

    do {
        v.emplace_back(audioFileRecordFrom(res));
    } while (res.nextRow());

    return v;
}

AudioFilesTable::AudioFilesTable(Database *db) : Table(db)
{}

bool AudioFilesTable::create()
{
    return true;
}

bool AudioFilesTable::add(AudioFileRecord entry)
{
    return db->execute("INSERT or ignore INTO audio_files"
                       "( media_type, title, artist, album, speed, genre, start_time, end_time, song_length, "
                       "file_length, bitrate, channels )"
                       "VALUES ( '%q', '%q', '%q', '%q', %d, '%q', %d, %d, %d, %d, %d, %d );",
                       entry.mediaType.c_str(),
                       entry.title.c_str(),
                       entry.artist.c_str(),
                       entry.album.c_str(),
                       entry.speed,
                       entry.genre.c_str(),
                       entry.startTime,
                       entry.endTime,
                       entry.songLength,
                       entry.fileLength,
                       entry.bitrate,
                       entry.channels);
}

bool AudioFilesTable::removeById(uint32_t id)
{
    return db->execute("DELETE FROM audio_files where _id = %u;", id);
}

bool AudioFilesTable::update(AudioFileRecord entry)
{
    return db->execute(
        "UPDATE audio_files SET"
        "media_type = '%q', title = '%q', artist = '%q', album = '%q', speed = %d, genre = '%q', start_time = %d, "
        "end_time = %d, song_length = %d, file_length = %d, bitrate = %d, channels = %d )"
        "WHERE _id=%lu;",
        entry.mediaType.c_str(),
        entry.title.c_str(),
        entry.artist.c_str(),
        entry.album.c_str(),
        entry.speed,
        entry.genre.c_str(),
        entry.startTime,
        entry.endTime,
        entry.songLength,
        entry.fileLength,
        entry.bitrate,
        entry.channels,
        entry.ID);
}

AudioFileRecord AudioFilesTable::getById(uint32_t id)
{
    const auto res = db->query("SELECT * FROM audio_files WHERE _id= %u;", id);

    if (!res) {
        return AudioFileRecord{};
    }

    if (res->getRowCount() != 1) {
        return AudioFileRecord{};
    }

    return audioFileRecordFrom(*res);
}

std::vector<AudioFileRecord> AudioFilesTable::getLimitOffset(uint32_t offset, uint32_t limit)
{
    const auto res = db->query("SELECT * from audio_files ORDER BY title DESC LIMIT %lu OFFSET %lu;", limit, offset);

    if (!res) {
        return {};
    }

    if (res->getRowCount() < 1) {
        return {};
    }

    return audioFileRecordsFrom(*res);
}

std::vector<AudioFileRecord> AudioFilesTable::getLimitOffsetByField(uint32_t offset,
                                                                    uint32_t limit,
                                                                    AudioFileField field,
                                                                    const char *str)
{
    const auto res = db->query("SELECT * from audio_files WHERE %s = '%q' ORDER BY title DESC LIMIT %lu OFFSET %lu;",
                               fieldName(field),
                               str,
                               limit,
                               offset);

    if (!res) {
        return {};
    }

    if (res->getRowCount() < 1) {
        return {};
    }

    return audioFileRecordsFrom(*res);
}

uint32_t AudioFilesTable::count()
{
    const auto res = db->query("SELECT COUNT(*) FROM audio_files;");

    if (!res) {
        return 0;
    }

    if (res->getRowCount() != 1) {
        return 0;
    }

    return (*res)[0].getUInt32();
}

uint32_t AudioFilesTable::countByFieldId(const char *field, uint32_t id)
{
    const auto res = db->query("SELECT COUNT(*) FROM audio_files WHERE %q=%u;", field, id);

    if (!res) {
        return 0;
    }

    if (res->getRowCount() != 1) {
        return 0;
    }

    return (*res)[0].getUInt32();
}
