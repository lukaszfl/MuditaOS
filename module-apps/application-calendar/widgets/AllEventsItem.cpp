#include "AllEventsItem.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include <Style.hpp>
#include <gui/widgets/Label.hpp>
#include <time/time_conversion.hpp>
#include <module-utils/date/include/date/date.h>

namespace gui
{
    AllEventsItem::AllEventsItem()
    {
        setMinimumSize(style::window::default_body_width, style::window::label::big_h);
        setMargins(gui::Margins(0, style::margins::big, 0, 0));

        hBox = new gui::HBox(this, 0, 0, 0, 0);
        hBox->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);

        startTime = new gui::Label(hBox, 0, 0, 0, 0);
        startTime->setMinimumSize(style::window::calendar::item::allEvents::start_time_min_w,
                                  style::window::label::big_h);
        startTime->setMaximumSize(style::window::default_body_width, style::window::label::big_h);
        startTime->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        startTime->setFont(style::window::font::small);
        startTime->setAlignment(gui::Alignment{gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center});
        startTime->setMargins(gui::Margins(style::margins::small, 0, 0, 0));

        description = new gui::Label(hBox, 0, 0, 0, 0);
        description->setMinimumSize(style::window::calendar::item::allEvents::description_w,
                                    style::window::label::big_h);
        description->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        description->setFont(style::window::font::bigbold);
        description->setAlignment(gui::Alignment{gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center});
    }

    bool AllEventsItem::onDimensionChanged(const BoundingBox &oldDim, const BoundingBox &newDim)
    {
        hBox->setPosition(0, 0);
        hBox->setSize(newDim.w, newDim.h);
        return true;
    }

    void AllEventsItem::setMarkerItem(UTF8 text)
    {
        description->setText("");
        startTime->setText(text);
        description->setMinimumSize(0, 0);
        startTime->setLineMode(true);
        activeItem = false;
        setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
    }

    UTF8 AllEventsItem::getLabelMarker() const
    {
        struct tm tm;
        strptime(this->record->date_from.c_str(), "%Y-%m-%d %H:%M", &tm);
        auto eventStartTime = std::mktime(&tm);

        std::chrono::system_clock::time_point start_tp = std::chrono::system_clock::from_time_t(eventStartTime);
        auto startDate = date::year_month_day{date::floor<date::days>(start_tp)};

        std::string monthStr =
            utils::time::Locale::get_month(utils::time::Locale::Month(unsigned(startDate.month()) - 1));
        return std::to_string(unsigned(startDate.day())) + " " + monthStr;
    }

    void AllEventsItem::setEvent(std::shared_ptr<EventsRecord> rec)
    {
        this->record = rec;
        if (rec != nullptr) {
            description->setText(this->record->title.c_str());

            struct tm tm;
            strptime(this->record->date_from.c_str(), "%Y-%m-%d %H:%M", &tm);
            auto eventStartTime = std::mktime(&tm);
            strptime(this->record->date_till.c_str(), "%Y-%m-%d %H:%M", &tm);
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
                startTime->setText(utils::localize.get("app_calendar_all_day"));
            }
            else {
                auto start_h   = std::to_string(start_time.hours().count());
                auto start_min = std::to_string(start_time.minutes().count());
                if (start_h.length() < style::window::calendar::time::max_time_length) {
                    start_h.insert(0, style::window::calendar::time::max_time_length / 2, '0');
                }
                if (start_min.length() < style::window::calendar::time::max_time_length) {
                    start_min.insert(0, style::window::calendar::time::max_time_length / 2, '0');
                }
                startTime->setText(start_h + ":" + start_min);
            }
        }
    }
} /* namespace gui */
