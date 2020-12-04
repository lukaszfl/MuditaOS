#include "Stream.hpp"

#include <algorithm>
#include <iterator>

using namespace audio;

/// TODO: push/pop vs peek and friends interactions
/// TODO: synchronization
/// TODO: ISR safe synchronization

Stream::Stream(Allocator &allocator, std::size_t blockSize, unsigned int bufferingSize)
    : _allocator(allocator), _blockSize(blockSize), _blockCount(bufferingSize),
      _buffer(_allocator.allocate(_blockSize * _blockCount)), _emptyBuffer(_allocator.allocate(_blockSize)),
      _dataStart(_buffer.get(), _blockSize * _blockCount, _buffer.get(), _blockSize), _dataEnd(_dataStart),
      _peekPosition(_dataStart), _writeReservationPosition(_dataStart)
{
    std::fill(_emptyBuffer.get(), _emptyBuffer.get() + blockSize, 0);
}

bool Stream::push(void *data, std::size_t dataSize)
{
    return push(Span{.data = static_cast<std::uint8_t *>(data), .dataSize = dataSize});
}

bool Stream::push(const Span &span)
{
    /// sanity - do not store buffers different than internal block size
    if (span.dataSize != _blockSize) {
        return false;
    }

    /// no space left
    if (isFull()) {
        return false;
    }

    auto nextDataBlock = *_dataEnd;
    std::copy(nextDataBlock.data, nextDataBlock.dataEnd(), span.data);

    _dataEnd++;
    _blocksUsed++;

    return true;
}

bool Stream::push()
{
    return push(getNullSpan());
}

bool Stream::pop(Span &span)
{
    if (isEmpty()) {
        return false;
    }

    std::copy(span.data, span.dataEnd(), (*_dataStart).data);
    _dataStart++;
    _blocksUsed--;

    return true;
}

void Stream::consume()
{
    _blocksUsed -= std::distance(_dataStart, _peekPosition);
    _dataStart = _peekPosition;
}

bool Stream::peek(Span &span)
{
    if (blocksAvailable()) {
        span = *++_peekPosition;
        return true;
    }

    return false;
}

void Stream::unpeek()
{
    _peekPosition = _dataStart;
}

bool Stream::reserve(Span &span)
{
    if (blocksAvailable()) {
        span = *++_writeReservationPosition;
        return true;
    }

    return false;
}

void Stream::commit()
{
    _blocksUsed += std::distance(_dataEnd, _writeReservationPosition);
    _dataEnd = _writeReservationPosition;
}

void Stream::release()
{
    _writeReservationPosition = _dataEnd;
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
    return _blocksUsed;
}

bool Stream::isEmpty() const noexcept
{
    return getUsedBlockCount() == 0;
}

bool Stream::isFull() const noexcept
{
    return getUsedBlockCount() == getBlockCount();
}

bool Stream::blocksAvailable() const noexcept
{
    return !isFull();
}

Stream::UniqueStreamBuffer StandardStreamAllocator::allocate(std::size_t size)
{
    return std::make_unique<uint8_t[]>(size);
}

Stream::UniqueStreamBuffer NonCacheableStreamAllocator::allocate(std::size_t size)
{
    return std::unique_ptr<std::uint8_t[], std::function<void(uint8_t[])>>(
        allocator.allocate(size), [this, size](std::uint8_t ptr[]) { allocator.deallocate(ptr, size); });
}

Stream::RawBlockIterator::RawBlockIterator(std::uint8_t *bufStart,
                                           std::size_t bufSize,
                                           std::uint8_t *ptr,
                                           std::size_t stepSize)
    : _bufStart(bufStart), _bufEnd(bufStart + bufSize), _curPos(ptr), _stepSize(stepSize)
{}

Stream::RawBlockIterator &Stream::RawBlockIterator::operator++()
{
    _curPos += _stepSize;
    if (_curPos == _bufEnd) {
        _curPos = _bufStart;
    }

    return *this;
}

bool Stream::RawBlockIterator::operator==(const Stream::RawBlockIterator &rhs)
{
    return _curPos == rhs._curPos;
}

bool Stream::RawBlockIterator::operator!=(const Stream::RawBlockIterator &rhs)
{
    return !operator==(rhs);
}

Stream::RawBlockIterator Stream::RawBlockIterator::operator++(int)
{
    RawBlockIterator tmp(*this);
    operator++();
    return tmp;
}

Stream::RawBlockIterator &Stream::RawBlockIterator::operator--()
{
    if (_curPos == _bufStart) {
        _curPos = _bufEnd - _stepSize;
    }
    else {
        _curPos -= _stepSize;
    }

    return *this;
}

Stream::RawBlockIterator Stream::RawBlockIterator::operator--(int)
{
    RawBlockIterator tmp(*this);
    operator--();
    return tmp;
}

Stream::Span Stream::RawBlockIterator::operator*()
{
    return Stream::Span{.data = _curPos, .dataSize = _stepSize};
}

std::uint8_t *Stream::Span::dataEnd() const noexcept
{
    return data + dataSize;
}

Stream::Span Stream::getNullSpan() const noexcept
{
    return Span{.data = _emptyBuffer.get(), .dataSize = _blockSize};
}
