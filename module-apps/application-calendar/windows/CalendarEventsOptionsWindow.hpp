#pragma once

#include <module-db/Interface/EventsRecord.hpp>
#include "OptionWindow.hpp"

namespace gui
{
    class CalendarEventsOptions : public OptionWindow
    {
        std::unique_ptr<EventsRecord> eventRecord = nullptr;

      public:
        CalendarEventsOptions(app::Application *app);
        ~CalendarEventsOptions() override = default;
        auto handleSwitchData(SwitchData *data) -> bool override;

      private:
        auto eventsOptionsList() -> std::list<gui::Option>;
        auto eventDelete() -> bool;
    };
}; // namespace gui
