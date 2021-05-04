// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <module-sys/Service/Service.hpp>

namespace locks
{
    class SimLockSubject
    {
      private:
        sys::Service *owner;

      public:
        explicit SimLockSubject(sys::Service *owner);

        void verifyInput(const std::vector<unsigned int> &inputData);
    };

} // namespace locks
