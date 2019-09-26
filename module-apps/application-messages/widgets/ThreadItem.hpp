/*
 * @file ThreadItem.hpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 26 wrz 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#ifndef MODULE_APPS_APPLICATION_MESSAGES_WIDGETS_THREADITEM_HPP_
#define MODULE_APPS_APPLICATION_MESSAGES_WIDGETS_THREADITEM_HPP_

#include "Label.hpp"
#include "ListItem.hpp"
#include "../ThreadsModel.hpp"

namespace gui {

/*
 * @brief Widget used to display information about thread in the threads list view.
 */
class ThreadItem: public ListItem {

	ThreadsModel* model = nullptr;
	//pointer to the thread's record
	std::shared_ptr<ThreadRecord> threadRecord = nullptr;
	//this is hour in the mode defined in settings
	gui::Label* hour = nullptr;
	gui::Label* title = nullptr;
	gui::Label* snippet = nullptr;
	//flag that defines if time should be displayed in 24h mode
	bool mode24H = false;

	public:
	ThreadItem( ThreadsModel* model, bool mode24H );
		virtual ~ThreadItem();
		//sets copy of alarm's
		void setThread( std::shared_ptr<ThreadRecord>& thread );

		//returns thread
		std::shared_ptr<ThreadRecord> getThread();
		//virtual methods from Item
		bool onDimensionChanged( const BoundingBox& oldDim, const BoundingBox& newDim) override;
		bool onActivated( void* data ) override;
};

} /* namespace gui */


#endif /* MODULE_APPS_APPLICATION_MESSAGES_WIDGETS_THREADITEM_HPP_ */
