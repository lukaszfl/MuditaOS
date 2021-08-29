// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <apps-common/BasePresenter.hpp>
#include <service-db/Settings.hpp>
#include <memory>

namespace gui
{
    class ListItemProvider;
}

namespace app
{
    class Application;
}

namespace app::powernap
{
    class PowerNapModel;
    class PowerNapMainWindowContract
    {
      public:
        class View
        {
          public:
            virtual ~View() noexcept = default;
        };

        class Presenter : public BasePresenter<PowerNapMainWindowContract::View>
        {
          public:
            virtual auto getNapTimeProvider() -> std::shared_ptr<gui::ListItemProvider> = 0;

            virtual void loadData() = 0;
            virtual void activate() = 0;
        };
    };

    class PowerNapMainWindowPresenter : public PowerNapMainWindowContract::Presenter
    {
        app::Application *app;
        std::shared_ptr<PowerNapModel> model;
        settings::Settings *settings;

      public:
        PowerNapMainWindowPresenter(app::Application *app, settings::Settings *settings);
        auto getNapTimeProvider() -> std::shared_ptr<gui::ListItemProvider> override;

        void loadData() override;
        void activate() override;
    };

} // namespace app::powernap
