// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <parser/HttpEnums.hpp>
#include <endpoints/ResponseContext.hpp>

namespace sys
{
    class Service;
} // namespace sys

namespace parserFSM
{
    class Context;
    enum class sent
    {
        yes,
        no
    };

    /// base helper class to avoid copies
    class BaseHelper
    {
      public:
        using ProcessResult = std::pair<sent, std::optional<endpoint::ResponseContext>>;

      protected:
        sys::Service *owner = nullptr;
        /// by default - result = not sent
        [[nodiscard]] virtual auto processPut(Context &) -> ProcessResult;
        /// by default - result = not sent
        [[nodiscard]] virtual auto processGet(Context &) -> ProcessResult;
        /// by default - result = not sent
        [[nodiscard]] virtual auto processPost(Context &) -> ProcessResult;
        /// by default - result = not sent
        [[nodiscard]] virtual auto processDelete(Context &) -> ProcessResult;

      public:
        explicit BaseHelper(sys::Service *p) : owner(p)
        {}

        /// generall processing function
        ///
        /// we should define processing functions, not copy switch cases
        /// as we are super ambiguous how we should really handle responses
        /// here we can either:
        /// return true - to mark that we responded on this request
        /// return fale - and optionally respond that we didn't handle the request
        [[nodiscard]] auto process(const http::Method &method, Context &context) -> ProcessResult;
    };
}; // namespace parserFSM
