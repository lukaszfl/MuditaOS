// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once
#include <vector>
#include "BluetoothWorker.hpp"
#include "Device.hpp"
#include <Bluetooth/Device.hpp>
#include <Bluetooth/Error.hpp>

namespace bluetooth::GAP
{

    extern std::vector<Devicei> devices;
    /// THIS have to be called prior to Bt system start!
    auto registerScan() -> Error;
    auto scan() -> Error;
    void stopScan();
    void setVisibility(bool visibility);
    auto pair(uint8_t *addr, std::uint8_t protectionLevel = 0) -> bool;
    void setOwnerService(sys::Service *service);
} // namespace bluetooth::GAP
