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

    auto isSuccessful = Worker::init(list);

    queueListener = std::make_shared<StreamQueuedEventsListener>(getQueueByName(listenerQueueName));
    if (!queueListener) {
        return false;
    }

    audioStreamOut.registerListener(queueListener);

    decoderBuffer = std::make_unique<BufferInternalType[]>(bufferSize);
    if (!decoderBuffer) {
        return false;
    }
    LOG_DEBUG("Allocating %d bytes sized buffer", bufferSize * sizeof(BufferInternalType));

    return isSuccessful;
}

bool audio::DecoderWorker::handleMessage(uint32_t queueID)
{
    auto queue = queues[queueID];
    if (queue->GetQueueName() == listenerQueueName && queueListener) {
        auto event = queueListener->getEvent();

        switch (event.second) {
        case Stream::Event::StreamOverflow:
            break;
        case Stream::Event::StreamUnderFlow:
            break;
        case Stream::Event::NoEvent:
            return true;
        case Stream::Event::StreamFull:
            return true;
        case Stream::Event::StreamHalfUsed:
            [[fallthrough]];
        case Stream::Event::StreamEmpty:
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
