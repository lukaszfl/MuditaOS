// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "DecoderWorker.hpp"
#include "Audio/decoder/Decoder.hpp"

audio::DecoderWorker::DecoderWorker(Stream &audioStreamOut, Decoder *decoder, EndOfFileCallback endOfFileCallback)
    : sys::Worker("DecoderWorker"), audioStreamOut(audioStreamOut), decoder(decoder),
      endOfFileCallback(endOfFileCallback), bufferSize(audioStreamOut.getBlockSize() / sizeof(BufferInternalType))
{}

auto audio::DecoderWorker::init(std::list<sys::WorkerQueueInfo> queues) -> bool
{
    audioStreamOut.registerListener(queueListener);

    std::list<sys::WorkerQueueInfo> list;
    list.push_back({queueListener.getQueueInfo().second, 1, 1, queueListener.getQueueInfo().first});
    sourceQueue = queues.begin()->handle;

    decoderBuffer = std::make_unique<BufferInternalType[]>(bufferSize);
    return Worker::init(list);
}

bool audio::DecoderWorker::handleMessage(uint32_t queueID)
{
    QueueHandle_t queue = queues[queueID];
    if (queue == sourceQueue) {
        Stream::Event streamEvent;
        if (xQueueReceive(queue, &streamEvent, 0) != pdTRUE) {
            LOG_ERROR("Receive failure!");
            return false;
        }

        switch (streamEvent) {
        case Stream::Event::StreamOverflow:
            LOG_DEBUG("StreamOverflow");
            break;
        case Stream::Event::StreamUnderFlow:
            LOG_DEBUG("NO event");
            break;
        case Stream::Event::NoEvent:
            LOG_DEBUG("StreamUnderFlow");
            return true;
        case Stream::Event::StreamFull:
            LOG_DEBUG("StreamFull");
            return true;
        case Stream::Event::StreamHalfUsed:
        case Stream::Event::StreamEmpty:
            LOG_DEBUG("StreamEmpty");
            auto samplesRead = 0;

            while (!audioStreamOut.isFull()) {
                samplesRead = decoder->decode(bufferSize, decoderBuffer.get());

                if (samplesRead == 0) {
                    endOfFileCallback();
                    break;
                }

                if (!audioStreamOut.push(decoderBuffer.get(), samplesRead * sizeof(BufferInternalType))) {
                    LOG_FATAL("Decoder failed to push to stream.");
                    break;
                }
            }
        }
    }
    return true;
}
