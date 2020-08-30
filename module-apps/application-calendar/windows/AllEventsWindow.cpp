#include "AllEventsWindow.hpp"
#include <gui/widgets/Window.hpp>
#include <gui/widgets/BottomBar.hpp>
#include <gui/widgets/TopBar.hpp>
#include <service-appmgr/ApplicationManager.hpp>

#include <module-services/service-db/messages/QueryMessage.hpp>
#include <module-db/queries/calendar/QueryEventsGetFiltered.hpp>
#include <module-db/queries/calendar/QueryEventsGetAllLimited.hpp>
#include <module-db/queries/calendar/QueryEventsGetAll.hpp>
#include <module-services/service-db/api/DBServiceAPI.hpp>
#include <module-apps/application-calendar/data/CalendarData.hpp>
#include <time/time_conversion.hpp>
#include <module-apps/application-calendar/ApplicationCalendar.hpp>

namespace gui
{

    AllEventsWindow::AllEventsWindow(app::Application *app, std::string name)
        : AppWindow(app, style::window::calendar::name::all_events_window),
          allEventsModel{std::make_shared<AllEventsModel>(this->application)}
    {
        buildInterface();
    }

    void AllEventsWindow::rebuild()
    {
        buildInterface();
    }

    void AllEventsWindow::buildInterface()
    {
        AppWindow::buildInterface();

        topBar->setActive(gui::TopBar::Elements::TIME, true);
        bottomBar->setActive(gui::BottomBar::Side::RIGHT, true);
        bottomBar->setText(gui::BottomBar::Side::RIGHT, utils::localize.get(style::strings::common::back));
        bottomBar->setText(gui::BottomBar::Side::CENTER, utils::localize.get(style::strings::common::open));
        bottomBar->setText(gui::BottomBar::Side::LEFT, utils::localize.get("app_calendar_bar_month"));

        setTitle(utils::localize.get("app_calendar_title_main"));
        leftArrowImage = new gui::Image(
            this, style::window::calendar::arrow_x, style::window::calendar::arrow_y, 0, 0, "arrow_left");
        newDayEventImage =
            new gui::Image(this, style::window::calendar::cross_x, style::window::calendar::cross_y, 0, 0, "cross");

        allEventsList = new gui::CalendarListView(this,
                                                  style::window::calendar::listView_x,
                                                  style::window::calendar::listView_y,
                                                  style::window::calendar::listView_w,
                                                  style::window::calendar::listView_h,
                                                  allEventsModel);
        setFocusItem(allEventsList);
    }

    void AllEventsWindow::onBeforeShow(gui::ShowMode mode, gui::SwitchData *data)
    {
        allEventsList->rebuildList();
        auto dataRecieved = dynamic_cast<PrevWindowData *>(data);
        if (dataRecieved != nullptr) {
            if (dataRecieved->getData() == PrevWindow::DELETE) {
                checkEmpty = true;
            }
        }
    }

    bool AllEventsWindow::onInput(const gui::InputEvent &inputEvent)
    {
        if (inputEvent.keyCode == gui::KeyCode::KEY_RF &&
            inputEvent.state == gui::InputEvent::State::keyReleasedShort) {
            LOG_DEBUG("Switch to desktop");
            sapm::ApplicationManager::messageSwitchPreviousApplication(application);
        }

        if (AppWindow::onInput(inputEvent)) {
            return true;
        }

        if (!inputEvent.isShortPress()) {
            return false;
        }

        if (inputEvent.keyCode == gui::KeyCode::KEY_LEFT) {
            LOG_DEBUG("Switch to new event window");
            std::unique_ptr<EventRecordData> data = std::make_unique<EventRecordData>();
            data->setDescription("New");
            auto rec = new EventsRecord();
            rec->date_from = getFilter() + " 00:00";
            rec->date_till = getFilter() + " 23:59";
            auto event     = std::make_shared<EventsRecord>(*rec);
            data->setData(event);
            data->setWindowName(style::window::calendar::name::all_events_window);
            application->switchWindow(
                style::window::calendar::name::new_edit_event, gui::ShowMode::GUI_SHOW_INIT, std::move(data));
            return true;
        }

        if (inputEvent.keyCode == gui::KeyCode::KEY_LF) {
            application->switchWindow(gui::name::window::main_window);
            LOG_DEBUG("Switch to month view - main window");
            return true;
        }

        return false;
    }

    std::string AllEventsWindow::getFilter()
    {
        std::chrono::system_clock::time_point start_tp =
            std::chrono::system_clock::from_time_t(utils::time::Timestamp().getTime() + 7200);
        auto actualDate = date::year_month_day{date::floor<date::days>(start_tp)};
        int yearUInt    = static_cast<decltype(yearUInt)>(actualDate.year());
        auto monthStr   = std::to_string(unsigned(actualDate.month()));
        auto dayStr     = std::to_string(unsigned(actualDate.day()));
        if (monthStr.length() < style::window::calendar::time::max_time_length) {
            monthStr.insert(0, style::window::calendar::time::max_time_length / 2, '0');
        }
        if (dayStr.length() < style::window::calendar::time::max_time_length) {
            dayStr.insert(0, style::window::calendar::time::max_time_length / 2, '0');
        }
        return std::to_string(yearUInt) + "-" + monthStr + "-" + dayStr;
    }

    bool AllEventsWindow::onDatabaseMessage(sys::Message *msgl)
    {
        auto msg = dynamic_cast<db::QueryResponse *>(msgl);
        if (msg != nullptr) {
            auto temp = msg->getResult();
            if (auto response = dynamic_cast<db::query::events::GetAllLimitedResult *>(temp.get())) {
                auto records_data = response->getResult();
                allEventsModel->setRecordsCount(*response->getCountResult());
                auto records = std::make_unique<std::vector<EventsRecord>>(records_data->begin(), records_data->end());
                if (checkEmpty) {
                    if (records->size() == 0) {
                        auto app = dynamic_cast<app::ApplicationCalendar *>(application);
                        assert(application != nullptr);
                        auto filter = getFilter() + " 00:00";
                        app->switchToNoEventsWindow(utils::localize.get("app_calendar_title_main"),
                                                    filter,
                                                    style::window::calendar::name::all_events_window);
                    }
                }
                application->refreshWindow(RefreshModes::GUI_REFRESH_FAST);
                return allEventsModel->updateRecords(std::move(records));
            }
            LOG_DEBUG("Response False");
            return false;
        }
        LOG_DEBUG("AllEventsWindow DB Message != QueryResponse");
        return false;
    }
} /* namespace gui */
