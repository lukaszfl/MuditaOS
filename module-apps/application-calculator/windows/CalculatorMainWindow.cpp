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
#include <gui/widgets/GridLayout.hpp>
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
    buildInterface();
}

void CalculatorMainWindow::rebuild()
{
}
void CalculatorMainWindow::buildInterface()
{
    setSize(480, 600);
    AppWindow::buildInterface();

    this->setTitle(utils::localize.get("app_desktop_tools_calculator"));

    bottomBar->setText(BottomBar::Side::LEFT, utils::localize.get("common_options"));
    //    bottomBar->setText(BottomBar::Side::LEFT, utils::localize.get("common"));
    bottomBar->setText(BottomBar::Side::RIGHT, utils::localize.get("common_back"));

    topBar->setActive(TopBar::Elements::SIGNAL, true);
    topBar->setActive(TopBar::Elements::BATTERY, true);
    topBar->setActive(TopBar::Elements::TIME, true);

    numberLabel = new gui::Label(this, numberLabel::x, numberLabel::y, numberLabel::w, numberLabel::h);
    numberLabel->setPenWidth(numberLabel::borderW);
    numberLabel->setFont(style::window::font::verybig);
    numberLabel->setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM);
    numberLabel->setAlignement(gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_TOP));

    for (auto actionNav : actionsNavs)
    {
        auto dir = actionNav.first;
        auto label = actionNav.second;
        LOG_DEBUG(label->getText().c_str());
        style::window::decorate(label);
        label->setAlignement(gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_CENTER));
        label->setFont(style::window::font::medium);
        label->setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM | RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM);
        uint16_t center[] = {(style::window_width - 80) / 2, (style::window_height - 80) / 2 + 50 };
        switch (dir)
        {
        case NavigationDirection::UP:
            label->setPosition(center[0], center[1] + 90);
            break;
        case NavigationDirection::LEFT:
            label->setPosition(center[0] - 90, center[1]);
            break;
        case NavigationDirection::RIGHT:
            label->setPosition(center[0] + 90, center[1]);
            break;
        case NavigationDirection::DOWN:
            label->setPosition(center[0], center[1] - 90);
            label->setNavigationItem(NavigationDirection::UP, label);
            label->setFocus(true);
            break;
        }
    }
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
