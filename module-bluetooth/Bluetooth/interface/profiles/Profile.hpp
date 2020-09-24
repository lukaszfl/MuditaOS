#pragma once

#include "Error.hpp"

namespace Bt
{
    class Profile
    {
      public:
        Profile()            = default;
        virtual ~Profile()   = default;
        virtual Error init() = 0;

      private:
    };

} // namespace Bt
