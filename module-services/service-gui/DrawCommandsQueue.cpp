// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "DrawCommandsQueue.hpp"

namespace service::gui
{
    namespace
    {
        constexpr std::chrono::milliseconds WaitTimeout{5000};
    } // namespace

    DrawCommandsQueue::DrawCommandsQueue(std::size_t expectedSize,
                                         std::unique_ptr<SynchronizationMechanism> &&synchronization)
        : synchronization{std::move(synchronization)}
    {
        queue.reserve(expectedSize);
    }

    void DrawCommandsQueue::enqueue(QueueItem &&item)
    {
        cpp_freertos::LockGuard lock{queueMutex};
        queue.push_back(std::move(item));

        synchronization->notify();
    }

    auto DrawCommandsQueue::dequeue() -> QueueItem
    {
        cpp_freertos::LockGuard lock{queueMutex};
        synchronization->wait(queueMutex, WaitTimeout, [this]() { return !queue.empty(); });

        auto item = std::move(queue.front());
        queue.erase(queue.begin());
        return item;
    }

    auto DrawCommandsQueue::getMaxRefreshModeAndClear() -> ::gui::RefreshModes
    {
        cpp_freertos::LockGuard lock{queueMutex};
        const auto deepRefreshRequested = std::any_of(queue.begin(), queue.end(), [](const auto &item) {
            return item.refreshMode == ::gui::RefreshModes::GUI_REFRESH_DEEP;
        });
        const auto maxRefreshMode =
            deepRefreshRequested ? ::gui::RefreshModes::GUI_REFRESH_DEEP : ::gui::RefreshModes::GUI_REFRESH_FAST;

        queue.clear();
        return maxRefreshMode;
    }

    void DrawCommandsQueue::clear()
    {
        cpp_freertos::LockGuard lock{queueMutex};
        queue.clear();
    }

    auto DrawCommandsQueue::size() const noexcept -> QueueContainer::size_type
    {
        cpp_freertos::LockGuard lock{queueMutex};
        return queue.size();
    }
} // namespace service::gui
