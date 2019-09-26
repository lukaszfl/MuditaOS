/*
 * @file ThreadsModel.cpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 26 wrz 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#include "ThreadsModel.hpp"


#include "service-db/api/DBServiceAPI.hpp"
#include "widgets/ThreadItem.hpp"

#include "ThreadsModel.hpp"

ThreadsModel::ThreadsModel(  app::Application* app) : DatabaseModel(app, 3){

}

ThreadsModel::~ThreadsModel() {
}

void ThreadsModel::requestRecordsCount() {
	uint32_t start = xTaskGetTickCount();
	recordsCount = DBServiceAPI::ThreadGetCount(application);
	uint32_t stop = xTaskGetTickCount();
	LOG_INFO("DBServiceAPI::ThreadGetCount %d records %d ms", recordsCount, stop-start);

	//request first and second page if possible
	if( recordsCount > 0 ){
		DBServiceAPI::ThreadGetLimitOffset(application, 0, pageSize );
		if( recordsCount > pageSize ) {
			DBServiceAPI::ThreadGetLimitOffset(application, pageSize, pageSize );
		}
	}
}

void ThreadsModel::requestRecords( const uint32_t offset, const uint32_t limit ) {
	DBServiceAPI::ThreadGetLimitOffset(application, offset, limit );
}

bool ThreadsModel::updateRecords( std::unique_ptr<std::vector<ThreadRecord>> records, const uint32_t offset, const uint32_t limit, uint32_t count ) {

	LOG_INFO("Offset: %d, Limit: %d Count:%d", offset, limit, count);
//	for( uint32_t i=0; i<records.get()->size(); ++i ) {
//		LOG_INFO("id: %d, filename: %s", records.get()->operator [](i).ID, records.get()->operator [](i).path.c_str());
//	}

	DatabaseModel::updateRecords( std::move(records), offset, limit, count );

	return true;
}

gui::ListItem* ThreadsModel::getItem( int index, int firstElement, int prevElement, uint32_t count, int remaining, bool topDown ) {

	std::shared_ptr<ThreadRecord> threadRecord = getRecord( index );

	SettingsRecord& settings = application->getSettings();

	if( threadRecord == nullptr )
		return nullptr;

	gui::ThreadItem* item = new gui::ThreadItem(this, !settings.timeFormat12 );
	if( item != nullptr ) {
		item->setThread( threadRecord );
		item->setID( index );
		return item;
	}

	return nullptr;
}
int ThreadsModel::getItemCount() {
	return recordsCount;
}

