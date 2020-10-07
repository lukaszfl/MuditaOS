#ifndef PUREPHONE_SERVICETIME_HPP
#define PUREPHONE_SERVICETIME_HPP

#include "Service/Service.hpp"
#include "ServiceTime.hpp"
#include <functional>
#include "MessageType.hpp"

#include <module-db/queries/calendar/QueryEventsGet.hpp>
#include <module-db/queries/calendar/QueryEventsEdit.hpp>
#include <module-sys/Service/Timer.hpp>

namespace service::name
{
    const inline std::string service_time = "ServiceTime";
};

namespace stm
{
    class TimeEvents
    {
      private:
        std::unique_ptr<sys::Timer> fireEventTimer = nullptr;
        sys::Service *service                      = nullptr;
        std::unique_ptr<sys::Timer> &Timer();

      protected:
        sys::Service *Service()
        {
            return service;
        };
        virtual const std::string TimerName() = 0;

        void StopTimer();
        void RecreateTimer(uint32_t interval);
        virtual void fireEventTimerCallback();

        virtual uint32_t CalcToNextEventInterval(std::unique_ptr<db::QueryResult> nextEventQueryResult) = 0;

      public:
        TimeEvents() = delete;
        TimeEvents(sys::Service *service);
        ~TimeEvents();

        virtual bool SendNextEventQuery() = 0;
        virtual bool ReceiveNextEventQuery(std::unique_ptr<db::QueryResult> nextEventQueryResult);
        virtual bool SendEventFiredQuery() = 0;
        virtual void InvokeEvent();
    };

    //************************************************************************************************

    class CalendarTimeEvents : public TimeEvents
    {
      private:
        EventsRecord eventRecord;
        TimePoint startTP = TIME_POINT_INVALID;

      protected:
        const std::string TimerName() override
        {
            return "CalendarTimeEvents_timer";
        };
        uint32_t CalcToNextEventInterval(std::unique_ptr<db::QueryResult> nextEventQueryResult) override;

      public:
        CalendarTimeEvents() = delete;
        CalendarTimeEvents(sys::Service *service);
        ~CalendarTimeEvents();

        bool SendNextEventQuery() override;
        bool SendEventFiredQuery() override;
        void InvokeEvent() override;
    };

    //************************************************************************************************
    //************************************************************************************************
    //************************************************************************************************

    class ServiceTime : public sys::Service
    {
      private:
        CalendarTimeEvents calendarEvents;
        EventsRecord eventRecord;
        TimePoint startTP = TIME_POINT_INVALID;
        uint32_t timerId  = 0;

        std::unique_ptr<sys::Timer> calendarReminderTimer = nullptr;

        bool timersProcessingStarted = false;

      protected:
        bool SendReloadQuery();
        void ReceiveReloadQuery(std::unique_ptr<std::vector<EventsRecord>> records);

        void StopTimer();
        uint32_t CalcInterval();
        void RecreateTimer(uint32_t interval);
        void InvokeReminder();
        bool SendReminderFiredQuery();

      public:
        ServiceTime();
        ~ServiceTime();

        // Invoked during initialization
        sys::ReturnCodes InitHandler() override;
        sys::ReturnCodes DeinitHandler() override;
        sys::ReturnCodes SwitchPowerModeHandler(const sys::ServicePowerMode mode) override final;

        static bool messageReloadTimers(sys::Service *sender);
        static bool messageTimersProcessingStart(sys::Service *sender);
        static bool messageTimersProcessingStop(sys::Service *sender);

        sys::Message_t DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp = nullptr) override;
        void reminderTimerCallback();
    };

} /* namespace stm */

#endif // PUREPHONE_SERVICETIME_HPP
