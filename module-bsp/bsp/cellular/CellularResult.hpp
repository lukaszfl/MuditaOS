#pragma once

#include <memory>

namespace bsp
{
    namespace cellular
    {
        enum CellularResultCode{
            ReceivedAndIdle,
            ReceivedAndFull,
            ReceivingNotStarted,
            TransmittingNotStarted,
        };

        struct CellularResult
        {
            const CellularResultCode result;
            const std::unique_ptr<uint8_t> data;
        };
    } // namespace cellular
} // namespace bsp
