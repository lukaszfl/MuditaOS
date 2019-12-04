#pragma once

// for global styles
#include <Style.hpp>
#include <Utils.hpp>

#define _addLabel(l)                                                                                                                                           \
    l = new Label(this, l::style::x, l::style::y, l::style::w, l::style::h);                                                                                   \
    l->setFilled(false);                                                                                                                                       \
    l->setBorderColor(l::style::borderColor);                                                                                                                  \
    l->setEdges(l::style::edges);                                                                                                                              \
    l->setFont(l::style::fontName);                                                                                                                            \
    l->setAlignement(l::style::alignment);                                                                                                                     \
    l->setLineMode(l::style::lineMode);

#define _addLabel2(l, text)                                                                                                                                    \
    _addLabel(l);                                                                                                                                              \
    l->setText(text);

#define _addLabel3(l, text, pageContainer)                                                                                                                     \
    _addLabel2(l, text);                                                                                                                                       \
    pageContainer.push_back(l);

/*
 * speedDialLabel = addLabel(this, nullptr, 196, 144, 89, 20, utils::localize.get("app_phonebook_contact_speed_dial_upper"), style::phonebook::font::tinybold,
                              RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES, Alignment(Alignment::ALIGN_HORIZONTAL_CENTER, Alignment::ALIGN_VERTICAL_CENTER));
 */
namespace Phonebook
{
    namespace ContactWindow
    {
        namespace speedDialValue
        {
            namespace style
            {
                const uint8_t x = 225;
                const uint8_t y = 105;
                const uint8_t w = 32;
                const uint8_t h = 32;
                const std::string fontName = ::style::footer::font::bold;
                const gui::RectangleEdgeFlags edges = gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES;
                const gui::Alignment alignment = gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_CENTER);
                const gui::Color borderColor = gui::ColorFullBlack;
                const bool lineMode = false;
            }; // namespace style
        };     // namespace speedDialValue

        namespace speedDialLabel
        {
            namespace style
            {
                const uint8_t x = 196;
                const uint8_t y = 144;
                const uint8_t w = 89;
                const uint8_t h = 20;
                const std::string fontName = ::style::phonebook::font::tinybold;
                const gui::RectangleEdgeFlags edges = gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES;
                const gui::Alignment alignment = gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_CENTER);
                const gui::Color borderColor = gui::ColorFullBlack;
                const bool lineMode = false;
            }; // namespace style
        };     // namespace speedDialLabel

        namespace favouritesLabel
        {
            namespace style
            {
                const uint8_t x = 65;
                const uint8_t y = 144;
                const uint8_t w = 89;
                const uint8_t h = 20;
                const std::string fontName = ::style::phonebook::font::tinybold;
                const gui::RectangleEdgeFlags edges = gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES;
                const gui::Alignment alignment = gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_CENTER);
                const gui::Color borderColor = gui::ColorFullBlack;
                const bool lineMode = false;
            }; // namespace style
        };     // namespace favouritesLabel

        namespace topSeparatorLabel
        {
            namespace style
            {
                const uint8_t x = 0;
                const uint8_t y = 179;
                const uint16_t w = 480;
                const uint8_t h = 1;
                const std::string fontName = ::style::phonebook::font::tinybold;
                const gui::RectangleEdgeFlags edges = gui::RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM;
                const gui::Alignment alignment = gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_CENTER);
                const gui::Color borderColor = gui::ColorFullBlack;
                const bool lineMode = false;
            }; // namespace style
        };     // namespace topSeparatorLabel

        namespace blockedLabel
        {
            namespace style
            {
                const uint16_t x = 329;
                const uint8_t y = 144;
                const uint8_t w = 75;
                const uint8_t h = 20;
                const std::string fontName = ::style::phonebook::font::tinybold;
                const gui::RectangleEdgeFlags edges = gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES;
                const gui::Alignment alignment = gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_CENTER);
                const gui::Color borderColor = gui::ColorFullBlack;
                const bool lineMode = false;
            }; // namespace style
        };     // namespace blockedLabel

        namespace informationLabel
        {
            namespace style
            {
                const uint8_t x = 30;
                const uint8_t y = 203;
                const uint16_t w = 413;
                const uint8_t h = 20;
                const std::string fontName = ::style::window::font::small;
                const gui::RectangleEdgeFlags edges = gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES;
                const gui::Alignment alignment = gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_CENTER);
                const gui::Color borderColor = gui::ColorFullBlack;
                const bool lineMode = false;
            }; // namespace style
        };     // namespace informationLabel
    };         // namespace ContactWindow
};             // namespace Phonebook
