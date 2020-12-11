#pragma once

#include "Stream.hpp"

#include <queue.hpp>

#include <cstdint>
#include <string>
#include <utility>

namespace audio
{
    using namespace cpp_freertos;

    class StreamQueuedEventsListener : public Stream::EventListener
    {
      private:
        struct EventStorage
        {
            Stream *stream;
            Stream::Event event;
        };

      public:
        using queueInfo   = std::pair<QueueHandle_t, std::string>;
        using queuedEvent = std::pair<Stream *, Stream::Event>;
        static constexpr auto listenerElementSize = sizeof(EventStorage);

        StreamQueuedEventsListener(std::shared_ptr<Queue> eventsQueue);

        void onEvent(Stream *stream, Stream::Event event, Stream::EventSourceMode source);

        queuedEvent waitForEvent();
        queuedEvent getEvent();

        std::size_t getEventsCount() const;

      private:
        std::shared_ptr<Queue> queue;
    };

}; // namespace audio
