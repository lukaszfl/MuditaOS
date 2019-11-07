/*
 * PresageRecord.hpp
 *
 *  Created on: 31 paź 2019
 *      Author: kuba
 */

#ifndef MODULE_DB_INTERFACE_PRESAGERECORD_HPP_
#define MODULE_DB_INTERFACE_PRESAGERECORD_HPP_

#include "Record.hpp"
#include <stdint.h>
#include "../Databases/PresageDB.hpp"
#include "utf8/UTF8.hpp"
#include "../Common/Common.hpp"

typedef std::vector<std::string> Ngram;
typedef std::vector<Ngram> NgramTable;
//
//struct PresageRecord{
//	uint32_t ID;
//	uint32_t time;
//	uint32_t snooze;
//	uint32_t status;
//	UTF8    path;
//
//};

enum class PresageRecordField{
    Time,
    Snooze,
    Status,
	Path
};

class PresageRecordInterface : public RecordInterface<Ngram,PresageRecordField > {
public:

    PresageRecordInterface(PresageDB* presageDb);
    ~PresageRecordInterface();

    bool Add(const Ngram& rec) override final;
    bool RemoveByID(uint32_t id) override final;
    bool RemoveByField(PresageRecordField field,const char* str) override final;
    bool Update(const Ngram& rec) override final;
    Ngram GetByID(uint32_t id) override final;

    uint32_t GetCount() override final;

    std::unique_ptr<std::vector<Ngram>> GetLimitOffset(uint32_t offset,uint32_t limit) override final;

    std::unique_ptr<std::vector<Ngram>> GetLimitOffsetByField(uint32_t offset,uint32_t limit,PresageRecordField field, const char* str) override final;


    std::vector<Ngram> ExecuteQuery(std::string query);

private:
    PresageDB* presageDB;
};





#endif /* MODULE_DB_INTERFACE_PRESAGERECORD_HPP_ */
