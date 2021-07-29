// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <application-music-player/presenters/SongsPresenter.hpp>

#include <AppWindow.hpp>
#include <Text.hpp>
#include <Image.hpp>

#include <vector>
#include <string>

namespace gui
{

    class MusicPlayerEmptyWindow : public AppWindow, public app::music_player::SongsContract::View
    {
        Image *img         = nullptr;
        Text *text         = nullptr;
        Image *placeHolder = nullptr;
        std::shared_ptr<app::music_player::SongsContract::Presenter> presenter;

      public:
        MusicPlayerEmptyWindow(app::Application *app, std::shared_ptr<app::music_player::SongsContract::Presenter> presenter);

        // virtual methods
        void onBeforeShow(ShowMode mode, SwitchData *data) override;

        void rebuild() override;
        void buildInterface() override;
        void destroyInterface() override;
        bool onDatabaseMessage(sys::Message *msg) override;
        bool onInput(const InputEvent &inputEvent) final;

        void updateSongsState() override;
        void refreshWindow() override;
    };

} /* namespace gui */
