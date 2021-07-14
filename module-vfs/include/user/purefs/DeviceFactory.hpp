#pragma once

#include "blkdev/disk.hpp"

#include <memory>

namespace purefs
{
    class DeviceFactory
    {
      public:
        virtual ~DeviceFactory()                                       = default;
        virtual std::unique_ptr<blkdev::disk> makeDefaultBlockDevice() = 0;
        virtual std::unique_ptr<blkdev::disk> makeDefaultNvmDevice()   = 0;
    };
} // namespace purefs
