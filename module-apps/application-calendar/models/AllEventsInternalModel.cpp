#include "AllEventsInternalModel.hpp"
#include "application-calendar/widgets/AllEventsItem.hpp"
#include "application-calendar/data/CalendarData.hpp"
#include <ListView.hpp>
#include <Utils.hpp>
#include <algorithm>

AllEventsInternalModel::AllEventsInternalModel(app::Application *app) : application(app)
{}

unsigned int AllEventsInternalModel::requestRecordsCount()
{
    return internalData.size();
}

unsigned int AllEventsInternalModel::getMinimalItemHeight() const
{
    return style::window::calendar::item::dayEvents::height;
}

void AllEventsInternalModel::requestRecords(const uint32_t offset, const uint32_t limit)
{
    setupModel(offset, limit);
    list->onProviderDataUpdate();
}

gui::ListItem *AllEventsInternalModel::getItem(gui::Order order)
{
    return getRecord(order);
}

void AllEventsInternalModel::loadData(std::unique_ptr<std::vector<EventsRecord>> records)
{
    auto app = application;
    assert(app != nullptr);

    // internalData.resize(records->size());

    list->clear();
    eraseInternalData();

    std::sort(records->begin(), records->end(), [](const EventsRecord &first, const EventsRecord &second) {
        return first.date_from < second.date_from;
    });

    for (auto &record : *records) {
        auto item = new gui::AllEventsItem();
        item->setEvent(std::make_shared<EventsRecord>(record));
        item->activatedCallback = [=](gui::Item &item) {
            LOG_INFO("Switch to event detail window");
            auto rec  = std::make_unique<EventsRecord>(record);
            auto data = std::make_unique<EventRecordData>(std::move(rec));
            app->switchWindow(style::window::calendar::name::details_window, std::move(data));
            return true;
        };
        internalData.push_back(item);
    }

    for (auto &item : internalData) {
        item->deleteByList = false;
    }

    list->rebuildList();
}
