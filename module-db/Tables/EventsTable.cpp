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
    // Prevent duplicates using ANDs:
    return db->execute("INSERT or IGNORE INTO events (title, date_from, date_till, reminder, repeat) "
                       "SELECT '%q', '%q','%q', %lu, %lu "
                       "WHERE NOT EXISTS "
                       "(SELECT 1 FROM events e "
                       "WHERE e.title='%q' "
                       "AND e.date_from='%q' "
                       "AND e.date_till='%q' "
                       "AND e.reminder=%lu "
                       "AND e.repeat=%lu );",
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat);
}

bool EventsTable::addDaily(EventsTableRow entry)
{
    return db->execute("INSERT or IGNORE INTO events (title, date_from, date_till, reminder, repeat) VALUES"
                       "('%q', datetime('%q',('+0 day')),datetime('%q',('+0 day')), %u, %u),"
                       "('%q', datetime('%q',('+1 day')),datetime('%q',('+1 day')), %u, %u),"
                       "('%q', datetime('%q',('+2 day')),datetime('%q',('+2 day')), %u, %u),"
                       "('%q', datetime('%q',('+3 day')),datetime('%q',('+3 day')), %u, %u),"
                       "('%q', datetime('%q',('+4 day')),datetime('%q',('+4 day')), %u, %u),"
                       "('%q', datetime('%q',('+5 day')),datetime('%q',('+5 day')), %u, %u),"
                       "('%q', datetime('%q',('+6 day')),datetime('%q',('+6 day')), %u, %u);",
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat);
}

bool EventsTable::addWeekly(EventsTableRow entry)
{
    LOG_DEBUG("ENTRY DATE FROM: %s", TimePointToString(entry.date_from).c_str());
    LOG_DEBUG("ENTRY DATE FROM: %s", TimePointToString(entry.date_till).c_str());
    return db->execute("INSERT or IGNORE INTO events (title, date_from, date_till, reminder, repeat) VALUES"
                       "('%q', datetime('%q',('+0 day')),datetime('%q',('+0 day')), %u, %u),"
                       "('%q', datetime('%q',('+7 day')),datetime('%q',('+7 day')), %u, %u),"
                       "('%q', datetime('%q',('+14 day')),datetime('%q',('+14 day')), %u, %u),"
                       "('%q', datetime('%q',('+21 day')),datetime('%q',('+21 day')), %u, %u);",
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat);
}

bool EventsTable::addTwoWeeks(EventsTableRow entry)
{
    return db->execute("INSERT or IGNORE INTO events (title, date_from, date_till, reminder, repeat) VALUES"
                       "('%q', datetime('%q',('+0 day')),datetime('%q',('+0 day')), %u, %u),"
                       "('%q', datetime('%q',('+14 day')),datetime('%q',('+14 day')), %u, %u),"
                       "('%q', datetime('%q',('+28 day')),datetime('%q',('+28 day')), %u, %u),"
                       "('%q', datetime('%q',('+42 day')),datetime('%q',('+42 day')), %u, %u);",
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat);
}

bool EventsTable::addMonth(EventsTableRow entry)
{
    return db->execute("INSERT or IGNORE INTO events (title, date_from, date_till, reminder, repeat) VALUES"
                       "('%q', datetime('%q',('+0 month')),datetime('%q',('+0 month')), %u, %u),"
                       "('%q', datetime('%q',('+1 month')),datetime('%q',('+1 month')), %u, %u),"
                       "('%q', datetime('%q',('+2 month')),datetime('%q',('+2 month')), %u, %u),"
                       "('%q', datetime('%q',('+3 month')),datetime('%q',('+3 month')), %u, %u),"
                       "('%q', datetime('%q',('+4 month')),datetime('%q',('+4 month')), %u, %u),"
                       "('%q', datetime('%q',('+5 month')),datetime('%q',('+5 month')), %u, %u),"
                       "('%q', datetime('%q',('+6 month')),datetime('%q',('+6 month')), %u, %u),"
                       "('%q', datetime('%q',('+7 month')),datetime('%q',('+7 month')), %u, %u),"
                       "('%q', datetime('%q',('+8 month')),datetime('%q',('+8 month')), %u, %u),"
                       "('%q', datetime('%q',('+9 month')),datetime('%q',('+9 month')), %u, %u),"
                       "('%q', datetime('%q',('+10 month')),datetime('%q',('+10 month')), %u, %u),"
                       "('%q', datetime('%q',('+11 month')),datetime('%q',('+11 month')), %u, %u);"
                       "('%q', datetime('%q',('+12 month')),datetime('%q',('+12 month')), %u, %u);",
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat);
}

bool EventsTable::addYear(EventsTableRow entry)
{
    return db->execute("INSERT or IGNORE INTO events (title, date_from, date_till, reminder, repeat) VALUES"
                       "('%q', datetime('%q',('+0 year')),datetime('%q',('+0 year')), %u, %u),"
                       "('%q', datetime('%q',('+1 year')),datetime('%q',('+1 year')), %u, %u),"
                       "('%q', datetime('%q',('+2 year')),datetime('%q',('+2 year')), %u, %u),"
                       "('%q', datetime('%q',('+3 year')),datetime('%q',('+3 year')), %u, %u),"
                       "('%q', datetime('%q',('+4 year')),datetime('%q',('+4 year')), %u, %u);",
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat);
}

bool EventsTable::addCustom(EventsTableRow entry, int weekDayOffset)
{
    return db->execute("INSERT or IGNORE INTO events (title, date_from, date_till, reminder, repeat) VALUES"
                       "('Repeated event', datetime('%q',('%i day')),datetime('%q',('%i day'), %u, %u);",
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       weekDayOffset,
                       TimePointToString(entry.date_till).c_str(),
                       weekDayOffset,
                       entry.reminder,
                       entry.repeat);
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
    return db->execute("UPDATE events SET title= '%q', date_from = '%q', date_till = '%q', reminder "
                       "= %u, repeat = %u WHERE _id = %u;",
                       entry.title.c_str(),
                       TimePointToString(entry.date_from).c_str(),
                       TimePointToString(entry.date_till).c_str(),
                       entry.reminder,
                       entry.repeat,
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
        (*retQuery)[0].getUInt32(),                              // ID
        (*retQuery)[1].getString(),                              // title
        TimePointFromString((*retQuery)[2].getString().c_str()), // date_from
        TimePointFromString((*retQuery)[3].getString().c_str()), // date_till
        (*retQuery)[4].getUInt32(),                              // reminder
        (*retQuery)[5].getUInt32()                               // repeat

    };
}

std::vector<EventsTableRow> EventsTable::selectByDatePeriod(TimePoint date_filter, TimePoint filter_till)
{
    /// TODO: Rework unit tests
    auto retQuery = db->query("SELECT * FROM events WHERE date_till > date('%q') AND date_till < date('%q');",
                              TimePointToString(date_filter).c_str(),
                              TimePointToString(filter_till).c_str());

    if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
        return std::vector<EventsTableRow>();
    }

    std::vector<EventsTableRow> ret;

    do {
        ret.push_back(EventsTableRow{
            (*retQuery)[0].getUInt32(),                              // ID
            (*retQuery)[1].getString(),                              // title
            TimePointFromString((*retQuery)[2].getString().c_str()), // date_from
            TimePointFromString((*retQuery)[3].getString().c_str()), // date_till
            (*retQuery)[4].getUInt32(),                              // reminder
            (*retQuery)[5].getUInt32()                               // repeat
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
            (*retQuery)[0].getUInt32(),                              // ID
            (*retQuery)[1].getString(),                              // title
            TimePointFromString((*retQuery)[2].getString().c_str()), // date_from
            TimePointFromString((*retQuery)[3].getString().c_str()), // date_till
            (*retQuery)[4].getUInt32(),                              // reminder
            (*retQuery)[5].getUInt32()                               // repeat

        });
    } while (retQuery->nextRow());

    return ret;
}

std::vector<EventsTableRow> EventsTable::getLimitOffsetByDate(uint32_t offset, uint32_t limit)
{

    auto retQuery = db->query("SELECT * from events ORDER BY datetime(date_from) LIMIT %u OFFSET %u;", limit, offset);

    if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
        return std::vector<EventsTableRow>();
    }

    std::vector<EventsTableRow> ret;

    do {
        ret.push_back(EventsTableRow{
            (*retQuery)[0].getUInt32(),                              // ID
            (*retQuery)[1].getString(),                              // title
            TimePointFromString((*retQuery)[2].getString().c_str()), // date_from
            TimePointFromString((*retQuery)[3].getString().c_str()), // date_till
            (*retQuery)[4].getUInt32(),                              // reminder
            (*retQuery)[5].getUInt32()                               // repeat
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
