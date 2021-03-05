// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#ifndef PUREPHONE_ATPARSER_HPP
#define PUREPHONE_ATPARSER_HPP

#include "ATCommon.hpp"
#include <FreeRTOS.h>
#include <Service/Service.hpp>
#include <memory>
#include <optional>
#include <task.h>
#include <vector>
#include <message_buffer.h>

#include <bsp/cellular/CellularResult.hpp>

namespace bsp
{
    class Cellular;
}

class ATParser : public at::Channel
{
  public:
    enum class Urc
    {
        MeInitializationSuccessful,
        FullFuncionalityAvailable,
        SimCardReady,
        SMSInitializationComplete,
        PhonebookInitializationComplete,
        Fota,
        NotHandled,
    };

    ATParser(bsp::Cellular *cellular);
    virtual ~ATParser() = default;

    int ProcessNewData(sys::Service *service, bsp::cellular::CellularATResult *result);

    virtual void cmd_init() override final;
    virtual void cmd_send(std::string cmd) override final;
    virtual bool cmd_receive(bsp::cellular::CellularResult &frame, std::chrono::milliseconds timeout) override final;
    virtual void cmd_post() override final;

  private:
    std::vector<Urc> ParseURC();
    bsp::Cellular *cellular              = nullptr;
    MessageBufferHandle_t responseBuffer = nullptr;
    std::string urcBuffer                = {};
    std::vector<ATParser::Urc> urcs      = {};
    bool isInitialized                   = false;
};

#endif // PUREPHONE_ATPARSER_HPP
