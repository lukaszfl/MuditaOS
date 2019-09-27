/*
 * @file MessageItem.cpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 27 wrz 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#include <memory>
#include "../data/MessagesSwitchData.hpp"
#include "MessageItem.hpp"

namespace gui {

MessageItem::MessageItem( MessagesModel* model, app::Application* app, bool mode24H) : model{model}, app{app}, mode24H{mode24H} {
	minWidth = 436;
	minHeight = 100;
	maxWidth = 436;
	maxHeight = 100;

	setRadius( 8 );

	setPenFocusWidth(3);
	setPenWidth(1);

	hour = new gui::Label( this, 0,0,0,0);
	hour->setPenFocusWidth(0);
	hour->setPenWidth(0);
	hour->setFont("gt_pressura_regular_24");
	hour->setAlignement(gui::Alignment { gui::Alignment::ALIGN_HORIZONTAL_RIGHT, gui::Alignment::ALIGN_VERTICAL_TOP } );

	text = new gui::Label( this, 0,0,0,0);
	text->setPenFocusWidth(0);
	text->setPenWidth(0);
	text->setFont("gt_pressura_regular_16");
	text->setAlignement(gui::Alignment { gui::Alignment::ALIGN_HORIZONTAL_LEFT, gui::Alignment::ALIGN_VERTICAL_CENTER} );

}

MessageItem::~MessageItem() {
	messageRecord = nullptr;
}

bool MessageItem::onDimensionChanged( const BoundingBox& oldDim, const BoundingBox& newDim) {
	hour->setPosition(11, 0 );
	hour->setSize(newDim.w-22, 40 );

	text->setPosition(11, 40 );
	text->setSize(newDim.w-22, newDim.h-40 );
	return true;
}

//sets copy of alarm's
void MessageItem::setMessage( std::shared_ptr<SMSRecord>& messageRecord ) {
	this->messageRecord = messageRecord;
	//TODO set values of the labels
	text->setText( messageRecord->body );
}

bool MessageItem::onActivated( void* data ) {
	return false;
}


} /* namespace gui */
