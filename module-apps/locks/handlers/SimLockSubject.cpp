//// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
//// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "SimLockSubject.hpp"

#include <service-appmgr/service-appmgr/model/ApplicationManager.hpp>
#include <locks/data/PhoneLockMessages.hpp>

namespace locks
{
    SimLockSubject::SimLockSubject(sys::Service *owner) : owner{owner}
    {
        if (owner == nullptr) {
            throw std::invalid_argument{"Subject's owner is invalid"};
        }
    }

    void SimLockSubject::verifyInput(const std::vector<unsigned int> &inputData)
    {
        LOG_ERROR("Chce sobie odblokowac sima !");
        owner->bus.sendUnicast(std::make_shared<UnLockSimInput>(inputData),
                               app::manager::ApplicationManager::ServiceName);
    }
} // namespace locks
