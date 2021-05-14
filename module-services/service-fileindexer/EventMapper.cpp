// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "EventMapper.hpp"
#include "messages/FileChangeMessage.hpp"

#include <purefs/vfs_subsystem.hpp>

#include <Service/Service.hpp>

#include <filesystem>
#include <fcntl.h>

namespace service::file_indexer
{

    EventMapper::EventMapper(std::shared_ptr<sys::Service> service) : mService(service)
    {
        mThread = cpp_freertos::Thread::GetCurrentThreadHandle();

        auto vfs = purefs::subsystem::vfs_core();
        if (!vfs) {
            LOG_ERROR("VFS not available yet!");
            return;
        }

        vfs->notifier.open.registerHandler([this, vfs](const std::string_view &path, int flags, int fd) -> void {
            std::cout << mService->GetName() << ": File " << path << " (fd " << fd << ") open with flags " << flags
                      << std::endl;
            if ((flags & O_ACCMODE) != O_RDONLY) {
                cpp_freertos::LockGuard _lock(mMutex);
                mOpenFiles[fd] = path;
                /* TODO: find a way to get absolute path */
                // mOpenFiles.emplace(fd, std::filesystem::absolute(path));
                // mOpenFiles.emplace(fd, vfs->absolute_path(path));
                std::cout << mService->GetName() << ": File " << mOpenFiles[fd] << " (fd " << fd
                          << ") enlisted for observation" << std::endl;
            }
        });
        vfs->notifier.close.registerHandler([this](int fd) -> void {
            std::cout << mService->GetName() << ": File (fd " << fd << ") closed" << std::endl;
            cpp_freertos::LockGuard _lock(mMutex);
            auto item = mOpenFiles.find(fd);
            if (item != mOpenFiles.end()) {
                //                for (auto &x : mOpenFiles)
                //                    std::cout << x.first << ": " << x.second << std::endl;
                const auto path = item->second;
                mOpenFiles.erase(item);
                notify(FileChange::Event::modified, path);
                std::cout << mService->GetName() << ": File " << mOpenFiles[fd] << "(fd " << fd
                          << "): removed from the list w/notification sent" << std::endl;
            }
        });
        vfs->notifier.move.registerHandler([this](const std::string_view &from, const std::string_view &to) -> void {
            std::cout << mService->GetName() << ": File " << from << " moved to \"" << to << "\"" << std::endl;
            notify(FileChange::Event::renamed, from, to);
        });
        vfs->notifier.remove.registerHandler([this](const std::string_view &path) -> void {
            std::cout << mService->GetName() << ": File " << path << " removed" << std::endl;
            notify(FileChange::Event::deleted, path);
        });
    }

    EventMapper::~EventMapper()
    {
        auto vfs = purefs::subsystem::vfs_core();
        vfs->notifier.open.unregisterHandler();
        vfs->notifier.close.unregisterHandler();
        vfs->notifier.move.unregisterHandler();
        vfs->notifier.remove.unregisterHandler();
    }

    void EventMapper::notify(FileChange::Event ev, std::string_view path, std::string_view old_path)
    {
        if (mThread == cpp_freertos::Thread::GetCurrentThreadHandle()) {
            return;
        }
        auto message = std::make_shared<FileChangeMessage>(ev, path, old_path);
        mService->bus.sendUnicast(message, mService->GetName());
    }

} // namespace service::file_indexer
