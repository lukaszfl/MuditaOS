// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "PowerNapProgressWindow.hpp"
#include "application-bell-powernap/ApplicationBellPowerNap.hpp"
#include "data/PowerNapData.hpp"
#include <apps-common/widgets/BellBaseLayout.hpp>
#include <Utils.hpp>
#include <Style.hpp>

namespace gui
{
    PowerNapProgressWindow::PowerNapProgressWindow(
        app::Application *app, std::shared_ptr<app::powernap::PowerNapProgressContract::Presenter> presenter)
        : AppWindow(app, gui::window::name::powernapProgress), presenter{std::move(presenter)}
    {
        this->presenter->attach(this);
        buildInterface();
    }

    void PowerNapProgressWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        if (auto napData = dynamic_cast<PowerNapData *>(data); napData != nullptr) {}
    }

    void PowerNapProgressWindow::buildInterface()
    {
        AppWindow::buildInterface();

        body            = new gui::BellBaseLayout(this, 0, 0, style::bell_base_layout::w, style::bell_base_layout::h);
        auto topMessage = new Label(body->firstBox);
        topMessage->setMaximumSize(style::bell_base_layout::w, style::bell_base_layout::outer_layouts_h);
        topMessage->setFont(style::window::font::medium);
        topMessage->setEdges(gui::RectangleEdge::All);
        topMessage->activeItem = false;
        topMessage->setAlignment(Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
        topMessage->setText(utils::translate("app_bellmain_power_nap"));

        body->firstBox->setAlignment(Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
        body->firstBox->setEdges(gui::RectangleEdge::All);
        body->centerBox->setEdges(gui::RectangleEdge::All);
        // body->centerBox->setAlignment(Alignment(gui::Alignment::Horizontal::Center,
        // gui::Alignment::Vertical::Center)); auto m2 = new Label(body->centerBox);
        // m2->setMaximumSize(style::bell_base_layout::w, style::bell_base_layout::outer_layouts_h);
        // // m2->setMaximumSize(200, 100);
        // // m2->setMinimumSize(200, 100);
        // m2->setAlignment(Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
        // m2->setFont(style::window::font::medium);
        // m2->setText(utils::translate("app_bellmain_power_nap"));

        body->lastBox->setEdges(gui::RectangleEdge::All);
        // body->lastBox->setAlignment(Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
        // auto m3 = new Label(body->lastBox);
        // m3->setMaximumSize(style::bell_base_layout::w, style::bell_base_layout::outer_layouts_h);
        // // m3->setMaximumSize(200, 100);
        // // m3->setMinimumSize(200, 100);
        // m3->setAlignment(Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
        // m3->setFont(style::window::font::medium);
        // m3->setText(utils::translate("app_bellmain_power_nap"));

        dimensionChangedCallback = [&](Item &, const BoundingBox &newDim) -> bool {
            body->setArea({0, 0, newDim.w, newDim.h});
            return true;
        };
    }
} // namespace gui
