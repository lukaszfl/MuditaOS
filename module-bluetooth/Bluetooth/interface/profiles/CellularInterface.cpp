// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <service-cellular/service-cellular/CellularServiceAPI.hpp>
#include "CellularInterface.hpp"

bool CellularInterface::answerIncomingCall(sys::Service *service)
{
    return CellularServiceAPI::AnswerIncomingCall(service);
}

bool CellularInterface::hangupCall(sys::Service *service)
{
    return CellularServiceAPI::HangupCall(service);
}
