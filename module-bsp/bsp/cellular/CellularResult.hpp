#pragma once

#include <memory>
#include <module-cellular/Modem/TS0710/TS0710_Frame.h>

namespace bsp
{
    namespace cellular
    {
        enum CellularResultCode
        {
            ReceivedAndIdle,
            ReceivedAndFull,
            ReceivingNotStarted,
            TransmittingNotStarted,
        };

        class CellularResult
        {
            const CellularResultCode resultCode;

          public:
            explicit CellularResult(const CellularResultCode resultCode) : resultCode{resultCode} {};

            auto getResultCode() -> CellularResultCode
            {
                return resultCode;
            }
        };

        class CellularDMAResult : public CellularResult
        {
            const std::vector<uint8_t> data;

          public:
            explicit CellularDMAResult(const CellularResultCode resultCode = ReceivedAndIdle)
                : CellularResult{resultCode} {};

            explicit CellularDMAResult(std::vector<uint8_t> &&data,
                                       const CellularResultCode resultCode = ReceivedAndIdle)
                : CellularResult{resultCode}, data{data} {};

            [[nodiscard]] auto getData() -> const std::vector<uint8_t> &
            {
                return data;
            }

            [[nodiscard]] auto getFrameDataAsString() -> std::string const
            {
                return std::string{data.begin(), data.end()};
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
