// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "DecoderWorker.hpp"
#include "Audio/decoder/Decoder.hpp"

audio::DecoderWorker::DecoderWorker(Stream &audioStreamOut, Decoder *decoder, EndOfFileCallback endOfFileCallback)
    : sys::Worker(DecoderWorker::workerName, DecoderWorker::workerPriority), audioStreamOut(audioStreamOut),
      decoder(decoder), endOfFileCallback(endOfFileCallback),
      bufferSize(audioStreamOut.getBlockSize() / sizeof(BufferInternalType))
{}

auto audio::DecoderWorker::init(std::list<sys::WorkerQueueInfo> queues) -> bool
{
    std::list<sys::WorkerQueueInfo> list{
        {listenerQueueName, StreamQueuedEventsListener::listenerElementSize, listenerQueueCapacity}};
    Worker::init(list);
    queueListener.emplace(StreamQueuedEventsListener(getQueueByName(listenerQueueName)));

    audioStreamOut.registerListener(queueListener);

    decoderBuffer = std::make_unique<BufferInternalType[]>(bufferSize);
    LOG_DEBUG("Allocating %d bytes sized buffer", bufferSize * sizeof(BufferInternalType));
    return
}

bool audio::DecoderWorker::handleMessage(uint32_t queueID)
{
    QueueHandle_t queue = queues[queueID];
    if (queue == sourceQueue) {
        auto event = queueListener.getEvent();

        switch (event.second) {
        case Stream::Event::StreamOverflow:
            // LOG_DEBUG("StreamOverflow");
            break;
        case Stream::Event::StreamUnderFlow:
            // LOG_DEBUG("NO event");
            break;
        case Stream::Event::NoEvent:
            // LOG_DEBUG("StreamUnderFlow");
            return true;
        case Stream::Event::StreamFull:
            // LOG_DEBUG("StreamFull");
            return true;
        case Stream::Event::StreamHalfUsed:
            // LOG_DEBUG("Half");
            [[fallthrough]];
        case Stream::Event::StreamEmpty:
            // LOG_DEBUG("Stream");
            auto samplesRead = 0;

            while (!audioStreamOut.isFull()) {
                samplesRead = decoder->decode(bufferSize, decoderBuffer.get());

                if (samplesRead == 0) {
                    audioStreamOut.cleanListeners();
                    endOfFileCallback();
                    break;
                }
                /*LOG_DEBUG("Buf: %x %x %x %x",
                          (decoderBuffer.get()[0]) & 0xff,
                          (decoderBuffer.get()[0] >> 8) & 0xff,
                          (decoderBuffer.get()[1]) & 0xff,
                          (decoderBuffer.get()[1] >> 8) & 0xff);*/
                if (!audioStreamOut.push(decoderBuffer.get(), samplesRead * sizeof(BufferInternalType))) {
                    LOG_FATAL("Decoder failed to push to stream.");
                    break;
                }
            }
        }
    }
    return true;
}
