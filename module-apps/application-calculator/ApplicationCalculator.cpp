/*
 * @file ApplicationSesttings.cpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 8 lip 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#include "Application.hpp"

#include "MessageType.hpp"
#include "windows/CalculatorMainWindow.hpp"

#include "ApplicationCalculator.hpp"

namespace app {

ApplicationCalculator::ApplicationCalculator(std::string name, std::string parent, bool startBackgound) :
	Application( name, parent, startBackgound ) {
}

ApplicationCalculator::~ApplicationCalculator() {
}

// Invoked upon receiving data message
sys::Message_t ApplicationCalculator::DataReceivedHandler(sys::DataMessage* msgl,sys::ResponseMessage* resp) {

	auto retMsg = Application::DataReceivedHandler(msgl);
	//if message was handled by application's template there is no need to process further.
	if( (reinterpret_cast<sys::ResponseMessage*>( retMsg.get() )->retCode ==
		sys::ReturnCodes::Success ) ){
		return retMsg;
	}

	//this variable defines whether message was processed.
	bool handled = true;

	if( handled )
		return std::make_shared<sys::ResponseMessage>();
	else
		return std::make_shared<sys::ResponseMessage>(sys::ReturnCodes::Unresolved);
}

// Invoked during initialization
sys::ReturnCodes ApplicationCalculator::InitHandler() {

	auto ret = Application::InitHandler();
	if( ret != sys::ReturnCodes::Success )
		return ret;

	createUserInterface();

	setActiveWindow("MainWindow");

	return ret;
}

sys::ReturnCodes ApplicationCalculator::DeinitHandler() {
	return sys::ReturnCodes::Success;
}

void ApplicationCalculator::createUserInterface() {

	gui::AppWindow* window = new gui::CalculatorMainWindow(this, "MainWindow");
    windows.insert(
        std::pair<std::string, gui::AppWindow*>(window->getName(), window));
}

void ApplicationCalculator::destroyUserInterface() {
}

} /* namespace app */
