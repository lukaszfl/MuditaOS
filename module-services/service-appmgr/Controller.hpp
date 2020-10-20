#pragma once

#include "ApplicationManager.hpp"

#include "module-sys/Service/Service.hpp"

namespace app::manager
{
    class Controller
    {
      public:
        Controller() = delete;

        static auto registerApplication(sys::Service *sender, bool status, bool startBackground) -> bool;
        static auto switchApplication(sys::Service *sender,
                                      const ApplicationHandle::Name &applicationName,
                                      const std::string &windowName,
                                      std::unique_ptr<gui::SwitchData> data = nullptr) -> bool;
        static auto switchBack(sys::Service *sender, std::unique_ptr<APMSwitchPrevApp> msg = nullptr) -> bool;
        static auto closeApplication(sys::Service *sender, const ApplicationHandle::Name &name) -> bool;
        static auto changeLanguage(sys::Service *sender, utils::Lang language) -> bool;
        static auto changePowerSaveMode(sys::Service *sender) -> bool;
        static auto stopApplicationManager(sys::Service *sender) -> bool;
        static auto preventBlockingDevice(sys::Service *sender) -> bool;
        static auto confirmSwitch(sys::Service *sender) -> bool;
        static auto confirmClose(sys::Service *sender) -> bool;
    };
} // namespace app::manager
