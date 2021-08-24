// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "FrontlightSpinnerListItem.hpp"
#include "BellSettingsStyle.hpp"

#include <gui/input/InputEvent.hpp>

namespace
{
    constexpr auto fmtSpinnerMin  = 0U;
    constexpr auto fmtSpinnerMax  = 9U;
    constexpr auto fmtSpinnerStep = 1U;
} // namespace

namespace gui
{
    FrontlightSpinnerListItem::FrontlightSpinnerListItem(
        gui::Length x, gui::Length y, gui::Length w, gui::Length h, std::string description)
        : BellSideListItem(std::move(description))
    {
        setMinimumSize(style::sidelistview::list_item::w, style::sidelistview::list_item::h);
        setEdges(RectangleEdge::None);
        setFocusItem(body);

        setMinimumSize(style::sidelistview::list_item::w, style::sidelistview::list_item::h);

        frontlightSetSpinner = new Spinner(fmtSpinnerMin, fmtSpinnerMax, fmtSpinnerStep, Boundaries::Continuous);
        frontlightSetSpinner->setMaximumSize(style::bell_base_layout::w, style::bell_base_layout::h);
        frontlightSetSpinner->setFont(bell_settings_style::time_fmt_set_list_item::font);

        frontlightSetSpinner->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));
        frontlightSetSpinner->setFixedFieldWidth(2);
        frontlightSetSpinner->setEdges(RectangleEdge::None);
        frontlightSetSpinner->setCurrentValue(fmtSpinnerMin);
        frontlightSetSpinner->setFocusEdges(RectangleEdge::None);
        body->centerBox->addWidget(frontlightSetSpinner);

        setFocusItem(body);

        dimensionChangedCallback = [&](gui::Item &, const BoundingBox &newDim) -> bool {
            body->setArea({0, 0, newDim.w, newDim.h});
            return true;
        };

        focusChangedCallback = [&](Item &item) {
            setFocusItem(focus ? body : nullptr);
            return true;
        };

        inputCallback = [&](Item &, const InputEvent &inputEvent) -> bool { return body->onInput(inputEvent); };
    }
} /* namespace gui */
