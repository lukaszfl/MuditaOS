#include "TextLine.hpp"
#include "Common.hpp"
#include "Label.hpp"
#include "TextBlock.hpp"
#include <cstdio>
#include <RawFont.hpp>
#include "TextCursor.hpp"
#include "log/log.hpp"

namespace gui
{

    /// helper function to get our text representation
    Label *buildUITextPart(const UTF8 &text, const TextFormat *format)
    {
        auto item = new gui::Label(nullptr);
        item->setText(text);
        item->setFont(format->getFont());
        item->setTextColor(format->getColor());
        item->setSize(item->getTextNeedSpace(), item->getTextHeight());
        item->setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        return item;
    }

    /// Note - line breaking could be done here with different TextLines to return
    /// or via different block types (i.e. numeric block tyle could be not "breakable"
    TextLine::TextLine(TextCursor &cursor, unsigned int max_width)
    {
        LOG_DEBUG("TextLine");
        auto local_cursor = cursor;
        LOG_DEBUG("Draw line from cursor: %s", local_cursor.str().c_str());
        int i =0;
        do {
            LOG_DEBUG("+ + + + + + + + + + pass %d", i++);
            if(!local_cursor) 
            {
                LOG_DEBUG("invalid local cursor");
                return;
            }

            if ( local_cursor.atEol()) 
            {
                LOG_DEBUG("reached end of line");
                width_used = max_width;
                return;
            }

            if (local_cursor.atEnd()) {
                LOG_DEBUG("document end reached");
                return;
            }

            // get curent block format
            auto text_format = local_cursor->getFormat();
            if (text_format == nullptr || !text_format->isValid()) {
                LOG_DEBUG("invalid text format");
                return;
            }

            // get curent block text
            std::string raw_text(local_cursor);
            if( raw_text.length() == 0 )
            {
                LOG_DEBUG("no more text");
                return;
            }

            LOG_INFO("Drawing: %s", raw_text.c_str());

            // calculate how much can we show
            auto can_show = text_format->getFont()->getCharCountInSpace(raw_text, max_width - width_used);
            if (can_show == 0) {
                return;
            }

            // create item for show and update Line data
            auto item = buildUITextPart(raw_text.substr(0, can_show), text_format);
            number_letters_shown += can_show;
            width_used += item->getTextNeedSpace();
            height_used = std::max(height_used, item->getTextHeight());
            elements_to_show_in_line.emplace_back(item);

            // not whole text shown, try again for next line if you want
            if (can_show < raw_text.length()) {
                return;
            }

            local_cursor += number_letters_shown;
        } while (true);
    }

    TextLine::TextLine(TextLine &&from)
    {
        elements_to_show_in_line = std::move(from.elements_to_show_in_line);
        number_letters_shown     = from.number_letters_shown;
        width_used               = from.width_used;
        height_used              = from.height_used;
    }

    TextLine::~TextLine()
    {
        for (auto &el : elements_to_show_in_line) {
            if (el->parent == nullptr) {
                delete el;
            }
        }
    }

    /// class to disown Item temporary to ignore callback
    class ScopedParentDisown
    {
        Item *parent = nullptr;
        Item *item   = nullptr;

      public:
        ScopedParentDisown(Item *it) : item(it)
        {
            if (item != nullptr) {
                parent = item->parent;
            }
        }

        ~ScopedParentDisown()
        {
            if (item != nullptr) {
                item->parent = parent;
            }
        }
    };

    void TextLine::setPosition(int32_t x, int32_t y)
    {
        auto line_x_position = x;
        for (auto &el : elements_to_show_in_line) {
            auto scoped_disown          = ScopedParentDisown(el);
            int32_t align_bottom_offset = height() - el->getHeight();
            el->setArea({line_x_position, y + align_bottom_offset, el->getWidth(), el->getHeight()});
            line_x_position += el->getWidth();
        }
    }

    void TextLine::setParent(Item *parent)
    {
        if (parent == nullptr) {
            return;
        }
        for (auto &el : elements_to_show_in_line) {
            parent->addWidget(el);
        }
    }

    Length TextLine::getWidth() const
    {
        Length width = 0;
        for (auto &line : elements_to_show_in_line) {
            width += line->getWidth();
        }
        return width;
    }

    uint32_t TextLine::getWidthTo(unsigned int pos) const
    {
        uint32_t width  = 0;
        auto curent_pos = 0;
        if (pos == text::npos) {
            return 0;
        }
        for (auto &el : elements_to_show_in_line) {
            if (el->getFont() == nullptr) {
                continue;
            }
            if (curent_pos + el->getTextLength() > pos) {
                width += el->getFont()->getPixelWidth(el->getText(), 0, pos - curent_pos);
                return width;
            }
            else {
                width += el->getWidth();
            }
            curent_pos += el->getTextLength();
        }
        return width;
    }

    void TextLine::erase()
    {
        for (auto &el : elements_to_show_in_line) {
            if (el->parent != nullptr) {
                auto p = el->parent;
                p->erase(el);
            }
            else {
                delete el;
            }
        }
        elements_to_show_in_line.clear();
    }

    void TextLine::align(Alignment &line_align, Length parent_length)
    {
        Length width = getWidth();
        if (parent_length <= width) {
            return;
        }
        Length offset = 0;
        if (line_align.horizontal == Alignment::Horizontal::Right) {
            offset = parent_length - width;
        }
        else if (line_align.horizontal == Alignment::Horizontal::Center) {
            offset = (parent_length - width) / 2;
        }
        for (auto &el : elements_to_show_in_line) {
            auto scoped_disown = ScopedParentDisown(el);
            el->setPosition(el->getPosition(Axis::X) + offset, Axis::X);
        }
    }
} // namespace gui
