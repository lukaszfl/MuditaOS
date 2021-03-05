#pragma once

#include <memory>
#include <module-cellular/Modem/TS0710/TS0710_Frame.h>
#include <module-utils/Utils.hpp>

namespace bsp
{
    namespace cellular
    {
        constexpr size_t EmptyCellularResultSize = 12;
        constexpr size_t CellularResultStructMaxDataSize = 70;
        constexpr size_t CellularResultStructSize = CellularResultStructMaxDataSize + EmptyCellularResultSize;

        enum CellularResultCode
        {
            ReceivedAndIdle,
            ReceivedAndFull,
            ReceivedAfterFull, // ??? strange case
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
            ssize_t dataSize;
            uint8_t data[CellularResultStructMaxDataSize];
        };

        class CellularATResult : public CellularResult
        {
            std::string data;

          public:
            explicit CellularATResult(const CellularResultCode resultCode = ReceivedAndIdle)
                : CellularResult{resultCode} {
                      data.reserve(64);
                  };

            explicit CellularATResult(std::vector<uint8_t> &data,
            const CellularResultCode resultCode = ReceivedAndIdle)
            : CellularResult{resultCode}, data{data.begin(), data.end()} {};

            explicit CellularATResult(CellularDMAResultStruct &dmaResult)
                : CellularResult{dmaResult.resultCode}, data{utils::to_string(dmaResult.data)} {};

            [[nodiscard]] auto getData() -> const std::string &
            {
                return data;
            }
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
