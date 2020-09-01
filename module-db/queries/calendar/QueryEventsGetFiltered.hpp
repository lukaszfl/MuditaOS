#pragma once

#include "module-db/Interface/EventsRecord.hpp"
#include <Common/Query.hpp>
#include <string>
#include <module-utils/date/include/date/dateCommon.h>

namespace db::query::events
{
    class GetFiltered : public Query
    {
      public:
        GetFiltered(TimePoint date_filter);
        [[nodiscard]] auto debugInfo() const -> std::string override;

        TimePoint date_filter;
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
