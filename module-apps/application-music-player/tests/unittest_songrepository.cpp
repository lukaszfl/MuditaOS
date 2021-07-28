// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <SongsRepository.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>

using testing::Return;
namespace fs = std::filesystem;

constexpr auto testDir  = "appmusic-test";
constexpr auto emptyDir = "empty";
constexpr auto musicDir = "music";
constexpr auto bazDir   = "bazdir";
auto testDirPath        = fs::path(testDir);
auto emptyDirPath       = testDirPath / emptyDir;
auto musicDirPath       = testDirPath / musicDir;
auto bazDirPath         = musicDirPath / bazDir;

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
        fs::create_directory(emptyDirPath);
        fs::create_directory(musicDirPath);

        createFile(musicDirPath / "foo");
        createFile(musicDirPath / "bar");

        fs::create_directory(bazDirPath);

        createFile(bazDirPath / "baz");

        fs::create_directory(musicDirPath / "bazzinga");
    }

    static void createFile(const std::string &path)
    {
        std::ofstream file(path);
        file << "app music test file";
    }

    static void TearDownTestSuite()
    {
        fs::remove_all(testDir);
    }

    auto getMockedRepository(const std::string &directoryToScan)
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

TEST_F(SongsRepositoryFixture, ScanEmptyFiles)
{
    auto tagsFetcherMock = std::make_unique<MockTagsFetcher>();
    auto rawMock         = tagsFetcherMock.get();
    auto repo = std::make_unique<app::music_player::SongsRepository>(std::move(tagsFetcherMock), musicDirPath);

    EXPECT_CALL(*rawMock, getFileTags).Times(2);
    repo->scanMusicFilesList();

    auto musicFiles = repo->getMusicFilesList();
    EXPECT_EQ(musicFiles.size(), 0);
}

TEST_F(SongsRepositoryFixture, ScanWithTagsReturn)
{
    auto tagsFetcherMock = std::make_unique<MockTagsFetcher>();
    auto rawMock         = tagsFetcherMock.get();
    auto repo = std::make_unique<app::music_player::SongsRepository>(std::move(tagsFetcherMock), musicDirPath);

    auto fooTags = ::audio::Tags();
    auto barTags = ::audio::Tags();

    fooTags.title = "foo";
    barTags.title = "bar";

    ON_CALL(*rawMock, getFileTags(fs::path(musicDirPath / "foo").c_str())).WillByDefault(Return(fooTags));
    ON_CALL(*rawMock, getFileTags(fs::path(musicDirPath / "bar").c_str())).WillByDefault(Return(barTags));
    EXPECT_CALL(*rawMock, getFileTags).Times(2);
    repo->scanMusicFilesList();

    auto musicFiles = repo->getMusicFilesList();
    EXPECT_EQ(musicFiles.size(), 2);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
