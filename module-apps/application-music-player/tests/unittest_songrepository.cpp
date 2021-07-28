// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <SongsRepository.hpp>

class MockTagsFetcher : public ::app::music_player::AbstractTagsFetcher
{
  public:
    MOCK_METHOD(std::optional<audio::Tags>, getFileTags, (const std::string &filePath), (override));
};

auto getMockedRepository()
{
    return std::make_unique<app::music_player::SongsRepository>(std::make_unique<MockTagsFetcher>());
}

TEST(SongsRepository, LazyInit)
{
    auto repo       = getMockedRepository();
    auto musicFiles = repo->getMusicFilesList();
    EXPECT_EQ(musicFiles.size(), 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
