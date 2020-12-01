#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <utility>

namespace audio
{
    class Stream
    {
      public:
        struct Span
        {
            std::uint8_t *data   = nullptr;
            std::size_t dataSize = 0;
        };

        enum class Event
        {
            NoEvent,
            StreamFull,
            StreamEmpty
        };

        enum class EventSourceMode
        {
            ISR,
            Thread
        };

        class EventListener
        {
          public:
            virtual void onEvent(Stream *stream, Event event, EventSourceMode source) = 0;
        };

        static constexpr auto defaultBufferingSize = 4U;

        Stream(std::size_t blockSize, unsigned int bufferingSize = defaultBufferingSize);

        bool push(void *data, std::size_t dataSize);
        bool push(const Span &span);

        bool reserve(Span &span);

        bool peek(Span &span);

        bool pop();
        bool pop(Span &span);

        std::size_t getBlockSize() const noexcept;
        std::size_t getBlockCount() const noexcept;
        std::size_t getUsedBlockCount() const noexcept;
        bool isEmpty() const noexcept;
        bool isFull() const noexcept;

        void registerListener(EventListener &listener);

      private:
        void broadcastEvent(Event event);

        std::size_t _blockSize  = 0;
        std::size_t _blockCount = 0;
        std::unique_ptr<uint8_t[]> _buffer;
        std::unique_ptr<uint8_t[]> _emptyBuffer;
        std::list<std::reference_wrapper<EventListener>> listeners;
    };

} // namespace audio
