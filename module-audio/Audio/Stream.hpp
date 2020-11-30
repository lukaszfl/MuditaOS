#pragma once

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
            virtual void onEvent(Event event, EventSourceMode source) = 0;
        };

        static constexpr auto defaultBufferingSize = 4U;

        Stream(std::size_t blockSize, unsigned int bufferingSize = defaultBufferingSize)
            : _blockSize(blockSize), _buffer(std::make_unique<std::uint8_t[]>(blockSize * bufferingSize)),
              _emptyBuffer(std::make_unique<std::uint8_t[]>(blockSize))
        {
            std::fill(_emptyBuffer.get(), _emptyBuffer.get() + blockSize, 0);
        }

        bool push(void *data, std::size_t dataSize);
        bool push(const Span &span)
        {
            return push(span.data, span.dataSize);
        }

        bool reserve(Span &span);

        bool peek(Span &span);

        bool pop();
        bool pop(Span &span)
        {
            return peek(span) && pop();
        }

        std::size_t getBlockSize() const noexcept
        {
            return _blockSize;
        }

        void registerListener(EventListener &listener)
        {
            listeners.push_back(std::ref(listener));
        }

      private:
        void broadcastEvent(Event event)
        {
            // TODO: detect isr mode
            for (auto listener : listeners) {
                listener.get().onEvent(event, EventSourceMode::Thread);
            }
        }

        std::size_t _blockSize = 0;
        std::unique_ptr<uint8_t[]> _buffer;
        std::unique_ptr<uint8_t[]> _emptyBuffer;
        std::list<std::reference_wrapper<EventListener>> listeners;
    };

} // namespace audio
