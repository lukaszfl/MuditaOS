#include <module-db/Interface/EventsRecord.hpp>
#include <module-services/service-db/api/DBServiceAPI.hpp>
#include <module-db/queries/calendar/QueryEventsAdd.hpp>
#include "NewEditEventWindow.hpp"

namespace gui
{

    NewEditEventWindow::NewEditEventWindow(app::Application *app, std::string name)
        : AppWindow(app, style::window::calendar::name::new_edit_event),
          newEditEventModel{std::make_shared<NewEditEventModel>(this->application)}
    {
        buildInterface();
    }

    void NewEditEventWindow::rebuild()
    {
        erase();
        buildInterface();
    }

    void NewEditEventWindow::buildInterface()
    {
        AppWindow::buildInterface();

        topBar->setActive(gui::TopBar::Elements::TIME, true);
        bottomBar->setActive(gui::BottomBar::Side::RIGHT, true);
        bottomBar->setActive(gui::BottomBar::Side::CENTER, true);
        bottomBar->setText(gui::BottomBar::Side::RIGHT, utils::localize.get(style::strings::common::back));
        bottomBar->setText(gui::BottomBar::Side::CENTER, utils::localize.get(style::strings::common::save));

        list = new gui::ListView(this,
                                 style::window::calendar::listView_x,
                                 style::window::calendar::listView_y,
                                 style::window::calendar::listView_w,
                                 style::window::calendar::listView_h,
                                 newEditEventModel);
        setFocusItem(list);
    }

    void NewEditEventWindow::onBeforeShow(gui::ShowMode mode, gui::SwitchData *data)
    {
        switch (eventAction) {
        case EventAction::None:
            break;
        case EventAction::Add:
            setTitle(utils::localize.get("app_calendar_new_event_title"));
            break;
        case EventAction::Edit:
            setTitle(utils::localize.get("app_calendar_edit_event_title"));
            break;
        }

        if (mode == ShowMode::GUI_SHOW_INIT) {
            newEditEventModel->loadData(false);
            list->rebuildList();
        }
    }

    //    bool CustomRepeatWindow::onInput(const InputEvent &inputEvent)
    //    {
    //        // check if any of the lower inheritance onInput methods catch the event
    //        if (Window::onInput(inputEvent)) {
    //            return true;
    //        }
    //        // process only if key is released
    //        if ((inputEvent.state != InputEvent::State::keyReleasedShort))
    //            return false;
    //
    //        switch (inputEvent.keyCode) {
    ////            case KeyCode::KEY_VOLUP: {
    ////                application->increaseCurrentVolume();
    ////                return true;
    ////            }
    ////            case KeyCode::KEY_VOLDN: {
    ////                application->decreaseCurrentVolume();
    ////                return true;
    ////            }
    //            case KeyCode::KEY_RF: {
    //                application->returnToPreviousWindow();
    //                return true;
    //            }
    ////            case KeyCode::KEY_TORCH: {
    ////                application->toggleTorchAndColourTemps();
    ////                return true;
    ////            }
    //            default:
    //                break;
    //        }
    //
    //        return false;
    //    }

    bool NewEditEventWindow::onInput(const gui::InputEvent &inputEvent)
    {
        if (AppWindow::onInput(inputEvent)) {
            return true;
        }

        if (!inputEvent.isShortPress()) {
            return false;
        }

        if (inputEvent.keyCode == gui::KeyCode::KEY_ENTER) {
            LOG_DEBUG("Save Event");
            EventsRecord event(EventsTableRow{{2}, "TEST2", "TEST2", 2008170006, 2008171536, 1, 2, 1});
            EventsRecord event2(EventsTableRow{{2}, "TEST1", "TEST", 2008200006, 2008201536, 1, 2, 1});
            EventsRecord event3(EventsTableRow{{2}, "TEST3", "TEST3", 2008171406, 2008171536, 1, 2, 1});
            EventsRecord event4(EventsTableRow{{2}, "TEST4", "TEST4", 2008210006, 2008211536, 1, 2, 1});
            EventsRecord event5(EventsTableRow{{2}, "TEST5", "TEST5", 2008220006, 2008221536, 1, 2, 1});
            DBServiceAPI::GetQuery(
                application, db::Interface::Name::Events, std::make_unique<db::query::events::Add>(event));
            DBServiceAPI::GetQuery(
                application, db::Interface::Name::Events, std::make_unique<db::query::events::Add>(event5));
            DBServiceAPI::GetQuery(
                application, db::Interface::Name::Events, std::make_unique<db::query::events::Add>(event2));
            DBServiceAPI::GetQuery(
                application, db::Interface::Name::Events, std::make_unique<db::query::events::Add>(event3));
            DBServiceAPI::GetQuery(
                application, db::Interface::Name::Events, std::make_unique<db::query::events::Add>(event4));
            return true;
        }

        return false;
    }

    bool NewEditEventWindow::handleSwitchData(gui::SwitchData *data)
    {
        if (data == nullptr) {
            return false;
        }

        if (data->getDescription() == "Edit") {
            eventAction = EventAction::Edit;
        }
        else if (data->getDescription() == "New") {
            eventAction = EventAction::Add;
        }
        return false;
    }
} /* namespace gui */
