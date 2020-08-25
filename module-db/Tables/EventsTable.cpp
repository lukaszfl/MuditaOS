#include "EventsTable.hpp"
#include "module-db/Interface/EventsRecord.hpp"

#include <log/log.hpp>
#include <Utils.hpp>

#include <cassert>

EventsTable::EventsTable(Database *db) : Table(db)
{}

bool EventsTable::create()
{
    if (!db->execute(createTableQuery)) {
        return false;
    }

    return true;
}

bool EventsTable::add(EventsTableRow entry)
{
    return db->execute(
        "INSERT or IGNORE INTO events (title, description, date_from, date_till, reminder, repeat, time_zone) "
        "VALUES ('%q','%q', %lu, %lu, %lu, %lu, %lu);",
        entry.title.c_str(),
        entry.description.c_str(),
        entry.date_from,
        entry.date_till,
        entry.reminder,
        entry.repeat,
        entry.time_zone);
}

bool EventsTable::removeById(uint32_t id)
{
    return db->execute("DELETE FROM events where _id = %u;", id);
}

bool EventsTable::removeByField(EventsTableFields field, const char *str)
{
    std::string fieldName;

    switch (field) {
    case EventsTableFields::date_from: {
        fieldName = "date_from";
        break;
    }
    case EventsTableFields::date_till: {
        fieldName = "date_till";
        break;
    }
    }

    return db->execute("DELETE FROM events where %q = '%q';", fieldName.c_str(), str);
}

bool EventsTable::update(EventsTableRow entry)
{
    return db->execute("UPDATE events SET title= '%q', description = '%q', date_from = %u, date_till = %u, reminder "
                       "= %u, repeat = %u, time_zone = %u WHERE _id = %u;",
                       entry.title.c_str(),
                       entry.description.c_str(),
                       entry.date_from,
                       entry.date_till,
                       entry.reminder,
                       entry.repeat,
                       entry.time_zone,
                       entry.ID);
}

EventsTableRow EventsTable::getById(uint32_t id)
{
    auto retQuery = db->query("SELECT * FROM events WHERE _id= %u;", id);

    if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
        return EventsTableRow();
    }

    assert(retQuery->getRowCount() == 1);

    return EventsTableRow{
        (*retQuery)[0].getUInt32(), // ID
        (*retQuery)[1].getString(), // title
        (*retQuery)[2].getString(), // description
        (*retQuery)[3].getUInt32(), // date_from
        (*retQuery)[4].getUInt32(), // date_till
        (*retQuery)[5].getUInt32(), // reminder
        (*retQuery)[6].getUInt32(), // repeat
        (*retQuery)[7].getUInt32()  // time_zone

    };
}

std::vector<EventsTableRow> EventsTable::selectByDatePeriod(uint32_t date_from, uint32_t date_till)
{
    auto retQuery = db->query("SELECT * FROM events WHERE date_from >= %u AND date_till <= %u;", date_from, date_till);

    if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
        return std::vector<EventsTableRow>();
    }

    std::vector<EventsTableRow> ret;

    do {
        ret.push_back(EventsTableRow{
            (*retQuery)[0].getUInt32(), // ID
            (*retQuery)[1].getString(), // title
            (*retQuery)[2].getString(), // description
            (*retQuery)[3].getUInt32(), // date_from
            (*retQuery)[4].getUInt32(), // date_till
            (*retQuery)[5].getUInt32(), // reminder
            (*retQuery)[6].getUInt32(), // repeat
            (*retQuery)[7].getUInt32()  // time_zone
        });
    } while (retQuery->nextRow());

    return ret;
}

std::vector<EventsTableRow> EventsTable::getLimitOffset(uint32_t offset, uint32_t limit)
{
    auto retQuery = db->query("SELECT * from events LIMIT %lu OFFSET %lu;", limit, offset);
    //"SELECT * FROM event WHERE id = "

    if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
        return std::vector<EventsTableRow>();
    }

    std::vector<EventsTableRow> ret;

    do {
        ret.push_back(EventsTableRow{
            (*retQuery)[0].getUInt32(), // ID
            (*retQuery)[1].getString(), // title
            (*retQuery)[2].getString(), // description
            (*retQuery)[3].getUInt32(), // date_from
            (*retQuery)[4].getUInt32(), // date_till
            (*retQuery)[5].getUInt32(), // reminder
            (*retQuery)[6].getUInt32(), // repeat
            (*retQuery)[7].getUInt32()  // time_zone
        });
    } while (retQuery->nextRow());

    return ret;
}

std::vector<EventsTableRow> EventsTable::getLimitOffsetByField(uint32_t offset,
                                                               uint32_t limit,
                                                               EventsTableFields field,
                                                               const char *str)
{
    std::vector<EventsTableRow> ret;

    assert(0 && "Not implemented");

    return ret;
}

uint32_t EventsTable::count()
{
    auto queryRet = db->query("SELECT COUNT(*) FROM events;");

    if (queryRet == nullptr || queryRet->getRowCount() == 0) {
        return 0;
    }

    return (*queryRet)[0].getUInt32();
}

uint32_t EventsTable::countByFieldId(const char *field, uint32_t id)
{
    assert(0 && "Not implemented");

    return 0;
}

// mlucki
EventsTypedTableRow EventsTable::getClosestInsideDay(uint32_t start_date, uint32_t day_date)
{
    /*
    select 'event' as type, date_from as my_date,* from (select * from events where date_from >= 141020142 and
    event_fired = -1 ORDER BY date_from LIMIT 1) UNION ALL select 'reminder' as type, reminder as my_date,* from (select
    * from events where reminder >= 141020142 and reminder_fired = -1 ORDER BY reminder LIMIT 1) order by my_date  LIMIT
    1
     */
    auto retQuery = db->query("SELECT 2 AS type, date_from AS date_selected,* FROM "
                              "(SELECT * from events WHERE date_from >= %u AND date_from < %u AND event_fired = -1 "
                              "ORDER BY date_from LIMIT 1) "
                              "UNION ALL "
                              "SELECT 1 as type, reminder AS date_selected,* FROM "
                              "(SELECT * from events WHERE reminder >= %u AND reminder < %u AND reminder_fired = -1 "
                              "ORDER BY reminder LIMIT 1) "
                              "ORDER BY found_date LIMIT 1",
                              start_date,
                              day_date,
                              start_date,
                              day_date);

    // mlucki
    // I tu zaczynają się małe schody:
    //  - może być tak, że event jest ustawiony na jutro ale reminder (jako liczba minut przed eventem) trafia w dziś
    //      trzeba to opanować w zapytaniu
    //  - albo pytać bez filtra na "day_date" a tu w kodzie decydować czy zwrócona data jest z dzisiaj!!!

    if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
        return EventsTypedTableRow();
    }

    EventsTypedTableRow ret;

    return EventsTypedTableRow{
        (*retQuery)[2].getUInt32(), // ID
        (*retQuery)[3].getString(), // title
        (*retQuery)[4].getString(), // description
        (*retQuery)[5].getUInt32(), // date_from
        (*retQuery)[6].getUInt32(), // date_till
        (*retQuery)[7].getUInt32(), // reminder
        (*retQuery)[8].getUInt32(), // repeat
        (*retQuery)[9].getUInt32(), // time_zone
        (*retQuery)[1].getUInt32(), // date_selected
        EventsTypedTableRow::typeFromUint((*retQuery)[0].getUInt32()),
    };
}
