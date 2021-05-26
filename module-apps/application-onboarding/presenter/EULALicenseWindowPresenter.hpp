// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <module-apps/application-onboarding/model/EULARepository.hpp>

#include "EULALicenseWindowContract.hpp"

#include <functional>
#include <string>

namespace app
{
    class Application;

    namespace onBoarding
    {
        class EULALicenseWindowPresenter : public EULALicenseWindowContract::Presenter
        {
          public:
            /**
             * Currently it is GUI (specifically gui::AppWindow) who gets access to app::Application*
             * but here it's rather a Presenter's responsibility to switch Views.
             * Or it might be just Application's?
             */
            explicit EULALicenseWindowPresenter(std::function<void()> acceptEULAApplication,
                                                std::unique_ptr<AbstractEULARepository> &&eulaRepository,
                                                app::Application *app);

            /* EULALicenseWindowContract::Presenter interface implementation */
            void onInterfaceReady() override;
            void onEULAAccepted() override;

          private:
            std::function<void()> acceptEULAApp;
            std::unique_ptr<AbstractEULARepository> eulaRepository;
            app::Application *app;
        };
    } // namespace onBoarding
} // namespace app
