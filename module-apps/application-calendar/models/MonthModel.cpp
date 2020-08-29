#include "MonthModel.hpp"
#include <time/time_locale.hpp>

MonthModel::MonthModel(date::year_month_day yearMonthDay)
{
    this->month                                = yearMonthDay.month();
    this->year                                 = yearMonthDay.year();
    date::year_month_day_last yearMonthDayLast = this->year / this->month / date::last;
    this->lastDay                              = static_cast<unsigned>(yearMonthDayLast.day());
    date::year_month_day yearMonthDayFirst     = this->year / this->month / 1;
    this->firstWeekDayNumb                     = date::weekday{yearMonthDayFirst}.c_encoding();
}

date::year MonthModel::getYear()
{
    return year;
}

date::month MonthModel::getMonth()
{
    return month;
}

uint32_t MonthModel::getLastDay()
{
    return this->lastDay;
}

uint32_t MonthModel::getFirstWeekOffset()
{
    if (this->firstWeekDayNumb == 0) {
        return 6;
    }
    else {
        return this->firstWeekDayNumb - 1;
    }
}

std::vector<std::string> MonthModel::split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

date::year_month_day MonthModel::parseDateFromDB(std::string dateDB)
{
    std::string yearStr = dateDB.substr(0, 4);
    LOG_DEBUG("DAY!!!!!!!!!!! : %s", yearStr.c_str());
    std::string monthStr = dateDB.substr(6, 2);
    LOG_DEBUG("MONTH!!!!!!!!!!! : %s", monthStr.c_str());
    std::string dayStr = dateDB.substr(9, 2);
    LOG_DEBUG("YEAR!!!!!!!!!!! : %s", dayStr.c_str());

    return date::year(atoi(yearStr.c_str())) / date::month(atoi(monthStr.c_str())) / date::day(atoi(dayStr.c_str()));
}

std::string MonthModel::parseDateToDB(date::year_month_day date)
{
    int yearUInt        = static_cast<decltype(yearUInt)>(date.year());
    std::string yearStr = std::to_string(yearUInt);

    unsigned int monthUInt = static_cast<unsigned>(date.month());
    std::string monthStr   = utils::time::Locale::get_month(utils::time::Locale::Month(monthUInt - 1));

    unsigned int dayUInt = static_cast<unsigned>(date.day());
    std::string dayStr   = std::to_string(dayUInt);

    return yearStr + "-" + monthStr + "-" + dayStr;
}

std::string MonthModel::getMonthText()
{
    unsigned int monthUInt = static_cast<unsigned>(month);
    std::string monthStr   = utils::time::Locale::get_month(utils::time::Locale::Month(monthUInt - 1));
    return monthStr;
}

std::string MonthModel::getMonthYearText()
{
    int yearUInt        = static_cast<decltype(yearUInt)>(year);
    std::string yearStr = std::to_string(yearUInt);
    std::string monthStr;
    unsigned int monthUInt = static_cast<unsigned>(month);
    monthStr               = utils::time::Locale::get_month(utils::time::Locale::Month(monthUInt - 1));

    return monthStr + " " + yearStr;
}
