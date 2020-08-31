
#include "UpdateEndpoint.hpp"
#include "DesktopMessages.hpp"
#include "ServiceDesktop.hpp"

UpdateEndpoint::UpdateEndpoint(sys::Service *ownerServicePtr) : Endpoint(ownerServicePtr)
{
    debugName = "UpdateEndpoint";
}

auto UpdateEndpoint::handle(Context &context) -> EpResult
{
    switch (context.getMethod()) {
    case http::Method::post:
        return run(context);
    case http::Method::get:
        return getUpdates(context);
    default:
        break;
    }
    return DefaultResult();
}

auto UpdateEndpoint::run(Context &context) -> EpResult
{
    auto result = std::make_unique<StateSwitchResult>();
    std::string fileName = context.getBody()["fileName"].string_value();

    result->setMessage(std::make_shared<sdesktop::UpdateOsMessage>(fileName, context.getUuid()));
    result->setResponse(CommunicationResponse{
        .endpoint = EndpointType::update,
        .body     = json11::Json::object({{ParserStateMachine::json::updateReady, true}}),
        .uuid     = context.getUuid(),
        .status   = http::Code::OK,
    });

    return std::move(result);
}

auto UpdateEndpoint::getUpdates(Context &context) -> EpResult
{
    auto result = std::make_unique<StateSwitchResult>();

    json11::Json fileList         = vfs.listdir(purefs::dir::os_updates.c_str(), updateos::extension::update, true);
    json11::Json responseBodyJson = json11::Json::object{{ParserStateMachine::json::updateFileList, fileList}};

    result->setResponse(CommunicationResponse{.endpoint = EndpointType::update,
                                              .body     = responseBodyJson,
                                              .uuid     = context.getUuid(),
                                              .status   = http::Code::OK});

    return std::move(result);
}
