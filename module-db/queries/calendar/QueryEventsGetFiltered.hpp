#pragma once

#include "module-db/Interface/EventsRecord.hpp"
#include <Common/Query.hpp>
#include <string>

namespace db::query::events
{
    class GetFiltered : public Query
    {
      public:
        GetFiltered(std::string date_filter);
        [[nodiscard]] auto debugInfo() const -> std::string override;

        std::string date_filter;
    };

    class GetFilteredResult : public QueryResult
    {
        std::unique_ptr<std::vector<EventsRecord>> records;

      public:
        GetFilteredResult(std::unique_ptr<std::vector<EventsRecord>> records);
        [[nodiscard]] auto getResult() -> std::unique_ptr<std::vector<EventsRecord>>;

        [[nodiscard]] auto debugInfo() const -> std::string override;
    };

} // namespace db::query::events
