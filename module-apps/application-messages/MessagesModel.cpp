/*
 * @file MessagesModel.cpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 26 wrz 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#include "MessagesModel.hpp"
#include "service-db/api/DBServiceAPI.hpp"
#include "widgets/MessageItem.hpp"

MessagesModel::MessagesModel(  app::Application* app) : DatabaseModel(app, 6){

}

MessagesModel::~MessagesModel() {
}

void MessagesModel::setThreadID( uint32_t id ) {
	threadID = id;
}

void MessagesModel::requestRecordsCount() {
	uint32_t start = xTaskGetTickCount();
	recordsCount = DBServiceAPI::SMSGetCount(application, threadID );
	uint32_t stop = xTaskGetTickCount();
	LOG_INFO("DBServiceAPI::MessagesGetCount %d records %d ms", recordsCount, stop-start);

	//request first and second page if possible
	if( recordsCount > 0 ){
		DBServiceAPI::SMSGetLimitOffsetByThreadID(application, 0, pageSize, threadID );
		if( recordsCount > pageSize ) {
			DBServiceAPI::SMSGetLimitOffsetByThreadID(application, pageSize, pageSize, threadID );
		}
	}
}

void MessagesModel::requestRecords( const uint32_t offset, const uint32_t limit ) {
	DBServiceAPI::SMSGetLimitOffsetByThreadID(application, offset, limit, threadID );
}

bool MessagesModel::updateRecords( std::unique_ptr<std::vector<SMSRecord>> records, const uint32_t offset, const uint32_t limit, uint32_t count ) {

	LOG_INFO("Offset: %d, Limit: %d Count:%d", offset, limit, count);
//	for( uint32_t i=0; i<records.get()->size(); ++i ) {
//		LOG_INFO("id: %d, filename: %s", records.get()->operator [](i).ID, records.get()->operator [](i).path.c_str());
//	}

	DatabaseModel::updateRecords( std::move(records), offset, limit, count );

	return true;
}

gui::ListItem* MessagesModel::getItem( int index, int firstElement, int prevElement, uint32_t count, int remaining, bool topDown ) {

	std::shared_ptr<SMSRecord> messageRecord = getRecord( index );

	SettingsRecord& settings = application->getSettings();

	if( messageRecord == nullptr )
		return nullptr;

	gui::MessageItem* item = new gui::MessageItem(this, application, !settings.timeFormat12 );
	if( item != nullptr ) {
		item->setMessage( messageRecord );
		item->setID( index );
		return item;
	}

	return nullptr;
}
int MessagesModel::getItemCount() {
	return recordsCount;
}

