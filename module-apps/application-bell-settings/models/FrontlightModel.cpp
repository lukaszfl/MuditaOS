// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "FrontlightModel.hpp"

#include <gui/widgets/ListViewEngine.hpp>
#include <gui/widgets/Style.hpp>
#include <gui/widgets/Text.hpp>
#include <service-time/Constants.hpp>
#include <widgets/FrontlightSpinnerListItem.hpp>

namespace app::bell_settings
{
    FrontlightModel::FrontlightModel(app::Application *app, app::settingsInterface::BellScreenLightSettings *settings) : application(app), screenLightSettings(settings)
    {}

    FrontlightModel::~FrontlightModel()
    {
        clearData();
    }

    auto FrontlightModel::requestRecordsCount() -> unsigned int
    {
        return internalData.size();
    }

    auto FrontlightModel::getMinimalItemSpaceRequired() const -> unsigned int
    {
        return style::sidelistview::list_item::w;
    }

    auto FrontlightModel::getItem(gui::Order order) -> gui::ListItem *
    {
        return getRecord(order);
    }

    void FrontlightModel::createData()
    {

        frontlightSpinnerListItem = new gui::FrontlightSpinnerListItem(
            0, 0, 0, 0, utils::translate("app_bell_settings_frontlight_top_message"));
        internalData.push_back(frontlightSpinnerListItem);

        for (auto item : internalData) {
            item->deleteByList = false;
        }
    }

    void FrontlightModel::clearData()
    {
        eraseInternalData();
    }

    void FrontlightModel::saveData()
    {
        int value = (frontlightSpinnerListItem->frontlightSetSpinner->getCurrentValue() + 1) * 10;
        LOG_DEBUG("Storing frontlight value %i", value);
        screenLightSettings->setBrightness(static_cast<float>(value));
    }

    void FrontlightModel::loadData()
    {
        auto values     = screenLightSettings->getCurrentValues();
        bsp::eink_frontlight::BrightnessPercentage brightnessValue = values.parameters.manualModeBrightness;
        LOG_DEBUG("Reading frontlight value %0.1f", brightnessValue);
        frontlightSpinnerListItem->frontlightSetSpinner->setCurrentValue(static_cast<int>(brightnessValue / 10) - 1);
    }

    auto FrontlightModel::requestRecords(uint32_t offset, uint32_t limit) -> void
    {
        setupModel(offset, limit);
        list->onProviderDataUpdate();
    }

} // namespace app::bell_settings
