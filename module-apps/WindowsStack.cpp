#include "WindowsStack.hpp"

namespace app 
{
    bool WindowsStack::popToWindow(const std::string &window)
    {
        if (window == gui::name::window::no_window) {
            bool ret = false;
            if (windowStack.size() <= 1) {
                windowStack.clear();
                ret = true;
            }
            return ret;
        }

        auto ret = std::find(windowStack.begin(), windowStack.end(), window);
        if (ret != windowStack.end()) {
            LOG_INFO(
                "Pop last window(s) [%d] :  %s", static_cast<int>(std::distance(ret, windowStack.end())), ret->c_str());
            windowStack.erase(std::next(ret), windowStack.end());
            return true;
        }
        return false;
    }

    void WindowsStack::pushWindow(const std::string &newWindow)
    {
        // handle if window was already on
        if (popToWindow(newWindow)) {
            return;
        }
        else {
            windowStack.push_back(newWindow);
        }
#if DEBUG_APPLICATION_MANAGEMENT == 1
        LOG_DEBUG("[%d] newWindow: %s", windowStack.size(), newWindow.c_str());
        for (auto &el : windowStack) {
            LOG_DEBUG("-> %s", el.c_str());
        }
        LOG_INFO("\n\n");
#endif
    };

    /// TODO HERE MIXED LOGIC AND FUNCTIONALITY!
    /// return nullptr and handle in application.cpp
    const std::string WindowsStack::getPrevWindow() const
    {
        if (this->windowStack.size() <= 1) {
            return gui::name::window::no_window;
        }
        return *std::prev(windowStack.end(), 2);
    }

    void WindowsStack::cleanPrevWindw()
    {
        this->windowStack.clear();
    }

    /// TODO HERE MIXED LOGIC AND FUNCTIONALITY!
    /// return nullptr and handle in application.cpp
    gui::AppWindow *WindowsStack::getCurrentWindow()
    {
        std::string name = "";
        if (windowStack.size() == 0) {
            return windowsFactory.getDefault().get();
        }
        return windowsFactory.get(windowStack.back())->second.get();
    }
}
