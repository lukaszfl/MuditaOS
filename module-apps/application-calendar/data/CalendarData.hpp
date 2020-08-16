#pragma once
#include "module-db/Interface/EventsRecord.hpp"
#include <module-utils/date/include/date/date.h>

class EventRecordData : public gui::SwitchData
{
  protected:
    std::unique_ptr<EventsRecord> record;

  public:
    EventRecordData() = default;
    EventRecordData(std::unique_ptr<EventsRecord> record) : record{std::move(record)} {};
    virtual ~EventRecordData() = default;
    std::unique_ptr<EventsRecord> getData()
    {
        return std::move(record);
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