#include "Stream.hpp"

using namespace audio;

Stream::Stream(Allocator &allocator, std::size_t blockSize, unsigned int bufferingSize)
    : _allocator(allocator), _blockSize(blockSize), _blockCount(bufferingSize),
      _buffer(_allocator.allocate(_blockSize * _blockCount)), _emptyBuffer(_allocator.allocate(_blockSize))
{
    std::fill(_emptyBuffer.get(), _emptyBuffer.get() + blockSize, 0);
}

bool Stream::push(void *data, std::size_t dataSize)
{
    /// TODO: implement
    return false;
}

bool Stream::pop()
{
    /// TODO: implement
    return false;
}

bool Stream::peek(Span &span)
{
    /// TODO: implement
    return false;
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

std::size_t Stream::getBlockCount() const noexcept
{
    return _blockCount;
}

std::size_t Stream::getUsedBlockCount() const noexcept
{
    /// TODO: return blocks used
    return 0;
}

bool Stream::isEmpty() const noexcept
{
    return getUsedBlockCount() == 0;
}

bool Stream::isFull() const noexcept
{
    return getUsedBlockCount() == getBlockCount();
}

#if 0
Stream::UniqueStreamBuffer Stream::allocateBuffer(std::size_t size)
{
    auto buf = std::unique_ptr(static_cast<std::uint8_t[]>(_allocator.allocate()),
                               [&_allocator](uint8_t[] buf) { _allocator.deallocate(buf); });
    return std::move(buf);
}
#endif

Stream::UniqueStreamBuffer StandardStreamAllocator::allocate(std::size_t size)
{
    return std::make_unique<uint8_t[]>(size);
}

Stream::UniqueStreamBuffer NonCacheableStreamAllocator::allocate(std::size_t size)
{
    return std::unique_ptr<std::uint8_t[], std::function<void(uint8_t[])>>(
        allocator.allocate(size), [this, size](std::uint8_t ptr[]) { allocator.deallocate(ptr, size); });
}
