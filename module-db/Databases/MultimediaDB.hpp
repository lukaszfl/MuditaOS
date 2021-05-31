// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "Database/Database.hpp"

#include "Tables/AudioFilesTable.hpp"

class MultimediaDB : public Database
{
  public:
    MultimediaDB(const char *name) : Database(name)
    {}

    AudioFilesTable audioFiles{this};
};
