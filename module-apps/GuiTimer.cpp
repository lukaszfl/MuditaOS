#include "GuiTimer.hpp"
#include "Item.hpp"
#include "Service/Timer.hpp"
#include "log/log.hpp"
#include "module-apps/Application.hpp"

namespace app
{
    void GuiTimer::start()
    {
        sys::Timer::start();
    }

    void GuiTimer::stop()
    {
        sys::Timer::stop();
    }

    void GuiTimer::reset()
    {
        sys::Timer::start();
    }

    void GuiTimer::setInterval(gui::ms time)
    {
        sys::Timer::setInterval(time);
    }

    GuiTimer::GuiTimer(Application *parent) : GuiTimer("GUI", parent)
    {}

    GuiTimer::GuiTimer(const std::string &name, Application *parent)
        : GuiTimer(name, parent, sys::Timer::timeout_infinite)
    {}

    GuiTimer::GuiTimer(const std::string &name, Application *parent, gui::ms timeout)
        : sys::Timer(name, parent, timeout, sys::Timer::Type::SingleShot), sysapi(*this)
    {}

    void GuiTimer::Sysapi::connect(gui::Item *item)
    {
        if (item != nullptr) {
            parent.connect([item, this](sys::Timer &timer) { item->onTimer(parent); });
        }
    }
} // namespace app
