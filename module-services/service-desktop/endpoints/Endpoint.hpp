#pragma once
#include "ParserUtils.hpp"
#include "json/json11.hpp"
#include "Common/Query.hpp"
#include "Context.hpp"
#include "Service/Service.hpp"
#include <string>
#include <memory>

namespace ParserStateMachine
{

    class MessageHandler;

    struct CommunicationResponse
    {
        EndpointType endpoint;
        http::Code::OK status;
        json11::Json uuid;
        json11::Json body;
        std::string encode()
        { /// here encode it all
            /// and dump str
            return "";
        }
    };

    class StateSwitchResult : HandleResult
    {
        sys::Service *owner = nullptr;
        std::unique_ptr<sys::Message> message = nullptr;
        CommunicationResponse response;

      public:
        StateSwitchResult()
        {
        }

      private:

        unique_ptr<sys::Message> getMessage() final
        {
            return std::move(message);
        }

        CommunicationResponse &getResponse() final
        {
            return response;
        }

      public:
        void setMessage(std::unique_ptr<sys::Message> to_send) 
        {
            message = std::move(to_send);
        }

        void setResponse(CommunicationResponse resp)
        {
            response = resp;
        }
    };

    class HandleResult
    {
        friend MessageHandler;
        public:
            HandleResult() = default;
            virtual ~HandleResult() = default;
        private:
            /// run state switches / change context of app
            virtual void execute() = 0;
            /// send response via interface
            virtual json11::Json respond() = 0;
    };

    class NoneResult : public HandleResult
    {
        public:
            NoneResult() = default;
            void execute() final {};
            json11::Json respond() final {};
    };

    using EpResult = std::unique_ptr<HandleResult>;

    inline auto DefaultResult() 
    {
        return std::make_unique<NoneResult>();
    }

    class Endpoint
    {

      public:
        Endpoint(sys::Service *_ownerServicePtr) : ownerServicePtr(_ownerServicePtr){};
        virtual ~Endpoint()                                  = default;
        virtual auto handle(Context &context) -> EpResult = 0;
        auto c_str() -> const char *
        {
            return debugName.c_str();
        }

        static auto buildResponseStr(std::size_t responseSize, std::string responsePayloadString) -> std::string
        {
            constexpr auto pos                 = 0;
            constexpr auto count               = 1;
            std::string responsePayloadSizeStr = std::to_string(responseSize);
            while (responsePayloadSizeStr.length() < message::size_length) {
                responsePayloadSizeStr.insert(pos, count, '0');
            }

            std::string responseStr = message::endpointChar + responsePayloadSizeStr + responsePayloadString;
            return responseStr;
        }

        // then here return ComResponse
        static auto createSimpleResponse(sys::ReturnCodes status, int endpoint, uint32_t uuid, json11::Json body)
            -> std::string
        {
            json11::Json responseJson = json11::Json::object{
                {json::endpoint, endpoint},
                {json::status,
                 static_cast<int>(status == sys::ReturnCodes::Success ? http::Code::OK
                                                                      : http::Code::InternalServerError)},
                {json::uuid, std::to_string(uuid)},
                {json::body, body}};
            return Endpoint::buildResponseStr(responseJson.dump().size(), responseJson.dump());
        }

        static auto createSimpleResponse(bool status, int endpoint, uint32_t uuid, json11::Json body) -> std::string
        {
            json11::Json responseJson = json11::Json::object{
                {json::endpoint, endpoint},
                {json::status, static_cast<int>(status ? http::Code::OK : http::Code::InternalServerError)},
                {json::uuid, std::to_string(uuid)},
                {json::body, body}};

            return Endpoint::buildResponseStr(responseJson.dump().size(), responseJson.dump());
        }

      protected:
        std::string debugName         = "";
        sys::Service *ownerServicePtr = nullptr;
    };

} // namespace ParserStateMachine
