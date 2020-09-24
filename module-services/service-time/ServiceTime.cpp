/*
 *  @file ServiceTime.cpp
 *  @author
 *  @date 08.05.20
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#include "ServiceTime.hpp"
#include "messages/TimeMessage.hpp"
#include <vector>

#include "service-db/messages/DBNotificationMessage.hpp"
#include "service-db/messages/QueryMessage.hpp"
#include <module-db/queries/calendar/QueryEventsGetAllLimited.hpp>

#include <module-db/queries/calendar/QueryEventsGetAll.hpp>
#include <module-db/queries/calendar/QueryEventsGetFiltered.hpp>
#include <module-db/queries/calendar/QueryEventsSelectFirstUpcoming.hpp>
#include <module-services/service-db/api/DBServiceAPI.hpp>

TimeEvents::TimeEvents()
{}

TimeEvents::~TimeEvents()
{}

void TimeEvents::StartTimer()
{
    if (timerActive)
        return;

    // timerId = CreateTimer(1000, false, "EventsTimer");
}

sys::Message_t TimeEvents::OnDataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
{
    return nullptr;
}

void TimeEvents::OnTickHandler(uint32_t id)
{}

bool TimeEvents::Reload()
{
    return false;
}

//************************************************************************************************

CalendarTimeEvents::CalendarTimeEvents()
{}

CalendarTimeEvents::~CalendarTimeEvents()
{}

void CalendarTimeEvents::DeleteTimer()
{}

void CalendarTimeEvents::FireReminder()
{}

sys::Message_t CalendarTimeEvents::OnDataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
{
    TimeEvents::OnDataReceivedHandler(msgl, resp);

    // Notifications:
    // - DateTime changed
    // - Timezone changed
    // - PurePhone ON or service started
    // - Midnight day switch

    // - DB add
    // - DB del
    // - DB change
    // - TickHandler

    return nullptr;
}

void CalendarTimeEvents::OnTickHandler(uint32_t id)
{
    TimeEvents::OnTickHandler(id);

    if (id != timerId) {
        return;
    }

    DeleteTimer();

    FireReminder();

    // Sprawdzenie, czy id to identyfikator naszego timera
    // Jeśli nie to return
}

bool CalendarTimeEvents::Reload()
{
    return false;
}

//************************************************************************************************
//************************************************************************************************
//************************************************************************************************

const char *ServiceTime::serviceName = "ServiceTime";

ServiceTime::ServiceTime() : sys::Service(serviceName, "", 4096 * 2, sys::ServicePriority::Idle)
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

void ServiceTime::SendReloadQuery()
{
    DestroyTimer();

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

    // mlucki
    // Przykład wołania GetFiltered
    /*DBServiceAPI::GetQuery(
        this, db::Interface::Name::Events, std::make_unique<db::query::events::GetFiltered>(filterFrom, filterTill));*/

    DBServiceAPI::GetQuery(this,
                           db::Interface::Name::Events,
                           std::make_unique<db::query::events::SelectFirstUpcoming>(filterFrom, filterTill));
}

void ServiceTime::ReceiveReloadQuery(std::unique_ptr<std::vector<EventsRecord>> records)
{
    if (records->size() == 0) {
        return;
    }

    auto firstRecord = records->at(0);

    startTP = firstRecord.date_from - minutes{firstRecord.reminder};

    // Store current record
    eventRecord = firstRecord;

    // Recreate timer
    RecreateTimer();

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

void ServiceTime::DestroyTimer()
{
    if (timerId == 0) {
        return;
    }

    stopTimer(timerId);
    DeleteTimer(timerId);
    timerId = 0;
}

void ServiceTime::RecreateTimer()
{
    DestroyTimer();

    timerId = CreateTimer(5000, false, "ServiceTime_EventsTimer");
    ReloadTimer(timerId);
}

void ServiceTime::InvokeEvent()
{
    // Send invoke notification
}

void ServiceTime::SendReminderFiredQuery()
{
    eventRecord.ID             = 1;
    eventRecord.reminder_fired = TimePointNow();
    DBServiceAPI::GetQuery(this, db::Interface::Name::Events, std::make_unique<db::query::events::Edit>(eventRecord));
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

            SendReloadQuery();

            return responseMsg;
        }
    }
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
            if (auto firstUpcomingQuery = dynamic_cast<db::query::events::SelectFirstUpcomingResult *>(result.get())) {
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

void ServiceTime::TickHandler(uint32_t id)
{
    calendarEvents.OnTickHandler(id);

    InvokeEvent();
    SendReminderFiredQuery();
}
