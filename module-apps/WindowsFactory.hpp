#pragma once

#include <memory>
#include <functional>
#include <map>
#include <string>

namespace app
{
    class Application;
};

namespace gui
{
    class AppWindow;
}

namespace app
{
    class WindowsFactory
    {
      public:
        using handle  = std::unique_ptr<gui::AppWindow>;
        using builder = std::function<handle(Application *, std::string)>;

        WindowsFactory() = delete;
        WindowsFactory(Application *owner);
        WindowsFactory(const WindowsFactory &) = delete;
        WindowsFactory(WindowsFactory &&)      = delete;
        void operator=(WindowsFactory) = delete;

      private:
        std::string default_window;
        Application *owner = nullptr;
        /// Map containing application current windows
        /// Right now all application windows are being created on application start in createUserInterface
        /// then all windows are removed at the end of application
        /// @note right now there is no mechanism to postphone window creation
        std::map<std::string, handle> windows;
        std::map<std::string, builder> builders;

      public:
        void attach(const std::string &name, builder builder);
        [[nodiscard]] std::map<std::string, handle>::const_iterator begin() const;
        [[nodiscard]] std::map<std::string, handle>::const_iterator end() const;
        [[nodiscard]] std::map<std::string, handle>::const_iterator get(const std::string &name) const;
        [[nodiscard]] auto isRegistered(const std::string &name) const -> bool;
        auto build(Application *app, const std::string &name) -> bool;
        void setDefault(const std::string &name);
        [[nodiscard]] auto getDefault() -> handle &;
    };
} // namespace app
