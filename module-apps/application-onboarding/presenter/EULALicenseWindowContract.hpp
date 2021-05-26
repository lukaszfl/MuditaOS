// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <MVPContract.hpp>

namespace app::onBoarding
{
    struct EULALicenseWindowContract
    {
        class Presenter;

        class View : public mvp::View
        {
          public:
            explicit View(Presenter *presenter) noexcept : mvp::View(presenter)
            {}

            /** View: GUI manipulation interface
             *
             * View accepts requests regarding its appearance
             */
            virtual void showEULA(const std::string &eula) = 0;
        };

        class Presenter : public mvp::Presenter<View>
        {
          public:
            /** Presenter: GUI events to process by presentation logic
             *
             *  Presenter accepts notifications - events are identified
             *    on a logic level, independent from GUI implementation
             */
            virtual void onInterfaceReady() = 0;
            virtual void onEULAAccepted()   = 0;
        };
    };
} // namespace app::onBoarding
