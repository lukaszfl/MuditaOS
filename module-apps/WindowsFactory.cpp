#include "WindowsFactory.hpp"
#include <AppWindow.hpp>

namespace app
{

    WindowsFactory::WindowsFactory(Application *owner) : owner(owner)
    {
        setDefault(gui::name::window::main_window);
    }

    void WindowsFactory::attach(const std::string &name, builder builder)
    {
        builders[name] = builder;
    }

    std::map<std::string, WindowsFactory::handle>::const_iterator WindowsFactory::begin() const
    {
        return std::begin(windows);
    }

    std::map<std::string, WindowsFactory::handle>::const_iterator WindowsFactory::end() const
    {
        return std::end(windows);
    }

    std::map<std::string, WindowsFactory::handle>::const_iterator WindowsFactory::get(const std::string &name) const
    {
        return windows.find(name);
    }

    auto WindowsFactory::isRegistered(const std::string &name) const -> bool
    {
        return get(name) != end();
    }

    void WindowsFactory::setDefault(const std::string &name)
    {
        default_window = name;
    }

    /// TODO MIXED LOGIC HERE?
    /// oly reason for owner here!
    auto WindowsFactory::getDefault() -> handle &
    {
        auto ret = windows.find(default_window);
        if (ret == windows.end()) {
            build(owner, default_window);
        }
        return windows[default_window];
    }

    /// TODO MIXED LOGIC HERE?
    /// TODO rename to rebuild ? (build or rebuild? / lazy rebuild?)
    auto WindowsFactory::build(Application *app, const std::string &name) -> bool
    {
        windows[name] = builders[name](app, name);
        return windows[name] == nullptr;
    }
} // namespace app
