// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md
//
#pragma once

#include <unordered_map>
#include <mutex.hpp>
#include <functional>
#include <string>
#include <thread.hpp>

#include <iostream>

namespace purefs::utils
{
    class notifier
    {
      public:
        //! Default constructor and destructor
        notifier()                 = default;
        notifier(const notifier &) = delete;
        notifier &operator=(const notifier &) = delete;

        template <typename... Args> class MyEvent
        {
            std::function<void(Args &&...)> fun;

          public:
            void operator()(Args &&... args)
            {
                if (fun)
                    fun(std::forward<Args>(args)...);
                else
                    std::cout << "Notification not handled" << std::endl;
            }
            template <typename H> void registerHandler(H handler)
            {
                fun = handler;
            }
        };

        MyEvent<const std::string & /*path*/, const std::string & /*mode*/> ev_open;
        MyEvent<const std::string & /*from*/, const std::string & /*to*/> ev_move;
        MyEvent<const std::string & /*path*/> ev_remove;

        void compile_check()
        {
            auto on_open = [](const std::string &path, const std::string &mode) -> void {
                std::cout << "File " << path << " open in mode \"" << mode << "\"" << std::endl;
            };
            auto on_move = [](const std::string &from, const std::string &to) -> void {
                std::cout << "File " << from << " moved to " << to << std::endl;
            };
            auto on_remove = [](const std::string &path) -> void {
                std::cout << "File " << path << " removed" << std::endl;
            };
            ev_open.registerHandler(on_open);
            ev_move.registerHandler(on_move);
            ev_remove.registerHandler(on_remove);
            ev_open("abcd.txt", "w");
            ev_move("abcd.txt", "efgh.txt");
            ev_remove("efgh.txt");
        }

        //! Event type notified for indexer
        enum class Event
        {
            initialized, //! Filesystem is intialized
            modified,    //! File is modified
            deleted,     //! File is deleted
            renamed
        };

        /** Notification handler type
         * Notify function callback defiinition
         */
        using NotifyHandler = std::function<void(std::string_view, Event, std::string_view)>;

        /** This method is called by the vfs layer when vfs open file
         * @param[in] filename Opened file path
         * @param[in] mode Open mode
         * @param[in] Handle to opened file
         * @return None
         */
        auto onFileOpen(std::string_view filename, const char *mode, const FILE *file) noexcept -> void;

        /** This method is called by the vfs layer when vfs close file
         * @param[in] filee Closed file handle
         * @return None
         */
        auto onFileClose(const FILE *file) noexcept -> void;

        /** This method is called by the vfs layer when file is removed
         * @param[in] filename Removed file path
         * @return None
         */
        auto onFileRemove(std::string_view filename) noexcept -> void;

        /** This method is called by the vfs layer when the vfs rename file
         * @param[in] new_file New path name for the file
         * @param[in] old_file Old path for the file
         * @return None
         */
        auto onFileRename(std::string_view new_file, std::string_view old_file) noexcept -> void;

        /** This method is called by the vfs layer when the disk is intiialized
         * @return None
         */
        auto onFileSystemInitialized() noexcept -> void
        {
            notify("/", Event::initialized);
        }

        /** Method for register notification handler
         * @note This function is called from the thread contest which use vfs call
         * @param[in] hwnd Notification handler funtion
         * @return None
         */
        auto registerNotificationHandler(NotifyHandler hwnd) -> void
        {
            notificationCallback = hwnd;
            threadHandle         = hwnd ? cpp_freertos::Thread::GetCurrentThreadHandle() : nullptr;
        }

      private:
        /** Private notification helper internal method
         * @param[in] file Modified file path
         * @param[in] event Notification event type
         * @param[in] old_file Old file path
         * @return None
         */
        auto notify(std::string_view file, Event event, std::string_view old_file = "") -> void
        {
            if (threadHandle != cpp_freertos::Thread::GetCurrentThreadHandle() && notificationCallback)
                notificationCallback(file, event, old_file);
        }

      private:
        //! Map for opened handles and paths
        std::unordered_map<const FILE *, std::string> mOpenedMap;
        //! Mutex for unordered map
        cpp_freertos::MutexStandard mMutex;
        //! Notification handler callback
        NotifyHandler notificationCallback;
        TaskHandle_t threadHandle;
    };
} // namespace purefs::utils
