/*
 * @file ThreadsModel.hpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 26 wrz 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#ifndef MODULE_APPS_APPLICATION_MESSAGES_THREADSMODEL_HPP_
#define MODULE_APPS_APPLICATION_MESSAGES_THREADSMODEL_HPP_

#include <vector>

#include "Interface/ThreadRecord.hpp"
#include "DatabaseModel.hpp"
#include "NotesRecord.hpp"
#include "Application.hpp"
#include "ListItemProvider.hpp"

class ThreadsModel: public gui::ListItemProvider, public app::DatabaseModel<ThreadRecord> {
	/**
	 * Number of favourite records in the database.
	 */
	uint32_t favouriteCount = 0;
public:
	ThreadsModel( app::Application* app );
	virtual ~ThreadsModel();

	//virtual methods
	void requestRecordsCount() override;
	bool updateRecords( std::unique_ptr<std::vector<ThreadRecord>> records, const uint32_t offset, const uint32_t limit, uint32_t count ) override;
	void requestRecords( const uint32_t offset, const uint32_t limit ) override;

	//virtual methods for ListViewProvider
	gui::ListItem* getItem( int index, int firstElement, int prevElement, uint32_t count, int remaining, bool topDown ) override;
	int getItemCount();
};

#endif /* MODULE_APPS_APPLICATION_MESSAGES_THREADSMODEL_HPP_ */
