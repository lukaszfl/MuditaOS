#pragma once

#include <memory>

namespace bsp
{
    namespace cellular
    {
        enum CellularResultCode{

        };

        struct CellularResult
        {
            const CellularResultCode result;
            const std::unique_ptr<> data;
        };
    } // namespace cellular
} // namespace bsp
