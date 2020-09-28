#pragma once

#include "Profile.hpp"
#include <memory>
namespace Bt
{
    class HSP : public Profile
    {
      public:
        HSP();
        ~HSP() override;

        HSP(const HSP &other);
        auto operator=(HSP rhs) -> HSP &;

        auto init() -> Error override;
        void setDeviceAddress(uint8_t *addr) override;
        void start();
        void stop();

      private:
        class HSPImpl;
        class SCO;
        class WavWriter;
        std::unique_ptr<HSPImpl> pimpl;
    };
} // namespace Bt