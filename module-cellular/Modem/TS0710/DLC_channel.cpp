// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "DLC_channel.h"

#include "TS0710_DATA.h"
#include "TS0710_DLC_ESTABL.h"
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

    DLC_ESTABL_SystemParameters_t system_parameters{};
    system_parameters.TypeOfFrame             = TypeOfFrame_e::SABM;
    system_parameters.ConvergenceLayer        = 1;
    system_parameters.Priority                = 1;
    system_parameters.AckTime                 = 100; // 100ms default
    system_parameters.MaxFrameSize            = 128;
    system_parameters.MaxNumOfRetransmissions = 3; // default 3
    system_parameters.ErrRecovWindowSize      = 2; // default 2

    // TODO set appropraite buffer size
    responseBuffer = xMessageBufferCreate(100);

    TS0710_DLC_ESTABL establ = TS0710_DLC_ESTABL(DLCI, system_parameters, cellular);
    pv_chanParams            = establ.getParams();

    // wait for return & set active
    active = establ.getResponse();
    LOG_DEBUG("Create channel %s: %s", pv_name.c_str(), active ? "TRUE" : "FALSE");
}

DLC_channel::~DLC_channel()
{
    TS0710_DLC_RELEASE release = TS0710_DLC_RELEASE(pv_DLCI);
}

void DLC_channel::SendData(std::vector<uint8_t> &data)
{
    TS0710_DATA _data = TS0710_DATA(pv_DLCI, pv_chanParams, data, pv_cellular);
}

void DLC_channel::cmd_init()
{}

void DLC_channel::cmd_send(std::string cmd)
{
    std::vector<uint8_t> data(cmd.begin(), cmd.end());
    SendData(data);
}

std::string DLC_channel::cmd_receive()
{
    bsp::cellular::CellularFrameResult result;
    if (!xMessageBufferReceive(responseBuffer, &result, sizeof(result), 0)) {
        return {};
    }

    return result.getFrameDataAsString();
}

void DLC_channel::cmd_post()
{}

std::vector<std::string> DLC_channel::SendCommandPrompt(const char *cmd, size_t rxCount, uint32_t timeout)
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
        if (!xMessageBufferReceive(responseBuffer, &responseFrame, sizeof(responseFrame), pdMS_TO_TICKS(timeout))) {
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
    if (blockedTaskHandle != nullptr) {
        xMessageBufferSend(responseBuffer, result, sizeof(*result), 0);
    }
    else if (pv_callback != nullptr) {
        std::string receivedData = result->getFrameDataAsString();
        pv_callback(receivedData);
    }

    return 1;
}
