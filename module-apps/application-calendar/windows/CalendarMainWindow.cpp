#include "CalendarMainWindow.hpp"
#include "application-calendar/ApplicationCalendar.hpp"
#include "application-calendar/models/MonthModel.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include "application-calendar/models/DayEventsModel.hpp"
#include "application-calendar/models/AllEventsModel.hpp"
#include "application-calendar/data/CalendarData.hpp"
#include "NoEvents.hpp"
#include <module-services/service-db/messages/QueryMessage.hpp>
#include <module-db/queries/calendar/QueryEventsGetAll.hpp>
#include <module-db/queries/calendar/QueryEventsGetFiltered.hpp>
#include <module-services/service-db/api/DBServiceAPI.hpp>
#include <time/time_conversion.hpp>

namespace gui
{
    DayLabel::DayLabel(app::Application *app,
                       gui::Item *parent,
                       const uint32_t &cellIndex,
                       const uint32_t &firstWeekOffset,
                       const uint32_t &width,
                       const uint32_t &height)
        : VBox(parent, 0, 0, 0, 0)
    {
        parent->addWidget(this);
        this->setSize(width, height);
        this->setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        this->dayNumber = new gui::Label(this, 0, 0, width, height - 33);
        this->dayNumber->setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        this->dayNumber->setAlignment(
            gui::Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
        this->dayNumber->setMargins(gui::Margins(0, 11, 0, 0));
        this->dayNumber->activeItem = false;
        this->dot                   = new gui::Label(this, 0, 0, width, 11);
        this->dot->setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        this->dot->setAlignment(gui::Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Top));
        this->dot->setText(".");
        this->dot->activeItem = false;

        if (cellIndex < style::window::calendar::week_days_number) {
            this->dayNumber->setText(utils::time::Locale::get_short_day(cellIndex));
            this->dayNumber->setFont(style::window::font::verysmall);
            this->dot->setVisible(false);
            this->activeItem = false;
        }
        else if (cellIndex >= style::window::calendar::week_days_number &&
                 cellIndex < style::window::calendar::week_days_number + firstWeekOffset) {
            this->setPenWidth(style::window::default_border_no_focus_w);
            this->dot->setVisible(false);
            this->activeItem = false;
        }
        else {
            uint32_t numb      = cellIndex - firstWeekOffset - style::window::calendar::week_days_number + 1;
            std::string number = std::to_string(numb);
            this->dayNumber->setText(number);
            this->activeItem = true;
            this->dayNumber->setFont(style::window::font::medium);
            this->activatedCallback = [=](gui::Item &item) {
                // LOG_DEBUG("Switch to DayEventsWindow");
                auto data           = std::make_unique<DayMonthData>();
                auto actualMonthBox = dynamic_cast<gui::MonthBox *>(parent);
                auto month          = actualMonthBox->month;
                auto filter         = actualMonthBox->monthFilterValue + numb * 10000;
                data->setData(number + " " + month, filter);

                auto application = dynamic_cast<app::ApplicationCalendar *>(app);
                assert(application != nullptr);
                application->switchToNoEventsWindow(data->getDayMonthText());
                // app->switchWindow("DayEventsWindow", std::move(data));
                return true;
            };
            this->setPenWidth(style::window::default_border_no_focus_w);
            this->setPenFocusWidth(style::window::default_border_focus_w);
            this->setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_TOP | RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM);
        }
        this->setAlignment(gui::Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
    }

    MonthBox::MonthBox(app::Application *app,
                       gui::Item *parent,
                       const int &offsetTop,
                       const uint32_t &width,
                       const uint32_t &height,
                       const uint32_t &dayWidth,
                       const uint32_t &dayHeight,
                       const std::unique_ptr<MonthModel> &model)
        : GridLayout(parent, style::window::default_left_margin, offsetTop, width, height, {dayWidth, dayHeight})
    {
        LOG_DEBUG("Call MonthBox constructor");

        assert(parent);
        parent->addWidget(this);
        month                  = model->getMonthText();
        date::year actualYear  = model->getYear();
        int yearInt            = static_cast<decltype(yearInt)>(actualYear) - 2000;
        uint32_t yearUInt      = yearInt * 100000000;
        unsigned int monthUInt = static_cast<unsigned>(model->getMonth()) * 1000000;
        monthFilterValue       = yearUInt + monthUInt;
        grid.x = dayWidth;
        grid.y = dayHeight;

        uint32_t firstDayOffset = model->getFirstWeekOffset();
        uint32_t lastDay        = model->getLastDay();
        uint32_t iterations     = style::window::calendar::week_days_number + firstDayOffset + lastDay;

        uint32_t i;
        for (i = 0; i < iterations; ++i) {
            auto day = new DayLabel(app,
                                    this,
                                    i,
                                    firstDayOffset,
                                    style::window::calendar::day_cell_width,
                                    style::window::calendar::day_cell_height);
            addWidget(day);
        }

        LOG_DEBUG("MonthBox constructor Completed Successfully!");
    }

    CalendarMainWindow::CalendarMainWindow(app::Application *app, std::string name) : AppWindow(app, name)
    {
        auto appCalendar = dynamic_cast<app::ApplicationCalendar *>(application);
        assert(appCalendar != nullptr);
        std::chrono::system_clock::time_point tp =
            std::chrono::system_clock::from_time_t(appCalendar->getCurrentTimeStamp());
        this->actualDate = date::year_month_day{date::floor<date::days>(tp)};
        buildInterface();
    }

    void CalendarMainWindow::refresh()
    {
        erase(dateLabel);
        month->erase();

        monthModel           = std::make_unique<MonthModel>(actualDate);
        std::string dateText = monthModel->getMonthYearText();
        this->buildMonth(monthModel);
        this->buildDateLabel(dateText);
    }

    void CalendarMainWindow::rebuild()
    {
        destroyInterface();
        buildInterface();
    }

    void CalendarMainWindow::buildMonth(std::unique_ptr<MonthModel> &model)
    {
        auto app = dynamic_cast<app::ApplicationCalendar *>(application);
        assert(app != nullptr);

        offsetFromTop = title->offset_h() + style::window::calendar::month_year_height;
        monthWidth    = style::window::default_body_width;
        monthHeight   = style::window_height - title->offset_h() - style::footer::height;
        dayWidth      = style::window::calendar::day_cell_width;
        dayHeight     = style::window::calendar::day_cell_height;

        // create empty month box
        month = new MonthBox(app, this, offsetFromTop, monthWidth, monthHeight, dayWidth, dayHeight, monthModel);
        // setup month box
        addWidget(month);

        month->borderCallback = [this](const InputEvent &inputEvent) -> bool {
            if (inputEvent.state != InputEvent::State::keyReleasedShort) {
                return false;
            }
            switch (inputEvent.keyCode) {
            case KeyCode::KEY_UP: {
                LOG_DEBUG("change month prev");
                if (actualDate.month() != date::January) {
                    date::month prevMonth = --actualDate.month();
                    actualDate            = actualDate.year() / prevMonth / date::last;
                }
                else {
                    date::month prevMonth = date::December;
                    date::year prevYear   = --actualDate.year();
                    actualDate            = prevYear / prevMonth / date::last;
                }
                this->refresh();
                return true;
            }
            case KeyCode::KEY_DOWN: {
                LOG_DEBUG("change month next");
                if (actualDate.month() != date::December) {
                    date::month nextMonth = ++actualDate.month();
                    actualDate            = actualDate.year() / nextMonth / 1;
                }
                else {
                    date::month nextMonth = date::January;
                    date::year nextYear   = ++actualDate.year();
                    actualDate            = nextYear / nextMonth / 1;
                }
                this->refresh();
                return true;
            }
            case KeyCode::KEY_LEFT: {
                LOG_DEBUG("Call borderCallback -> go to the previous element");
                auto it = month->getNavigationFocusedItem();
                if (month->nextNavigationItem(std::prev(it)) != nullptr) {
                    month->setFocusItem(month->nextNavigationItem(std::prev(it)));
                }
                else {
                    month->setFocusOnLastElement();
                }
                return true;
            }
            case KeyCode::KEY_RIGHT: {
                LOG_DEBUG("Call borderCallback -> go to the next element");
                auto it = month->getNavigationFocusedItem();
                if (month->nextNavigationItem(std::next(it)) != nullptr) {
                    month->setFocusItem(month->nextNavigationItem(std::next(it)));
                }
                else {
                    month->setFocusOnElement(0);
                }
                return true;
            }
            default: {
                return false;
            }
            }
        };

        setFocusItem(month);
    }

    void CalendarMainWindow::buildDateLabel(std::string actualDateTime)
    {
        dateLabel = new Label(this,
                              style::window::default_left_margin,
                              title->offset_h(),
                              style::window::default_body_width,
                              style::window::calendar::month_year_height,
                              actualDateTime);

        dateLabel->setPenWidth(style::window::default_border_no_focus_w);
        dateLabel->setFont(style::window::font::mediumbold);
        dateLabel->setAlignment(gui::Alignment(gui::Alignment::Vertical::Center));
        addWidget(dateLabel);
    }

    void CalendarMainWindow::buildInterface()
    {
        LOG_DEBUG("AppWindow build interface");
        AppWindow::buildInterface();

        LOG_DEBUG("Start build interface for calendar main window");
        [[maybe_unused]] auto app = dynamic_cast<app::ApplicationCalendar *>(application);
        assert(app != nullptr);

        setTitle(utils::localize.get("app_calendar_title_main"));

        monthModel = std::make_unique<MonthModel>(actualDate);
        this->buildMonth(monthModel);
        this->buildDateLabel(monthModel->getMonthYearText());

        bottomBar->setActive(gui::BottomBar::Side::CENTER, true);
        bottomBar->setActive(gui::BottomBar::Side::RIGHT, true);
        bottomBar->setActive(gui::BottomBar::Side::LEFT, true);
        bottomBar->setText(gui::BottomBar::Side::RIGHT, utils::localize.get(style::strings::common::back));
        bottomBar->setText(gui::BottomBar::Side::CENTER, utils::localize.get(style::strings::common::open));
        bottomBar->setText(gui::BottomBar::Side::LEFT, utils::localize.get("app_calendar_bar_list"));
    }

    void CalendarMainWindow::destroyInterface()
    {
        erase();
    }

    bool CalendarMainWindow::onInput(const gui::InputEvent &inputEvent)
    {
        if (AppWindow::onInput(inputEvent)) {
            return true;
        }

        if (!inputEvent.isShortPress()) {
            return false;
        }

        if (inputEvent.keyCode == gui::KeyCode::KEY_LF) {
            /*std::shared_ptr<AllEventsModel> allEventsModel = std::make_shared<AllEventsModel>(this->application);
            if (allEventsModel->requestRecordsCount() == 0) {
                auto app = dynamic_cast<app::ApplicationCalendar *>(application);
                assert(app != nullptr);
                app->switchToNoEventsWindow(utils::localize.get("app_calendar_title_main").c_str());
            }
            else {
                LOG_DEBUG("Switch to List Window");
                application->switchWindow(style::window::calendar::name::all_events_window);
            }*/
            application->switchWindow(style::window::calendar::name::all_events_window);
            return true;
        }

        return false;
    }

    void CalendarMainWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        /*if (mode == ShowMode::GUI_SHOW_INIT) {
            LOG_DEBUG("On before show MainWindow");
            auto msg = DBServiceAPI::GetQueryWithReply(
                application,
                db::Interface::Name::Events,
                std::make_unique<db::query::events::GetFiltered>(
                    month->monthFilterValue, month->monthFilterValue + monthModel->getLastDay() * 10000 + 2359),
                1000);

            LOG_DEBUG("Type id %s", typeid(*msg.second).name());
            auto msgl = msg.second.get();
            assert(msgl != nullptr);
            onDatabaseMessage(msgl);
        }*/
    }

    bool CalendarMainWindow::onDatabaseMessage(sys::Message *msgl)
    {
        /*auto msg = dynamic_cast<db::QueryResponse *>(msgl);
        if (msg != nullptr) {
            auto temp = msg->getResult();
            if (auto response = dynamic_cast<db::query::events::GetFilteredResult *>(temp.get())) {
                unique_ptr<vector<EventsRecord>> records = response->getResult();
                for (auto &rec : *records) {
                    LOG_DEBUG("record: %s", rec.title.c_str());
                    isDayEmpty[((rec.date_from % 100000000 - rec.date_from % 10000) / 10000) % 100] = false;
                }
            }
            LOG_DEBUG("Response False");
            return false;
        }
        LOG_DEBUG("Calendar MainWindow DB Message != QueryResponse");*/
        return false;
    }

} // namespace gui
