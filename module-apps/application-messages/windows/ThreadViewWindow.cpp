#include <functional>
#include <memory>

#include "service-appmgr/ApplicationManager.hpp"

#include "i18/i18.hpp"
#include "time/time_conversion.hpp"

#include "Label.hpp"
#include "ListView.hpp"
#include "Margins.hpp"

#include "service-db/messages/DBMessage.hpp"
#include "service-db/api/DBServiceAPI.hpp"

#include <log/log.hpp>

#include "ThreadViewWindow.hpp"
#include <Style.hpp>

#include "../widgets/ThreadModel.hpp"

#include "../ApplicationMessages.hpp"
#include "../data/SMSdata.hpp"
#include "OptionsMessages.hpp"
#include "service-cellular/api/CellularServiceAPI.hpp"
#include <application-phonebook/data/PhonebookItemData.hpp>
#include <gui/widgets/Text.hpp>
#include <widgets/ListItem.hpp>
#include <widgets/ListItemProvider.hpp>
#include <widgets/ListView.hpp>

namespace style
{
    namespace window
    {
        inline const uint32_t sms_radius = 7;
        inline const uint32_t sms_border_no_focus = 1;
        /// TODO 100 is static size, sms elements should have size depending on text amount
        inline const uint16_t sms_height = 100;
        inline const uint16_t bubble_width = 300;
        inline const uint16_t spacing_close = 3;
        inline const uint16_t spacing_far = 12;

    };
}; // namespace style

namespace gui
{

    ThreadViewWindow::ThreadViewWindow(app::Application *app) : AppWindow(app, name::window::thread_view)
    {
        AppWindow::buildInterface();
        setTitle(utils::localize.get("app_messages_title_main"));
        topBar->setActive(TopBar::Elements::TIME, true);
        bottomBar->setActive(BottomBar::Side::LEFT, true);
        bottomBar->setActive(BottomBar::Side::CENTER, true);
        bottomBar->setActive(BottomBar::Side::RIGHT, true);
        bottomBar->setText(BottomBar::Side::LEFT, utils::localize.get("common_options"));
        bottomBar->setText(BottomBar::Side::CENTER, utils::localize.get("common_send"));
        bottomBar->setText(BottomBar::Side::RIGHT, utils::localize.get("common_back"));
        auto elements_width = this->getWidth() - style::window::default_left_margin * 2;
        body = new gui::VBox(this, style::window::default_left_margin, title->offset_h(), elements_width, bottomBar->getY() - title->offset_h());
        body->setPenWidth(style::window::default_border_no_focus_w);
        body->setPenFocusWidth(style::window::default_border_no_focus_w);

        // new text, TODO - send it and reload sms
        auto text = new gui::Text(nullptr, 0, 0, elements_width, style::window::sms_height, "", gui::Text::ExpandMode::EXPAND_UP);
        text->setInputMode(new InputMode(
            {InputMode::ABC, InputMode::abc}, [=](const UTF8 &text) { textModeShowCB(text); }, [=]() { textSelectSpecialCB(); }));
        text->setPenFocusWidth(style::window::default_border_focucs_w);
        text->setPenWidth(style::window::default_border_focucs_w);
        text->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM);
        // TODO IFS
        text->activatedCallback = [=](gui::Item &item) {
            LOG_INFO("Send SMS callback");

            SMSRecord record;
            record.number = title->getText();
            record.body = text->getText();
            record.type = SMSType::QUEUED;
            auto time = utils::time::Time();
            record.date = time.getTime();
            DBServiceAPI::SMSAdd(this->application, record);

            return true;
        };

        body->tryAddWidget(text);
        /// setup
        body->setReverseOrder(true);
        body->setVisible(true);
        setFocusItem(body);
    }

    void ThreadViewWindow::cleanMessages()
    {
        // remove items from list and and set text to no navigation (text is all the time same widget)
        auto text = body->children.front();
        body->setFocusItem(text);
        text->setNavigationItem(NavigationDirection::UP, nullptr);
        text->setNavigationItem(NavigationDirection::DOWN, nullptr);
        if (body->children.size() > 1)
        {
            std::for_each(std::next(body->children.begin(), 1), body->children.end(), [&](auto &el) { delete el; });
            body->children.erase(std::next(body->children.begin()), body->children.end());
        }
    }

    void ThreadViewWindow::addMessages(uint32_t thread_id)
    {
        auto elements_width = this->getWidth() - style::window::default_left_margin * 2;
        LOG_INFO("=============================");
        std::unique_ptr<std::vector<SMSRecord>> sms = DBServiceAPI::SMSGetLimitOffsetByThreadID(application, 0, 3, thread_id); // it fetches in reverse date order
        LOG_INFO("=============================");
        /// dummy sms thread - TODO TODO load from db - on switchData
        /// this is not 'static' do it on window focus (on swtich window)
        auto labelmeta = gui::meta::Label({0, 0, elements_width, style::window::sms_height});
        labelmeta.edges = RectangleEdgeFlags::GUI_RECT_ALL_EDGES;
        labelmeta.radius = style::window::sms_radius;
        labelmeta.align = gui::Alignment::ALIGN_VERTICAL_TOP;
        labelmeta.focus = style::window::default_border_focucs_w;
        labelmeta.no_focus = style::window::sms_border_no_focus;
        labelmeta.margins = Margins(0, 2, 0, 2);
        // hack - tmp to fix fitting
        body->setReverseOrder(false);
        for (auto &el : *sms)
        {
            auto messageLabel = new gui::Label(nullptr, labelmeta);
            messageLabel->setText(el.body);
            messageLabel->activatedCallback = [=](Item &) {
                LOG_INFO("Message activated!");
                auto app = dynamic_cast<app::ApplicationMessages *>(application);
                if (app == nullptr)
                {
                    LOG_ERROR("Something went horribly wrong");
                    return false;
                }
                if (app->windowOptions != nullptr)
                {
                    app->windowOptions->clearOptions();
                    /// TODO get record properly...
                    app->windowOptions->addOptions(smsWindowOptions(app, el));
                    app->switchWindow(app->windowOptions->getName(), nullptr);
                }
                return true;
            };

            // encapsulating hbox, only to store time in the same row
            auto messageBoundingBox = new gui::HBox(nullptr, messageLabel->getX(), messageLabel->getY(), messageLabel->getWidth(), messageLabel->getHeight());
            messageBoundingBox->setBorderColor(gui::ColorFullBlack);

            switch (el.type) // it should probably be elif as SMSType looks like bitfield and cas be multiple values at once
            {
            case SMSType::INBOX:
                messageLabel->setYaps(RectangleYapFlags::GUI_RECT_YAP_TOP_LEFT);
                messageLabel->setSize(style::window::bubble_width - messageLabel->radius, messageLabel->getHeight());
                messageLabel->setPosition(messageLabel->getX() + messageLabel->radius, messageLabel->getY());
                break;
            case SMSType::OUTBOX:
                messageLabel->setYaps(RectangleYapFlags::GUI_RECT_YAP_TOP_RIGHT);
                messageLabel->setSize(style::window::bubble_width - messageLabel->radius, messageLabel->getHeight());
                messageLabel->setPosition( (body->getWidth() - style::window::bubble_width) - messageLabel->radius, messageLabel->getY());
                messageBoundingBox->setReverseOrder(true);
                break;
            case SMSType::FAILED:
                break;
            case SMSType::QUEUED:
                break;
            default:
                LOG_ERROR("Unhandled sms type");
                break;
            }

//            LOG_DEBUG("%d, %d, %d, %d", messageLabel->getX(), messageLabel->getY(), messageLabel->getWidth(), messageLabel->getHeight());
            LOG_DEBUG("added ? %c", messageBoundingBox->tryAddWidget(messageLabel) ? 'v':'x');

////            messageWideBox->tryAddWidget(messageLabel);
////            LOG_DEBUG("Label added ? %d", );
//
                messageLabel->focusChangedCallback = [el](gui::Item & item){
//                messageLabel->setFocus(item.focus);
                if (item.focus)
                {
                  auto arrival = new Label(nullptr, item.getX(), item.getY(), 20, 20);
                  arrival->setText(std::to_string(getTime()));
                  LOG_DEBUG("date: %d", el.date);
//                  LOG_DEBUG("type: %s", typeid(item).name());
//                    this->body->tryAddWidget(arrival);
                }
              return true;
            }; // lambda showing time

            LOG_INFO("Add sms: %s %s", el.body.c_str(), el.number.c_str());
            if (body->tryAddWidget(messageBoundingBox)) {
                // add margin. maybe dependant on same minute/different minute
                auto marginRect = new Rect(nullptr, messageLabel->getX(), messageLabel->getY(), messageLabel->getWidth(), style::window::spacing_far);
                marginRect->setBorderColor(gui::ColorNoColor);
                marginRect->activeItem = false;
                // bounty hunt: detect last element in for_range loop here   LOG_DEBUG("end: %llu, next %llu", sms.get()->end(), std::next(&el)); // cannot compare. doesn't work
//                body->tryAddWidget(marginRect); // it will fail for the most bottom one
            }
            else
            {
                delete messageLabel;
                break;
            }
        }
        body->setReverseOrder(true);
        body->setNavigation();
    }

    void ThreadViewWindow::rebuild()
    {
        destroyInterface();
        buildInterface();
    }

    void ThreadViewWindow::buildInterface()
    {
    }

    void ThreadViewWindow::destroyInterface()
    {
        AppWindow::destroyInterface();
        children.clear();
    }

    ThreadViewWindow::~ThreadViewWindow()
    {
        destroyInterface();
    }

    void ThreadViewWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        {
            auto pdata = dynamic_cast<SMSThreadData *>(data);
            if (pdata)
            {
                LOG_INFO("We have it! %d", pdata->thread->dbID);
                cleanMessages();
                addMessages(pdata->thread->dbID);
                auto ret = DBServiceAPI::ContactGetByID(application, pdata->thread->contactID);
                // should be name number for now - easier to handle
                setTitle(ret->front().number);
                return;
            }
        }
        {
            auto pdata = dynamic_cast<SMSSendRequest *>(data);
            if (pdata)
            {
                LOG_INFO("Phonebook sms send request!");
                // TODO agree what should be used and how. Now Request have only contact,
                // maybe it should have additional info - which nr to use and how to show it
                if (pdata->contact->numbers.size() != 0)
                {
                    LOG_DEBUG("SEND SMS TO: %s %s %s %s %s", pdata->contact->number.c_str(), pdata->contact->numbers[0].numberE164.c_str(),
                              pdata->contact->numbers[0].numberUser.c_str(), pdata->contact->primaryName.c_str(), pdata->contact->alternativeName.c_str());
                    setTitle(pdata->contact->numbers[0].numberUser);
                }
                else
                {
                    // TODO handle error better
                    setTitle("NO CONTACT");
                }
            }
        }
        {
            if (auto pdata = dynamic_cast<PhonebookSearchReuqest *>(data))
            {
                LOG_INFO("PhonebookSearchRequest data");
            }
        }
    }

    bool ThreadViewWindow::onInput(const InputEvent &inputEvent)
    {
        return AppWindow::onInput(inputEvent);
    }

    bool ThreadViewWindow::onDatabaseMessage(sys::Message *msgl)
    {
        //	DBContactResponseMessage* msg = reinterpret_cast<DBContactResponseMessage*>( msgl );
        //	if( phonebookModel->updateRecords( std::move(msg->records), msg->offset, msg->limit, msg->count, msg->favourite ) )
        //		return true;

        return false;
    }

} /* namespace gui */
