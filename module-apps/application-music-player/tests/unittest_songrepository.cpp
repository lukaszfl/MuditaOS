// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <SongsRepository.hpp>

#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

constexpr auto testDir  = "appmusic-test";
constexpr auto emptyDir = "empty";
auto testDirPath        = fs::path(testDir);
class MockTagsFetcher : public ::app::music_player::AbstractTagsFetcher
{
  public:
    MOCK_METHOD(std::optional<audio::Tags>, getFileTags, (const std::string &filePath), (override));
};

class SongsRepositoryFixture : public ::testing::Test
{
  protected:
    static void SetUpTestSuite()
    {
        if (fs::exists(testDirPath)) {
            TearDownTestSuite();
        }

        fs::create_directory(testDirPath);
        fs::create_directory(testDirPath / emptyDir);
    }

    static void TearDownTestSuite()
    {
        fs::remove_all(testDir);
    }

    auto getMockedRepository(std::string directoryToScan)
    {
        return std::make_unique<app::music_player::SongsRepository>(std::make_unique<MockTagsFetcher>(),
                                                                    directoryToScan);
    }
};

TEST_F(SongsRepositoryFixture, LazyInit)
{
    auto repo       = getMockedRepository(testDir);
    auto musicFiles = repo->getMusicFilesList();
    EXPECT_EQ(musicFiles.size(), 0);
}

TEST_F(SongsRepositoryFixture, Empty)
{
    auto repo = getMockedRepository(testDirPath / emptyDir);
    repo->scanMusicFilesList();

    auto musicFiles = repo->getMusicFilesList();
    EXPECT_EQ(musicFiles.size(), 0);
}

TEST_F(SongsRepositoryFixture, Scan)
{}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
