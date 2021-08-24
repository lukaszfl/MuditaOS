// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <widgets/BellSideListItem.hpp>
#include <widgets/Spinner.hpp>

#include <string>

namespace gui
{
    class SpinBox;

    class FrontlightSpinnerListItem : public BellSideListItem
    {
      public:
        Spinner *frontlightSetSpinner = nullptr;

        FrontlightSpinnerListItem(gui::Length x, gui::Length y, gui::Length w, gui::Length h, std::string description);
    };
} /* namespace gui */
