#pragma once

#include "Stream.hpp"

namespace audio
{
    class Endpoint
    {
      public:
        void setStream(Stream &stream);
        Stream *getStream(void) const noexcept;
        void unsetStream();
        bool isConnected() const noexcept;

      private:
        Stream *_stream = nullptr;
    };

    class Sink : public Endpoint
    {};

    class Source : public Endpoint
    {
      public:
        void connect(Sink &sink, Stream &stream);
        void disconnectStream();

      private:
        Sink *connectedSink = nullptr;
    };

}; // namespace audio
