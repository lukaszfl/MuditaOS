#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

namespace gui
{
    class AppWindow;
}

namespace gui::window
{
    class Memento;
}

namespace app
{
    class WindowsStack
    {
        /// stack of visited windows in application
        std::vector<std::string> windowStack;
        /// mementos of destroyed winodows
        std::map<std::string, std::unique_ptr<gui::window::Memento>> memento;

        public:

        /// get to the first time we entered this &window
        bool popToWindow(const std::string &window);
        /// push window to the top of windows stack
        void pushWindow(const std::string &newWindow);
        /// getter for previous window name
        const std::string getPrevWindow() const;
        /// clears windows stack
        void cleanPrevWindw();
        /// getter to get window by name
        /// @note could be possibly used to implement building window on request
        gui::AppWindow *getWindow(const std::string &window);
        /// getter for current wisible window in application
        gui::AppWindow *getCurrentWindow();
    };
}
