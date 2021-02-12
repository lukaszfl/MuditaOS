// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <endpoints/Context.hpp>

#include <Common/Query.hpp>
#include <Service/Common.hpp>
#include <Service/Service.hpp>
#include <bsp/keyboard/key_codes.hpp>
#include <input/InputEvent.hpp>
#include <module-db/Interface/SMSRecord.hpp>

// notes
#include <module-db/Interface/NotesRecord.hpp>
#include <module-db/queries/notes/QueryNoteStore.hpp>

namespace sys
{
    class Service;
} // namespace sys

namespace parserFSM
{

    class DeveloperModeHelper
    {
        sys::Service *ownerServicePtr = nullptr;
        static auto getKeyCode(int val) noexcept -> bsp::KeyCodes;
        void sendKeypress(bsp::KeyCodes keyCode, gui::InputEvent::State state);

        void requestSimChange(const int simSelected);
        auto smsRecordFromJson(json11::Json msgJson) -> SMSRecord;
        auto prepareSMS(Context &context) -> sys::ReturnCodes;

        auto getNoteFromJson(json11::Json msgJson) -> NotesRecord;
        auto addNote(Context &context) -> sys::ReturnCodes;
        auto editNote(Context &context) -> sys::ReturnCodes;
        auto removeNote(Context &context) -> sys::ReturnCodes;
        auto searchNote(Context &context) -> sys::ReturnCodes;
        auto getNote(Context &context) -> sys::ReturnCodes;
        auto getNoteByText(Context &context) -> sys::ReturnCodes;
        auto noteToJson(NotesRecord &record) -> json11::Json;

        auto setClipboardCache(Context &context) -> sys::ReturnCodes;

      public:
        DeveloperModeHelper(sys::Service *_ownerServicePtr) : ownerServicePtr(_ownerServicePtr){};
        auto processPutRequest(Context &context) -> sys::ReturnCodes;
        auto processGetRequest(Context &context) -> sys::ReturnCodes;
    };

    namespace json::developerMode
    {
        inline constexpr auto keyPressed       = "keyPressed";
        inline constexpr auto state            = "state";
        inline constexpr auto systemStarted    = "systemStarted";
        inline constexpr auto ATResponse       = "ATResponse";
        inline constexpr auto AT               = "AT";
        inline constexpr auto focus            = "focus";
        inline constexpr auto isLocked         = "isLocked";
        inline constexpr auto btState          = "btState";
        inline constexpr auto btOn             = "on";
        inline constexpr auto btCommand        = "btCommand";
        inline constexpr auto changeSim        = "changeSim";
        inline constexpr auto smsCommand       = "smsCommand";
        inline constexpr auto notesCommand     = "notesCommand";
        inline constexpr auto clipboardCommand = "clipboardCommand";
        inline constexpr auto getInfo          = "getInfo";
        /// values for getInfo cmd
        inline constexpr auto simStateInfo = "simState";
        /// values for smsCommand
        inline constexpr auto smsAdd = "smsAdd";
        /// values for notesCommand
        inline constexpr auto addNote       = "addNote";
        inline constexpr auto removeNote    = "removeNote";
        inline constexpr auto editNote      = "editNote";
        inline constexpr auto searchNote    = "searchNote";
        inline constexpr auto getNote       = "getNote";
        inline constexpr auto getNoteByText = "getNoteByText";
        /// values for clipboardCommand
        inline constexpr auto copyClipboard = "copy";
    } // namespace json::developerMode
} // namespace parserFSM
