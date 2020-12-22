// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <string>
#include <memory>

#include <at/Result.hpp>
#include <at/Commands.hpp>

#include "service-cellular/requests/EchoCancelRequest.hpp"

namespace cellular
{
    std::string EchoCancelRequest::command()
    {
        if (request == "EchoCancelEarSpk")
            return std::string(at::factory(at::AT::EEC_EARSPK));
        else
            return std::string(at::factory(at::AT::EEC_LOUDSPK));
    }

    void EchoCancelRequest::handle(RequestHandler &h, at::Result &result)
    {
        h.handle(*this, result);
    }

    std::unique_ptr<EchoCancelRequest> EchoCancelRequest::create(const std::string &data, GroupMatch)
    {
        return std::make_unique<EchoCancelRequest>(data);
    }

}; // namespace cellular
