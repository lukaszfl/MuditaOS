#include <catch2/catch.hpp>
#include <TextFormat.hpp>
#include <FontManager.hpp>
#include <Font.hpp>

TEST_CASE("ifValid()")
{
    auto &fm = gui::FontManager::getInstance();
    if (fm.isInitialized() == false) {
        fm.init("assets");
    }
    gui::TextFormat format(nullptr);
    REQUIRE(format.isValid() == false);
    format.setFont(fm.getFont("default"));
    REQUIRE(format.isValid() == true);
}
