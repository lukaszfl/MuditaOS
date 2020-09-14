#include "SpecialInputModel.hpp"
#include "module-apps/application-special-input/widgets/SpecialInputTableWidget.hpp"
#include <ListView.hpp>

SpecialInputModel::SpecialInputModel(app::Application *app) : application(app)
{}

auto SpecialInputModel::requestRecordsCount() -> unsigned int
{
    return internalData.size();
}

auto SpecialInputModel::getMinimalItemHeight() const -> unsigned int
{
    return specialInputStyle::specialCharacterTableWidget::window_grid_h;
}

void SpecialInputModel::requestRecords(const uint32_t offset, const uint32_t limit)
{
    setupModel(offset, limit);
    list->onProviderDataUpdate();
}

auto SpecialInputModel::getItem(gui::Order order) -> gui::ListItem *
{
    return getRecord(order);
}
void SpecialInputModel::buildGrids(const std::vector<char32_t> elements)
{
    int offset        = 0;
    int elements_left = elements.size();
    int max_elements  = (specialInputStyle::specialCharacterTableWidget::window_grid_w /
                        specialInputStyle::specialCharacterTableWidget::char_grid_w) *
                       (specialInputStyle::specialCharacterTableWidget::window_grid_h /
                        specialInputStyle::specialCharacterTableWidget::char_grid_h);
    int last, last2, chars_number, start, i, end;
    while (offset < (int)elements.size()) {
        chars_number = std::min(elements_left, max_elements);
        start        = offset;
        i            = 0;
        if (elements_left < max_elements) {
            last = 0;
        }
        else {
            last = 1;
            if (offset < max_elements && elements[0] == U'.') {
                i = 1;
            }
        }
        if (((int)elements_left / (int)max_elements - 1) < 0) {
            last2 = 0;
        }
        else {
            last2 = (elements_left / max_elements - 1);
        }
        end    = elements.size() - (last * (elements.size() % max_elements) + last2 * max_elements + i);
        auto g = new gui::SpecialInputTableWidget(application, start, end, elements);
        internalData.push_back(g);
        offset        = end;
        elements_left = elements_left - chars_number;
    }
}

void SpecialInputModel::createData(specialInputStyle::CharactersType type)
{
    if (type == specialInputStyle::CharactersType::SpecialCharacters) {
        buildGrids(specialInputStyle::special_chars);
        for (auto item1 : internalData) {
            item1->deleteByList = false;
        }
        list->rebuildList();
    }
    else if (type == specialInputStyle::CharactersType::Emoji) {
        buildGrids(specialInputStyle::emojis);
        for (auto item1 : internalData) {
            item1->deleteByList = false;
        }
        list->rebuildList();
    }
}

void SpecialInputModel::clearData()
{
    list->clear();
    eraseInternalData();
}
