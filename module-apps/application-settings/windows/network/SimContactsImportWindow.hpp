// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <application-settings/windows/BaseSettingsWindow.hpp>
#include <application-settings/presenter/network/SimContactsImportWindowPresenter.hpp>
#include <application-settings/models/network/SimContactsImportModel.hpp>

namespace gui
{
    class SimContactsImportWindow : public AppWindow, public SimContactsImportWindowContract::View
    {

      public:
        SimContactsImportWindow(app::Application *app,
                                std::shared_ptr<SimContactsImportWindowContract::Presenter> presenter);

      private:
        void buildInterface() override;

        std::shared_ptr<SimContactsImportWindowPresenter::Presenter> presenter;
    };

} // namespace gui
