/*
 * PresageTable.cpp
 *
 *  Created on: 31 paź 2019
 *      Author: kuba
 */


#include "PresageTable.hpp"


PresageTable::PresageTable(Database *db) : Table(db) {
}

PresageTable::~PresageTable() {

}

bool PresageTable::Create() {
	bool ret = true;
	ret = db->Execute(createTableQuery);

    if (!ret) {
           return false;
       }

    ret = db->Execute(alarmsInitialization);
    return ret;
}

bool PresageTable::Add(PresageTableRow entry) {
    return db->Execute(
            "INSERT or ignore INTO alarms ( time, snooze, status, path ) VALUES (%lu,%lu,%lu,'%s');",
            entry.time,
			entry.snooze,
			entry.status,
			entry.path.c_str()
    );
}

bool PresageTable::RemoveByID(uint32_t id) {
    return db->Execute("DELETE FROM alarms where _id = %u;", id);
}

bool PresageTable::RemoveByField(PresageTableFields field, const char *str) {
    std::string fieldName;

    switch(field){
        case PresageTableFields ::Time:
            fieldName = "time";
            break;

        case PresageTableFields ::Snooze:
            fieldName = "snooze";
            break;

        case PresageTableFields ::Status:
            fieldName = "status";
            break;
        default:
            return false;
    }

    return db->Execute("DELETE FROM alarms where %s = '%s';", fieldName.c_str(),str);

}


bool PresageTable::Update(PresageTableRow entry) {
    return db->Execute(
            "UPDATE alarms SET time = %lu, snooze = %lu ,status = %lu, path = '%s' WHERE _id=%lu;",
            entry.time,
			entry.snooze,
			entry.status,
			entry.path.c_str(),
            entry.ID
    );
}

PresageTableRow PresageTable::GetByID(uint32_t id) {
    auto retQuery = db->Query("SELECT * FROM alarms WHERE _id= %u;", id);

    if ((retQuery == nullptr) || (retQuery->GetRowCount() == 0)) {
        return PresageTableRow();
    }

    return PresageTableRow{(*retQuery)[0].GetUInt32(),  // ID
                       (*retQuery)[1].GetUInt32(),    // time
                       (*retQuery)[2].GetUInt32(),    // snooze
                       (*retQuery)[3].GetUInt32(),    // status
                       (*retQuery)[4].GetString(),    // path
    };
}

std::vector<PresageTableRow> PresageTable::GetLimitOffset(uint32_t offset, uint32_t limit) {
    auto retQuery = db->Query("SELECT * from alarms ORDER BY time ASC LIMIT %lu OFFSET %lu;",
                              limit,
                              offset);

    if ((retQuery == nullptr) || (retQuery->GetRowCount() == 0)) {
        return std::vector<PresageTableRow>();
    }

    std::vector<PresageTableRow> ret;

    do {
        ret.push_back(PresageTableRow{(*retQuery)[0].GetUInt32(),  // ID
            							(*retQuery)[1].GetUInt32(),    // time
										(*retQuery)[2].GetUInt32(),    // snooze
										(*retQuery)[3].GetUInt32(),    // status
										(*retQuery)[4].GetString(),    // path
        });
    } while (retQuery->NextRow());

    return ret;
}

std::vector<PresageTableRow>
PresageTable::GetLimitOffsetByField(uint32_t offset, uint32_t limit, PresageTableFields field, const char *str) {

    std::string fieldName;
    switch (field) {
        case PresageTableFields::Time:
            fieldName = "time";
            break;
        case PresageTableFields ::Snooze:
            fieldName = "snooze";
            break;
        case PresageTableFields ::Status:
            fieldName = "status";
            break;
        default:
            return std::vector<PresageTableRow>();
    }

    auto retQuery = db->Query("SELECT * from alarms WHERE %s='%s' ORDER BY time LIMIT %lu OFFSET %lu;",
                              fieldName.c_str(),
                              str,
                              limit,
                              offset);

    if ((retQuery == nullptr) || (retQuery->GetRowCount() == 0)) {
        return std::vector<PresageTableRow>();
    }

    std::vector<PresageTableRow> ret;

    do {
    	 ret.push_back(PresageTableRow{(*retQuery)[0].GetUInt32(),  // ID
										(*retQuery)[1].GetUInt32(),    // time
										(*retQuery)[2].GetUInt32(),    // snooze
										(*retQuery)[3].GetUInt32(),    // status
										(*retQuery)[4].GetString(),    // path
    	        });
    } while (retQuery->NextRow());

    return ret;
}

uint32_t PresageTable::GetCount() {
    auto queryRet = db->Query("SELECT COUNT(*) FROM alarms;");

    if (queryRet->GetRowCount() == 0) {
        return 0;
    }

    return uint32_t{(*queryRet)[0].GetUInt32()};
}

uint32_t PresageTable::GetCountByFieldID(const char *field, uint32_t id) {
    auto queryRet = db->Query("SELECT COUNT(*) FROM alarms WHERE %s=%lu;", field, id);

    if ((queryRet == nullptr) || (queryRet->GetRowCount() == 0)) {
        return 0;
    }

    return uint32_t{(*queryRet)[0].GetUInt32()};
}

PresageTableRow PresageTable::GetNext(time_t time)
{
	auto retQuery = db->Query("SELECT * from alarms WHERE status=1 AND time>=%u ORDER BY time ASC LIMIT 1;", time );

	if ((retQuery == nullptr) || (retQuery->GetRowCount() == 0)) {
		return PresageTableRow();
	}

	return PresageTableRow{(*retQuery)[0].GetUInt32(),  // ID
					   (*retQuery)[1].GetUInt32(),    // time
					   (*retQuery)[2].GetUInt32(),    // snooze
					   (*retQuery)[3].GetUInt32(),    // status
					   (*retQuery)[4].GetString(),    // path
	};
}

