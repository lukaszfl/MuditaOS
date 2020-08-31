#pragma once

#include "Endpoint.hpp"
#include "Service/Service.hpp"

using namespace ParserStateMachine;

class UpdateEndpoint : public Endpoint
{
  private:
    auto run(Context &context) -> EpResult;
    auto getUpdates(Context &context) -> EpResult;

  public:
    UpdateEndpoint(sys::Service *ownerServicePtr);

    auto handle(Context &context) -> EpResult;
};
