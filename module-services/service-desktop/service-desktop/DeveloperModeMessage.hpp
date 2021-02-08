// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <service-desktop/endpoints/Context.hpp>
#include <Service/Message.hpp>

namespace sdesktop::developerMode
{
    class Event
    {
      protected:
        parserFSM::Context context;

      public:
        void send();
        virtual ~Event() = default;
    };

    class DeveloperModeRequest : public sys::DataMessage
    {
      public:
        std::unique_ptr<Event> event;
        DeveloperModeRequest(std::unique_ptr<Event> event);
        DeveloperModeRequest();
        ~DeveloperModeRequest() override = default;
    };
} // namespace sdesktop::developerMode
