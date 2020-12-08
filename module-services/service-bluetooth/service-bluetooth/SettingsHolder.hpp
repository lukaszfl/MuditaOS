// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <json/json11.hpp>
#include <string>
#include <variant>

enum class Settings : char
{
    DeviceName = 'a',
    Visibility
};

using SettingEntry = std::pair<Settings, std::variant<int, bool, std::string>>;

class SettingsHolder
{
  public:
    auto getValue(Settings setting) -> std::variant<int, bool, std::string>;
    void setValue(const SettingEntry &entry);
    [[nodiscard]] auto dump() const -> std::string;
    void import(const std::string &jsonString);

  private:
    json11::Json settingsJson;
};
