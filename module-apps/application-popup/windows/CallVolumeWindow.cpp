// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "CallVolumeWindow.hpp"
#include "application-popup/ApplicationPopup.hpp"
#include <module-gui/gui/input/InputEvent.hpp>
#include <module-services/service-appmgr/service-appmgr/Controller.hpp>
#include <i18n/i18n.hpp>


namespace gui
{
    CallVolumeWindow::CallVolumeWindow(app::Application *app, const std::string &name) : AppWindow(app, name)
    {

        buildInterface();

        volumeWindowTimer = std::make_unique<sys::Timer>(
                timer::name::music_volume_window, app, timer::duration::music_volume_window, sys::Timer::Type::SingleShot);
        volumeWindowTimer->connect([=](sys::Timer &) { windowTimerCallback(); });

    }

    void CallVolumeWindow::addVolumeText()
    {
        volumeText = new Label(this,
                               style::window::default_left_margin,
                               title->offset_h(),
                               style::window::default_body_width,
                               style::window::volume::title_height,
                               utils::localize.get(style::window::volume::title_key));

        volumeText->setPenWidth(style::window::default_border_no_focus_w);
        volumeText->setFont(style::window::font::mediumbold);
        volumeText->setAlignment(gui::Alignment(gui::Alignment::Vertical::Center));
        addWidget(volumeText);
    }

    void CallVolumeWindow::addVolumeBar()
    {
        volumeBar = new VBarGraph(this,
                                  style::window::default_left_margin,
                                  style::window::volume::bar::top_offset,
                                  style::window::volume::bar::volume_levels);
    }

    void CallVolumeWindow::buildInterface()
    {
        AppWindow::buildInterface();
        addVolumeText();
        addVolumeBar();
    }

    void CallVolumeWindow::rebuild()
    {}

    void CallVolumeWindow::destroyInterface()
    {
        destroyTimer();
        erase();
    }

    CallVolumeWindow::~CallVolumeWindow()
    {
        destroyInterface();
    }

    void CallVolumeWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        audio::Volume volume;
        const auto ret = application->getCurrentVolume(volume);
        if (ret == audio::RetCode::Success) {
            volumeBar->setValue(volume);
        }
        else {
            LOG_ERROR("Current volume not recieved");
        }

        if(isMuted())
        {
            volumeText->setText(utils::localize.get(style::window::volume::muted_title_key));
        }

        startTimer();
    }

    auto CallVolumeWindow::isMuted() -> bool
    {
        return !volumeBar->getValue();
    }

    auto CallVolumeWindow::onInput(const gui::InputEvent &inputEvent) -> bool
    {
        if (!inputEvent.isShortPress()) {
            return false;
        }

        if (inputEvent.keyCode == gui::KeyCode::KEY_VOLUP) {
            volumeBar->update(1);
            volumeWindowTimer->reload();
            if(!isMuted() && volumeText->getText() != utils::localize.get(style::window::volume::title_key))
            {
                volumeText->setText(utils::localize.get(style::window::volume::title_key));
            }
        }

        if (inputEvent.keyCode == gui::KeyCode::KEY_VOLDN) {
            volumeBar->update(-1);
            volumeWindowTimer->reload();
            if(isMuted())
            {
                volumeText->setText(utils::localize.get(style::window::volume::muted_title_key));
            }
        }

        if (inputEvent.keyCode == gui::KeyCode::KEY_RF) {
            return app::manager::Controller::sendAction(application, app::manager::actions::ClosePopup);
        }


        return AppWindow::onInput(inputEvent);
    }

    void CallVolumeWindow::onClose()
    {
        destroyTimer();
    }

    void CallVolumeWindow::windowTimerCallback()
    {
        closeWindow();
    }

    void CallVolumeWindow::startTimer()
    {
        volumeWindowTimer->connect([=](sys::Timer &) { windowTimerCallback(); });
        volumeWindowTimer->reload();
    }

    void CallVolumeWindow::destroyTimer()
    {
        volumeWindowTimer->stop();
    }

    auto CallVolumeWindow::closeWindow() -> bool
    {
        LOG_DEBUG("Switch to previous window");
        destroyTimer();
        return app::manager::Controller::sendAction(application, app::manager::actions::ClosePopup);
    }
} // namespace gui
