// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md
#pragma once

class CellularInterface
{
  public:
    virtual ~CellularInterface()                           = default;
    virtual bool answerIncomingCall(sys::Service *service);
    virtual bool hangupCall(sys::Service *service);
};
