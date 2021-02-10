// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

namespace purefs
{
    template <typename... Args> class Event
    {
        std::function<void(const Args &...)> hnd;

      public:
        template <typename... CallArgs> void operator()(CallArgs &&... args) const
        {
            if (hnd)
                hnd(std::forward<CallArgs>(args)...);
            else
                std::cout << "Notification not handled" << std::endl;
        }
        template <typename H> void registerHandler(H handler)
        {
            hnd = handler;
        }
        void unregisterHandler()
        {
            registerHandler(nullptr);
        }
    };
} // namespace purefs
