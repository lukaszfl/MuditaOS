#pragma once

#include "Error.hpp"

namespace Bt
{
    class Profile
    {
      public:
        Profile()                                    = default;
        virtual ~Profile()                           = default;
        virtual auto init() -> Error                 = 0;
        virtual void setDeviceAddress(uint8_t *addr) = 0;

      private:
    };

} // namespace Bt
