// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <catch2/catch.hpp>

#include <Databases/MultimediaDB.hpp>

TEST_CASE("Multimedia DB tests")
{
    REQUIRE(Database::initialize());

    const auto smsPath = (std::filesystem::path{"sys/user"} / "multimedia.db");
    if (std::filesystem::exists(smsPath)) {
        REQUIRE(std::filesystem::remove(smsPath));
    }

    MultimediaDB db(smsPath.c_str());
    REQUIRE(db.isInitialized());

    REQUIRE(Database::deinitialize());

    const auto PageSize = 8;

    REQUIRE(db.audioFiles.count() == 0);
    REQUIRE(db.audioFiles.getLimitOffset(0, PageSize).empty());

    REQUIRE(db.audioFiles.add(AudioFileRecord{}));

    REQUIRE(db.audioFiles.count() == 1);
    REQUIRE(db.audioFiles.getLimitOffset(0, PageSize).size() == 1);
}
