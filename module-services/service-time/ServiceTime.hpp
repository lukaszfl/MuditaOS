/*
 *  @file ServiceTime.hpp
 *  @author
 *  @date 08.05.20
 *  @brief
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */

#ifndef PUREPHONE_SERVICETIME_HPP
#define PUREPHONE_SERVICETIME_HPP

#include "Service/Service.hpp"
#include <functional>
#include "MessageType.hpp"

#include <module-db/queries/calendar/QueryEventsGet.hpp>
#include <module-db/queries/calendar/QueryEventsEdit.hpp>

class ServiceTime;

class TimeEvents
{
  protected:
    uint32_t timerId = 0;
    uint32_t lastDT  = 0;
    bool timerActive = false;

    void StartTimer();
    // auto id    = CreateTimer(interval, isPeriodic, name);

  public:
    TimeEvents();
    ~TimeEvents();

    virtual sys::Message_t OnDataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp = nullptr);
    virtual void OnTickHandler(uint32_t id);

    virtual bool Reload();
};

//************************************************************************************************

class CalendarTimeEvents : public TimeEvents
{
  private:
    uint32_t todayEventsNum = 0;
    EventsRecord eventRecord;

  protected:
    void DeleteTimer();
    void FireReminder();

  public:
    CalendarTimeEvents();
    ~CalendarTimeEvents();

    sys::Message_t OnDataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp = nullptr);
    void OnTickHandler(uint32_t id);

    bool Reload() override;
};

//************************************************************************************************
//************************************************************************************************
//************************************************************************************************

class ServiceTime : public sys::Service
{
  private:
    CalendarTimeEvents calendarEvents;

  public:
    ServiceTime();

    ~ServiceTime();

    sys::Message_t DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp = nullptr) override;

    // Invoked when timer ticked
    void TickHandler(uint32_t id) override final;

    // Invoked during initialization
    sys::ReturnCodes InitHandler() override;

    sys::ReturnCodes DeinitHandler() override;

    sys::ReturnCodes SwitchPowerModeHandler(const sys::ServicePowerMode mode) override final;

    static const char *serviceName;
};

#endif // PUREPHONE_SERVICETIME_HPP
