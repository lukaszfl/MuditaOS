/*
 * PresageDB.cpp
 *
 *  Created on: 31 paź 2019
 *      Author: kuba
 */

#include "PresageDB.hpp"


const char *PresageDB::dbName = "sys/Presage.db";

PresageDB::PresageDB() :
        Database(dbName),
        presage(this){

        if(presage.Create() == false) return;


        isInitialized = true;

}

PresageDB::~PresageDB() {

}



