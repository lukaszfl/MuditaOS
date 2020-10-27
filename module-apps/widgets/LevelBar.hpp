// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <BoxLayout.hpp>
#include <module-gui/gui/widgets/ProgressBar.hpp>

namespace style::levelbar
{
    constexpr inline auto rect_axis_length_sml = 8;
    constexpr inline auto rect_axis_length_lrg = 20;
    constexpr inline auto spacing              = 17;
    constexpr inline auto radius               = 4;
} // namespace style::levelbar

namespace gui
{
    class LevelBar : public Progress
    {
      protected:
        std::vector<gui::Rect *> rectangles;
        uint32_t numberOfRectangels;
        uint32_t currentLevel = 0;

      public:
        Rect *createRectangle(uint32_t width, uint32_t height);

        Rect *createSpace(uint32_t width, uint32_t height);

        void setMaximum(unsigned int value) noexcept override{};

        void setValue(unsigned int value) noexcept override{};

        void setPercentageValue(unsigned int value) noexcept override{};
    };

    class VLevelBar : VBox, public LevelBar
    {
      public:
        VLevelBar(Item *parent = nullptr, Position x = 0, Position y = 0, uint32_t numberOfRectangels = 0);

        void incrementWith(uint32_t levels);

        void decrementWith(uint32_t levels);
    };

    class HLevelBar : HBox, public LevelBar
    {
      public:
        HLevelBar(Item *parent = nullptr, Position x = 0, Position y = 0, uint32_t numberOfRectangels = 0);

        void incrementWith(uint32_t levels);

        void decrementWith(uint32_t levels);
    };

} /* namespace gui */
