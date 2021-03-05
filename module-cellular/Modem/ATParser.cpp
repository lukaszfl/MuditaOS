// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ATParser.hpp"
#include <service-fota/FotaServiceAPI.hpp>
#include "bsp/cellular/bsp_cellular.hpp"
#include <service-cellular/CellularMessage.hpp>
#include "ticks.hpp"
#include <Utils.hpp>
#include <utility>
#include <vector>

ATParser::ATParser(bsp::Cellular *cellular) : cellular(cellular)
{
    isInitialized = true;

    responseBuffer = xMessageBufferCreate(256);
}

/// plz see 12.7 summary of urc in documentation
std::vector<ATParser::Urc> ATParser::ParseURC()
{
    std::vector<ATParser::Urc> resp;
    size_t maxPos = 0, pos = 0;

    cpp_freertos::LockGuard lock(mutex);

    std::vector<std::pair<std::string, ATParser::Urc>> vals = {
        {"RDY", ATParser::Urc::MeInitializationSuccessful},
        {"+CFUN: 1", ATParser::Urc::FullFuncionalityAvailable},
    };

    for (const auto &el : vals) {
        pos = urcBuffer.find(el.first);
        if (pos != std::string::npos) {
            resp.push_back(el.second);
            maxPos = std::max(pos + el.first.length(), maxPos);
            LOG_DEBUG("%s", ("[URC]: " + el.first).c_str());
        }
    }

    if (urcBuffer.find("+QIND: \"FOTA\"") != std::string::npos) {
        LOG_DEBUG("%s", urcBuffer.c_str());
        resp.push_back(ATParser::Urc::Fota);
        return resp;
    }

    // manage string buffer
    if (maxPos == 0) {}
    else if (urcBuffer.size() >= maxPos) {
        urcBuffer.erase();
    }
    else {
        urcBuffer = urcBuffer.substr(maxPos);
    }

    return resp;
}

int ATParser::ProcessNewData(sys::Service *service, bsp::cellular::CellularATResult *result)
{
    {
        cpp_freertos::LockGuard lock(mutex);
        urcBuffer.append(result->getData());
    }

    LOG_DEBUG("[AT] %s", result->getData().c_str());

    auto ret = ParseURC();

    if (blockedTaskHandle != nullptr) {
        LOG_DEBUG("[AT] Adding data to message buffer");
        xMessageBufferSend(responseBuffer, result, sizeof(*result), 0);
    }
    else if (!ret.empty()) {
        if (ret.size() == 1 && ret[0] == ATParser::Urc::Fota) {
            std::string fotaData(urcBuffer);
            LOG_DEBUG("parsing FOTA:\"%s\"", fotaData.c_str());
            FotaService::API::sendRawProgress(service, fotaData);
            urcBuffer.erase();
        }
        else {
            urcs.insert(std::end(urcs), std::begin(ret), std::end(ret));
        }
        // GSM modem is considered as operational when it outputs URCs specified below:
        // 1) RDY
        // 2) +CFUN: 1
        if (urcs.size() == 2) {
            cpp_freertos::LockGuard lock(mutex);
            auto msg = std::make_shared<CellularNotificationMessage>(
                CellularNotificationMessage::Type::PowerUpProcedureComplete);
            service->bus.sendMulticast(msg, sys::BusChannel::ServiceCellularNotifications);
            LOG_DEBUG("[!!!] Fucking away data");
            urcBuffer.erase();
            urcs.clear();
        }
    }
    return 1;
}

void ATParser::cmd_init()
{
    cpp_freertos::LockGuard lock(mutex);
    urcBuffer.erase();
}

void ATParser::cmd_send(std::string cmd)
{
    cellular->Write(const_cast<char *>(cmd.c_str()), cmd.size());
}

bool ATParser::cmd_receive(bsp::cellular::CellularResult &frame,
                           std::chrono::milliseconds timeout = std::chrono::milliseconds{0})
{
    // TODO add constexpr somewhere with AT max size and MUX max size
    return (xMessageBufferReceive(responseBuffer, &frame, 128, pdMS_TO_TICKS(timeout.count())) != 0);
}

void ATParser::cmd_post()
{
    cpp_freertos::LockGuard lock(mutex);
    urcBuffer.erase(); // TODO:M.P is it okay to flush buffer here ?
}
