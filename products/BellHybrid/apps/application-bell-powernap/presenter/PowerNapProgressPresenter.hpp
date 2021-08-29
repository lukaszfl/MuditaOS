// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <apps-common/BasePresenter.hpp>

namespace app::powernap
{
    class PowerNapProgressContract
    {
      public:
        class View
        {
          public:
            virtual ~View() = default;
        };

        class Presenter : public BasePresenter<PowerNapProgressContract::View>
        {};
    };

    class PowerNapProgressPresenter : public PowerNapProgressContract::Presenter
    {};
} // namespace app::powernap
