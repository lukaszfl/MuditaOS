#include "Endpoint.hpp"

#include <cassert> // assert

using namespace audio;

void Endpoint::setStream(Stream &stream)
{
    assert(_stream == nullptr);
    _stream = &stream;
}

Stream *Endpoint::getStream(void) const noexcept
{
    return _stream;
}

void Endpoint::unsetStream()
{
    assert(_stream != nullptr);
    _stream = nullptr;
}

bool Endpoint::isConnected() const noexcept
{
    return _stream != nullptr;
}

void Source::connect(Sink &sink, Stream &stream)
{
    connectedSink = &sink;
    connectedSink->setStream(stream);
    this->setStream(stream);
}

void Source::disconnectStream()
{
    this->unsetStream();
    connectedSink->unsetStream();
    connectedSink = nullptr;
}
