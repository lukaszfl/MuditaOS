#include "Stream.hpp"

using namespace audio;

Stream::Stream(std::size_t blockSize, unsigned int bufferingSize)
    : _blockSize(blockSize), _buffer(std::make_unique<std::uint8_t[]>(blockSize * bufferingSize)),
      _emptyBuffer(std::make_unique<std::uint8_t[]>(blockSize))
{
    std::fill(_emptyBuffer.get(), _emptyBuffer.get() + blockSize, 0);
}

bool Stream::push(const Span &span)
{
    return push(span.data, span.dataSize);
}

bool Stream::pop(Span &span)
{
    return peek(span) && pop();
}

std::size_t Stream::getBlockSize() const noexcept
{
    return _blockSize;
}

void Stream::registerListener(EventListener &listener)
{
    listeners.push_back(std::ref(listener));
}

void Stream::broadcastEvent(Event event)
{
    // TODO: detect isr mode
    for (auto listener : listeners) {
        listener.get().onEvent(this, event, EventSourceMode::Thread);
    }
}
