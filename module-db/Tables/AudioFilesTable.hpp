// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "Table.hpp"
#include "Record.hpp"

enum class AudioSpeed
{
    Unset    = 0,
    Slow     = 1,
    Medium   = 2,
    Hardcore = 4,
};

enum class AudioChannels
{
    Mono   = 1,
    Stereo = 2,
};

struct AudioFileRecord : public Record
{
    std::string mediaType{};
    std::string title{};
    std::string artist{};
    std::string album{};
    AudioSpeed speed{AudioSpeed::Unset};
    std::string genre{};
    std::chrono::seconds startTime{0};
    std::chrono::seconds endTime{0};
    std::chrono::seconds songLength{0};
    std::uint32_t fileLength{0};
    std::uint32_t bitrate{0};
    AudioChannels channels{AudioChannels::Stereo};
};

enum class AudioFileField
{
    MediaType,
    Title,
    Artist,
    Album,
    Speed,
    Genre,
    StartTime,
    EndTime,
    SongLength,
    FileLength,
    Bitrate,
    Channels,
};

class AudioFilesTable : public Table<AudioFileRecord, AudioFileField>
{
  public:
    AudioFilesTable(Database *db);

    bool create() override;
    bool add(AudioFileRecord entry) override;
    bool removeById(uint32_t id) override;
    bool update(AudioFileRecord entry) override;

    AudioFileRecord getById(uint32_t id) override;
    std::vector<AudioFileRecord> getLimitOffset(uint32_t offset, uint32_t limit) override;
    std::vector<AudioFileRecord> getLimitOffsetByField(uint32_t offset,
                                                       uint32_t limit,
                                                       AudioFileField field,
                                                       const char *str) override;

    uint32_t count() override;
    uint32_t countByFieldId(const char *field, uint32_t id) override;
};
