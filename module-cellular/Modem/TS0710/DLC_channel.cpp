// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "DLC_channel.h"

#include "TS0710_DATA.h"
#include "TS0710_DLC_RELEASE.h"
#include "TS0710_Frame.h"

#include <module-utils/log/log.hpp>
#include <ticks.hpp>
#include <Utils.hpp>

DLC_channel::DLC_channel(DLCI_t DLCI, const std::string &name, bsp::Cellular *cellular, const Callback_t &callback)
{
    LOG_DEBUG("Creating DLCI %i channel \"%s\"", DLCI, name.c_str());
    pv_name     = name;
    pv_DLCI     = DLCI;
    pv_cellular = cellular;

    if (callback != nullptr)
        pv_callback = callback;

    pv_chanParams.TypeOfFrame             = TypeOfFrame_e::SABM;
    pv_chanParams.ConvergenceLayer        = 1;
    pv_chanParams.Priority                = 1;
    pv_chanParams.AckTime                 = 100; // 100ms default
    pv_chanParams.MaxFrameSize            = 128;
    pv_chanParams.MaxNumOfRetransmissions = 3; // default 3
    pv_chanParams.ErrRecovWindowSize      = 2; // default 2

    responseBuffer = xMessageBufferCreate(pv_chanParams.MaxFrameSize);
}

bool DLC_channel::init()
{
    active = establish();
    LOG_INFO("Create channel %s: %s", pv_name.c_str(), active ? "TRUE" : "FALSE");

    return active;
}

DLC_channel::~DLC_channel()
{
    TS0710_DLC_RELEASE release = TS0710_DLC_RELEASE(pv_DLCI);
}

void DLC_channel::SendData(std::vector<uint8_t> &data)
{
    TS0710_DATA _data = TS0710_DATA(pv_DLCI, pv_chanParams, data, pv_cellular);
}

bool DLC_channel::establish()
{
    LOG_DEBUG("Sending %s frame to DLCI %i", TypeOfFrame_text[pv_chanParams.TypeOfFrame].c_str(), pv_DLCI);

    TS0710_Frame frame_c(TS0710_Frame::frame_t(static_cast<uint8_t>(pv_DLCI << 2) | (1 << 1),
                                               static_cast<uint8_t>(pv_chanParams.TypeOfFrame)));

    LOG_DEBUG("[DLC] Ser data len: %d", frame_c.getSerData().size());

    {
        cpp_freertos::LockGuard lock(mutex);
        blockedTaskHandle = xTaskGetCurrentTaskHandle();
    }

    bsp::cellular::CellularFrameResult response{};

    for (int retries = 0; retries < pv_chanParams.MaxNumOfRetransmissions; ++retries) {
        pv_cellular->Write(static_cast<void *>(frame_c.getSerData().data()), frame_c.getSerData().size());

        if (cmd_receive(response)) {
            LOG_INFO("Got response");

            TS0710_Frame frame = response.getFrame();

            if (response.getFrame().getFrameDLCI() == pv_DLCI &&
                (frame.getFrame().Control == (static_cast<uint8_t>(TypeOfFrame_e::UA) & ~(1 << 4)))) {
                LOG_INFO("Frame correct");
                return true;
            }

            LOG_INFO("ERROR - discarding frame !");
            return false;
        }
    }

    LOG_ERROR("Sending frame failed");
    {
        cpp_freertos::LockGuard lock(mutex);
        blockedTaskHandle = nullptr;
    }
    return false;
}

void DLC_channel::cmd_init()
{}

void DLC_channel::cmd_send(std::string cmd)
{
    std::vector<uint8_t> data(cmd.begin(), cmd.end());
    SendData(data);
}

bool DLC_channel::cmd_receive(bsp::cellular::CellularResult &frame, std::chrono::milliseconds timeout)
{
    return (xMessageBufferReceive(
                responseBuffer, &frame, 2 * pv_chanParams.MaxFrameSize, pdMS_TO_TICKS(timeout.count())) != 0);
}

void DLC_channel::cmd_post()
{}

std::vector<std::string> DLC_channel::SendCommandPrompt(const char *cmd,
                                                        size_t rxCount,
                                                        std::chrono::milliseconds timeout)
{
    std::vector<std::string> tokens;

    blockedTaskHandle = xTaskGetCurrentTaskHandle();
    at::Result result;
    bsp::cellular::CellularFrameResult responseFrame;

    cmd_init();
    std::string cmdFixed = formatCommand(cmd);
    cmd_send(cmdFixed);

    // Wait for response:
    while (true) {
        if (!xMessageBufferReceive(
                responseBuffer, &responseFrame, pv_chanParams.MaxFrameSize, pdMS_TO_TICKS(timeout.count()))) {
            result.code = at::Result::Code::TIMEOUT;
        }
        else {
            auto str = responseFrame.getFrameDataAsString();
            // tokenize responseBuffer
            auto pos = str.find('>');
            if (pos != std::string::npos) {
                tokens.push_back(str.substr(pos, strlen(">")));
                break;
            }
            if (tokens.size() >= rxCount) {
                break;
            }
        }
    }

    cmd_log(cmdFixed, result, timeout);
    cmd_post();
    blockedTaskHandle = nullptr;

    return tokens;
}

int DLC_channel::ParseInputData(bsp::cellular::CellularFrameResult *result)
{
    cpp_freertos::LockGuard lock(mutex);

    LOG_DEBUG("[DLC] Parse input data: DLC %d", this->getDLCI());
    if (blockedTaskHandle != nullptr) {
        LOG_DEBUG("[DLC] Adding to message buffer");
        xMessageBufferSend(responseBuffer, result, sizeof(*result), 0);
    }
    else if (pv_callback != nullptr) {
        std::string receivedData = result->getFrameDataAsString();
        pv_callback(receivedData);
    }

    return 1;
}
