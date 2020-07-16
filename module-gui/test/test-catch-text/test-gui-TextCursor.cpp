#include <catch2/catch.hpp>
#include <TextCursor.hpp>
#include <Text.hpp>

TEST_CASE("str()")
{
    using namespace gui;
    Text text;
    auto cur = new TextCursor(&text);
    REQUIRE(cur->str() != "");
}

TEST_CASE("getPosOnScreen")
{
    using namespace gui;
    Text text;
    auto cur = new TextCursor(&text);
    REQUIRE(cur->getPosOnScreen() == 0);
}

TEST_CASE("setToStart")
{
    using namespace gui;
    Text text;
    auto cur = new TextCursor(&text);
    REQUIRE(cur->getPosOnScreen() == 0);
    REQUIRE(text.getCursor()->getPosOnScreen() == 0);

    *cur << TextBlock("any text", nullptr, TextBlock::End::Newline);
    *cur << TextBlock("any text", nullptr, TextBlock::End::Newline);
    REQUIRE(cur->getPosOnScreen() != 0);
    cur->setToStart();
    REQUIRE(cur->getPosOnScreen() == 0);

    REQUIRE( text.getText().length() > 0);

    text.addText(TextBlock("any text", nullptr, TextBlock::End::Newline));
    text.addText(TextBlock("any text", nullptr, TextBlock::End::Newline));
    REQUIRE(cur->getPosOnScreen() == 0); // our cursor didn't change positon
    REQUIRE(text.getCursor()->getPosOnScreen() != 0); // but text cursor did
    REQUIRE(text.getCursor()->getPosOnScreen() != cur->getPosOnScreen()); // but text cursor did

    *cur += 100;
    cur->setToStart();
    REQUIRE(cur->getPosOnScreen() == 0);
}
