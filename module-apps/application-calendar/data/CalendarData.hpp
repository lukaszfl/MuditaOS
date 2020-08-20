#pragma once
#include "module-db/Interface/EventsRecord.hpp"
#include <module-utils/date/include/date/date.h>

class EventRecordData : public gui::SwitchData
{
  protected:
    std::unique_ptr<EventsRecord> record;
    std::map<uint32_t, std::string> reminderOptions;
    std::map<uint32_t, std::string> repeatOptions;

  public:
    EventRecordData() = default;
    EventRecordData(std::unique_ptr<EventsRecord> record) : record{std::move(record)}
    {
        reminderOptions[0xFFFFFF] = "app_calendar_reminder_never";
        reminderOptions[0]        = "app_calendar_reminder_event_time";
        reminderOptions[5]        = "app_calendar_reminder_5_min_before";
        reminderOptions[15]       = "app_calendar_reminder_15_min_before";
        reminderOptions[30]       = "app_calendar_reminder_30_min_before";
        reminderOptions[100]      = "app_calendar_reminder_1_hour_before";
        reminderOptions[200]      = "app_calendar_reminder_2_hour_before";
        reminderOptions[10000]    = "app_calendar_reminder_1_day_before";
        reminderOptions[20000]    = "app_calendar_reminder_2_days_before";
        reminderOptions[70000]    = "app_calendar_reminder_1_week_before";
        repeatOptions[0]          = "app_calendar_repeat_never";
        repeatOptions[1]          = "app_calendar_repeat_daily";
        repeatOptions[2]          = "app_calendar_repeat_weekly";
        repeatOptions[3]          = "app_calendar_repeat_two_weeks";
        repeatOptions[4]          = "app_calendar_repeat_month";
        repeatOptions[5]          = "app_calendar_repeat_year";
        repeatOptions[6]          = "app_calendar_repeat_custom";
    };
    virtual ~EventRecordData() = default;
    std::unique_ptr<EventsRecord> getData()
    {
        return std::move(record);
    };
    std::map<uint32_t, std::string> getReminderOptionMap()
    {
        return reminderOptions;
    };
    std::map<uint32_t, std::string> getRepeatOptionMap()
    {
        return repeatOptions;
    };
    virtual void setData(std::unique_ptr<EventsRecord> rec)
    {
        record = std::move(rec);
    };
};

class WeekDaysRepeatData : public gui::SwitchData
{
  protected:
    bool weekDays[7];

  public:
    WeekDaysRepeatData()          = default;
    virtual ~WeekDaysRepeatData() = default;
    [[nodiscard]] auto getData(const uint32_t &weekDay) const -> bool
    {
        return weekDays[weekDay];
    };
    virtual void setData(const uint32_t &weekDay)
    {
        weekDays[weekDay] = true;
    };
};

class DayMonthData : public gui::SwitchData
{
  protected:
    std::string dayMonth;
    uint32_t dateFilter;

  public:
    DayMonthData()          = default;
    virtual ~DayMonthData() = default;
    std::string getDayMonthText()
    {
        return dayMonth;
    };

    uint32_t getDateFilter()
    {
        return dateFilter;
    };

    virtual void setData(std::string dayMonthText, const uint32_t &dateNumb)
    {
        dayMonth   = dayMonthText;
        dateFilter = dateNumb;
    };
};

// class SMSThreadData : public gui::SwitchData
//{
// public:
//    std::shared_ptr<ThreadRecord> thread = nullptr;
//    SMSThreadData(std::shared_ptr<ThreadRecord> thread) : thread(thread)
//    {}
//    virtual ~SMSThreadData() = default;
//};
//
// class SMSRequest : public gui::SwitchData
//{
// protected:
//    utils::PhoneNumber::View phoneNumber;
//
// public:
//    SMSRequest() = delete;
//    SMSRequest(const utils::PhoneNumber::View &phoneNumber) : phoneNumber(phoneNumber)
//    {}
//    virtual ~SMSRequest() = default;
//
//    const utils::PhoneNumber::View &getPhoneNumber() const
//    {
//        return phoneNumber;
//    };
//};
//
// class SMSSendRequest : public SMSRequest
//{
// public:
//    SMSSendRequest(const utils::PhoneNumber::View &phoneNumber) : SMSRequest(phoneNumber)
//    {}
//    virtual ~SMSSendRequest() = default;
//};
//
// class SMSSendTemplateRequest : public SMSRequest
//{
// public:
//    SMSSendTemplateRequest(const utils::PhoneNumber::View &phoneNumber) : SMSRequest(phoneNumber)
//    {}
//    virtual ~SMSSendTemplateRequest() = default;
//};
//
// class SMSTemplateSent : public gui::SwitchData
//{};
//
// class SMSTextData : public gui::SwitchData
//{
// public:
//    enum class Concatenate : bool
//    {
//        True,
//        False
//    };
//
//    SMSTextData(const UTF8 &text, Concatenate concatenate = Concatenate::False) : text(text), concatenate(concatenate)
//    {}
//    virtual ~SMSTextData() = default;
//    UTF8 text;
//    Concatenate concatenate = Concatenate::False;
//};
//
// class SMSTemplateRequest : public gui::SwitchData
//{
// public:
//    SMSTemplateRequest(const std::string &requestingWindow) : requestingWindow(requestingWindow)
//    {}
//    virtual ~SMSTemplateRequest() = default;
//
//    std::string requestingWindow;
//};