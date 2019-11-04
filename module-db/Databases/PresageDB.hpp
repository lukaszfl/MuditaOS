/*
 * PresageDB.hpp
 *
 *  Created on: 31 paź 2019
 *      Author: kuba
 */

#ifndef MODULE_DB_DATABASES_PRESAGEDB_HPP_
#define MODULE_DB_DATABASES_PRESAGEDB_HPP_

#include "Database/Database.hpp"
#include "../Tables/PresageTable.hpp"

class PresageDB : public Database {
public:
    PresageDB();
    ~PresageDB();

    PresageTable presage;

    static const char* GetDBName(){return dbName;}

private:

    static const char *dbName;
};






#endif /* MODULE_DB_DATABASES_PRESAGEDB_HPP_ */
