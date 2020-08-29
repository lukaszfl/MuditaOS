#pragma once

#include "windows/AppWindow.hpp"
#include "Application.hpp"
#include <gui/widgets/GridLayout.hpp>
#include <gui/widgets/Item.hpp>
#include <Text.hpp>
#include <module-utils/date/include/date/date.h>
#include <map>
#include <string>
#include <vector>

class MonthModel
{
  public:
    date::month month;
    unsigned int lastDay;
    // first week offset
    uint32_t firstWeekDayNumb;
    date::year year;

  public:
    MonthModel(date::year_month_day ymd);
    MonthModel()          = default;
    virtual ~MonthModel() = default;

    date::year getYear();
    date::month getMonth();
    uint32_t getLastDay();
    uint32_t getFirstWeekOffset();
    std::string getMonthYearText();
    std::string getMonthText();
    std::vector<std::string> split(const std::string &s, char delim);
    date::year_month_day parseDateFromDB(std::string dateDB);
    std::string parseDateToDB(date::year_month_day date);
};
