// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "presenter/PowerNapProgressPresenter.hpp"
#include <AppWindow.hpp>

namespace gui
{
    class BellBaseLayout;
    class PowerNapProgressWindow : public AppWindow, public app::powernap::PowerNapProgressContract::View
    {
        std::shared_ptr<app::powernap::PowerNapProgressContract::Presenter> presenter;
        BellBaseLayout *body = nullptr;

        void buildInterface() override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;

      public:
        PowerNapProgressWindow(app::Application *app,
                               std::shared_ptr<app::powernap::PowerNapProgressContract::Presenter> presenter);
    };
} // namespace gui
