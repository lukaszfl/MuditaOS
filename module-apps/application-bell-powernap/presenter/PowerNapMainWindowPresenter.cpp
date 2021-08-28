// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "PowerNapMainWindowPresenter.hpp"
#include "models/PowerNapModel.hpp"
#include <apps-common/Application.hpp>
#include <service-db/agents/settings/SystemSettings.hpp>

namespace
{
    constexpr auto napTimeValue = settings::Temperature::unit;
} // namespace

namespace app::powernap
{
    PowerNapMainWindowPresenter::PowerNapMainWindowPresenter(app::Application *app)
        : model{std::make_shared<PowerNapModel>()}
    {
        settings.init(service::ServiceProxy{app->weak_from_this()});
    }

    auto PowerNapMainWindowPresenter::getNapTimePresenter() -> std::shared_ptr<gui::ListItemProvider>
    {
        return model;
    }

    void PowerNapMainWindowPresenter::loadData()
    {
        const auto value = settings.getValue(napTimeValue, settings::SettingsScope::AppLocal);
        // constexpr auto dbFakeValue = std::chrono::minutes{15};
        model->createData();
        model->setValue(std::chrono::minutes{utils::getNumericValue<int>(value)});
    }

    void PowerNapMainWindowPresenter::activate()
    {
        const auto currentValue = model->getCurrentValue();
        settings.setValue(napTimeValue, utils::to_string(currentValue.count()), settings::SettingsScope::AppLocal);
    }
} // namespace app::powernap
