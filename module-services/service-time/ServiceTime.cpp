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
#include <module-services/service-cellular/messages/CellularMessage.hpp>
#include <module-gui/gui/SwitchData.hpp>
#include "module-apps/application-calendar/data/CalendarData.hpp"
#include "module-apps/application-calendar/ApplicationCalendar.hpp"
#include "module-apps/application-calendar/data/dateCommon.hpp"

namespace stm
{
    TimeEvents::TimeEvents(sys::Service *service) : service(service)
    {}

    TimeEvents::~TimeEvents()
    {}

    std::unique_ptr<sys::Timer> &TimeEvents::Timer()
    {
        if (fireEventTimer == nullptr) {
            fireEventTimer = std::make_unique<sys::Timer>(TimerName(), service, 1000, sys::Timer::Type::SingleShot);
        }
        return fireEventTimer;
    }

    void TimeEvents::StopTimer()
    {
        Timer()->stop();
    }

    void TimeEvents::RecreateTimer(uint32_t interval)
    {
        StopTimer();
        Timer()->connect([=](sys::Timer &) { fireEventTimerCallback(); });
        Timer()->reload(interval);
    }

    void TimeEvents::fireEventTimerCallback()
    {
        InvokeEvent();
        SendEventFiredQuery();
    }

    bool TimeEvents::ReceiveNextEventQuery(std::unique_ptr<db::QueryResult> nextEventQueryResult)
    {
        if (nextEventQueryResult == nullptr) {
            return false;
        }
        uint32_t interval = CalcToNextEventInterval(std::move(nextEventQueryResult));
        if (interval > 0) {
            RecreateTimer(interval);
        }
        return true;
    }

    uint32_t TimeEvents::CalcToNextEventInterval(std::unique_ptr<db::QueryResult> nextEventQueryResult)
    {
        return 0;
    }

    void TimeEvents::InvokeEvent()
    {
    }

    //************************************************************************************************

    CalendarTimeEvents::CalendarTimeEvents(sys::Service *service) : TimeEvents(service)
    {}

    CalendarTimeEvents::~CalendarTimeEvents()
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

    //************************************************************************************************
    //************************************************************************************************
    //************************************************************************************************

    ServiceTime::ServiceTime()
        : sys::Service(service::name::service_time, "", 4096 * 2, sys::ServicePriority::Idle), calendarEvents(this)
    {
        LOG_INFO("[ServiceTime] Initializing");
        busChannels.push_back(sys::BusChannels::ServiceDBNotifications);

        calendarReminderTimer =
            std::make_unique<sys::Timer>("CalendarReminderTimer_service", this, 1000, sys::Timer::Type::SingleShot);
        // calendarReminderTimer->connect([=](sys::Timer &) { reminderTimerCallback(); });
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

    bool ServiceTime::SendReloadQuery()
    {
        if (!timersProcessingStarted) {
            LOG_ERROR("Reload query called when timers' processing not started!");
            return false;
        }

        TimePoint filterFrom = TimePointNow();
        TimePoint filterTill = TimePointNow();
        if (startTP != TIME_POINT_INVALID) {
            filterFrom = std::min(startTP, filterFrom);
            filterTill = filterFrom;
        }

        // mlucki
        // Temporary values:
        filterFrom = TimePointFromString("2020-09-16 00:00:00");
        filterTill = TimePointFromString("2020-09-20 00:00:00");

        return DBServiceAPI::GetQuery(this,
                                      db::Interface::Name::Events,
                                      std::make_unique<db::query::events::SelectFirstUpcoming>(filterFrom, filterTill));
    }

    void ServiceTime::ReceiveReloadQuery(std::unique_ptr<std::vector<EventsRecord>> records)
    {
        if (records->size() == 0) {
            return;
        }

        eventRecord = records->at(0);
        startTP     = eventRecord.date_from - minutes{eventRecord.reminder};

        // Recreate timer
        RecreateTimer(CalcInterval());

        /*if (records->size()) {
            auto firstRec = records->at(0);

            ReceiveReloadQuery(std::move(records));

            int c   = 0;
            auto s1 = TimePointToString(firstRec.date_from);
            auto s2 = TimePointToString(firstRec.date_till);
            if (s1.length() < s2.length()) {
                c++;
            }
        }*/
    }

    void ServiceTime::StopTimer()
    {
        /*if (timerId == 0) {
            return;
        }

        stopTimer(timerId);
        DeleteTimer(timerId);
        timerId = 0;
         */
        calendarReminderTimer->stop();
    }

    uint32_t ServiceTime::CalcInterval()
    {
        auto duration = eventRecord.date_from - std::chrono::minutes{eventRecord.reminder} - TimePointNow();
        if (duration.count() <= 0) {
            duration = std::chrono::milliseconds(100);
        }
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }

    void ServiceTime::RecreateTimer(uint32_t interval)
    {
        /*StopTimer();

        auto duration = eventRecord.date_from - chrono::minutes{eventRecord.reminder} - TimePointNow();
        if (duration.count() <= 0) {
            duration = chrono::milliseconds(100);
        }
        [[maybe_unused]] uint32_t interval = chrono::duration_cast<chrono::milliseconds>(duration).count();

        timerId = CreateTimer(10000, false, "ServiceTime_EventsTimer");
        // timerId = CreateTimer(interval, false, "ServiceTime_EventsTimer");
        ReloadTimer(timerId);*/

        StopTimer();
        [[maybe_unused]] uint32_t interval1 = interval;

        // timerId = CreateTimer(interval, false, "ServiceTime_EventsTimer");
        ////calendarReminderTimer->setInterval(10000);
        ////calendarReminderTimer->start();

        calendarReminderTimer->connect([=](sys::Timer &) { reminderTimerCallback(); });
        calendarReminderTimer->reload(5000);
    }

    void ServiceTime::InvokeReminder()
    {
        // Invoke notification
        // return sys::Bus::SendUnicast(msg, service::name::db, serv);

        /*std::shared_ptr<AudioMessage> msg;
        auto msg = std::make_shared<AntennaLockRequestMessage>(MessageType::AntennaLockService, request);
        auto ret = sys::Bus::SendUnicast(msg, ServiceAntenna::serviceName, this);*/

        // auto msg = std::make_shared<CalendarReminderFiredNotificationMessage>();
        // sys::Bus::SendUnicast(msg, this->serviceName, this);
        if (0) {
            auto msg = std::make_shared<cellular::StateChange>(cellular::State::ST::ModemFatalFailure);
            sys::Bus::SendMulticast(msg, sys::BusChannels::ServiceCellularNotifications, this);
        }

        // mlucki
        // application->switchWindow("Reboot", gui::ShowMode::GUI_SHOW_RETURN);

        /*
            auto data = std::make_unique<gui::SwitchData>("ServiceTime_switch");
            //gui::SwitchData x("ServiceTime_switch");
            data->ignoreCurrentWindowOnStack = true;

            LOG_DEBUG("show all calendar events!");
            sapm::ApplicationManager::messageSwitchApplication(
                // this, app::name_calendar, gui::name::window::main_window, nullptr);
                this,
                "ApplicationCalendar",
                "CustomRepeat",
                std::move(data));
        */

        // LOG_DEBUG("show all calendar events!");
        /*auto data = std::make_unique<gui::SwitchData>("ServiceTime_switch");
        data->ignoreCurrentWindowOnStack = true;
        //data->disableAppClose = true;
        sapm::ApplicationManager::messageSwitchApplication(this, "ApplicationDesktop", "MenuWindow", std::move(data));*/
        // sapm::ApplicationManager::messageSwitchApplication(this, "ApplicationDesktop", "MenuWindow", nullptr);

        // sapm::ApplicationManager::messageSwitchApplication(this, "ApplicationCall", "CallWindow", nullptr);

#if 1
        ////sapm::ApplicationManager::messageSwitchApplication(this, "ApplicationDesktop", "MenuWindow", nullptr);

        // auto ad = sapm::ApplicationManager::ApplicationDescription *appGet(const std::string &name);

        std::unique_ptr<EventRecordData> eventData = std::make_unique<EventRecordData>();
        eventData->setDescription(style::window::calendar::name::event_reminder_window);
        auto event = std::make_shared<EventsRecord>(eventRecord);
        /*event = eventRecord;
        event->date_from = dateFilter;
        event->date_till = dateFilter;*/
        eventData->setData(event);
        eventData->setWindowName("");

        sapm::ApplicationManager::messageSwitchApplication(
            this, "ApplicationCalendar", "EventReminderWindow", std::move(eventData));

        /*auto app = dynamic_cast<app::ApplicationDesktop *>(application);
        assert(app != nullptr);

        if (app->lockHandler.lock.isLocked() && app->lockHandler.lock.getLockType() == PinLock::LockType::Screen) {
            bottomBar->restore();
            bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get("app_desktop_unlock"));
            topBar->setActive(TopBar::Elements::LOCK, true);
            inputCallback = nullptr;
            setFocusItem(nullptr);
            erase(notifications);
        }
        else if (app->lockHandler.lock.isLocked()) {*/

#endif

        // mlucki
        // to działa
        // LOG_DEBUG("show all calendar events!");
        // sapm::ApplicationManager::messageSwitchApplication(this, "ApplicationDesktop", "PinLockWindow", nullptr);

        // application->switchWindow("MenuWindow");
        // PowerOffWindow
        /*
                windows.insert(std::pair<std::string, gui::AppWindow *>(desktop_pin_lock, new
           gui::PinLockWindow(this))); windows.insert(std::pair<std::string, gui::AppWindow *>(desktop_menu, new
           gui::MenuWindow(this))); windows.insert(std::pair<std::string, gui::AppWindow *>(desktop_poweroff, new
           gui::PowerOffWindow(this))); windows.insert(std::pair<std::string, gui::AppWindow *>(desktop_reboot, new
           gui::RebootWindow(this)));
         */

        /*
                    sys::Bus::SendMulticast(
                        std::make_shared<CellularCallMessage>(CellularCallMessage::Type::Ringing, msg->number),
                        sys::BusChannels::ServiceCellularNotifications,
                        this);
                    break;
         */

        //    auto ApplicationDesktop::handle(cellular::StateChange *msg) -> bool

        // W applicationDesktop mamy:
        /*else if (auto msg = dynamic_cast<cellular::StateChange *>(msgl)) {
            handled = handle(msg);*/
        // który woła
        //    auto ApplicationDesktop::handle(cellular::StateChange *msg) -> bool
        // gdzie mamy wywołanie
        /*
            if (msg->request == cellular::State::ST::ModemFatalFailure) {
                switchWindow(app::window::name::desktop_reboot);
            }
         */

        // Przełączanie okien, switch
        //"LastWindow"
    }

    bool ServiceTime::SendReminderFiredQuery()
    {
        // eventRecord.ID             = 1;
        eventRecord.reminder_fired = TimePointNow();
        return DBServiceAPI::GetQuery(
            this, db::Interface::Name::Events, std::make_unique<db::query::events::Edit>(eventRecord));
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

                // mlucki
                // timersProcessingStarted = true;

                StopTimer();
                SendReloadQuery();

                return responseMsg;
            }
            /*if (msg->interface == db::Interface::Name::Events && msg->type == db::Query::Type::Read) {

                ReceiveFilterQuery();

                return responseMsg;
            }*/
        } break;
        case MessageType::ReloadTimers: {
            StopTimer();
            SendReloadQuery();
        } break;
        case MessageType::TimersProcessingStart: {
            timersProcessingStarted = true;
            StopTimer();
            SendReloadQuery();
        } break;
        case MessageType::TimersProcessingStop: {
            timersProcessingStarted = false;
            StopTimer();
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

                // mlucki
                // Przykład odbioru GetFiltered
                /*if (auto getFilteredQuery = dynamic_cast<db::query::events::GetFilteredResult *>(result.get())) {
                    std::unique_ptr<std::vector<EventsRecord>> records = getFilteredQuery->getResult();
                    // Do something you want
                    // Example:
                    int c = 0;
                    for (auto &rec : *records) {
                        auto s1 = TimePointToString(rec.date_from);
                        auto s2 = TimePointToString(rec.date_till);
                        if (s1.length() < s2.length()) {
                            c++;
                        }
                    }
                    responseHandled = true;
                }*/
                if (auto firstUpcomingQuery =
                        dynamic_cast<db::query::events::SelectFirstUpcomingResult *>(result.get())) {
                    std::unique_ptr<std::vector<EventsRecord>> records = firstUpcomingQuery->getResult();

                    ReceiveReloadQuery(std::move(records));
                    /*if (records->size()) {
                        auto firstRec = records->at(0);

                        ReceiveReloadQuery(std::move(records));

                        int c   = 0;
                        auto s1 = TimePointToString(firstRec.date_from);
                        auto s2 = TimePointToString(firstRec.date_till);
                        if (s1.length() < s2.length()) {
                            c++;
                        }
                    }*/
                    responseHandled = true;
                }

                // mlucki
                // Można podarować tu sobie reakcję na powrót z update-u reminder_fired,
                // bo przyjdzie przecież notyfikacja: msg->type == db::Query::Type::Update
                // która wymusi SendReloadQuery
                /*if (auto updateQuery = dynamic_cast<db::query::events::EditResult *>(result.get())) {
                    [[maybe_unused]]bool ret = updateQuery->getResult();

                    ReceiveReminderFiredQuery();

                    responseHandled = true;
                }*/
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

    bool ServiceTime::messageReloadTimers(sys::Service *sender)
    {
        auto msg = std::make_shared<ReloadTimersMessage>();
        return sys::Bus::SendUnicast(msg, service::name::service_time, sender);
    }

    bool ServiceTime::messageTimersProcessingStart(sys::Service *sender)
    {
        auto msg = std::make_shared<TimersProcessingStartMessage>();
        return sys::Bus::SendUnicast(msg, service::name::service_time, sender);
    }

    bool ServiceTime::messageTimersProcessingStop(sys::Service *sender)
    {
        auto msg = std::make_shared<TimersProcessingStopMessage>();
        return sys::Bus::SendUnicast(msg, service::name::service_time, sender);
    }

    void ServiceTime::reminderTimerCallback()
    {
        // calendarEvents.OnTickHandler(id);

        InvokeReminder();
        SendReminderFiredQuery();
    }
} /* namespace stm */
