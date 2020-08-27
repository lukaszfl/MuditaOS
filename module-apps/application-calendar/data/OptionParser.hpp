#pragma once

#include "CalendarData.hpp"

class OptionParser
{
    // std::vector<bool> weekDayOptions;
  public:
    OptionParser() = default;
    std::unique_ptr<WeekDaysRepeatData> setWeekDayOptions(const uint32_t &dataDB,
                                                          std::unique_ptr<WeekDaysRepeatData> weekDayOptions)
    {
        uint32_t startBit        = 16;
        uint32_t numberOfOptions = 7;
        for (uint32_t i = startBit; i < startBit + numberOfOptions; i++) {
            if (dataDB & (1 << i)) {
                LOG_DEBUG("SET OPTION ARRAY!!!%u", i);
                weekDayOptions->setData(i - startBit);
            }
        }
        return weekDayOptions;
    }

    uint32_t getDatabaseFieldValue(std::unique_ptr<WeekDaysRepeatData> weekDayOptions)
    {
        uint32_t dataDB          = 0;
        uint32_t startBit        = 16;
        uint32_t numberOfOptions = 7;
        for (uint32_t i = startBit; i < startBit + numberOfOptions; i++) {
            if (weekDayOptions->getData(i - startBit)) {
                LOG_DEBUG("GET DB VALUE!!!");
                dataDB += (1 << i);
            }
        }
        return dataDB;
    }

    uint32_t getNoCustomValue(uint32_t dataDB)
    {
        /// Set all bits in custom field to zero
        /// DataDB input is not modifyed
        uint32_t startBit        = 16;
        uint32_t numberOfOptions = 7;
        for (uint32_t i = startBit; i < startBit + numberOfOptions; i++) {
            if (dataDB & (1 << i)) {
                dataDB -= (1 << i);
            }
        }
        LOG_DEBUG("DATA DB: %u", dataDB);
        return dataDB;
    }
};