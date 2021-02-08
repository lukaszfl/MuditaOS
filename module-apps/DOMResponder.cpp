// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "DOMResponder.hpp"
#include <service-appmgr/service-appmgr/messages/DOMRequest.hpp>
#include <module-gui/gui/dom/Item2JsonSerializer.hpp>
#include <memory>
#include <Item.hpp>
#include <module-utils/time/ScopedTime.hpp>

namespace app
{
    DOMResponder::DOMResponder(const std::string &name, gui::Item &item) : name(name), item(item)
    {}

    [[nodiscard]] auto DOMResponder::build() -> std::shared_ptr<sys::Message>
    {
        auto t          = utils::time::Scoped("Time to build dom");
        auto serializer = gui::Item2JsonSerializer();
        serializer.traverse(item);
        return std::make_shared<manager::DOMResponse>(name, serializer.get());
    }
} // namespace app
