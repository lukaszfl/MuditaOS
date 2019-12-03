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
#include <gui/widgets/Style.hpp>
#include <memory>

#include "service-appmgr/ApplicationManager.hpp"

#include "../ApplicationCalculator.hpp"
#include "../data/CalculatorAppStyle.hpp"

#include "i18/i18.hpp"

namespace gui
{

using namespace calculatorAppStyle;

CalculatorMainWindow::CalculatorMainWindow(app::Application *app, std::string name) : AppWindow(app, name)
{
    setSize(480, 600);
    AppWindow::buildInterface();

    this->setTitle(utils::localize.get("app_desktop_tools_calculator"));
    topBar->setActive(TopBar::Elements::SIGNAL, true);
    topBar->setActive(TopBar::Elements::BATTERY, true);
    topBar->setActive(TopBar::Elements::TIME, true);

    numberLabel = new gui::Label(this, numberLabel::x, numberLabel::y, numberLabel::w, numberLabel::h);
    numberLabel->setPenWidth(numberLabel::borderW);
    numberLabel->setFont(style::window::font::verybig);
    numberLabel->setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM);
    numberLabel->setAlignement(gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_TOP));
}

void CalculatorMainWindow::rebuild()
{
}
void CalculatorMainWindow::buildInterface()
{
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
