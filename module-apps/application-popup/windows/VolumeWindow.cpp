// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "VolumeWindow.hpp"
#include "application-popup/ApplicationPopup.hpp"
#include <module-gui/gui/input/InputEvent.hpp>
#include <module-services/service-appmgr/service-appmgr/Controller.hpp>
#include <i18n/i18n.hpp>


namespace gui
{
    VolumeWindow::VolumeWindow(app::Application *app, const std::string &name) : AppWindow(app, name)
    {

        buildInterface();

        volumeWindowTimer = std::make_unique<sys::Timer>(
                timer::name::volume_window, app, timer::duration::volume_window, sys::Timer::Type::SingleShot);
        volumeWindowTimer->connect([=](sys::Timer &) { windowTimerCallback(); });

    }

    void VolumeWindow::addVolumeText()
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

    void VolumeWindow::addVolumeBar()
    {
        volumeBar = new VBarGraph(this,
                                  style::window::default_left_margin,
                                  style::window::volume::bar::top_offset,
                                  style::window::volume::bar::volume_levels);

        volumeBar->setColors(ColorFullBlack,ColorFullBlack,ColorFullBlack,ColorFullWhite);
    }

    void VolumeWindow::buildInterface()
    {
        AppWindow::buildInterface();
        addVolumeText();
        addVolumeBar();
    }

    void VolumeWindow::rebuild()
    {}

    void VolumeWindow::destroyInterface()
    {
        destroyTimer();
        erase();
    }

    VolumeWindow::~VolumeWindow()
    {
        destroyInterface();
    }

    void VolumeWindow::onBeforeShow(ShowMode mode, SwitchData *data)
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

    auto VolumeWindow::isMuted() -> bool
    {
        return !volumeBar->getValue();
    }

    auto VolumeWindow::onInput(const gui::InputEvent &inputEvent) -> bool
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

        return AppWindow::onInput(inputEvent);
    }

    void VolumeWindow::onClose()
    {
        destroyTimer();
    }

    void VolumeWindow::windowTimerCallback()
    {
        closeWindow();
    }

    void VolumeWindow::startTimer()
    {
        volumeWindowTimer->connect([=](sys::Timer &) { windowTimerCallback(); });
        volumeWindowTimer->reload();
    }

    void VolumeWindow::destroyTimer()
    {
        volumeWindowTimer->stop();
    }

    auto VolumeWindow::closeWindow() -> bool
    {
        LOG_DEBUG("Switch to previous window");
        destroyTimer();
        return app::manager::Controller::sendAction(application, app::manager::actions::ClosePopup);
    }
} // namespace gui
