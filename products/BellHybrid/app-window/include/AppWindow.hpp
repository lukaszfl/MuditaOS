// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <gui/widgets/StatusBar.hpp>
#include <gui/widgets/header/Header.hpp>
#include <gui/widgets/BottomBar.hpp>
#include <gui/widgets/Window.hpp>
#include <Service/Service.hpp>
#include <Service/Message.hpp>

namespace app
{
    class Application;
};

namespace gui
{
    namespace name
    {
        namespace window
        {
            inline constexpr auto main_window = "MainWindow";
            inline constexpr auto no_window   = "";
        } // namespace window
    }     // namespace name

    /*
     * @brief This is wrapper for gui window used within applications.
     */
    class AppWindow : public Window
    {
      protected:
        app::Application *application = nullptr;

      public:
        AppWindow() = delete;
        AppWindow(app::Application *app, std::string name);

        app::Application *getApplication()
        {
            return application;
        };

        std::string getUniqueName() override;

        virtual bool onDatabaseMessage(sys::Message *msg);
        virtual bool updateTime();

        void buildInterface() override;
        bool onInput(const InputEvent &inputEvent) override;
        void accept(GuiVisitor &visitor) override;
    };

} /* namespace gui */
