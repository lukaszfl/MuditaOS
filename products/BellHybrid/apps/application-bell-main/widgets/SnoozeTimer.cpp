// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SnoozeTimer.hpp"
#include <FontManager.hpp>
#include <RawFont.hpp>
#include <gui/widgets/Label.hpp>
#include <gui/widgets/ImageBox.hpp>
#include <apps-common/widgets/TimeSetSpinner.hpp>

namespace gui
{
    SnoozeTimer::SnoozeTimer(Item *parent, Position x, Position y, Length w, Length h) : HBox(parent, x, y, w, h)
    {
        setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));
        setEdges(RectangleEdge::None);

        auto alarmImg = new ImageBox(this, 0, 0, 0, 0, new Image("bell_alarm_snooze_W_M"));
        alarmImg->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));
        alarmImg->setMargins(Margins(10U, 0, 10U, 0));
        alarmImg->setMinimumSizeToFitImage();

        minusText = new TextFixedSize(this, 0, 0, 0, 0);
        minusText->setText("-");
        minusText->setFont(style::window::font::largelight);
        minusText->setMargins(Margins(0, 0, 0, 0));
        minusText->setMinimumHeightToFitText();
        minusText->setMinimumWidthToFitText();
        minusText->drawUnderline(false);
        minusText->setEditMode(EditMode::Browse);
        minusText->activeItem = false;
        minusText->setAlignment(Alignment(Alignment::Horizontal::Right, Alignment::Vertical::Center));

        timeSpinner = new TimeSetSpinner(this);
        timeSpinner->setFont(style::window::font::largelight);
        timeSpinner->setEditMode(EditMode::Browse);
        timeSpinner->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));
        timeSpinner->setMargins(Margins(0, 0, 10U, 0));
        timeSpinner->setEdges(RectangleEdge::None);

        resizeItems();
    }

    auto SnoozeTimer::setFont(std::string newFontName) noexcept -> void
    {
        fontName = std::move(newFontName);
        minusText->setFont(fontName);
        timeSpinner->setFont(fontName);
    }

    auto SnoozeTimer::setTime(std::uint8_t mins, std::uint8_t secs) noexcept -> void
    {
        timeSpinner->setTime(mins, secs);
    }

} /* namespace gui */
