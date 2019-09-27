/*
 * @file ThreadViewWindow.cpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 25 wrz 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#include <functional>
#include <memory>

#include "service-appmgr/ApplicationManager.hpp"

#include "i18/i18.hpp"

#include "Label.hpp"
#include "ListView.hpp"
#include "Margins.hpp"

#include "service-db/messages/DBMessage.hpp"
#include "service-db/api/DBServiceAPI.hpp"

#include <log/log.hpp>

#include "../data/MessagesSwitchData.hpp"
#include "ThreadViewWindow.hpp"

namespace gui {

ThreadViewWindow::ThreadViewWindow(app::Application *app) :
	AppWindow(app, "ThreadViewWindow"),
	messagesModel{ nullptr }
{
    setSize(480, 600);
    buildInterface();

}

void ThreadViewWindow::rebuild() {
    destroyInterface();
    buildInterface();
}
void ThreadViewWindow::buildInterface() {

	AppWindow::buildInterface();

	messagesModel = new MessagesModel(application);

	list = new gui::ListView(this, 11, 105, 480-22, 600-105-50 );
	list->setMaxElements(7);
	list->setPageSize(7);
	list->setPenFocusWidth(0);
	list->setPenWidth(0);
	list->setProvider( messagesModel );

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
    title->setAlignement(gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_BOTTOM));
}
void ThreadViewWindow::destroyInterface() {
    AppWindow::destroyInterface();

    if( title ) { removeWidget(title);    delete title; title = nullptr; }
    if( list ) { removeWidget(list);    delete list; list = nullptr; }

    children.clear();
    delete messagesModel;
}

ThreadViewWindow::~ThreadViewWindow() {
	destroyInterface();
}

bool ThreadViewWindow::handleSwitchData( SwitchData* data ) {

	if( data == nullptr ) {
		LOG_ERROR("Received null pointer");
		return false;
	}

	app::MessagesSwitchData* msgData = reinterpret_cast<app::MessagesSwitchData*>(data);
	if( msgData->getType() == app::MessagesSwitchData::Type::PROVIDE_THREAD ) {
		app::ThreadSwitchData* threadData = reinterpret_cast<app::ThreadSwitchData*>( data );
		threadRecord = threadData->getThread();
		messagesModel->setThreadID( threadRecord->dbID );
		LOG_INFO("Thread: ID: %d msg count: %d", threadRecord->dbID, threadRecord->msgCount );
		UTF8 str = "Contact ID:" + std::to_string( threadRecord->contactID);
		title->setText( str );
	}

	return true;
}


void ThreadViewWindow::onBeforeShow(ShowMode mode, uint32_t command, SwitchData *data) {
	LOG_INFO("getting messages");

	uint32_t count = DBServiceAPI::SMSGetCount( application, threadRecord->dbID );

	LOG_INFO("Number of SMSs in thread: %d is %d", threadRecord->dbID, count);

	setFocusItem(list);

	messagesModel->setThreadID( threadRecord->dbID );
	messagesModel->clear();
	messagesModel->requestRecordsCount();

	list->clear();
	list->setElementsCount( messagesModel->getItemCount() );
}

bool ThreadViewWindow::onInput(const InputEvent &inputEvent) {
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

	if( inputEvent.keyCode == KeyCode::KEY_ENTER ) {
		LOG_INFO("Enter pressed");
	}
	else if( inputEvent.keyCode == KeyCode::KEY_RF ) {
		application->switchWindow( "MainWindow", 0, nullptr );
		return true;
	}

	return false;

}

bool ThreadViewWindow::onDatabaseMessage( sys::Message* msgl ) {
	DBSMSResponseMessage* msg = reinterpret_cast<DBSMSResponseMessage*>( msgl );
	if( messagesModel->updateRecords( std::move(msg->records), msg->offset, msg->limit, msg->count ) )
		return true;

	return false;
}

} /* namespace gui */
