// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <Service/MessageForward.hpp>
#include <string>

namespace gui
{
    class Item;
}

namespace app
{
    class DOMResponder
    {
        std::string name;
        gui::Item &item;

      public:
        DOMResponder(const std::string &, gui::Item &);
        [[nodiscard]] auto build() -> std::shared_ptr<sys::Message>;
    };

}; // namespace app
