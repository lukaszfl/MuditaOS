#pragma once

#include "Application.hpp"
#include <OptionWindow.hpp>
#include <service-db/api/DBServiceAPI.hpp>

namespace app
{
    namespace UiCommon
    {
        bool call(Application *app, const ContactRecord &contact);
        bool call(Application *app, const UTF8 &e164number);
        bool sms(Application *app, const ContactRecord &contact);
        bool sms(Application *app, const std::string &number);
        // TODO use contact here
        bool addContact(Application *app, const ContactRecord &contact);
        bool addContact(Application *app, const std::string &number);

        enum class ActiveCallOption : bool
        {
            False,
            True
        };
        gui::Option callOption(Application *app, const ContactRecord &contact, ActiveCallOption activeCallOption);
        gui::Option contactDetails(Application *app, const ContactRecord &contact);
    } // namespace UiCommon
} // namespace app
