// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <Service/Service.hpp>
#include <module-sys/Timers/TimerHandle.hpp>

#include <list>

namespace service::file_indexer
{
    class FileChangeMessage;

    // RFC6838 IANA MIME types
    enum class mimeType
    {
        _none_,
        application,
        audio,
        example,
        font,
        image,
        text,
        video
    };
    class StartupIndexer
    {
        static constexpr auto timer_indexing_time = 100U;

      public:
        StartupIndexer()                       = default;
        ~StartupIndexer()                      = default;
        StartupIndexer(const StartupIndexer &) = delete;
        StartupIndexer &operator=(StartupIndexer) = delete;
        auto start(std::shared_ptr<sys::Service> svc) -> void
        {
            collectStartupFiles();
            setupTimers(svc);
        }
        static auto getFileType(std::string_view path) -> mimeType;

      private:
        // Collect startup files when service starts
        void collectStartupFiles();
        // Setup timers for notification
        void setupTimers(std::shared_ptr<sys::Service> svc);

      private:
        std::list<std::shared_ptr<FileChangeMessage>> mMsgs;
        sys::TimerHandle mIdxTimer;
    };
} // namespace service::detail
