#pragma once

#include "Table.hpp"
#include "Record.hpp"
#include <Database/Database.hpp>
#include <Common/Common.hpp>
#include <utf8/UTF8.hpp>

struct EventsTableRow : public Record
{
    std::string title;
    std::string description;
    uint32_t date_from = 0;
    uint32_t date_till = 0;
    uint32_t reminder  = 0;
    uint32_t repeat    = 0;
    uint32_t time_zone = 0;
};

enum CalendarEventType
{
    unknown,
    reminder,
    event
};

struct EventsTypedTableRow : public EventsTableRow
{
    uint32_t date_selected = 0;
    CalendarEventType type = unknown;
    static CalendarEventType typeFromUint(uint32_t iType)
    {
        switch (iType) {
        case 1:
            return CalendarEventType::reminder;
        case 2:
            return CalendarEventType::event;
        default:
            return CalendarEventType::unknown;
        }
    };
};

enum class EventsTableFields
{
    date_from,
    date_till
};

class EventsTable : public Table<EventsTableRow, EventsTableFields>
{
  public:
    EventsTable(Database *db);
    ~EventsTable() override = default;

    bool create() override final;
    bool add(EventsTableRow entry) override final;
    bool removeById(uint32_t id) override final;
    bool removeByField(EventsTableFields field, const char *str) override final;
    bool update(EventsTableRow entry) override final;
    EventsTableRow getById(uint32_t id) override final;
    std::vector<EventsTableRow> selectByDatePeriod(uint32_t date_from, uint32_t date_till);
    uint32_t count() override final;
    uint32_t countByFieldId(const char *field, uint32_t id) override final;
    std::vector<EventsTableRow> getLimitOffset(uint32_t offset, uint32_t limit) override final;
    std::vector<EventsTableRow> getLimitOffsetByField(uint32_t offset,
                                                      uint32_t limit,
                                                      EventsTableFields field,
                                                      const char *str) override final;

    EventsTypedTableRow getClosestInsideDay(uint32_t start_date, uint32_t day_date);

  private:
    const char *createTableQuery = "CREATE TABLE IF NOT EXISTS events("
                                   "_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                   "title TEXT,"
                                   "description TEXT,"
                                   "date_from INTEGER,"
                                   "date_till INTEGER,"
                                   "reminder INTEGER,"
                                   "repeat INTEGER,"
                                   "time_zone INTEGER);";
};
