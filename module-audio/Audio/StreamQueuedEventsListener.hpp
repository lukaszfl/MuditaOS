#pragma once

#include "Stream.hpp"

#include <queue.hpp>

#include <cstdint>
#include <string>
#include <utility>

namespace audio
{
    class StreamQueuedEventsListener : public Stream::EventListener
    {
      private:
        static constexpr std::size_t defaultEventsQueueCapacity = 16U;
        struct EventStorage
        {
            Stream *stream;
            Stream::Event event;
        };

      public:
        using queueInfo   = std::pair<QueueHandle_t, std::string>;
        using queuedEvent = std::pair<Stream *, Stream::Event>;

        StreamQueuedEventsListener(std::size_t eventsQueueCapacity = defaultEventsQueueCapacity);

        void onEvent(Stream *stream, Stream::Event event, Stream::EventSourceMode source);

        queuedEvent waitForEvent();
        std::size_t getEventsCount() const;
        queuedEvent getEvent();
        queueInfo getQueueInfo() const;

      private:
        QueueHandle_t eventsQueue;
    };

}; // namespace audio
