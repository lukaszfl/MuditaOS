#pragma once

//#include "BaseSettingsWindow.hpp"
#include <OptionWindow.hpp>

namespace gui
{

    class ChangeTimeZone : public OptionWindow
    {
      public:
        explicit ChangeTimeZone(app::Application *app);
        void buildInterface() override;
        auto handleSwitchData(SwitchData *data) -> bool override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;
        auto onInput(const InputEvent &inputEvent) -> bool override;

        auto buildOptionsList() -> std::list<Option>;

        // auto buildOptionsList() -> std::list<Option> override;
        //        unsigned int selectedTimeZone{0};
    };
} // namespace gui
