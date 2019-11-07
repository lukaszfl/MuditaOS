/*
 * PresageRecord.cpp
 *
 *  Created on: 31 paź 2019
 *      Author: kuba
 */

#include "PresageRecord.hpp"
#include "PresageTable.hpp"

#include "log/log.hpp"

PresageRecordInterface::PresageRecordInterface(PresageDB* PresageDb): presageDB(PresageDb) {

}

PresageRecordInterface::~PresageRecordInterface() {

}

bool PresageRecordInterface::Add(const Ngram &rec) {

    uint32_t contactID = 0;

    // Create alarm
//    presageDB->presage.Add(Ngram{
//    		.time = rec.time,
//    		.snooze = rec.snooze,
//			.status = rec.status,
//			.path = rec.path
//    });

    //TODO: error check

    return true;
}

uint32_t PresageRecordInterface::GetCount() {
    return presageDB->presage.GetCount();
}

std::unique_ptr<std::vector<Ngram>> PresageRecordInterface::GetLimitOffsetByField(uint32_t offset, uint32_t limit,
                                                                                  PresageRecordField field,
                                                                                  const char *str) {
//    auto records = std::make_unique<std::vector<PresageRecord>>();
//
//    std::vector<PresageTableRow> alarm;
//
//    switch(field){
//        case PresageRecordField::Time:
//        	alarm = presageDB->presage.GetLimitOffsetByField(offset,limit,PresageTableFields::Time,str);
//            break;
//        case PresageRecordField::Snooze:
//        	alarm = presageDB->presage.GetLimitOffsetByField(offset,limit,PresageTableFields::Snooze,str);
//            break;
//        case PresageRecordField::Status:
//        	alarm = presageDB->presage.GetLimitOffsetByField(offset,limit,PresageTableFields::Status,str);
//        	break;
//        default:
//            return records;
//    }
//    return records;
}


std::unique_ptr<std::vector<Ngram>> PresageRecordInterface::GetLimitOffset(uint32_t offset, uint32_t limit) {
//    auto alarm = presageDB->presage.GetLimitOffset(offset,limit);
//
//    auto records = std::make_unique<std::vector<PresageRecord>>();
//
//    PresageRecordInterface presageInterface(presageDB);
//    for(const auto &w : alarm){
//
//        auto PresageRec = presageInterface.GetByID(w.ID);
//
//        records->push_back({
//           .ID=w.ID,
//           .time = w.time,
//		   .snooze = w.snooze,
//		   .status = w.status,
//		   .path = w.path,
//        });
//    }
//
//    return records;
}

bool PresageRecordInterface::Update(const Ngram &rec) {

//    auto alarm = presageDB->presage.GetByID(rec.ID);
//    if(alarm.ID == 0){
//        return false;
//    }
//
//    presageDB->presage.Update(PresageTableRow{
//        .ID=rec.ID,
//        .time = rec.time,
//		.snooze = rec.snooze,
//		.status = rec.status,
//		.path = rec.path
//    });
//
//    return true;
}

bool PresageRecordInterface::RemoveByID(uint32_t id) {

//    auto alarm = presageDB->presage.GetByID(id);
//    if(alarm.ID == 0){
//        return false;
//    }
//
//    // Remove alarm
//    if(presageDB->presage.RemoveByID(id) == false){
//        return false;
//    }
//
//    return true;
}

bool PresageRecordInterface::RemoveByField(PresageRecordField field, const char *str) {

    switch(field){
        case PresageRecordField ::Time:
            return presageDB->presage.RemoveByField(PresageTableFields::Time,str);

        default:
            return false;

    }

}

Ngram PresageRecordInterface::GetByID(uint32_t id) {
//    auto alarm = presageDB->presage.GetByID(id);
//
//    return PresageRecord{
//        .ID = alarm.ID,
//        .time = alarm.time,
//		.snooze = alarm.snooze,
//		.status = alarm.status,
//		.path = alarm.path
//    };

}

std::vector<Ngram> PresageRecordInterface::ExecuteQuery(std::string query)
{
	auto queryRet = presageDB->presage.ExecuteQuery(query);

	LOG_INFO("Query ret size: %d", queryRet.size());
	return queryRet;
}






