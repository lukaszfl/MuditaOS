// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <exception>
#include <module-sys/Service/Message.hpp>
#include <service-db/DBServiceName.hpp>
#include "SettingsScope.hpp"
#include "SettingsMessages.hpp"

#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace settings
{
    class Failure : public std::runtime_error
    {
      public:
        Failure(const std::string &error);
    };
    class SettingsCache;
    class Settings
    {
      public:
        using ValueChangedCallback           = std::function<void(const std::string &)>;
        using ValueChangedCallbackWithName   = std::function<void(const std::string &, const std::string &value)>;
        using ProfileChangedCallback         = std::function<void(const std::string &)>;
        using ModeChangedCallback            = ProfileChangedCallback;
        using ListOfProfiles                 = std::list<std::string>;
        using ListOfModes                    = ListOfProfiles;
        using OnAllProfilesRetrievedCallback = std::function<void(const ListOfProfiles &)>;
        using OnAllModesRetrievedCallback    = std::function<void(const ListOfModes &)>;

        explicit Settings(sys::Service *app,
                          const std::string &dbAgentName = service::name::db,
                          SettingsCache *cache           = nullptr);
        ~Settings();

        void setValue(const std::string &variableName,
                      const std::string &variableValue,
                      SettingsScope scope = SettingsScope::AppLocal);
        void registerValueChange(const std::string &variableName,
                                 ValueChangedCallback cb,
                                 SettingsScope scope = SettingsScope::AppLocal);
        void registerValueChange(const std::string &variableName,
                                 ValueChangedCallbackWithName cb,
                                 SettingsScope scope = SettingsScope::AppLocal);
        void unregisterValueChange(const std::string &variableName, SettingsScope scope = SettingsScope::AppLocal);
        /// unregisters all registered variables (both global and local)
        void unregisterValueChange();
        std::string getValue(const std::string &variableName, SettingsScope scope = SettingsScope::AppLocal);

        void getAllProfiles(OnAllProfilesRetrievedCallback cb);
        void setCurrentProfile(const std::string &profile);
        void addProfile(const std::string &profile);
        void registerProfileChange(ProfileChangedCallback);
        void unregisterProfileChange();

        void getAllModes(OnAllModesRetrievedCallback cb);
        void setCurrentMode(const std::string &mode);
        void addMode(const std::string &mode);
        void registerModeChange(ModeChangedCallback);
        void unregisterModeChange();

      private:
        std::string dbAgentName;

        SettingsCache *cache = nullptr;
        std::weak_ptr<sys::Service> app;
        std::string serviceName;
        std::string phoneMode;
        std::string profile;


        using ValueCb = std::map<EntryPath, std::pair<ValueChangedCallback, ValueChangedCallbackWithName>>;
        ValueCb cbValues;
        ModeChangedCallback cbMode;
        OnAllModesRetrievedCallback cbAllModes;
        ProfileChangedCallback cbProfile;
        OnAllProfilesRetrievedCallback cbAllProfiles;
        void sendMsg(std::shared_ptr<settings::Messages::SettingsMessage> &&msg);
        enum Change
        {
            Register,
            Deregister
        };
        /// using owner service either
        /// - to register handlers
        /// - to unregister handlers
        /// handlers are called per service if for some reason service will stop
        /// existing - handlers shouldn't be called
        void changeHandlers(enum Change change);
        void registerHandlers();
        void deregisterHandlers();
        auto handleVariableChanged(sys::Message *req) -> sys::MessagePointer;
        auto handleCurrentProfileChanged(sys::Message *req) -> sys::MessagePointer;
        auto handleCurrentModeChanged(sys::Message *req) -> sys::MessagePointer;
        auto handleProfileListResponse(sys::Message *req) -> sys::MessagePointer;
        auto handleModeListResponse(sys::Message *req) -> sys::MessagePointer;
    };
} // namespace settings
