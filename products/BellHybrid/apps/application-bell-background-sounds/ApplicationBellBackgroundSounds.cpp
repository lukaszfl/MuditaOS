// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ApplicationBellBackgroundSounds.hpp"
#include "models/BGSoundsRepository.hpp"
#include "presenter/BGSoundsMainWindowPresenter.hpp"
#include "presenter/BGSoundsTimerSelectPresenter.hpp"
#include "presenter/BGSoundsProgressPresenter.hpp"
#include "presenter/BGSoundsVolumePresenter.hpp"
#include "windows/BGSoundsMainWindow.hpp"
#include "windows/BGSoundsPausedWindow.hpp"
#include "windows/BGSoundsProgressWindow.hpp"
#include "windows/BGSoundsTimerSelectWindow.hpp"
#include "windows/BGSoundsVolumeWindow.hpp"
#include "widgets/BGSoundsPlayer.hpp"
#include <apps-common/messages/AppMessage.hpp>
#include <common/models/TimeModel.hpp>
#include <common/models/AudioModel.hpp>
#include <audio/AudioMessage.hpp>

#include <log/log.hpp>
namespace app
{
    ApplicationBellBackgroundSounds::ApplicationBellBackgroundSounds(std::string name,
                                                                     std::string parent,
                                                                     StatusIndicators statusIndicators,
                                                                     StartInBackground startInBackground,
                                                                     uint32_t stackDepth)
        : Application(std::move(name), std::move(parent), statusIndicators, startInBackground, stackDepth),
          audioModel{std::make_unique<AudioModel>(this)}, player{
                                                              std::make_unique<bgSounds::BGSoundsPlayer>(*audioModel)}
    {
        bus.channels.push_back(sys::BusChannel::ServiceAudioNotifications);
        connect(typeid(service::AudioEOFNotification), [&](sys::Message *msg) -> sys::MessagePointer {
            auto notification = static_cast<service::AudioEOFNotification *>(msg);
            return player->handle(notification);
        });
    }
    ApplicationBellBackgroundSounds::~ApplicationBellBackgroundSounds() = default;

    sys::ReturnCodes ApplicationBellBackgroundSounds::InitHandler()
    {
        auto ret = Application::InitHandler();
        if (ret != sys::ReturnCodes::Success) {
            return ret;
        }
        createUserInterface();

        return sys::ReturnCodes::Success;
    }

    void ApplicationBellBackgroundSounds::createUserInterface()
    {
        windowsFactory.attach(gui::name::window::main_window, [this](ApplicationCommon *app, const std::string &name) {
            auto tagsFetcher      = std::make_unique<bgSounds::BGSoundsTagsFetcher>(app);
            auto soundsRepository = std::make_shared<bgSounds::BGSoundsRepository>(std::move(tagsFetcher));
            auto presenter = std::make_unique<bgSounds::BGSoundsMainWindowPresenter>(std::move(soundsRepository));
            return std::make_unique<gui::BGSoundsMainWindow>(app, std::move(presenter));
        });
        windowsFactory.attach(
            gui::window::name::bgSoundsTimerSelect, [this](ApplicationCommon *app, const std::string &name) {
                auto presenter = std::make_unique<bgSounds::BGSoundsTimerSelectPresenter>(settings.get());
                return std::make_unique<gui::BGSoundsTimerSelectWindow>(app, std::move(presenter));
            });
        windowsFactory.attach(gui::window::name::bgSoundsProgress,
                              [this](ApplicationCommon *app, const std::string &name) {
                                  auto timeModel = std::make_unique<app::TimeModel>();
                                  auto presenter = std::make_unique<bgSounds::BGSoundsProgressPresenter>(
                                      settings.get(), *player, std::move(timeModel));
                                  return std::make_unique<gui::BGSoundsProgressWindow>(app, std::move(presenter));
                              });
        windowsFactory.attach(gui::window::name::bgSoundsPaused, [](ApplicationCommon *app, const std::string &name) {
            return std::make_unique<gui::BGSoundsPausedWindow>(app);
        });

        windowsFactory.attach(gui::popup::window::volume_window,
                              [this](ApplicationCommon *app, const std::string &name) {
                                  auto presenter = std::make_unique<bgSounds::BGSoundsVolumePresenter>(*audioModel);
                                  return std::make_unique<gui::BGSoundsVolumeWindow>(app, std::move(presenter));
                              });

        attachPopups({gui::popup::ID::AlarmActivated,
                      gui::popup::ID::AlarmDeactivated,
                      gui::popup::ID::PowerOff,
                      gui::popup::ID::Reboot,
                      gui::popup::ID::BedtimeNotification});
    }

    sys::MessagePointer ApplicationBellBackgroundSounds::DataReceivedHandler(sys::DataMessage *msgl,
                                                                             sys::ResponseMessage *resp)
    {
        auto retMsg = Application::DataReceivedHandler(msgl);
        if (auto response = dynamic_cast<sys::ResponseMessage *>(retMsg.get());
            response != nullptr && response->retCode == sys::ReturnCodes::Success) {
            return retMsg;
        }

        return handleAsyncResponse(resp);
    }

    sys::MessagePointer ApplicationBellBackgroundSounds::handleSwitchWindow(sys::Message *msgl)
    {
        if (auto msg = dynamic_cast<AppSwitchWindowMessage *>(msgl); msg) {
            const auto newWindowName = msg->getWindowName();
            if (newWindowName == gui::window::name::bgSoundsProgress ||
                newWindowName == gui::window::name::bgSoundsPaused) {
                stopIdleTimer();
            }
            else {
                startIdleTimer();
            }
        }
        return ApplicationCommon::handleSwitchWindow(msgl);
    }
} // namespace app
