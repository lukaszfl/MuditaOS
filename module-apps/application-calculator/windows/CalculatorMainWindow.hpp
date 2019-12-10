/*
 * @file CallLogMainWindow.hpp
 * @author Aleksander Rudnik (aleksander.rudnik@mudita.com)
 * @date 19.09.2019
 * @brief Application Call Log Main Window
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#pragma once

#include <functional>
#include <gui/widgets/GridLayout.hpp>
#include <gui/widgets/Style.hpp>
#include <string>

#include "Application.hpp"
#include "windows/AppWindow.hpp"

namespace gui
{

    class CalculatorMainWindow : public AppWindow
    {
      private:
        gui::Label *numberLabel = nullptr;
        std::map<NavigationDirection, Label*> actionsNavs = {{NavigationDirection::UP, new Label(this, 0, 0, 80, 80, UTF8("?"))},
                                                             {NavigationDirection::LEFT, new Label(this, 0, 0, 80, 80, UTF8("-"))},
                                                             {NavigationDirection::RIGHT, new Label(this, 0, 0, 80, 80, UTF8("+"))},
                                                             {NavigationDirection::DOWN, new Label(this, 0, 0, 80, 80, UTF8("?"))}};

      public:
        CalculatorMainWindow(app::Application *app, std::string name = "MainWindow");
        virtual ~CalculatorMainWindow();

        // virtual methods
        bool onInput(const InputEvent &inputEvent) override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;

        void rebuild() override;
        void buildInterface() override;
        void destroyInterface() override;
        bool onDatabaseMessage(sys::Message *msg) override;
    };

} /* namespace gui */
