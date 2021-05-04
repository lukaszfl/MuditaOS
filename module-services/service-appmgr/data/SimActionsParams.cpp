// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <service-appmgr/data/SimActionsParams.hpp>

using namespace app::manager::actions;

SimLockParams::SimLockParams(Store::GSM::SIM _sim, unsigned int _attempts, std::string _passcodeName)
    : sim{_sim}, attempts{_attempts}, passcodeName{std::move(_passcodeName)}
{}

Store::GSM::SIM SimLockParams::getSim() const noexcept
{
    return sim;
}
unsigned int SimLockParams::getAttempts() const noexcept
{
    return attempts;
}
const std::string &SimLockParams::getPasscodeName() const noexcept
{
    return passcodeName;
}

SimStateParams::SimStateParams(Store::GSM::SIM _sim) : sim{_sim}
{}

Store::GSM::SIM SimStateParams::getSim() const noexcept
{
    return sim;
}

UnhandledCMEParams::UnhandledCMEParams(Store::GSM::SIM _sim, unsigned int _cmeCode) : sim{_sim}, cmeCode{_cmeCode}
{}

Store::GSM::SIM UnhandledCMEParams::getSim() const noexcept
{
    return sim;
}
unsigned int UnhandledCMEParams::getCMECode() const noexcept
{
    return cmeCode;
}
