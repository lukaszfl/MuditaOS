// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "MusicPlayerAllSongsWindow.hpp"
#include <application-music-player/ApplicationMusicPlayer.hpp>
#include <data/MusicPlayerStyle.hpp>

#include <Style.hpp>
#include <cassert>
#include <i18n/i18n.hpp>
#include <service-audio/AudioServiceAPI.hpp>
#include <gui/widgets/ListView.hpp>

namespace gui
{

    MusicPlayerAllSongsWindow::MusicPlayerAllSongsWindow(
        app::Application *app, std::shared_ptr<app::music_player::SongsContract::Presenter> windowPresenter)
        : AppWindow(app, gui::name::window::all_songs_window), presenter{windowPresenter}
    {
        presenter->attach(this);
        buildInterface();
    }

    void MusicPlayerAllSongsWindow::rebuild()
    {
        destroyInterface();
        buildInterface();
    }

    void MusicPlayerAllSongsWindow::buildInterface()
    {
        AppWindow::buildInterface();

        setTitle(utils::translate("app_music_player_music_library_camel"));

        bottomBar->setText(BottomBar::Side::CENTER, utils::translate("app_music_player_play"));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::translate(style::strings::common::back));

        songsList = new gui::ListView(this,
                                      musicPlayerStyle::allSongsWindow::x,
                                      musicPlayerStyle::allSongsWindow::y,
                                      musicPlayerStyle::allSongsWindow::w,
                                      musicPlayerStyle::allSongsWindow::h,
                                      presenter->getMusicPlayerItemProvider(),
                                      listview::ScrollBarType::Fixed);

        setFocusItem(songsList);
    }

    void MusicPlayerAllSongsWindow::destroyInterface()
    {
        erase();
    }

    void MusicPlayerAllSongsWindow::onBeforeShow([[maybe_unused]] ShowMode mode, [[maybe_unused]] SwitchData *data)
    {
        presenter->attach(this);
        auto index = presenter->getMusicPlayerItemProvider()->getCurrentIndex();

        songsList->rebuildList(listview::RebuildType::OnPageElement, index == static_cast<size_t>(-1) ? 0 : index);
    }

    void MusicPlayerAllSongsWindow::updateSongsState(bool needRefresh) 
    {
        songsList->rebuildList(gui::listview::RebuildType::InPlace);

        if(needRefresh)
        {
            application->refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
        }
    }

} /* namespace gui */
