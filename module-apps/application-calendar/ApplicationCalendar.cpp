#include "ApplicationCalendar.hpp"
#include "windows/CalendarMainWindow.hpp"
#include "windows/DayEventsWindow.hpp"
#include "windows/CalendarEventsOptionsWindow.hpp"
#include "windows/AllEventsWindow.hpp"
#include "windows/EventDetailWindow.hpp"
#include "windows/NewEditEventWindow.hpp"
#include "windows/CustomRepeatWindow.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include "NoEvents.hpp"
#include "Dialog.hpp"
#include <time/time_conversion.hpp>
#include <module-services/service-db/api/DBServiceAPI.hpp>
#include <module-db/queries/calendar/QueryEventsGet.hpp>
#include <module-db/queries/calendar/QueryEventsAdd.hpp>
#include <module-db/queries/calendar/QueryEventsEdit.hpp>
#include <module-db/queries/calendar/QueryEventsGetAll.hpp>
#include <module-db/queries/calendar/QueryEventsRemove.hpp>
#include <module-db/queries/calendar/QueryEventsGetFiltered.hpp>
#include <module-services/service-db/messages/QueryMessage.hpp>
#include <messages/QueryMessage.hpp>
#include <map>

namespace app
{

    ApplicationCalendar::ApplicationCalendar(std::string name,
                                             std::string parent,
                                             uint32_t stackDepth,
                                             sys::ServicePriority priority)
        : Application(name, parent, false, stackDepth, priority)
    {}

    sys::Message_t ApplicationCalendar::DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp)
    {
        auto retMsg = Application::DataReceivedHandler(msgl);
        // if message was handled by application's template there is no need to process further.
        if (reinterpret_cast<sys::ResponseMessage *>(retMsg.get())->retCode == sys::ReturnCodes::Success) {
            return retMsg;
        }
        // this variable defines whether message was processed.
        bool handled = false;
        // handle database response
        if (resp != nullptr) {
            handled = true;
            switch (resp->responseTo) {
            case MessageType::DBQuery: {
                if (getCurrentWindow()->onDatabaseMessage(resp))
                    refreshWindow(gui::RefreshModes::GUI_REFRESH_FAST);
            } break;
            default:
                break;
            }
        }
        if (handled) {
            return std::make_shared<sys::ResponseMessage>();
        }
        else {
            return std::make_shared<sys::ResponseMessage>(sys::ReturnCodes::Unresolved);
        }
    }

    sys::ReturnCodes ApplicationCalendar::InitHandler()
    {
        auto timestamp       = new utils::time::Timestamp();
        applicationStartTime = timestamp->getTime();
        auto ret             = Application::InitHandler();
        // EventsRecord event(EventsTableRow{{1}, "TEST 19.08", "TEST 19.08", 2008190142, 2008190153, 1, 2, 1});
        // EventsRecord event2(EventsTableRow{{2}, "TEST2 15.08", "TEST2 15.08", 2008150152, 2008150163, 1, 2, 1});
        // DBServiceAPI::GetQuery(this, db::Interface::Name::Events, std::make_unique<db::query::events::Add>(event));
        // DBServiceAPI::GetQuery(this, db::Interface::Name::Events, std::make_unique<db::query::events::Add>(event2));
        createUserInterface();
        return ret;
    }

    sys::ReturnCodes ApplicationCalendar::DeinitHandler()
    {
        return sys::ReturnCodes::Success;
    }

    void ApplicationCalendar::createUserInterface()
    {

        windows.insert(std::pair<std::string, gui::AppWindow *>(
            gui::name::window::main_window, new gui::CalendarMainWindow(this, gui::name::window::main_window)));
        windows.insert(std::pair<std::string, gui::AppWindow *>(
            style::window::calendar::name::day_events_window,
            new gui::DayEventsWindow(this, style::window::calendar::name::day_events_window)));
        windows.insert(std::pair<std::string, gui::AppWindow *>(
            style::window::calendar::name::no_events_window,
            new gui::NoEvents(this, style::window::calendar::name::no_events_window, gui::NoEvents::Meta())));
        windows.insert(std::pair<std::string, gui::AppWindow *>(style::window::calendar::name::events_options,
                                                                new gui::CalendarEventsOptions(this)));
        windows.insert(std::pair<std::string, gui::AppWindow *>(
            style::window::calendar::name::dialog_yes_no,
            new gui::DialogYesNo(this, style::window::calendar::name::dialog_yes_no)));
        windows.insert(std::pair<std::string, gui::AppWindow *>(
            style::window::calendar::name::all_events_window,
            new gui::AllEventsWindow(this, style::window::calendar::name::all_events_window)));
        windows.insert(std::pair<std::string, gui::AppWindow *>(
            style::window::calendar::name::details_window,
            new gui::EventDetailWindow(this, style::window::calendar::name::details_window)));
        windows.insert(std::pair<std::string, gui::AppWindow *>(
            style::window::calendar::name::new_edit_event,
            new gui::NewEditEventWindow(this, style::window::calendar::name::new_edit_event)));
        windows.insert(std::pair<std::string, gui::AppWindow *>(
            style::window::calendar::name::custom_repeat_window,
            new gui::CustomRepeatWindow(this, style::window::calendar::name::custom_repeat_window)));
    }

    void ApplicationCalendar::destroyUserInterface()
    {}

    void ApplicationCalendar::switchToNoEventsWindow(const std::string &title)
    {
        auto dialog = dynamic_cast<gui::NoEvents *>(getWindow(style::window::calendar::name::no_events_window));
        assert(dialog != nullptr);
        auto meta   = dialog->meta;
        meta.text   = "app_calendar_no_events_information";
        meta.title  = title;
        meta.icon   = "phonebook_empty_grey_circle_W_G";
        meta.action = [=]() -> bool {
            LOG_DEBUG("Switch to new event window");
            std::unique_ptr<gui::SwitchData> data = std::make_unique<gui::SwitchData>();
            data->setDescription("New");
            switchWindow(style::window::calendar::name::new_edit_event, gui::ShowMode::GUI_SHOW_INIT, std::move(data));
            return true;
        };
        dialog->update(meta);
        switchWindow(dialog->getName());
        LOG_DEBUG("Switch to no events window");
    }

} /* namespace app */
