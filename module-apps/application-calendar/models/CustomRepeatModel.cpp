#include "CustomRepeatModel.hpp"
#include "application-calendar/widgets/CheckBoxWithLabelItem.hpp"
#include <ListView.hpp>
#include <Utils.hpp>

CustomRepeatModel::CustomRepeatModel(app::Application *app) : application(app)
{}

unsigned int CustomRepeatModel::requestRecordsCount()
{
    return internalData.size();
}

unsigned int CustomRepeatModel::getMinimalItemHeight() const
{
    return style::window::calendar::item::checkBox::height;
}

void CustomRepeatModel::requestRecords(const uint32_t offset, const uint32_t limit)
{
    setupModel(offset, limit);
    list->onProviderDataUpdate();
}

gui::ListItem *CustomRepeatModel::getItem(gui::Order order)
{
    return getRecord(order);
}

void CustomRepeatModel::createData(const std::shared_ptr<WeekDaysRepeatData> &data)
{
    auto monday  = new gui::CheckBoxWithLabelItem(application, utils::localize.get(style::strings::common::Monday));
    auto tuesday = new gui::CheckBoxWithLabelItem(application, utils::localize.get(style::strings::common::Tuesday));
    auto wednesday =
        new gui::CheckBoxWithLabelItem(application, utils::localize.get(style::strings::common::Wednesday));
    auto thursday = new gui::CheckBoxWithLabelItem(application, utils::localize.get(style::strings::common::Thursday));
    auto friday   = new gui::CheckBoxWithLabelItem(application, utils::localize.get(style::strings::common::Friday));
    auto saturday = new gui::CheckBoxWithLabelItem(application, utils::localize.get(style::strings::common::Saturday));
    auto sunday   = new gui::CheckBoxWithLabelItem(application, utils::localize.get(style::strings::common::Sunday));

    monday->checkBox->setImageVisible(data->getData(date::Monday.iso_encoding() - 1));
    tuesday->checkBox->setImageVisible(data->getData(date::Tuesday.iso_encoding() - 1));
    wednesday->checkBox->setImageVisible(data->getData(date::Wednesday.iso_encoding() - 1));
    thursday->checkBox->setImageVisible(data->getData(date::Thursday.iso_encoding() - 1));
    friday->checkBox->setImageVisible(data->getData(date::Friday.iso_encoding() - 1));
    saturday->checkBox->setImageVisible(data->getData(date::Saturday.iso_encoding() - 1));
    sunday->checkBox->setImageVisible(data->getData(date::Sunday.iso_encoding() - 1));

    internalData.push_back(monday);
    internalData.push_back(tuesday);
    internalData.push_back(wednesday);
    internalData.push_back(thursday);
    internalData.push_back(friday);
    internalData.push_back(saturday);
    internalData.push_back(sunday);

    for (auto &item : internalData) {
        item->deleteByList = false;
    }
}

void CustomRepeatModel::loadData(const std::shared_ptr<WeekDaysRepeatData> &data)
{
    list->clear();
    eraseInternalData();

    createData(data);

    list->rebuildList();
}
