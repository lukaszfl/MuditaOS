#include "AllEventsWindow.hpp"
#include <gui/widgets/Window.hpp>
#include <gui/widgets/BottomBar.hpp>
#include <gui/widgets/TopBar.hpp>
#include <service-appmgr/ApplicationManager.hpp>

#include <module-services/service-db/messages/QueryMessage.hpp>
#include <module-db/queries/calendar/QueryEventsGetFiltered.hpp>
#include <module-db/queries/calendar/QueryEventsGetAll.hpp>
#include <module-services/service-db/api/DBServiceAPI.hpp>

namespace gui
{

    AllEventsWindow::AllEventsWindow(app::Application *app, std::string name)
        : AppWindow(app, style::window::calendar::name::all_events_window),
          allEventsModel{std::make_shared<AllEventsInternalModel>(this->application)}
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

        allEventsList->setPenFocusWidth(style::window::default_border_no_focus_w);
        allEventsList->setPenWidth(style::window::default_border_no_focus_w);

        setFocusItem(allEventsList);
    }

    void AllEventsWindow::onBeforeShow(gui::ShowMode mode, gui::SwitchData *data)
    {
        auto msg = DBServiceAPI::GetQueryWithReply(
            application, db::Interface::Name::Events, std::make_unique<db::query::events::GetAll>(), 1000);

        LOG_DEBUG("Type id %s", typeid(*msg.second).name());
        auto msgl = msg.second.get();
        assert(msgl != nullptr);
        onDatabaseMessage(msgl);
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
            std::unique_ptr<gui::SwitchData> data = std::make_unique<SwitchData>();
            data->setDescription("New");
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

    bool AllEventsWindow::onDatabaseMessage(sys::Message *msgl)
    {
        auto msg = dynamic_cast<db::QueryResponse *>(msgl);
        if (msg != nullptr) {
            auto temp = msg->getResult();
            if (auto response = dynamic_cast<db::query::events::GetAllResult *>(temp.get())) {
                unique_ptr<vector<EventsRecord>> records = response->getResult();
                for (auto &rec : *records) {
                    LOG_DEBUG("record: %s", rec.title.c_str());
                }
                allEventsList->rebuildList();
                allEventsModel->loadData(std::move(records));
            }
            LOG_DEBUG("Response False");
            return false;
        }
        LOG_DEBUG("AllEventsWindow DB Message != QueryResponse");
        return false;
    }
} /* namespace gui */
