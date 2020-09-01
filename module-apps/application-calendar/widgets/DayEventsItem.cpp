#include "DayEventsItem.hpp"
#include "CalendarStyle.hpp"
#include <ListView.hpp>
#include <gui/widgets/Label.hpp>
#include <Style.hpp>
#include <time/time_conversion.hpp>
#include <module-utils/date/include/date/date.h>

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

            struct tm tm;
            strptime(record->date_from.c_str(), "%Y-%m-%d %H:%M", &tm);
            auto eventStartTime = std::mktime(&tm);
            strptime(record->date_till.c_str(), "%Y-%m-%d %H:%M", &tm);
            auto eventEndTime = std::mktime(&tm);

            std::chrono::system_clock::time_point start_tp = std::chrono::system_clock::from_time_t(eventStartTime);
            auto start_time = date::make_time(
                std::chrono::duration_cast<std::chrono::minutes>(start_tp - date::floor<date::days>(start_tp)));
            std::chrono::system_clock::time_point end_tp = std::chrono::system_clock::from_time_t(eventEndTime);
            auto end_time = date::make_time(
                std::chrono::duration_cast<std::chrono::minutes>(end_tp - date::floor<date::days>(end_tp)));

            if (start_time.hours().count() == 0 && start_time.minutes().count() == 0 &&
                end_time.hours().count() == style::window::calendar::time::max_hour_24H_mode &&
                end_time.minutes().count() == style::window::calendar::time::max_minutes) {
                title->setText(utils::localize.get("app_calendar_all_day"));
            }
            else {
                auto start_h   = std::to_string(start_time.hours().count());
                auto start_min = std::to_string(start_time.minutes().count());
                auto end_h     = std::to_string(end_time.hours().count());
                auto end_min   = std::to_string(end_time.minutes().count());
                if (start_h.length() < style::window::calendar::time::max_time_length) {
                    start_h.insert(0, style::window::calendar::time::max_time_length / 2, '0');
                }
                if (start_min.length() < style::window::calendar::time::max_time_length) {
                    start_min.insert(0, style::window::calendar::time::max_time_length / 2, '0');
                }
                if (end_h.length() < style::window::calendar::time::max_time_length) {
                    end_h.insert(0, style::window::calendar::time::max_time_length / 2, '0');
                }
                if (end_min.length() < style::window::calendar::time::max_time_length) {
                    end_min.insert(0, style::window::calendar::time::max_time_length / 2, '0');
                }
                title->setText(start_h + ":" + start_min + " - " + end_h + ":" + end_min);
            }
            if (record->reminder == style::window::calendar::reminder::never) {
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
