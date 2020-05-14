#pragma once

#include "Item.hpp"
#include "../core/Font.hpp"

namespace gui
{

    class RawText : public Item
    {
        Color color = {0, 0};
        Font *font  = nullptr;
        UTF8 text   = "";

      public:
        enum class Weight
        {
            bold,
            light,
            regular
        } weight;
        void setFont(std::string name, unsigned int size, Weight weight = Weight::regular);
        std::list<DrawCommand *> buildDrawList() override;
        // sets text to `text` and size to render width/length
        void setText(UTF8 text);
    };
} // namespace gui

// TODO move to Font...
inline const char *c_str(enum gui::RawText::Weight weight)
{
    switch (weight) {
    case gui::RawText::Weight::bold:
        return "bold";
    case gui::RawText::Weight::light:
        return "light";
    case gui::RawText::Weight::regular:
        return "regular";
    }
    return "";
};
