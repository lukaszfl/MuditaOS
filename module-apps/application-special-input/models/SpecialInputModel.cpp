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

auto SpecialInputModel::getMaxGridElements() const -> ssize_t
{
    return 
    (specialInputStyle::specialCharacterTableWidget::window_grid_w /
                        specialInputStyle::specialCharacterTableWidget::char_grid_w) *
                       (specialInputStyle::specialCharacterTableWidget::window_grid_h /
                        specialInputStyle::specialCharacterTableWidget::char_grid_h) - 1;
}

void SpecialInputModel::buildGrids(std::vector<char32_t> elements)
{
    auto recursive_algorithm_max = 10;
    auto add_start = elements.begin();
    while (add_start != std::end(elements)) {
        auto carrier_size = std::min(std::distance(add_start, elements.end()), getMaxGridElements());
        std::list<gui::Carier> carriers;
        for (auto val : std::list(add_start, add_start + carrier_size)) {
            if ( val == specialInputStyle::symbol_for_newline ) 
            {
                carriers.push_back(gui::generateNewLineSign());
            }
            carriers.push_back(gui::generateCharSign(val));
        }
        add_start = std::next(add_start,carriers.size());
        internalData.push_back(new gui::SpecialInputTableWidget(application, std::move(carriers)));

        if ( recursive_algorithm_max-- == 0 ) 
        {
            LOG_ERROR("Recursive algorithm failure!");
            break;
        }
    }
}

void SpecialInputModel::createData(specialInputStyle::CharactersType type)
{
    auto create = [&](auto &vector) {
        buildGrids(vector);
        for (auto &dataitem : internalData) {
            dataitem->deleteByList = false;
        }
        list->rebuildList();
    };

    switch (type) {
    case specialInputStyle::CharactersType::SpecialCharacters:
        create(specialInputStyle::special_chars );
        break;
    case specialInputStyle::CharactersType::Emoji:
        create(specialInputStyle::emojis);
        break;
    }
}

void SpecialInputModel::clearData()
{
    list->clear();
    eraseInternalData();
}
