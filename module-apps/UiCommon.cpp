#include "UiCommon.hpp"
#include "application-call/ApplicationCall.hpp"
#include "application-call/data/CallSwitchData.hpp"
#include "application-messages/ApplicationMessages.hpp"
#include "application-messages/data/SMSdata.hpp"
#include "application-messages/windows/ThreadViewWindow.hpp"
#include "application-phonebook/ApplicationPhonebook.hpp"
#include "application-phonebook/data/PhonebookItemData.hpp"
#include "service-appmgr/ApplicationManager.hpp"
#include <cassert>
#include <i18/i18.hpp>
#include <log/log.hpp>

namespace app
{
    namespace UiCommon
    {
        namespace
        {
            enum class CallOperation
            {
                ExecuteCall,
                PrepareCall
            };

            bool switchToCall(sys::Service *sender, const UTF8 &e164number, CallOperation callOperation = CallOperation::ExecuteCall)
            {
                assert(sender != nullptr);
                std::string window = window::name_enterNumber;

                std::unique_ptr<CallSwitchData> data;

                switch (callOperation)
                {
                case CallOperation::ExecuteCall: {
                    data = std::make_unique<ExecuteCallData>(e164number.c_str());
                }
                break;
                case CallOperation::PrepareCall: {
                    data = std::make_unique<EnterNumberData>(e164number.c_str());
                }
                break;
                default: {
                    LOG_ERROR("callOperation not supported %d", static_cast<uint32_t>(callOperation));
                    return false;
                }
                }
                return sapm::ApplicationManager::messageSwitchApplication(sender, name_call, window, std::move(data));
            }
        } // namespace

        bool call(Application *app, const ContactRecord &contact)
        {
            assert(app != nullptr);

            if (contact.numbers.size() != 0)
            {
                return switchToCall(app, contact.numbers[0].numberE164);
            }
            else
            {
                LOG_ERROR("No contact numbers!");
                return false;
            }
        }

        bool call(Application *app, const UTF8 &e164number)
        {
            assert(app != nullptr);

            return switchToCall(app, e164number);
        }

        gui::Option callOption(Application *app, const ContactRecord &contact, ActiveCallOption activeCallOption = ActiveCallOption::True)
        {
            assert(app != nullptr);
            return {UTF8(utils::localize.get("sms_call_text")) + contact.primaryName, [app, contact, activeCallOption](gui::Item &item) {
                        if (activeCallOption == ActiveCallOption::True)
                        {
                            LOG_DEBUG("Try call!");
                            return call(app, contact);
                        }
                        else
                        {
                            LOG_ERROR("Inactive call option");
                            return false;
                        }
                    }};
        }

        gui::Option contactDetails(Application *app, const ContactRecord &contact)
        {
            if (app == nullptr)
            {
                LOG_ERROR("app = nullptr");
                return gui::Option();
            }

            return {utils::localize.get("sms_contact_details"), [=](gui::Item &item) {
                        return sapm::ApplicationManager::messageSwitchApplication(
                            app, name_phonebook, "Contact", std::make_unique<PhonebookItemData>(std::shared_ptr<ContactRecord>(new ContactRecord(contact))));
                    }};
        }

        bool sms(Application *app, const ContactRecord &contact)
        {
            assert(app != nullptr);
            // TODO return to current application doesn't change application window >_>
            auto param = std::shared_ptr<ContactRecord>(new ContactRecord(contact));
            return sapm::ApplicationManager::messageSwitchApplication(app, name_messages, gui::name::window::thread_view,
                                                                      std::make_unique<SMSSendRequest>(param));
        }

        bool sms(Application *app, const std::string &number)
        {
            assert(app != nullptr);
            auto param = std::shared_ptr<ContactRecord>(new ContactRecord());
            param->numbers = std::vector<ContactRecord::Number>({ContactRecord::Number(number, number)});
            return sapm::ApplicationManager::messageSwitchApplication(app, name_messages, gui::name::window::thread_view,
                                                                      std::make_unique<SMSSendRequest>(param));
        }

        bool addContact(Application *app, const ContactRecord &contact)
        {
            assert(app != nullptr);
            return sapm::ApplicationManager::messageSwitchApplication(
                app, name_phonebook, "New", std::make_unique<PhonebookItemData>(std::shared_ptr<ContactRecord>(new ContactRecord(contact))));
        }

        bool addContact(Application *app, const std::string &number)
        {
            assert(app != nullptr);

            auto contact = std::make_shared<ContactRecord>();
            contact->numbers = std::vector<ContactRecord::Number>({ContactRecord::Number(number, number)});

            return sapm::ApplicationManager::messageSwitchApplication(app, name_phonebook, "New", std::make_unique<PhonebookItemData>(contact));
        }
    } // namespace UiCommon
} // namespace app
