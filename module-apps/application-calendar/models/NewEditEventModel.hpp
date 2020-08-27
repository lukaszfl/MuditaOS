#pragma once
#include "Application.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include "application-calendar/widgets/CalendarListItem.hpp"
#include "application-calendar/widgets/TextWithLabelItem.hpp"
#include "application-calendar/widgets/EventTimeItem.hpp"
#include "application-calendar/widgets/SeveralOptionsItem.hpp"
#include "application-calendar/widgets/CheckBoxWithLabelItem.hpp"
#include "InternalModel.hpp"
#include <ListItemProvider.hpp>

class NewEditEventModel : public app::InternalModel<gui::CalendarListItem *>, public gui::ListItemProvider
{
    app::Application *application = nullptr;
    bool mode24H                  = false;

    gui::TextWithLabelItem *eventNameInput          = nullptr;
    gui::CheckBoxWithLabelItem *allDayEventCheckBox = nullptr;
    gui::EventTimeItem *startTime                   = nullptr;
    gui::EventTimeItem *endTime                     = nullptr;
    gui::SeveralOptionsItem *reminder               = nullptr;
    gui::SeveralOptionsItem *repeat                 = nullptr;

  public:
    NewEditEventModel(app::Application *app, bool mode24H = false);

    void loadData(std::shared_ptr<EventsRecord> record);
    void loadDataWithoutTimeItem();
    void reloadDataWithTimeItem();
    void saveData(std::shared_ptr<EventsRecord> event, bool edit, const std::string &windowName);
    void addReapetedRecords(std::shared_ptr<EventsRecord> event);

    [[nodiscard]] unsigned int getMinimalItemHeight() const override;
    [[nodiscard]] unsigned int requestRecordsCount() override;
    gui::ListItem *getItem(gui::Order order) override;
    void requestRecords(const uint32_t offset, const uint32_t limit) override;

    uint32_t getRepeatOptionValue()
    {
        if (repeat != nullptr) {
            return repeat->repeatOptionValue;
        }
        else {
            LOG_ERROR("Repeat option value returned before assigmend!");
            return 0;
        }
    }
    void setRepeatOptionValue(const uint32_t &value)
    {
        if (repeat != nullptr) {
            repeat->repeatOptionValue = value;
        }
        else {
            LOG_ERROR("Value not assigned. Repeat option item was not ready!");
        }
    }

  private:
    void createData(bool allDayEvent);
};
