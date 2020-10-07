#include "ServiceTime.hpp"
#include "messages/TimeMessage.hpp"
#include <vector>

#include "service-db/messages/DBNotificationMessage.hpp"
#include "service-db/messages/QueryMessage.hpp"
#include <module-db/queries/calendar/QueryEventsGetAllLimited.hpp>

#include <module-db/queries/calendar/QueryEventsSelectFirstUpcoming.hpp>
#include <module-services/service-db/api/DBServiceAPI.hpp>

#include "service-appmgr/ApplicationManager.hpp"

// mlucki
#include <module-gui/gui/SwitchData.hpp>
#include "module-apps/application-calendar/data/CalendarData.hpp"
#include "module-apps/application-calendar/ApplicationCalendar.hpp"
#include "module-apps/application-calendar/data/dateCommon.hpp"

namespace stm
{
#if 0
    TimeEvents::TimeEvents(sys::Service *service) : service(service)
    {}

    TimeEvents::~TimeEvents()
    {
        StopTimer();
    }

    std::unique_ptr<sys::Timer> &TimeEvents::Timer()
    {
        if (fireEventTimer == nullptr) {
            fireEventTimer = std::make_unique<sys::Timer>(TimerName(), service, 1000, sys::Timer::Type::SingleShot);
        }
        return fireEventTimer;
    }

    void TimeEvents::StartProcessing()
    {
        timersProcessingStarted = true;
        ProcessNextEvent();
    }

    void TimeEvents::StopProcessing()
    {
        StopTimer();
        timersProcessingStarted = false;
    }

    void TimeEvents::ProcessNextEvent()
    {
        StopTimer();
        if (!IsStarted()) {
            return;
        }

        SendNextEventQuery();
    }

    void TimeEvents::StopTimer()
    {
        Timer()->stop();
    }

    void TimeEvents::RecreateTimer(uint32_t interval)
    {
        StopTimer();
        if (!IsStarted()) {
            return;
        }

        Timer()->connect([=](sys::Timer &) { fireEventTimerCallback(); });
        Timer()->reload(interval);
    }

    void TimeEvents::fireEventTimerCallback()
    {
        if (!IsStarted()) {
            return;
        }

        InvokeEvent();
        SendEventFiredQuery();
    }

    bool TimeEvents::ReceiveNextEventQuery(std::unique_ptr<db::QueryResult> nextEventQueryResult)
    {
        if (!IsStarted()) {
            return true;
        }
        if (nextEventQueryResult == nullptr) {
            return false;
        }

        uint32_t interval = CalcToNextEventInterval(std::move(nextEventQueryResult));
        if (interval > 0) {
            RecreateTimer(interval);
        }
        return true;
    }


    //************************************************************************************************

    CalendarTimeEvents::CalendarTimeEvents(sys::Service *service) : TimeEvents(service)
    {}

    bool CalendarTimeEvents::SendNextEventQuery()
    {
        TimePoint filterFrom = TimePointNow();
        TimePoint filterTill = filterFrom;
        if (startTP != TIME_POINT_INVALID) {
            filterFrom = std::min(startTP, filterFrom);
            filterTill = filterFrom;
        }

        // mlucki
        // Temporary values:
        filterFrom = TimePointFromString("2020-09-16 00:00:00");
        filterTill = TimePointFromString("2020-09-20 00:00:00");

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

        auto duration = eventRecord.date_from - std::chrono::minutes{eventRecord.reminder} - TimePointNow();
        if (duration.count() <= 0) {
            duration = std::chrono::milliseconds(100);
        }
        // mlucki
        // return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        return 5000;
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
        eventData->setWindowName("");

        sapm::ApplicationManager::messageSwitchApplication(
            Service(), app::name_calendar, style::window::calendar::name::event_reminder_window, std::move(eventData));
    }
#endif

    ServiceTime::ServiceTime()
        : sys::Service(service::name::service_time, "", 4096 * 2, sys::ServicePriority::Idle), calendarEvents(this)
    {
        LOG_INFO("[ServiceTime] Initializing");
        busChannels.push_back(sys::BusChannels::ServiceDBNotifications);
    }

    ServiceTime::~ServiceTime()
    {
        LOG_INFO("[ServiceTime] Cleaning resources");
    }

    sys::ReturnCodes ServiceTime::InitHandler()
    {
        return sys::ReturnCodes::Success;
    }

    sys::ReturnCodes ServiceTime::DeinitHandler()
    {
        return sys::ReturnCodes::Success;
    }

    sys::ReturnCodes ServiceTime::SwitchPowerModeHandler(const sys::ServicePowerMode mode)
    {
        LOG_FATAL("[ServiceTime] PowerModeHandler: %s", c_str(mode));
        return sys::ReturnCodes::Success;
    }

    sys::Message_t ServiceTime::DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
    {
        std::shared_ptr<sys::ResponseMessage> responseMsg = nullptr;

        switch (static_cast<MessageType>(msgl->messageType)) {
        case MessageType::DBServiceNotification: {
            auto msg = dynamic_cast<db::NotificationMessage *>(msgl);
            if (msg == nullptr) {
                responseMsg = std::make_shared<TimeResponseMessage>(false);
                break;
            }
            if (msg->interface == db::Interface::Name::Events &&
                (msg->type == db::Query::Type::Create || msg->type == db::Query::Type::Update ||
                 msg->type == db::Query::Type::Delete)) {

                calendarEvents.ProcessNextEvent();

                return responseMsg;
            }
        } break;
        case MessageType::ReloadTimers: {
            calendarEvents.ProcessNextEvent();
        } break;
        case MessageType::TimersProcessingStart: {
            calendarEvents.StartProcessing();
        } break;
        case MessageType::TimersProcessingStop: {
            calendarEvents.StopProcessing();
        } break;
        default:
            break;
        }

        if (responseMsg != nullptr) {
            responseMsg->responseTo = msgl->messageType;
            return responseMsg;
        }

        // handle database response
        bool responseHandled = false;
        if (resp != nullptr) {
            if (auto msg = dynamic_cast<db::QueryResponse *>(resp)) {
                auto result = msg->getResult();

                if (dynamic_cast<db::query::events::SelectFirstUpcomingResult *>(result.get())) {

                    calendarEvents.ReceiveNextEventQuery(std::move(result));
                    responseHandled = true;
                }
            }
            if (responseHandled) {
                return std::make_shared<sys::ResponseMessage>();
            }
            else {
                return std::make_shared<sys::ResponseMessage>(sys::ReturnCodes::Unresolved);
            }
        }
        else {
            return std::make_shared<sys::ResponseMessage>();
        }
    }
} /* namespace stm */
