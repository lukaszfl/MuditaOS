#include "StreamQueuedEventsListener.hpp"

using namespace audio;

// TODO: create queue in the constructor
StreamQueuedEventsListener::StreamQueuedEventsListener(std::size_t eventsQueueCapacity)
{}

void StreamQueuedEventsListener::onEvent(Stream *stream, Stream::Event event, Stream::EventSourceMode source)
{
    /// TODO: use eventstorage to store the incoming event in the queue
    /// TODO: use proper "queue push" method depending if in the IRQ mode
}

StreamQueuedEventsListener::queuedEvent StreamQueuedEventsListener::waitForEvent()
{
    /// TODO: get single event (blocking), return "no event" if there is no event
    return std::make_pair(static_cast<Stream *>(nullptr), Stream::Event::NoEvent);
}

std::size_t StreamQueuedEventsListener::getEventsCount() const
{
    /// TODO: get queue current capacity
    return 0;
}

StreamQueuedEventsListener::queuedEvent StreamQueuedEventsListener::getEvent()
{
    /// TODO: get single event if any, return "no event" otherwise
    return std::make_pair(static_cast<Stream *>(nullptr), Stream::Event::NoEvent);
}

StreamQueuedEventsListener::queueInfo StreamQueuedEventsListener::getQueueInfo() const
{
    /// TODO: get the internal queue handle and name
    /// use this method to register the queue in the Worker
    return std::make_pair(static_cast<QueueHandle_t>(nullptr), "");
}
