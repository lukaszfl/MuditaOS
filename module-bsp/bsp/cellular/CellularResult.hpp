#pragma once

#include <memory>
#include <module-cellular/Modem/TS0710/TS0710_Frame.h>

namespace bsp
{
    namespace cellular
    {
        constexpr size_t EmptyCellularResultSize = 12;

        enum CellularResultCode
        {
            ReceivedAndIdle,
            ReceivedAndFull,
            ReceivingNotStarted,
            TransmittingNotStarted,
        };

        class CellularResult
        {
            CellularResultCode resultCode;

          public:
            explicit CellularResult(const CellularResultCode resultCode) : resultCode{resultCode} {};

            auto getResultCode() -> CellularResultCode
            {
                return resultCode;
            }

            auto setResultCode(CellularResultCode code) -> void
            {
                resultCode = code;
            }
        };

        struct __attribute__((__packed__)) CellularDMAResultStruct
        {
            CellularResultCode resultCode;
            size_t dataSize;
            uint8_t data[127];
        };

        class CellularFrameResult : public CellularResult
        {
            TS0710_Frame frame;

          public:
            explicit CellularFrameResult(const CellularResultCode resultCode = ReceivedAndIdle)
                : CellularResult{resultCode} {};

            explicit CellularFrameResult(std::vector<uint8_t> &data,
                                         const CellularResultCode resultCode = ReceivedAndIdle)
                : CellularResult{resultCode}, frame{data} {};

            [[nodiscard]] auto getFrame() -> const TS0710_Frame &
            {
                return frame;
            }

            [[nodiscard]] auto getFrameDataAsString() -> std::string const
            {
                return std::string{frame.getData().begin(), frame.getData().end()};
            }
        };
    } // namespace cellular
} // namespace bsp
