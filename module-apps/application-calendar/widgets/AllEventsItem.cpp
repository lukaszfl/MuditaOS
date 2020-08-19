#include "AllEventsItem.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include <Style.hpp>
#include <gui/widgets/Label.hpp>
#include <time/time_conversion.hpp>

namespace gui
{
    AllEventsItem::AllEventsItem()
    {
        setMinimumSize(style::window::default_body_width, style::window::label::big_h);

        hBox = new gui::HBox(this, 0, 0, 0, 0);
        hBox->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);

        startTime = new gui::Label(hBox, 0, 0, 0, 0);
        startTime->setMinimumSize(style::window::calendar::item::allEvents::start_time_min_w,
                                  style::window::label::big_h);
        startTime->setMaximumSize(style::window::default_body_width, style::window::label::big_h);
        startTime->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        startTime->setFont(style::window::font::small);
        startTime->setAlignment(gui::Alignment{gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center});

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
        uint32_t start_time    = this->record->date_from % 10000;
        uint32_t month_and_day = (this->record->date_from % 100000000 - start_time) / 10000;
        unsigned int dayUInt   = static_cast<unsigned>(month_and_day % 100);
        std::string monthStr =
            utils::time::Locale::get_month(utils::time::Locale::Month(((month_and_day - dayUInt) / 100) - 1));
        return std::to_string(dayUInt) + " " + monthStr;
    }

    void AllEventsItem::setEvent(std::shared_ptr<EventsRecord> rec)
    {
        this->record = rec;

        if (rec != nullptr) {
            description->setText(this->record->title.c_str());
            std::string start_time = std::to_string(this->record->date_from % 10000);
            if (this->record->date_from % 10000 == 0 && this->record->date_till % 10000 == 2359) {
                startTime->setText("All day");
            }
            else {
                startTime->setText(start_time);
            }
        }
    }
} /* namespace gui */
