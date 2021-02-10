// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "FileChangeMessage.hpp"

namespace service::file_indexer
{
    FileChangeMessage::FileChangeMessage(FileChange::Event ev, std::string_view path, std::string_view old_path)
        : DataMessage(MessageType::FileContentModified), mPath(path), mEvent(ev), mOldPath(old_path)
    {}
} // namespace service::msg
