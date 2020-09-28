#pragma once

#include "Profile.hpp"
#include <memory>
namespace Bt
{

    class A2DP : public Profile
    {
      public:
        A2DP();
        ~A2DP() override;
        // A2DP() = default;

        A2DP(const A2DP &other);
        auto operator=(A2DP rhs) -> A2DP &;

        auto init() -> Error override;
        void start();
        void stop();
        void setDeviceAddress(uint8_t *addr);

      private:
        class A2DPImpl;
        class AVRCP;
        class AVDTP;
        class Player;
        std::unique_ptr<A2DPImpl> pimpl;
    };
} // namespace Bt