/*
 * PresageTable.hpp
 *
 *  Created on: 31 paź 2019
 *      Author: kuba
 */

#ifndef MODULE_DB_TABLES_PRESAGETABLE_HPP_
#define MODULE_DB_TABLES_PRESAGETABLE_HPP_


#include "Table.hpp"
#include "Database/Database.hpp"
#include "utf8/UTF8.hpp"
#include "Common/Common.hpp"

typedef std::vector<std::string> Ngram;
typedef std::vector<Ngram> NgramTable;

//struct PresageTableRow{
//    uint32_t ID;
//    uint32_t time;
//    uint32_t snooze;
//    uint32_t status;
//    UTF8    path;
//};

enum class PresageTableFields{
    Time,
    Snooze,
    Status,
	Path
};

class PresageTable : public Table<Ngram,PresageTableFields> {
public:

	PresageTable(Database* db);
    virtual ~PresageTable();

    bool Create() override final;
    bool Add(Ngram entry) override final;
    bool RemoveByID(uint32_t id) override final;
    bool RemoveByField(PresageTableFields field, const char* str) override final;
    bool Update(Ngram entry) override final;
    Ngram GetByID(uint32_t id) override final;
    std::vector<Ngram> GetLimitOffset(uint32_t offset,uint32_t limit) override final;
    std::vector<Ngram> GetLimitOffsetByField(uint32_t offset,uint32_t limit,PresageTableFields field,const char* str) override final;

    uint32_t GetCount() override final;
    uint32_t GetCountByFieldID(const char* field,uint32_t id) override final;

    NgramTable ExecuteQuery(std::string query);

private:

    const char* createTableQuery =
            "CREATE TABLE IF NOT EXISTS alarms("
            "_id INTEGER PRIMARY KEY,"
            "time INTEGER,"
            "snooze INTEGER,"
            "status INTEGER,"
            "path TEXT DEFAULT '');";
   // "INSERT or ignore INTO alarms (_id, time, snooze, status, path ) VALUES (1,0,0,0,'');"
    const char* alarmsInitialization = "INSERT or ignore INTO alarms (_id, time, snooze, status, path ) VALUES (1,0,0,0,'');";
};



#endif /* MODULE_DB_TABLES_PRESAGETABLE_HPP_ */
