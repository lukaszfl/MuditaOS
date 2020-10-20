#include "CalendarTimeEvents.hpp"

#include <module-services/service-appmgr/ApplicationManager.hpp>
#include <module-services/service-db/api/DBServiceAPI.hpp>
#include <module-services/service-db/messages/DBNotificationMessage.hpp>
#include <module-db/queries/calendar/QueryEventsSelectFirstUpcoming.hpp>

#include <module-gui/gui/SwitchData.hpp>
#include <module-apps/application-calendar/data/CalendarData.hpp>
#include <module-apps/application-calendar/ApplicationCalendar.hpp>
#include <module-apps/application-calendar/data/dateCommon.hpp>

namespace stm
{
    constexpr static const int eventTimerMinSkipInterval = 100;

    CalendarTimeEvents::CalendarTimeEvents(sys::Service *service) : TimeEvents(service)
    {}

    bool CalendarTimeEvents::SendNextEventQuery()
    {
        TimePoint filterFrom = TimePointNow();

        // mlucki
        std::string nowStr1 = TimePointToString(filterFrom);

        time_t time = TimePointToTimeT(TimePointNow());
        // mlucki
        // bsp::rtc_GetCurrentTimestamp(&time);
        std::string nowStr2 = TimePointToString(TimePointFromTimeT(time));

        TimePoint filterTill = filterFrom;
        if (startTP != TIME_POINT_INVALID) {
            filterFrom = std::min(startTP, filterFrom);
            filterTill = filterFrom;
        }

        // mlucki
        // Temporary values:
        // filterFrom = TimePointFromString("2020-09-16 00:00:00");
        // filterTill = TimePointFromString("2020-09-20 00:00:00");

        return DBServiceAPI::GetQuery(Service(),
                                      db::Interface::Name::Events,
                                      std::make_unique<db::query::events::SelectFirstUpcoming>(filterFrom, filterTill));
    }

    uint32_t CalendarTimeEvents::CalcToNextEventInterval(std::unique_ptr<db::QueryResult> nextEventQueryResult)
    {
        auto firstUpcomingQuery =
            dynamic_cast<db::query::events::SelectFirstUpcomingResult *>(nextEventQueryResult.get());
        if (firstUpcomingQuery == nullptr) {
            return 0;
        }

        std::unique_ptr<std::vector<EventsRecord>> records = firstUpcomingQuery->getResult();
        if (records->size() == 0) {
            return 0;
        }

        eventRecord = records->at(0);
        startTP     = eventRecord.date_from - minutes{eventRecord.reminder};

        [[maybe_unused]] std::string x1 = TimePointToString(eventRecord.date_from);
        [[maybe_unused]] std::string x2 =
            TimePointToString(eventRecord.date_from - std::chrono::minutes{eventRecord.reminder});
        [[maybe_unused]] std::string x3 = TimePointToString(TimePointNow());

        auto duration = eventRecord.date_from - std::chrono::minutes{eventRecord.reminder} - TimePointNow();
        if (duration.count() <= 0) {
            duration = std::chrono::milliseconds(eventTimerMinSkipInterval);
        }

        [[maybe_unused]] uint32_t x4 = std::chrono::duration_cast<std::chrono::minutes>(duration).count();

        // mlucki
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        // return 5000;
    }

    bool CalendarTimeEvents::SendEventFiredQuery()
    {
        eventRecord.reminder_fired = TimePointNow();
        return DBServiceAPI::GetQuery(
            Service(), db::Interface::Name::Events, std::make_unique<db::query::events::Edit>(eventRecord));
    }

    void CalendarTimeEvents::InvokeEvent()
    {
        std::unique_ptr<EventRecordData> eventData = std::make_unique<EventRecordData>();
        eventData->setDescription(style::window::calendar::name::event_reminder_window);
        auto event = std::make_shared<EventsRecord>(eventRecord);
        eventData->setData(event);

        sapm::ApplicationManager::messageSwitchApplication(
            Service(), app::name_calendar, style::window::calendar::name::event_reminder_window, std::move(eventData));
    }
} // namespace stm
