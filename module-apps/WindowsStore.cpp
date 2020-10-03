#include "WindowsStore.hpp"
#include <AppWindow.hpp>

namespace app
{

    WindowsStore::WindowsStore(Application *owner) : owner(owner)
    {
        setDefault(gui::name::window::main_window);
    }

    void WindowsStore::attach(const std::string &name, builder builder)
    {
        builders[name] = builder;
    }

    auto WindowsStore::begin() const
    {
        return std::begin(windows);
    }

    auto WindowsStore::end() const
    {
        return std::end(windows);
    }

    auto WindowsStore::get(const std::string &name) const
    {
        return windows.find(name);
    }

    auto WindowsStore::isRegistered(const std::string &name) const -> bool
    {
        return get(name) != end();
    }

    void WindowsStore::setDefault(const std::string &name)
    {
        default_window = name;
    }

    auto WindowsStore::getDefault() -> handle &
    {
        auto ret = windows.find(default_window);
        if (ret == windows.end()) {
            build(owner, default_window);
        }
        return windows[default_window];
    }

    auto WindowsStore::build(Application *app, const std::string &name) -> bool
    {
        windows[name] = builders[name](app, name);
        return windows[name] == nullptr;
    }
} // namespace app
