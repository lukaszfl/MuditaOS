// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "WorkerDesktop.hpp"
#include "MessageHandler.hpp"
#include "ServiceDesktop.hpp"
#include "endpoints/Context.hpp"

using namespace cpp_freertos;

WorkerDesktop::WorkerDesktop(sys::Service *ownerServicePtr)
    : sys::Worker(ownerServicePtr),
      Timer("WorkerDesktop::Timeout", cpp_freertos::Ticks::MsToTicks(sdesktop::file_transfer_timeout), false),
      ownerService(ownerServicePtr), parser(ownerServicePtr), fileDes(nullptr)
{
}

bool WorkerDesktop::init(std::list<sys::WorkerQueueInfo> queues)
{
    Worker::init(queues);

    receiveQueue                         = Worker::getQueueByName(WorkerDesktop::RECEIVE_QUEUE_BUFFER_NAME);
    parserFSM::MessageHandler::sendQueue = Worker::getQueueByName(WorkerDesktop::SEND_QUEUE_BUFFER_NAME);

    cdcVcomStruct = bsp::usbInit(receiveQueue);
    if (cdcVcomStruct == NULL) {
        LOG_ERROR("won't start desktop service without serial port");
        return false;
    }

    BaseType_t xReturned = xTaskCreate(&WorkerDesktop::deviceTask, "WorkerDesktop::deviceTask", 2048L / sizeof(portSTACK_TYPE), this, 2, &usbTaskHandle);

    if (xReturned == pdPASS) {
        LOG_DEBUG("init created device task");
    } else {
        LOG_DEBUG("init can't create device task");
        return false;
    }

    return true;
}

bool WorkerDesktop::deinit(void)
{
    LOG_DEBUG("deinit");

    if (usbTaskHandle != NULL) {
        LOG_DEBUG("deinit usbTaskHandle valid, delete task");
        vTaskDelete(usbTaskHandle);
    }

    if (fileDes != nullptr) {
        LOG_DEBUG("deinit close opened fileDes");
        vfs.fclose(fileDes);
    }

    Worker::deinit();

    LOG_DEBUG("deinit end");
    return true;
}

bool WorkerDesktop::handleMessage(uint32_t queueID)
{
    QueueHandle_t queue = queues[queueID];

    std::string qname = queueNameMap[queue];
    LOG_INFO("handleMessage received data from queue: %s", qname.c_str());
    static std::string *sendMsg;
    static std::string receiveMsg;

    if (qname == RECEIVE_QUEUE_BUFFER_NAME) {
        if (xQueueReceive(queue, &receiveMsg, 0) != pdTRUE) {
            LOG_ERROR("handleMessage failed to receive from \"%s\"", RECEIVE_QUEUE_BUFFER_NAME.c_str());
            return false;
        }
        else {
            parser.processMessage(receiveMsg);
        }
    }

    if (qname == SEND_QUEUE_BUFFER_NAME) {
        if (xQueueReceive(queue, &sendMsg, 0) != pdTRUE)
            return false;

        VirtualComSend(cdcVcomStruct, sendMsg->c_str(), sendMsg->length());
    }

    return true;
}

void WorkerDesktop::deviceTask(void *handle)
{
    WorkerDesktop *worker = static_cast<WorkerDesktop *>(handle);
    char *input = (char *)pvPortMalloc(SERIAL_BUFFER_LEN);
    usb_cdc_vcom_struct_t *cdcVcom = worker->getCdcVcomStruct();
    xQueueHandle receiveQueue      = worker->getReceiveQueue();
    size_t len;

    if (!input) {
        LOG_ERROR("deviceTask buffer allocation failed");
        return;
    }

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    LOG_INFO("deviceTask task started");

    while (1) {
        /*if (static_cast<uint8_t>(worker->getState()) > 2) {
            LOG_DEBUG("deviceTask looks like the owner service is beeing stopped, exit now");
            vPortFree(input);
        }*/

        len = VirtualComRecv(cdcVcom, input, SERIAL_BUFFER_LEN);

        if (len > 0) {
            if (worker->getTransferType() != WorkerDesktop::TransferType::JSONCommands) {
                // file tranafer in progress, write/read data from the stream, ignore queues
                worker->transferDataReceived(input, len);
            }
            else if (uxQueueSpacesAvailable(receiveQueue) != 0) {
                static std::string receiveMessage;
                receiveMessage = std::string(input, len);
                if (xQueueSend(receiveQueue, &receiveMessage, portMAX_DELAY) == errQUEUE_FULL) {
                    LOG_ERROR("deviceTask can't send data [%s] to receiveQueue", receiveMessage.c_str());
                }
            } else {
                vPortFree(input);
            }
        }
        else {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    }
}

WorkerDesktop::TransferType WorkerDesktop::getTransferType()
{
    return currentTransferType;
}

sys::ReturnCodes WorkerDesktop::startDownload(const fs::path &destinationPath, const uint32_t fileSize)
{
    filePath = destinationPath;
    fileDes  = vfs.fopen(filePath.c_str(), "w");

    if (fileDes == nullptr)
        return sys::ReturnCodes::Failure;

    if (fileSize <= 0)
        return sys::ReturnCodes::Failure;

    if (Start(cpp_freertos::Ticks::MsToTicks(1000)) == false)
        return sys::ReturnCodes::Failure;

    writeFileSizeExpected = fileSize;
    currentTransferType   = TransferType::DownloadFile;

    LOG_DEBUG("startDownload all checks passed starting download");
    return sys::ReturnCodes::Success;
}

sys::ReturnCodes WorkerDesktop::startUpload(const fs::path &sourcePath, const uint32_t fileSize)
{
    currentTransferType = TransferType::UploadFile;
    return sys::ReturnCodes::Success;
}

void WorkerDesktop::stopTransfer(const bool removeDestinationFile)
{
    LOG_DEBUG("stopTransfer %s", removeDestinationFile ? "remove desination file" : "" );
    parser.setState(parserFSM::State::NoMsg);
    currentTransferType = TransferType::JSONCommands;

    parserFSM::Context response;
    response.setResponseStatus(removeDestinationFile ? parserFSM::http::Code::NotAcceptable : parserFSM::http::Code::Accepted);
    response.setEndpoint(parserFSM::EndpointType::filesystemUpload);
    json11::Json responseJson = json11::Json::object
        {
            { parserFSM::json::fileSize, std::to_string(writeFileDataWritten) },
            { parserFSM::json::fileName, filePath.filename().c_str() }
        };
    response.setResponseBody(responseJson);

    // close the file descriptor
    vfs.fclose(fileDes);

    // stop the timeout timer
    Stop();

    // reset all counters
    writeFileSizeExpected = 0;
    writeFileDataWritten  = 0;

    if (removeDestinationFile) {
        if (vfs.remove(filePath.c_str()) != 0) {
            LOG_ERROR("stopTransfer can't delete file(requested) %s",
                      filePath.c_str());
        }
    }

    parserFSM::MessageHandler::putToSendQueue(response.createSimpleResponse());
}

void WorkerDesktop::transferDataReceived(const char *data, uint32_t dataLen)
{
    if (getTransferType() == TransferType::DownloadFile) {
        Reset(cpp_freertos::Ticks::MsToTicks(sdesktop::file_transfer_timeout));

        if (data == nullptr || dataLen == 0) {
            LOG_ERROR("transferDataReceived invalid data");
            return;
        }

        const uint32_t bytesWritten = vfs.fwrite(data, 1, dataLen, fileDes);

        if (bytesWritten != dataLen) {
            LOG_ERROR("transferDataReceived vfs write failed bytesWritten=%lu != dataLen=%lu", bytesWritten, dataLen);
            return;
        }

        writeFileDataWritten += dataLen;

        if (writeFileDataWritten >= writeFileSizeExpected) {
            LOG_INFO("transferDataReceived all data transferred, stop now");
            stopTransfer(false);
        }
    }
    else {
        LOG_DEBUG("transferDataReceived not in a transfer state");
    }
}

void WorkerDesktop::Run()
{
    LOG_DEBUG("timeout timer: run");

    if (getTransferType() != TransferType::JSONCommands) {
        LOG_DEBUG("timeout timer: stopping transfer");
        stopTransfer(true);
    }
}