#include "NewEditEventModel.hpp"
#include "application-calendar/widgets/CheckBoxWithLabelAndModel.hpp"
#include "application-calendar/widgets/AddRepeatedEvents.hpp"
#include "application-calendar/models/MonthModel.hpp"
#include <ListView.hpp>
#include <Utils.hpp>
#include <BottomBar.hpp>
#include <module-services/service-db/api/DBServiceAPI.hpp>
#include <module-db/queries/calendar/QueryEventsAdd.hpp>
#include <module-db/queries/calendar/QueryEventsEdit.hpp>
#include <module-apps/application-calendar/data/CalendarData.hpp>

NewEditEventModel::NewEditEventModel(app::Application *app, bool mode24H) : application(app), mode24H(mode24H)
{}

unsigned int NewEditEventModel::requestRecordsCount()
{
    return internalData.size();
}

unsigned int NewEditEventModel::getMinimalItemHeight() const
{
    return style::window::calendar::item::checkBox::height;
}

void NewEditEventModel::requestRecords(const uint32_t offset, const uint32_t limit)
{
    setupModel(offset, limit);
    list->onProviderDataUpdate();
}

gui::ListItem *NewEditEventModel::getItem(gui::Order order)
{
    return getRecord(order);
}

void NewEditEventModel::createData(bool allDayEvent)
{
    auto app = application;
    assert(app != nullptr);

    eventNameInput = new gui::TextWithLabelItem(
        utils::localize.get("app_calendar_new_edit_event_name"),
        [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text); },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); },
        [app]() { app->getCurrentWindow()->selectSpecialCharacter(); });

    allDayEventCheckBox = new gui::CheckBoxWithLabelAndModel(
        application, utils::localize.get("app_calendar_new_edit_event_allday"), true, this);

    startTime = new gui::EventTimeItem(
        utils::localize.get("app_calendar_new_edit_event_start"),
        mode24H,
        [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); });

    endTime = new gui::EventTimeItem(
        utils::localize.get("app_calendar_new_edit_event_end"),
        mode24H,
        [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); });

    reminder = new gui::SeveralOptionsItem(
        application,
        utils::localize.get("app_calendar_event_detail_reminder"),
        [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); });

    repeat = new gui::SeveralOptionsItem(
        application,
        utils::localize.get("app_calendar_event_detail_repeat"),
        [app](const UTF8 &text) { app->getCurrentWindow()->bottomBarTemporaryMode(text, false); },
        [app]() { app->getCurrentWindow()->bottomBarRestoreFromTemporaryMode(); });

    endTime->setConnectionToSecondItem(startTime);
    startTime->setConnectionToSecondItem(endTime);

    internalData.push_back(eventNameInput);
    internalData.push_back(allDayEventCheckBox);
    if (!allDayEvent) {
        internalData.push_back(startTime);
        internalData.push_back(endTime);
    }
    internalData.push_back(reminder);
    internalData.push_back(repeat);

    for (auto &item : internalData) {
        item->deleteByList = false;
    }
}

void NewEditEventModel::loadData(std::shared_ptr<EventsRecord> record)
{
    list->clear();
    eraseInternalData();
    auto isAllDayEvent = [&]() -> bool { return record->date_from % 10000 == 0 && record->date_till % 10000 == 2359; };

    createData(isAllDayEvent());

    for (auto &item : internalData) {
        if (item->onLoadCallback) {
            item->onLoadCallback(record);
        }
    }

    list->rebuildList();
}

void NewEditEventModel::loadDataWithoutTimeItem()
{
    internalData.erase(std::find(internalData.begin(), internalData.end(), startTime));
    internalData.erase(std::find(internalData.begin(), internalData.end(), endTime));
    list->rebuildList();
}

void NewEditEventModel::reloadDataWithTimeItem()
{
    internalData.clear();

    internalData.push_back(eventNameInput);
    internalData.push_back(allDayEventCheckBox);
    internalData.push_back(startTime);
    internalData.push_back(endTime);
    internalData.push_back(reminder);
    internalData.push_back(repeat);

    for (auto &item : internalData) {
        item->deleteByList = false;
    }

    list->rebuildList();
}

void NewEditEventModel::saveData(std::shared_ptr<EventsRecord> event, bool edit, const std::string &windowName)
{
    for (auto &item : internalData) {
        if (item->onSaveCallback) {
            item->onSaveCallback(event);
        }
    }
    ////event->repeat = repeat->dataFromRecord
    // event->repeat = repeat->getRepeatOptionValue();
    // TODO: Change bool edit to existing enum
    if (edit) {
        auto record = event.get();
        if (record->title != "") {
            DBServiceAPI::GetQuery(application,
                                   db::Interface::Name::Events,
                                   std::make_unique<db::query::events::Edit>(*record, record->date_from));
            if (event->repeat) {
                auto repeatObject = new AddRepeatedEvents(application);
                repeatObject->addRepeatedEvents(std::move(event));
            }
        }
        application->switchWindow(windowName);
    }
    else {
        auto record = event.get();
        if (record->title != "") {
            DBServiceAPI::GetQuery(
                application, db::Interface::Name::Events, std::make_unique<db::query::events::Add>(*record));
            if (event->repeat) {
                auto repeatObject = new AddRepeatedEvents(application);
                repeatObject->addRepeatedEvents(std::move(event));
            }
            if (application->getPrevWindow() == style::window::calendar::name::no_events_window) {
                // TODO: Create method in month model to convert filter to date time
                auto data         = std::make_unique<DayMonthData>();
                auto filter       = record->date_from - record->date_from % 10000;
                uint32_t yearInt  = record->date_from - record->date_from % 100000000;
                auto year         = date::year((yearInt + 2000) / 100000000);
                uint32_t monthInt = record->date_from - record->date_from % 1000000 - yearInt;
                auto month        = date::month(monthInt / 1000000);
                auto dayInt       = (filter - yearInt - monthInt) / 10000;
                auto day          = date::day(dayInt);
                auto monthModel   = std::make_unique<MonthModel>(year / month / day);
                auto monthTxt     = monthModel->getMonthText();
                auto dayTxt       = std::to_string(dayInt);
                data->setData(monthTxt + " " + dayTxt, filter);

                application->switchWindow(gui::name::window::main_window);
                application->switchWindow(windowName, std::move(data));
            }
            else {
                application->returnToPreviousWindow();
            }
        }
        else {
            application->returnToPreviousWindow();
        }
    }
}
