// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "AppWindow.hpp"
#include "MusicVolumeWindow.hpp"
#include "application-popup/ApplicationPopup.hpp"
#include <module-gui/gui/input/InputEvent.hpp>
#include <module-services/service-appmgr/service-appmgr/Controller.hpp>
#include <i18n/i18n.hpp>


namespace gui
{
    MusicVolumeWindow::MusicVolumeWindow(app::Application *app, const std::string &name) : VolumeWindow(app, name)
    {

        volumeText->setText(utils::localize.get(style::window::volume::music::title_key));
    }

    MusicVolumeWindow::~MusicVolumeWindow()
    {
        destroyInterface();
    }

    void MusicVolumeWindow::onBeforeShow(ShowMode mode, SwitchData *data)
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

    auto MusicVolumeWindow::onInput(const gui::InputEvent &inputEvent) -> bool
    {
        if (!inputEvent.isShortPress()) {
            return false;
        }

        if (inputEvent.keyCode == gui::KeyCode::KEY_VOLUP) {
            volumeBar->update(1);
            volumeWindowTimer->reload();
            if(!isMuted() && volumeText->getText() != utils::localize.get(style::window::volume::music::title_key))
            {
                volumeText->setText(utils::localize.get(style::window::volume::music::title_key));
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

} // namespace gui
