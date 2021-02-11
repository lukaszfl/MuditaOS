#include "ChangeTimeZone.hpp"
#include <application-settings-new/ApplicationSettings.hpp>
#include <OptionSetting.hpp>

#include <date/tz.h>

namespace gui
{

    ChangeTimeZone::ChangeTimeZone(app::Application *app) : OptionWindow(app, window::name::change_date_and_time)
    {
        date::set_install("current/assets/tzdb");
        buildInterface();
    }

    void ChangeTimeZone::buildInterface()
    {
        setTitle(utils::localize.get("app_settings_date_and_time_time_zone"));
        bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get(style::strings::common::select));
    }

    bool ChangeTimeZone::handleSwitchData(SwitchData * /*data*/)
    {
        return true;
    }

    std::list<Option> ChangeTimeZone::buildOptionsList()
    {
        std::list<gui::Option> options;
        auto &tzdb = date::get_tzdb();

        for (auto &zoneItem : tzdb.zones) {
            options.emplace_back(std::make_unique<gui::option::OptionSettings>(
                zoneItem.name(),
                [=](const gui::Item &item) { return true; },
                nullptr,
                this,
                gui::option::SettingRightItem::Disabled

                ));
        }

        return options;
    }

    void ChangeTimeZone::onBeforeShow(ShowMode /*mode*/, SwitchData * /*data*/)
    {
        clearOptions();
        addOptions(buildOptionsList());
        setFocus(optionsList);
    }

    bool ChangeTimeZone::onInput(const InputEvent &inputEvent)
    {
        return AppWindow::onInput(inputEvent);
    }

} // namespace gui
