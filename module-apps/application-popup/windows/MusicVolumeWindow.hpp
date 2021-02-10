// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "module-apps/Application.hpp"
#include "VolumeWindow.hpp"
#include "module-apps/widgets/BarGraph.hpp"
#include <functional>

namespace style::window::volume
{
    namespace music
    {
        constexpr inline auto title_key          = "app_popup_music_volume_text";
    }

} // namespace style::window::volume

namespace gui
{
    class MusicVolumeWindow : public VolumeWindow
    {
    public:
        MusicVolumeWindow(app::Application *app, const std::string &name);
        ~MusicVolumeWindow() override;

        void onBeforeShow(ShowMode mode, SwitchData *data) override;
        auto onInput(const gui::InputEvent &inputEvent) -> bool override;
    };
}; // namespace gui
