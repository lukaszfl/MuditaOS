// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <Service/Message.hpp>

namespace service::file_indexer
{
    namespace FileChange
    {
        enum class Event
        {
            modified, //! File is modified
            deleted,  //! File is deleted
            renamed
        };
    }

    class FileChangeMessage final : public sys::DataMessage
    {
      public:
        FileChangeMessage(FileChange::Event ev, std::string_view path, std::string_view old_path);
        virtual ~FileChangeMessage()           = default;
        FileChangeMessage(FileChangeMessage &) = delete;
        FileChangeMessage &operator=(FileChangeMessage &) = delete;
        [[nodiscard]] auto newPath() const noexcept
        {
            return mPath;
        }
        [[nodiscard]] auto oldPath() const noexcept
        {
            return mOldPath;
        }
        [[nodiscard]] auto event() const noexcept
        {
            return mEvent;
        }

      private:
        const std::string mPath;
        const FileChange::Event mEvent;
        const std::string mOldPath;
    };
} // namespace service::file_indexer
