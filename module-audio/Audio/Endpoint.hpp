#pragma once

#include "Stream.hpp"

#include <cassert> // assert

namespace audio
{
    class Endpoint
    {
      public:
        void setStream(Stream &stream)
        {
            assert(_stream == nullptr);
            _stream = &stream;
        }

        Stream *getStream(void) const noexcept
        {
            return _stream;
        }

        void unsetStream()
        {
            assert(_stream != nullptr);
            _stream = nullptr;
        }

        bool isConnected() const noexcept
        {
            return _stream != nullptr;
        }

      private:
        Stream *_stream = nullptr;
    };

    class Sink : public Endpoint
    {};

    class Source : public Endpoint
    {
      public:
        void connect(Sink &sink, Stream &stream)
        {
            connectedSink = &sink;
            connectedSink->setStream(stream);
            this->setStream(stream);
        }

        void disconnectStream()
        {
            this->unsetStream();
            connectedSink->unsetStream();
            connectedSink = nullptr;
        }

      private:
        Sink *connectedSink = nullptr;
    };

}; // namespace audio
