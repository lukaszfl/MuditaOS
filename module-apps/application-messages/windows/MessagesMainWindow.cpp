/*
 * @file MessagesMainWindow.cpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 25 wrz 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#include <functional>
#include <memory>

#include "bsp/rtc/rtc.hpp"
#include "service-appmgr/ApplicationManager.hpp"

//#include "../ApplicationPhonebook.hpp"
#include "service-db/messages/DBMessage.hpp"
#include "i18/i18.hpp"

#include "Label.hpp"
#include "ListView.hpp"
#include "Margins.hpp"
#include "MessagesMainWindow.hpp"

#include "service-db/api/DBServiceAPI.hpp"

#include <log/log.hpp>

namespace gui {

MessagesMainWindow::MessagesMainWindow(app::Application *app) :
	AppWindow(app, "MainWindow"),
	threadsModel{ new ThreadsModel(app)}
{
    setSize(480, 600);
    buildInterface();

}

void MessagesMainWindow::rebuild() {
    destroyInterface();
    buildInterface();
}
void MessagesMainWindow::buildInterface() {

	AppWindow::buildInterface();

	list = new gui::ListView(this, 11, 105, 480-22, 600-105-50 );
	list->setMaxElements(3);
	list->setPageSize(3);
	list->setPenFocusWidth(0);
	list->setPenWidth(0);
	list->setProvider( threadsModel );

	bottomBar->setActive(BottomBar::Side::LEFT, true);
    bottomBar->setActive(BottomBar::Side::CENTER, true);
    bottomBar->setActive(BottomBar::Side::RIGHT, true);
    bottomBar->setText(BottomBar::Side::LEFT, utils::localize.get("common_options"));
    bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get("common_open"));
    bottomBar->setText(BottomBar::Side::RIGHT, utils::localize.get("common_back"));

    topBar->setActive(TopBar::Elements::TIME, true);

    title = new gui::Label(this, 0, 50, 480, 54);
    title->setFilled(false);
    title->setBorderColor( gui::ColorFullBlack );
    title->setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM );
    title->setMargins( Margins(0,0,0,18));
    title->setFont("gt_pressura_bold_24");
    title->setText(utils::localize.get("app_messages_title_main"));
    title->setAlignement(gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_BOTTOM));

    leftArrowImage  = new gui::Image( this, 30,62,0,0, "arrow_left" );
	rightArrowImage = new gui::Image( this, 480-30-13,62,0,0, "arrow_right" );
	newMessageImage = new gui::Image( this, 48,55,0,0, "cross" );
	searchImage     = new gui::Image( this, 480-48-26,55,0,0, "search" );
}
void MessagesMainWindow::destroyInterface() {
    AppWindow::destroyInterface();
    if( title ) { removeWidget(title);    delete title; title = nullptr; }
    if( list ) { removeWidget(list);    delete list; list = nullptr; }
    if( leftArrowImage ) { removeWidget(leftArrowImage);    delete leftArrowImage; leftArrowImage = nullptr; }
    if( rightArrowImage ) { removeWidget(rightArrowImage);    delete rightArrowImage; rightArrowImage = nullptr; }
    if( newMessageImage ) { removeWidget(newMessageImage);    delete newMessageImage; newMessageImage = nullptr; }
    if( searchImage ) { removeWidget(searchImage);    delete searchImage; searchImage = nullptr; }

    children.clear();
    delete threadsModel;
}

MessagesMainWindow::~MessagesMainWindow() {
	destroyInterface();
}


void MessagesMainWindow::onBeforeShow(ShowMode mode, uint32_t command, SwitchData *data) {

	uint32_t count = DBServiceAPI::ThreadGetCount( application );

	LOG_INFO("Number of SMS threads: %d", count);

	setFocusItem(list);

	threadsModel->clear();
	threadsModel->requestRecordsCount();

	list->clear();
	list->setElementsCount( threadsModel->getItemCount() );
}

bool MessagesMainWindow::onInput(const InputEvent &inputEvent) {
	//check if any of the lower inheritance onInput methods catch the event
	bool ret = AppWindow::onInput( inputEvent );
	if( ret ) {
		//refresh window only when key is other than enter
		if( inputEvent.keyCode != KeyCode::KEY_ENTER )
			application->render( RefreshModes::GUI_REFRESH_FAST );
		return true;
	}

	//process only if key is released
	if(( inputEvent.state != InputEvent::State::keyReleasedShort ) &&
	   (( inputEvent.state != InputEvent::State::keyReleasedLong )))
		return false;

    if( inputEvent.keyCode == KeyCode::KEY_LEFT) {
        LOG_INFO("Adding TEST SMS");

        SMSRecord sms;
        time_t timestamp;
        bsp::rtc_GetCurrentTimestamp(&timestamp );
        sms.date = timestamp;
        sms.dateSent = sms.date;
        std::string s = "+";
		for( uint32_t k=0; k<9; k++)
			s+=std::to_string(rand()%10);
        sms.number = s;
        sms.body = "this is short sms";
        sms.type = SMSType::INBOX;

        DBServiceAPI::SMSAdd( application, sms );

    }
    else
	if( inputEvent.keyCode == KeyCode::KEY_ENTER ) {
		LOG_INFO("Entering thread");
//		application->switchWindow("SearchWindow",0, nullptr );
	}
	else if( inputEvent.keyCode == KeyCode::KEY_RF ) {
		sapm::ApplicationManager::messageSwitchApplication( application, "ApplicationDesktop", "MenuWindow", nullptr );
		return true;
	}

	return false;

}

bool MessagesMainWindow::onDatabaseMessage( sys::Message* msgl ) {
	DBThreadResponseMessage* msg = reinterpret_cast<DBThreadResponseMessage*>( msgl );
	if( threadsModel->updateRecords( std::move(msg->records), msg->offset, msg->limit, msg->count) )
		return true;

	return false;
}

} /* namespace gui */
