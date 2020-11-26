#pragma once

#include <cassert> // assert

namespace audio
{
    class Stream
    {};

    class Endpoint
    {
      public:
        /// TODO: make setStream and unsetStream not accessible to the clients of
        /// Endpoint childrens' clients
        void setStream(Stream &stream)
        {
            assert(_stream == nullptr);
            _stream = &stream;
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
