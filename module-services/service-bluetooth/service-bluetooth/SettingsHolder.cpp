// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SettingsHolder.hpp"
#include <log/log.hpp>

auto SettingsHolder::getValue(Settings setting) -> std::variant<int, bool, std::string>
{
    auto selectedSetting = settingsJson[static_cast<char>(setting)];
    if (selectedSetting.is_bool()) {
        return std::variant<int, bool, std::string>(selectedSetting.bool_value());
    }
    if (selectedSetting.is_number()) {
        return std::variant<int, bool, std::string>(selectedSetting.int_value());
    }
    if (selectedSetting.is_string()) {
        return std::variant<int, bool, std::string>(selectedSetting.string_value());
    }
    return std::variant<int, bool, std::string>();
}
void SettingsHolder::setValue(const SettingEntry &entry)
{
    auto setting                  = std::get<Settings>(entry);
    auto value                    = entry.second;
    json11::Json &selectedSetting = const_cast<json11::Json &>(settingsJson[static_cast<char>(setting)]);
    if (selectedSetting.is_bool()) {
        selectedSetting = std::get<bool>(value);
    }
    else if (selectedSetting.is_number()) {
        selectedSetting = std::get<int>(value);
    }
    else if (selectedSetting.is_string()) {
        selectedSetting = std::get<std::string>(value);
    }
}
auto SettingsHolder::dump() const -> std::string
{
    return settingsJson.dump();
}
void SettingsHolder::import(const std::string &jsonString)
{
    std::string err;
    settingsJson.parse(jsonString, err);
    if (!err.empty()) {
        LOG_ERROR("can't load json: %s", err.c_str());
    }
}
