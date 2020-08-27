#pragma once
#include "module-db/Interface/EventsRecord.hpp"
#include <module-utils/date/include/date/date.h>

class EventRecordData : public gui::SwitchData
{
  protected:
    std::string backWindowName;
    std::shared_ptr<EventsRecord> record;

  public:
    EventRecordData() = default;
    EventRecordData(std::shared_ptr<EventsRecord> record) : record{std::move(record)} {};
    virtual ~EventRecordData() = default;
    std::shared_ptr<EventsRecord> getData()
    {
        return std::move(record);
    };
    virtual void setData(std::shared_ptr<EventsRecord> rec)
    {
        record = std::move(rec);
    };
    std::string getWindowName()
    {
        return backWindowName;
    };
    virtual void setWindowName(const std::string &windowName)
    {
        backWindowName = windowName;
    };
};

class WeekDaysRepeatData : public gui::SwitchData
{
  protected:
    bool weekDays[7];

  public:
    WeekDaysRepeatData()
    {
        for (uint32_t i = 0; i < 7; i++) {
            weekDays[i] = false;
        }
    }
    virtual ~WeekDaysRepeatData() = default;
    [[nodiscard]] auto getData(const uint32_t &weekDay) const -> bool
    {
        return weekDays[weekDay];
    };
    virtual void setData(const uint32_t &weekDay)
    {
        if (weekDays[weekDay]) {
            weekDays[weekDay] = false;
        }
        else {
            weekDays[weekDay] = true;
        }
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

enum PrevWindow
{
    DELETE = 0,
    EDTIT,
    ADD
};

class PrevWindowData : public gui::SwitchData
{
  protected:
    PrevWindow prevWindow;

  public:
    PrevWindowData()          = default;
    virtual ~PrevWindowData() = default;

    PrevWindow getData()
    {
        return prevWindow;
    };

    virtual void setData(PrevWindow data)
    {
        prevWindow = data;
    };
};
