// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "LevelBar.hpp"

namespace gui
{
    static inline auto rectAxisLenghtFrom(uint32_t numberOfRectangels) -> uint32_t
    {
        return numberOfRectangels * (style::levelbar::spacing + style::levelbar::rect_axis_length_sml) -
               style::levelbar::spacing;
    }

    Rect *LevelBar::createRectangle(uint32_t width, uint32_t height)
    {
        auto rectangle = new Rect(nullptr, 0, 0, width, height);
        rectangle->setFillColor(ColorFullBlack);
        rectangle->setFilled(true);
        rectangle->setRadius(style::levelbar::radius);
        rectangle->setPenWidth(style::window::default_border_focus_w);
        return rectangle;
    }

    Rect *LevelBar::createSpace(uint32_t width, uint32_t height)
    {
        auto space = new Rect(nullptr, 0, 0, width, height);
        space->setEdges(RectangleEdge::None);
        return space;
    }

    VLevelBar::VLevelBar(Item *parent, Position x, Position y, uint32_t numberOfRectangels)
        : VBox(parent, x, y, style::levelbar::rect_axis_length_lrg, rectAxisLenghtFrom(numberOfRectangels))
    {
        setRadius(style::levelbar::radius);
        VBox::setEdges(RectangleEdge::None);
        this->numberOfRectangels = numberOfRectangels;
        for (uint32_t i = 0; i < numberOfRectangels; ++i) {
            auto rectangle =
                createRectangle(style::levelbar::rect_axis_length_lrg, style::levelbar::rect_axis_length_sml);
            VBox::addWidget(rectangle);
            rectangles.push_back(rectangle);
            VBox::addWidget(createSpace(style::levelbar::rect_axis_length_lrg, style::levelbar::spacing));
        }
    }

    void VLevelBar::incrementWith(uint32_t levels)
    {
        if (currentLevel >= levels) {
            for (uint32_t i = levels; i > 0; --i) {
                --currentLevel;
                rectangles[currentLevel]->setFillColor(ColorFullBlack);
                rectangles[currentLevel]->setBorderColor(ColorFullBlack);
            }
        }
        else {
            LOG_ERROR("Levelbar incremented out of size");
        }
    }

    void VLevelBar::decrementWith(uint32_t levels)
    {
        if ((currentLevel + levels) <= numberOfRectangels) {
            for (uint32_t i = 0; i < levels; ++i) {
                rectangles[currentLevel]->setFillColor(ColorTray);
                rectangles[currentLevel]->setBorderColor(ColorTray);
                ++currentLevel;
            }
        }
        else {
            LOG_ERROR("Levelbar incremented out of size");
        }
    }

    HLevelBar::HLevelBar(Item *parent, Position x, Position y, uint32_t numberOfRectangels)
        : HBox(parent, x, y, rectAxisLenghtFrom(numberOfRectangels), style::levelbar::rect_axis_length_lrg)
    {
        currentLevel = numberOfRectangels;
        setRadius(style::levelbar::radius);
        HBox::setEdges(RectangleEdge::None);
        this->numberOfRectangels = numberOfRectangels;
        for (uint32_t i = 0; i < numberOfRectangels; ++i) {
            auto rectangle =
                createRectangle(style::levelbar::rect_axis_length_sml, style::levelbar::rect_axis_length_lrg);
            HBox::addWidget(rectangle);
            rectangles.push_back(rectangle);
            HBox::addWidget(createSpace(style::levelbar::spacing, style::levelbar::rect_axis_length_lrg));
        }
    }

    void HLevelBar::incrementWith(uint32_t levels)
    {
        if ((currentLevel + levels) <= numberOfRectangels) {
            for (uint32_t i = 0; i < levels; ++i) {
                rectangles[currentLevel]->setFillColor(ColorFullBlack);
                rectangles[currentLevel]->setBorderColor(ColorFullBlack);
                ++currentLevel;
            }
        }
        else {
            LOG_ERROR("Levelbar incremented out of size");
        }
    }

    void HLevelBar::decrementWith(uint32_t levels)
    {
        if (currentLevel >= levels) {
            for (uint32_t i = levels; i > 0; --i) {
                --currentLevel;
                rectangles[currentLevel]->setFillColor(ColorFullWhite);
                rectangles[currentLevel]->setBorderColor(ColorFullBlack);
            }
        }
        else {
            LOG_ERROR("Levelbar incremented out of size");
        }
    }

} /* namespace gui */
