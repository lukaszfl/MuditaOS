// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "UITestWindow.hpp"

#include <application-settings/windows/WindowNames.hpp>

#include <Font.hpp>
#include <i18n/i18n.hpp>

namespace gui
{

    UiTestWindow::UiTestWindow(app::ApplicationCommon *app) : AppWindow(app, "TEST_UI")
    {
        AppWindow::buildInterface();
        buildInterface();
        // prebuild
        navBar->setActive(nav_bar::Side::Center, true);
        navBar->setActive(nav_bar::Side::Right, true);
        navBar->setText(nav_bar::Side::Center, utils::translate(style::strings::common::select));
        navBar->setText(nav_bar::Side::Right, utils::translate(style::strings::common::back));
        setTitle("UI TEST");
        text = new gui::Text(this,
                             style::window::default_left_margin,
                             style::window::default_vertical_pos,
                             style::window::default_body_width,
                             300);
        text->setInputMode(new InputMode(
            {InputMode::ABC, InputMode::abc, InputMode::digit},
            [=](const UTF8 &text) { navBarTemporaryMode(text); },
            [=]() { navBarRestoreFromTemporaryMode(); },
            [=]() { selectSpecialCharacter(); }));
        text->setEditMode(EditMode::Edit);
        text->setFont(style::window::font::medium);

        text->addText("Test TEST test xtEsT 02414128312");

        text->setInputMode(new InputMode(
            {InputMode::phone},
            [=](const UTF8 &text) { navBarTemporaryMode(text); },
            [=]() { navBarRestoreFromTemporaryMode(); },
            [=]() { selectSpecialCharacter(); }));

        text->addText("Test TEST test xtEsT 02414128312");

        text->setPenFocusWidth(0);
        setFocusItem(text);
    }
} // namespace gui
