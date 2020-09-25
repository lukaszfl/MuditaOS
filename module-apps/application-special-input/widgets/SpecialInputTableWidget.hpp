#pragma once

#include <ListItem.hpp>
#include <Text.hpp>
#include <TextFixedSize.hpp>
#include <ImageBox.hpp>
#include <module-gui/gui/widgets/GridLayout.hpp>
#include <module-apps/Application.hpp>
#include <module-apps/application-special-input/data/SpecialCharactersTableStyle.hpp>
#include <module-apps/application-special-input/windows/SpecialInputMainWindow.hpp>

namespace gui
{

    struct Carier
    {
        gui::Item* item = nullptr;
        std::string val;
    };

    auto generateNewLineSign() -> Carier;
    auto generateCharSign(uint32_t val) -> Carier;

    class SpecialInputTableWidget : public ListItem
    {

      public:
        SpecialInputTableWidget(app::Application *app, std::list<Carier> &&carier);
        auto onDimensionChanged(const BoundingBox &oldDim, const BoundingBox &newDim) -> bool override;
        void decorateActionActivated(Item *it, const std::string &str);
        GridLayout *box               = nullptr;
        app::Application *application = nullptr;
    };

} /* namespace gui */
