#include "DayEventsItem.hpp"
#include "CalendarStyle.hpp"
#include <ListView.hpp>
#include <gui/widgets/Label.hpp>
#include <Style.hpp>
#include <time/time_conversion.hpp>

namespace gui
{

    DayEventsItem::DayEventsItem()
    {
        setMinimumSize(style::window::default_body_width, style::window::calendar::item::dayEvents::height);
        setMargins(gui::Margins(0, style::margins::small, 0, 0));

        vBox = new gui::VBox(this, 0, 0, 0, 0);
        vBox->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        hBox = new gui::HBox(vBox, 0, 0, 0, 0);
        hBox->setMinimumSize(style::window::default_body_width, style::window::calendar::item::dayEvents::box_height);
        hBox->setMargins(gui::Margins(0, style::margins::small + style::margins::big, 0, 0));
        hBox->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);

        title = new gui::Label(hBox, 0, 0, 0, 0);
        title->setMinimumSize(style::window::calendar::item::dayEvents::title_w,
                              style::window::calendar::item::dayEvents::box_height);
        title->setMargins(gui::Margins(style::margins::small, 0, style::margins::small + style::margins::big, 0));
        title->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        title->setFont(style::window::font::bigbold);
        title->setAlignment(gui::Alignment{gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center});

        clock = new gui::Image(hBox, 0, 0, 0, 0, "small_tick");
        clock->setAlignment(gui::Alignment{gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center});

        description = new gui::Label(vBox, 0, 0, 0, 0);
        description->setMinimumSize(style::window::default_body_width,
                                    style::window::calendar::item::dayEvents::box_height);
        description->setMargins(gui::Margins(style::margins::small, style::margins::big, 0, style::margins::very_big));
        description->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        description->setFont(style::window::font::medium);
        description->setAlignment(gui::Alignment{gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center});
    }

    void DayEventsItem::setEvent(std::shared_ptr<EventsRecord> rec)
    {
        this->record = rec;

        if (rec != nullptr) {
            description->setText(this->record->title.c_str());
            uint32_t start_time = this->record->date_from % 10000;
            uint32_t end_time   = this->record->date_till % 10000;
            if (start_time == 0 && end_time == 2359) {
                title->setText(utils::localize.get("app_calendar_all_day"));
            }
            else {
                auto start = std::to_string(start_time);
                auto end   = std::to_string(end_time);
                if (start.length() < style::window::calendar::max_time_length) {
                    start.insert(0, style::window::calendar::max_time_length - start.length(), '0');
                }
                if (end.length() < style::window::calendar::max_time_length) {
                    end.insert(0, style::window::calendar::max_time_length - end.length(), '0');
                }
                auto text =
                    start.substr(0, style::window::calendar::max_time_length / 2) + ":" +
                    start.substr(style::window::calendar::max_time_length / 2,
                                 style::window::calendar::max_time_length) +
                    " - " + end.substr(0, style::window::calendar::max_time_length / 2) + ":" +
                    end.substr(style::window::calendar::max_time_length / 2, style::window::calendar::max_time_length);
                title->setText(text);
            }
            if (record->reminder == 0xFFFFFF) {
                clock->setVisible(false);
            }
        }
    }

    bool DayEventsItem::onDimensionChanged(const BoundingBox &oldDim, const BoundingBox &newDim)
    {
        vBox->setPosition(0, 0);
        vBox->setSize(newDim.w, newDim.h);
        return true;
    }

} /* namespace gui */
