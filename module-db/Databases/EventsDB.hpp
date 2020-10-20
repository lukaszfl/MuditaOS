#pragma once

#include "Database/Database.hpp"
#include "module-db/Tables/EventsTable.hpp"

class EventsDB : public Database
{
  public:
    EventsDB();
    ~EventsDB() override = default;

    EventsTable events;

    static const char *GetDBName()
    {
        return dbName.c_str();
    }

  private:
    static std::string dbName;
    // static const char *dbName;
};
