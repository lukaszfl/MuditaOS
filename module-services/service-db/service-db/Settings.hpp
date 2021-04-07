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

    /// interface for settings
    /// it will throw in your face if used straight with Service constructor
    /// this is because weak_ptr on shared_from_this doestn exist on creation
    /// creation of setting should be delayed till we call init from Service
    /// virtual ReturnCodes InitHandler() = 0;
    ///
    /// TODO invalidate interface on shared resource
    /// (easiest : unique ptr to actual impl and clear it)
    class Interface
    {
      private:
        sys::Service *app       = nullptr;
        std::string dbAgentName = service::name::db;
        SettingsCache *cache    = nullptr;

      public:
        explicit Interface(sys::Service *);
        sys::Service &getApp();
        SettingsCache *getCache();
        const std::string &agent()
        {
            return dbAgentName;
        }
        explicit operator bool() const noexcept;
    };

    /// see InitHandler note for settings usage for interface
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

        explicit Settings(Interface interface);
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
        Interface interface;
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
        void changeHandlers(enum Change change, sys::Service &);
        void registerHandlers(sys::Service &);
        void deregisterHandlers(sys::Service &);
        auto handleVariableChanged(sys::Message *req) -> sys::MessagePointer;
        auto handleCurrentProfileChanged(sys::Message *req) -> sys::MessagePointer;
        auto handleCurrentModeChanged(sys::Message *req) -> sys::MessagePointer;
        auto handleProfileListResponse(sys::Message *req) -> sys::MessagePointer;
        auto handleModeListResponse(sys::Message *req) -> sys::MessagePointer;
    };
} // namespace settings
