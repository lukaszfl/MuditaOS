// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "DeveloperModeHelper.hpp"
#include <service-desktop/DesktopMessages.hpp>
#include <parser/ParserUtils.hpp>

#include <service-desktop/parser/MessageHandler.hpp>
#include <service-evtmgr/Constants.hpp>
#include <service-cellular/CellularMessage.hpp>
#include <service-cellular/ServiceCellular.hpp>
#include <service-bluetooth/messages/Status.hpp>
#include <service-cellular/CellularServiceAPI.hpp>

// notes
#include <module-db/queries/notes/QueryNoteRemove.hpp>
#include <module-db/queries/notes/QueryNotesGet.hpp>
#include <module-db/queries/notes/QueryNotesGetByText.hpp>
#include <module-services/service-db/service-db/DBNotesMessage.hpp>
#include <common_data/Clipboard.hpp>

#include <gui/Common.hpp>
#include <service-appmgr/Actions.hpp>
#include <messages/AppMessage.hpp>

#include <service-db/DBServiceAPI.hpp>
#include <time/time_conversion.hpp>

namespace parserFSM
{
    class Context;
} // namespace parserFSM

using namespace parserFSM;
auto DeveloperModeHelper::processPutRequest(Context &context) -> sys::ReturnCodes
{
    auto body = context.getBody();
    if (body[json::developerMode::keyPressed].is_number()) {
        auto keyValue = body[json::developerMode::keyPressed].int_value();
        auto state    = body[json::developerMode::state].int_value();
        sendKeypress(getKeyCode(keyValue), static_cast<gui::InputEvent::State>(state));
        MessageHandler::putToSendQueue(context.createSimpleResponse());
    }
    else if (body[json::developerMode::AT].is_string()) {
        auto msg     = std::make_shared<cellular::RawCommand>();
        msg->command = body[json::developerMode::AT].string_value();
        msg->timeout = 3000;
        ownerServicePtr->bus.sendUnicast(std::move(msg), ServiceCellular::serviceName);
    }
    else if (body[json::developerMode::focus].bool_value()) {
        auto event = std::make_unique<sdesktop::developerMode::AppFocusChangeEvent>();
        auto msg   = std::make_shared<sdesktop::developerMode::DeveloperModeRequest>(std::move(event));
        ownerServicePtr->bus.sendUnicast(std::move(msg), service::name::evt_manager);
    }
    else if (body[json::developerMode::isLocked].bool_value()) {
        auto event = std::make_unique<sdesktop::developerMode::ScreenlockCheckEvent>();
        auto msg   = std::make_shared<sdesktop::developerMode::DeveloperModeRequest>(std::move(event));
        ownerServicePtr->bus.sendUnicast(std::move(msg), "ApplicationDesktop");
    }
    else if (body[json::developerMode::btState].bool_value()) {

        auto event = std::make_unique<sdesktop::developerMode::BluetoothStatusRequestEvent>();
        auto msg   = std::make_shared<sdesktop::developerMode::DeveloperModeRequest>(std::move(event));
        ownerServicePtr->bus.sendUnicast(std::move(msg), "ServiceBluetooth");
    }
    else if (auto state = body[json::developerMode::btCommand].string_value(); !state.empty()) {
        BluetoothMessage::Request request;
        if (state == json::developerMode::btOn) {
            request = BluetoothMessage::Request::Start;
            LOG_INFO("turning on BT from harness!");
        }
        else {
            request = BluetoothMessage::Request::StopPlayback;
            LOG_INFO("turning off BT from harness!");
        }
        std::shared_ptr<BluetoothMessage> msg = std::make_shared<BluetoothMessage>(request);
        ownerServicePtr->bus.sendUnicast(std::move(msg), "ServiceBluetooth");

        MessageHandler::putToSendQueue(context.createSimpleResponse());
    }
    else if (body[json::developerMode::changeSim].is_number()) {
        int simSelected = body[json::developerMode::changeSim].int_value();
        requestSimChange(simSelected);
        MessageHandler::putToSendQueue(context.createSimpleResponse());
    }
    else if (body[json::developerMode::smsCommand].is_string()) {
        if (body[json::developerMode::smsCommand].string_value() == json::developerMode::smsAdd) {
            SMSType smsType = static_cast<SMSType>(context.getBody()[json::messages::type].int_value());
            if (smsType == SMSType::DRAFT || smsType == SMSType::QUEUED || smsType == SMSType::FAILED) {
                return prepareSMS(context);
            }
            else {
                context.setResponseStatus(http::Code::NotAcceptable);
                MessageHandler::putToSendQueue(context.createSimpleResponse());
            }
        }
    }
    else if (body[json::developerMode::notesCommand].is_string()) {
        if (body[json::developerMode::notesCommand].string_value() == json::developerMode::addNote) {
            return addNote(context);
        }
        else if (body[json::developerMode::notesCommand].string_value() == json::developerMode::removeNote) {
            return removeNote(context);
        }
        else if (body[json::developerMode::notesCommand].string_value() == json::developerMode::editNote) {
            return editNote(context);
        }
    }
    else if (body[json::developerMode::clipboardCommand].is_string()) {
        if (body[json::developerMode::clipboardCommand].string_value() == json::developerMode::copyClipboard) {
            return setClipboardCache(context);
        }
    }
    else {
        context.setResponseStatus(http::Code::BadRequest);
        MessageHandler::putToSendQueue(context.createSimpleResponse());
    }
    return sys::ReturnCodes::Unresolved;
}

auto DeveloperModeHelper::processGetRequest(Context &context) -> sys::ReturnCodes
{
    auto body = context.getBody();
    if (body[json::developerMode::getInfo].is_string()) {
        auto keyValue = body[json::developerMode::getInfo].string_value();
        if (keyValue == json::developerMode::simStateInfo) {
            context.setResponseBody(json11::Json::object(
                {{json::selectedSim, std::to_string(static_cast<int>(Store::GSM::get()->selected))},
                 {json::sim, std::to_string(static_cast<int>(Store::GSM::get()->sim))},
                 {json::trayState, std::to_string(static_cast<int>(Store::GSM::get()->tray))}}));
        }
        else {
            context.setResponseStatus(http::Code::BadRequest);
        }
    }
    else if (body[json::developerMode::notesCommand].is_string()) {
        if (body[json::developerMode::notesCommand].string_value() == json::developerMode::searchNote) {}
        else if (body[json::developerMode::notesCommand].string_value() == json::developerMode::getNote) {
            return getNote(context);
        }
        else if (body[json::developerMode::notesCommand].string_value() == json::developerMode::getNoteByText) {
            return getNoteByText(context);
        }
    }
    else {
        context.setResponseStatus(http::Code::BadRequest);
    }
    MessageHandler::putToSendQueue(context.createSimpleResponse());
    return sys::ReturnCodes::Unresolved;
}

auto DeveloperModeHelper::getKeyCode(int val) noexcept -> bsp::KeyCodes
{
    switch (val) {
    case 0:
        return bsp::KeyCodes::NumericKey0;
    case 1:
        return bsp::KeyCodes::NumericKey1;
    case 2:
        return bsp::KeyCodes::NumericKey2;
    case 3:
        return bsp::KeyCodes::NumericKey3;
    case 4:
        return bsp::KeyCodes::NumericKey4;
    case 5:
        return bsp::KeyCodes::NumericKey5;
    case 6:
        return bsp::KeyCodes::NumericKey6;
    case 7:
        return bsp::KeyCodes::NumericKey7;
    case 8:
        return bsp::KeyCodes::NumericKey8;
    case 9:
        return bsp::KeyCodes::NumericKey9;
    case '*':
        return bsp::KeyCodes::NumericKeyAst;
    case '#':
        return bsp::KeyCodes::NumericKeyPnd;
    case 'a':
        return bsp::KeyCodes::JoystickLeft;
    case 'd':
        return bsp::KeyCodes::JoystickRight;
    case 'w':
        return bsp::KeyCodes::JoystickUp;
    case 's':
        return bsp::KeyCodes::JoystickDown;
    case '\n':
        return bsp::KeyCodes::JoystickEnter;
    case 11:
        return bsp::KeyCodes::FnLeft;
    case 12:
        return bsp::KeyCodes::FnRight;
    case 13:
        return bsp::KeyCodes::VolUp;
    case 14:
        return bsp::KeyCodes::VolDown;
    case 15:
        return bsp::KeyCodes::Torch;
    case 16:
        return bsp::KeyCodes::SSwitchUp;
    case 17:
        return bsp::KeyCodes::SSwitchDown;
    case 18:
        return bsp::KeyCodes::SSwitchMid;
    default:
        LOG_ERROR("invalid keycode");
        return bsp::KeyCodes::Undefined;
    };
}

void DeveloperModeHelper::sendKeypress(bsp::KeyCodes keyCode, gui::InputEvent::State state)
{
    RawKey key{.state = RawKey::State::Released, .key_code = keyCode};

    gui::InputEvent event(key, state, static_cast<gui::KeyCode>(keyCode));
    LOG_INFO("Sending %s", event.str().c_str());
    auto message = std::make_shared<app::AppInputEventMessage>(std::move(event));

    ownerServicePtr->bus.sendUnicast(std::move(message), service::name::evt_manager);
}

void DeveloperModeHelper::requestSimChange(const int simSelected)
{
    Store::GSM::SIM sim = Store::GSM::SIM::SIM1;
    if (simSelected == static_cast<int>(Store::GSM::SIM::SIM2)) {
        sim = Store::GSM::SIM::SIM2;
    }
    CellularServiceAPI::SetSimCard(ownerServicePtr, sim);
}

auto DeveloperModeHelper::smsRecordFromJson(json11::Json msgJson) -> SMSRecord
{
    auto record = SMSRecord();

    record.type = static_cast<SMSType>(msgJson[json::messages::type].int_value());
    record.date = utils::time::getCurrentTimestamp().getTime();
    utils::PhoneNumber phoneNumber(msgJson[json::messages::phoneNumber].string_value());
    record.number = phoneNumber.getView();
    record.body   = UTF8(msgJson[json::messages::messageBody].string_value());
    return record;
}

auto DeveloperModeHelper::prepareSMS(Context &context) -> sys::ReturnCodes
{
    SMSRecord record = smsRecordFromJson(context.getBody());

    LOG_INFO("Adding sms of type %d to database", static_cast<int>(record.type));

    auto listener = std::make_unique<db::EndpointListener>(
        [=](db::QueryResult *result, Context context) {
            bool res = false;
            if (auto SMSAddResult = dynamic_cast<db::query::SMSAddResult *>(result)) {
                context.setResponseStatus(SMSAddResult->result ? http::Code::OK : http::Code::InternalServerError);
                MessageHandler::putToSendQueue(context.createSimpleResponse());
                LOG_INFO("Adding sms of type %d to database - %s",
                         static_cast<int>(record.type),
                         SMSAddResult->result ? "OK" : "NOK");
                res = true;
            }
            else {
                context.setResponseStatus(http::Code::InternalServerError);
                MessageHandler::putToSendQueue(context.createSimpleResponse());
            }
            return res;
        },
        context);

    DBServiceAPI::AddSMS(ownerServicePtr, record, std::move(listener));
    return sys::ReturnCodes::Success;
}

auto DeveloperModeHelper::getNoteFromJson(json11::Json msgJson) -> NotesRecord
{
    auto record    = NotesRecord();
    record.ID      = msgJson[json::messages::id].int_value();
    record.date    = utils::time::getCurrentTimestamp().getTime();
    record.snippet = UTF8(msgJson[json::messages::messageBody].string_value());

    return record;
}

auto DeveloperModeHelper::addNote(Context &context) -> sys::ReturnCodes
{
    LOG_INFO("Add note to database");

    auto query = std::make_unique<db::query::QueryNoteStore>(getNoteFromJson(context.getBody()));

    query->setQueryListener(std::make_unique<db::EndpointListener>(
        [=](db::QueryResult *result, Context context) {
            bool res = false;
            context.setResponseStatus(http::Code::InternalServerError);

            if (auto noteStoreResult = dynamic_cast<db::query::NoteStoreResult *>(result)) {
                if (noteStoreResult->succeed()) {
                    context.setResponseStatus(http::Code::OK);
                }

                res = true;
            }

            LOG_INFO("Adding note to database - %s", res ? "OK" : "NOK");
            MessageHandler::putToSendQueue(context.createSimpleResponse());
            return res;
        },
        context));

    const auto [succeed, _] = DBServiceAPI::GetQuery(ownerServicePtr, db::Interface::Name::Notes, std::move(query));
    return succeed ? sys::ReturnCodes::Success : sys::ReturnCodes::Failure;
}

auto DeveloperModeHelper::editNote(Context &context) -> sys::ReturnCodes
{
    LOG_INFO("Editing note");
    return sys::ReturnCodes::Timeout;
}

auto DeveloperModeHelper::removeNote(Context &context) -> sys::ReturnCodes
{
    LOG_INFO("Remove note from database");

    auto query = std::make_unique<db::query::QueryNoteRemove>(context.getBody()[json::messages::id].int_value());

    query->setQueryListener(std::make_unique<db::EndpointListener>(
        [=](db::QueryResult *result, Context context) {
            bool res = false;
            context.setResponseStatus(http::Code::InternalServerError);

            if (auto noteRemoveResult = dynamic_cast<db::query::NoteRemoveResult *>(result)) {
                if (noteRemoveResult->succeed()) {
                    context.setResponseStatus(http::Code::OK);
                }

                res = true;
            }

            LOG_INFO("Removing note from database - %s", res ? "OK" : "NOK");
            MessageHandler::putToSendQueue(context.createSimpleResponse());
            return res;
        },
        context));

    const auto [succeed, _] = DBServiceAPI::GetQuery(ownerServicePtr, db::Interface::Name::Notes, std::move(query));
    return succeed ? sys::ReturnCodes::Success : sys::ReturnCodes::Failure;
}

auto DeveloperModeHelper::searchNote(Context &context) -> sys::ReturnCodes
{
    return sys::ReturnCodes::Timeout;
}

auto DeveloperModeHelper::getNoteByText(Context &context) -> sys::ReturnCodes
{
    LOG_INFO("Get note from database by text");

    auto query =
        std::make_unique<db::query::QueryNotesGetByText>(context.getBody()[json::messages::messageBody].string_value());

    query->setQueryListener(std::make_unique<db::EndpointListener>(
        [=](db::QueryResult *result, Context context) {
            bool res = false;
            context.setResponseStatus(http::Code::InternalServerError);

            if (auto noteGetByTextResult = dynamic_cast<db::query::NotesGetByTextResult *>(result)) {
                if (!noteGetByTextResult->getRecords().empty()) {
                    json11::Json::array results;
                    for (auto note : noteGetByTextResult->getRecords()) {
                        results.emplace_back(noteToJson(note));
                    }

                    context.setResponseBody(results);
                    context.setResponseStatus(http::Code::OK);
                    res = true;
                }
            }

            LOG_INFO("Getting note from database by text - %s", res ? "OK" : "NOK");
            MessageHandler::putToSendQueue(context.createSimpleResponse());
            return res;
        },
        context));

    const auto [succeed, _] = DBServiceAPI::GetQuery(ownerServicePtr, db::Interface::Name::Notes, std::move(query));
    return succeed ? sys::ReturnCodes::Success : sys::ReturnCodes::Failure;
}

auto DeveloperModeHelper::getNote(Context &context) -> sys::ReturnCodes
{
    LOG_INFO("Get note from database");

    auto query = std::make_unique<db::query::QueryNotesGet>(context.getBody()[json::messages::id].int_value());

    query->setQueryListener(std::make_unique<db::EndpointListener>(
        [=](db::QueryResult *result, Context context) {
            bool res = false;
            context.setResponseStatus(http::Code::InternalServerError);

            if (auto noteGetByTextResult = dynamic_cast<db::query::NotesGetResult *>(result)) {
                if (!noteGetByTextResult->getRecords().empty()) {
                    json11::Json::array results;
                    for (auto note : noteGetByTextResult->getRecords()) {
                        results.emplace_back(noteToJson(note));
                    }

                    context.setResponseBody(results);
                    context.setResponseStatus(http::Code::OK);
                    res = true;
                }
            }

            LOG_INFO("Getting note from database - %s", res ? "OK" : "NOK");
            MessageHandler::putToSendQueue(context.createSimpleResponse());
            return res;
        },
        context));

    const auto [succeed, _] = DBServiceAPI::GetQuery(ownerServicePtr, db::Interface::Name::Notes, std::move(query));

    return succeed ? sys::ReturnCodes::Success : sys::ReturnCodes::Failure;
}

auto DeveloperModeHelper::setClipboardCache(Context &context) -> sys::ReturnCodes
{
    LOG_INFO("Set clipboard cache (copy)");

    auto result = false;
    context.setResponseStatus(http::Code::InternalServerError);

    auto data = context.getBody()[json::messages::messageBody].string_value();
    Clipboard::getInstance().copy(data);

    if (Clipboard::getInstance().gotData() && (Clipboard::getInstance().paste() == data)) {
        context.setResponseStatus(http::Code::OK);
        result = true;
    }

    LOG_INFO("Set clipboard cache (copy) - %s", result ? "OK" : "NOK");
    MessageHandler::putToSendQueue(context.createSimpleResponse());
    return result ? sys::ReturnCodes::Success : sys::ReturnCodes::Failure;
}

auto DeveloperModeHelper::noteToJson(NotesRecord &record) -> json11::Json
{
    return json11::Json::object{{json::messages::id, static_cast<int>(record.ID)},
                                {json::messages::date, static_cast<int>(record.date)},
                                {json::messages::messageBody, record.snippet.c_str()}};
}
