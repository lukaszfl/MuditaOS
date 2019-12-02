/*
 * @file CalculatorMainWindow.cpp
 * @author Mateusz Grzywacz
 * @date 02.12.2019
 * @brief Application Calculator Main Window
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#include "CalculatorMainWindow.hpp"
#include <functional>
#include <memory>

#include "service-appmgr/ApplicationManager.hpp"

#include "../ApplicationCalculator.hpp"

#include "i18/i18.hpp"


using namespace style;
using namespace CalculatorStyle;

namespace gui {

CalculatorMainWindow::CalculatorMainWindow( app::Application* app ) :
	AppWindow( app, Calculator::settings::MainWindowStr ), CalculatorModel{ new CalculatorModel( app ) } {

	buildInterface();
}

void CalculatorMainWindow::rebuild() {
	destroyInterface();
	buildInterface();
}
void CalculatorMainWindow::buildInterface() {
	AppWindow::buildInterface();

	topBar->setActive( TopBar::Elements::TIME, true );

	setFocusItem( list );
}
void CalculatorMainWindow::destroyInterface() {
	AppWindow::destroyInterface();

	if( list ) { removeWidget(list); delete list; list= nullptr; };

	children.clear();
	delete CalculatorModel;
}

CalculatorMainWindow::~CalculatorMainWindow() {
	destroyInterface();
}

void CalculatorMainWindow::onBeforeShow( ShowMode mode, SwitchData* data ) {
}

bool CalculatorMainWindow::onInput( const InputEvent& inputEvent ) {
	return AppWindow::onInput( inputEvent );
}

bool CalculatorMainWindow::onDatabaseMessage( sys::Message* msgl ) {
	return false;
}

} /* namespace gui */
