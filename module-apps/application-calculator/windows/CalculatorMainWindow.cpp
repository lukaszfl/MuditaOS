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

namespace gui
{
CalculatorMainWindow::CalculatorMainWindow( app::Application* app, std::string name ) : AppWindow( app, name ) {
    setSize(480,600);
    buildInterface();
}

void CalculatorMainWindow::rebuild()
{
    destroyInterface();
    buildInterface();
}
void CalculatorMainWindow::buildInterface()
{
    AppWindow::buildInterface();

    topBar->setActive(TopBar::Elements::TIME, true);
    this->setTitle(utils::localize.get("app_desktop_tools_calculator"));
    bottomBar->setText(BottomBar::Side::CENTER,
                       utils::localize.get("common_open"));
}
void CalculatorMainWindow::destroyInterface()
{
    AppWindow::destroyInterface();

    children.clear();
}

CalculatorMainWindow::~CalculatorMainWindow()
{
    destroyInterface();
}

void CalculatorMainWindow::onBeforeShow(ShowMode mode, SwitchData *data)
{
    this->setFocus(topBar);
}

bool CalculatorMainWindow::onInput(const InputEvent &inputEvent)
{
    return AppWindow::onInput(inputEvent);
}

bool CalculatorMainWindow::onDatabaseMessage(sys::Message *msgl)
{
    return false;
}

} /* namespace gui */
