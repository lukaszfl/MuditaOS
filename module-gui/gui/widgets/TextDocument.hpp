#pragma once

#include <list>
#include <utility>
#include "TextBlock.hpp"
#include "TextBlockCursor.hpp"

namespace gui
{
    class TextLine;

    class TextDocument
    {
        friend BlockCursor;

        static const std::string newline;
        std::list<TextBlock> blocks;

      public:
        TextDocument(const std::list<TextBlock> blocks);
        ~TextDocument();
        void destroy();

        void append(std::list<TextBlock> &&blocks);
        void append(TextBlock &&text);
        void addNewline(BlockCursor &cursor, TextBlock::End eol);
        [[nodiscard]] auto getText() const -> UTF8;

        /// --- in progress
        BlockCursor getBlockCursor(unsigned int position);
        /// get part of TextBlock based on cursor

        /// needed for tests, alternatively could be mocked in test...
        [[nodiscard]] const std::list<TextBlock> &getBlocks() const;
        [[nodiscard]] const TextBlock *getBlock(BlockCursor *cursor) const;

        const TextBlock &operator()(const BlockCursor &cursor) const;
        void removeBlock(unsigned int block_nr);
        void removeBlock(std::list<TextBlock>::iterator it);
        // TODO this is very unoptimal...
        bool isEmpty() const
        {
            return getText().length() == 0;
        }

      private:
        /// splits text block in document and returns two new blocks (in place of last one)
        auto split(BlockCursor &cursor) -> std::pair<TextBlock &, TextBlock &>;
    };
}; // namespace gui
