// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <AppWindow.hpp>
#include "Application.hpp"
#include "InputEvent.hpp"
#include "StatusBar.hpp"
#include "status-bar/Time.hpp"
#include <Style.hpp>
#include <i18n/i18n.hpp>
#include <service-appmgr/Controller.hpp>
#include <service-audio/AudioServiceAPI.hpp>

using namespace style::header;

namespace gui
{

    AppWindow::AppWindow(app::Application *app, std::string name) : Window(name), application{app}
    {
        setSize(style::window_width, style::window_height);
    }

    void AppWindow::buildInterface()
    {}

    bool AppWindow::updateTime()
    {
        return true;
    }

    bool AppWindow::onDatabaseMessage(sys::Message *msg)
    {
        return false;
    }

    bool AppWindow::onInput(const InputEvent &inputEvent)
    {
        // check if any of the lower inheritance onInput methods catch the event
        if (Window::onInput(inputEvent)) {
            return true;
        }

        if (inputEvent.isLongRelease(gui::KeyCode::KEY_RF)) {
            LOG_INFO("exit to main menu");
            app::manager::Controller::sendAction(application, app::manager::actions::Home);
        }

        if (inputEvent.isLongRelease(gui::KeyCode::KEY_PND)) {
            LOG_INFO("Locking phone");

            application->getPhoneLockSubject().lock();
        }

        if ((inputEvent.isShortRelease())) {
            switch (inputEvent.getKeyCode()) {
            case KeyCode::HEADSET_VOLUP:
                [[fallthrough]];
            case KeyCode::KEY_VOLUP: {
                return application->increaseCurrentVolume();
            }
            case KeyCode::HEADSET_VOLDN:
                [[fallthrough]];
            case KeyCode::KEY_VOLDN: {
                return application->decreaseCurrentVolume();
            }
            case KeyCode::KEY_RF: {
                application->returnToPreviousWindow();
                return true;
            }
            default:
                break;
            }
        }

        if (inputEvent.is(KeyCode::KEY_TORCH)) {
            if (inputEvent.isLongRelease()) {
                application->toggleTorchOnOff();
                return true;
            }
            else if (inputEvent.isShortRelease()) {
                application->toggleTorchColor();
                return true;
            }
        }

        return false;
    }

    void AppWindow::accept(GuiVisitor &visitor)
    {
        visitor.visit(*this);
    }

    std::string AppWindow::getUniqueName()
    {
        constexpr auto separator = "/";
        return application->GetName() + separator + getName();
    }

} /* namespace gui */
