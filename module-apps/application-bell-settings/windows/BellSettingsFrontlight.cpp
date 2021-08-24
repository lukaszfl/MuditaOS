// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "application-bell-settings/ApplicationBellSettings.hpp"
#include "BellSettingsStyle.hpp"
#include "BellSettingsFrontlight.hpp"
#include "data/FinishedWindowMessageData.hpp"

#include <gui/input/InputEvent.hpp>
#include <apps-common/options/OptionStyle.hpp>

namespace gui
{
    BellSettingsFrontlightWindow::BellSettingsFrontlightWindow(
        app::Application *app,
        std::unique_ptr<app::bell_settings::FrontlightWindowContract::Presenter> &&windowPresenter,
        std::string name)
        : AppWindow(app, name), presenter{std::move(windowPresenter)}
    {
        presenter->attach(this);
        buildInterface();
    }

    void BellSettingsFrontlightWindow::rebuild()
    {
        erase();
        buildInterface();
    }

    void BellSettingsFrontlightWindow::buildInterface()
    {
        AppWindow::buildInterface();
        statusBar->setVisible(false);
        header->setTitleVisibility(false);
        bottomBar->setVisible(false);

        presenter->createData();

        sidelistview = new SideListView(
            this, 0U, 0U, this->getWidth(), this->getHeight(), presenter->getPagesProvider(), PageBarType::None);
        sidelistview->setEdges(RectangleEdge::None);

        sidelistview->rebuildList(listview::RebuildType::Full);

        presenter->loadData();

        setFocusItem(sidelistview);
    }

    bool BellSettingsFrontlightWindow::onInput(const gui::InputEvent &inputEvent)
    {
        if (sidelistview->onInput(inputEvent)) {
            return true;
        }
        if (inputEvent.isShortRelease(KeyCode::KEY_ENTER)) {
            presenter->saveData();

            auto finishedMessageData = std::make_unique<FinishedWindowMessageData>(
                utils::translate("app_bell_settings_frontlight_finished_message"));
            application->switchWindow(window::name::bellSettingsFinished, std::move(finishedMessageData));
            return true;
        }
        if (AppWindow::onInput(inputEvent)) {
            return true;
        }

        return AppWindow::onInput(inputEvent);
    }
} /* namespace gui */
