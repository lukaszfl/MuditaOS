#include "RawText.hpp"
#include "../core/Font.hpp"
#include "../core/DrawCommand.hpp"
#include "log/log.hpp"
#include "utf8/UTF8.hpp"
#include <Style.hpp>

namespace gui
{

    void RawText::setText(const UTF8 text)
    {
        this->text = text;
        area().w   = font->getPixelWidth(text);
        area().h   = font->info.line_height;
    }

    std::string fullFontName(std::string name, unsigned int size, RawText::Weight weight)
    {
        return name + "_" + c_str(weight) + "_" + std::to_string(size);
    }

    void RawText::setFont(std::string name, unsigned int size, Weight weight)
    {
        auto font_name = fullFontName(name, size, weight);
        LOG_INFO("-----> font set to: %s <-----", font_name.c_str());
        font = FontManager::getInstance().getFont(font_name);
    }

    std::list<DrawCommand *> RawText::buildDrawList()
    {
        std::list<DrawCommand *> commands;
        if (visible == false) {
            return commands;
        }
        std::list<DrawCommand *> commandsBase;
        if (font) {
            CommandText *textCmd = new CommandText();
            textCmd->str         = text;
            textCmd->fontID      = font->id;
            textCmd->color       = color;

            textCmd->x     = drawArea.x;
            textCmd->y     = drawArea.y;
            textCmd->w     = drawArea.w;
            textCmd->h     = drawArea.h;
            textCmd->tx    = 0;
            textCmd->ty    = font->info.base;
            textCmd->tw    = drawArea.w;
            textCmd->th    = drawArea.h;
            textCmd->areaX = widgetArea.x;
            textCmd->areaY = widgetArea.y;
            textCmd->areaW = widgetArea.w;
            textCmd->areaH = widgetArea.h;
            commands.push_back(textCmd);
        }
        return commands;
    }
} // namespace gui
