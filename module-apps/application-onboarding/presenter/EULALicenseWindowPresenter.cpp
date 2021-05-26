// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "EULALicenseWindowPresenter.hpp"

#include <Application.hpp>

#include "../ApplicationOnBoarding.hpp"     // next window name
#include "../data/OnBoardingSwitchData.hpp" // switch data

namespace app::onBoarding
{
    EULALicenseWindowPresenter::EULALicenseWindowPresenter(std::function<void()> acceptEULAApp,
                                                           std::unique_ptr<AbstractEULARepository> &&eulaRepository,
                                                           app::Application *app)
        : acceptEULAApp(std::move(acceptEULAApp)), eulaRepository{std::move(eulaRepository)}, app(app)
    {}

    void EULALicenseWindowPresenter::onEULAAccepted()
    {
        acceptEULAApp();
        /* window switching sequence is a logic part, not GUI's */
        app->switchWindow(gui::window::name::onBoarding_start_configuration,
                          gui::ShowMode::GUI_SHOW_INIT,
                          std::make_unique<onBoarding::OnBoardingSwitchData>());
    }

    void EULALicenseWindowPresenter::onInterfaceReady()
    {
        getView()->showEULA(eulaRepository->getEulaText());
    }

} // namespace app::onBoarding
