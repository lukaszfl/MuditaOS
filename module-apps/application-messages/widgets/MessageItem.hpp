/*
 * @file MessageItem.hpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 27 wrz 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#ifndef MODULE_APPS_APPLICATION_MESSAGES_WIDGETS_MESSAGEITEM_HPP_
#define MODULE_APPS_APPLICATION_MESSAGES_WIDGETS_MESSAGEITEM_HPP_

#include "Label.hpp"
#include "ListItem.hpp"
#include "../MessagesModel.hpp"
#include "Application.hpp"

namespace gui {

/*
 * @brief Widget used to display information about thread in the threads list view.
 */
class MessageItem: public ListItem {

	MessagesModel* model = nullptr;
	app::Application* app = nullptr;
	//pointer to the message record
	std::shared_ptr<SMSRecord> messageRecord = nullptr;
	//this is hour in the mode defined in settings
	gui::Label* hour = nullptr;
	gui::Label* text = nullptr;
	//flag that defines if time should be displayed in 24h mode
	bool mode24H = false;

	public:
	MessageItem( MessagesModel* model, app::Application* app, bool mode24H );
		virtual ~MessageItem();
		//sets copy of alarm's
		void setMessage( std::shared_ptr<SMSRecord>& thread );

		//returns thread
		std::shared_ptr<ThreadRecord> getThread();
		//virtual methods from Item
		bool onDimensionChanged( const BoundingBox& oldDim, const BoundingBox& newDim) override;
		bool onActivated( void* data ) override;
};

} /* namespace gui */

#endif /* MODULE_APPS_APPLICATION_MESSAGES_WIDGETS_MESSAGEITEM_HPP_ */
