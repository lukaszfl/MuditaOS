// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "windows/AppWindow.hpp"
#include "Application.hpp"
#include "application-calculator/widgets/MathOperationsBox.hpp"
#include <module-gui/gui/widgets/BoxLayout.hpp>
#include <module-gui/gui/widgets/Image.hpp>
#include <module-gui/gui/widgets/TextFixedSize.hpp>
#include <module-utils/tinyexpr/tinyexpr.h>

namespace gui
{

    class CalculatorMainWindow : public gui::AppWindow
    {
        gui::HBox *inputBox                    = nullptr;
        gui::Image *ellipsis                   = nullptr;
        gui::TextFixedSize *mathOperationInput = nullptr;
        gui::MathOperationsBox *mathBox        = nullptr;

        bool clearInput = false;
        void writeEquation(bool lastCharIsSymbol, const UTF8 &symbol);
        void applyInputCallback();
        bool isPreviousNumberDecimal();
        bool isSymbol(uint32_t character);
        bool isDecimalSeparator(uint32_t character);
        uint32_t getPenultimate();
        void showEllipsis();
        void hideEllipsis();

      public:
        CalculatorMainWindow(app::Application *app, std::string name);

        ~CalculatorMainWindow() override = default;
        void buildInterface() override;
        bool onInput(const gui::InputEvent &inputEvent) override;
    };

} // namespace gui
