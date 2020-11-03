#include "EventsDB.hpp"

#include <module-vfs/vfs.hpp>

// const char *EventsDB::dbName = USER_PATH("events.db");

// std::string EventsDB::dbName = std::string("Data Source=") + USER_PATH("events.db") +
// std::string(";DateTimeKind=Utc");

// mlucki
// std::string EventsDB::dbName = USER_PATH("events.db");
std::string EventsDB::dbName = std::string("file:") + USER_PATH("events.db") + std::string("?DateTimeKind=utc");
// std::string EventsDB::dbName = std::string("file:") + USER_PATH("events.db") +
// std::string("?DateTimeKind=utc?BindDateTimeWithKind");

// std::string EventsDB::dbName = std::string("file:'") + USER_PATH("events.db") + std::string("'?DateTimeKind=Utc");

EventsDB::EventsDB() : Database(dbName.c_str()), events(this)
{
    if (events.create() == false) {
        return;
    }
    isInitialized_ = true;
}
