// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <mutex.hpp>
#include <unordered_map>
#include <memory>

#include "messages/FileChangeMessage.hpp"

namespace sys
{
    class Service;
}

namespace service::file_indexer
{
    class EventMapper
    {
      public:
        explicit EventMapper(std::shared_ptr<sys::Service> service);
        virtual ~EventMapper();

      private:
        //! Mutex for unordered map
        cpp_freertos::MutexStandard mMutex;
        //! Map for opened handles and paths
        std::unordered_map<int, std::string> mOpenFiles;
        //! Send bus message
        void notify(FileChange::Event ev, std::string_view path, std::string_view old_path = "");
        //! File Indexer Service
        std::shared_ptr<sys::Service> mService;
        //! Creator thread
        TaskHandle_t mThread;
    };
} // namespace service::file_indexer
