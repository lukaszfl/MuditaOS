#pragma once

#include "module-db/Interface/EventsRecord.hpp"
#include <Common/Query.hpp>
#include <string>
#include <module-apps/application-calendar/data/dateCommon.hpp>

namespace db::query::events
{
    class GetClosestInsideDay : public Query
    {
      public:
        GetClosestInsideDay(TimePoint start_date, TimePoint day_date);
        [[nodiscard]] auto debugInfo() const -> std::string override;

        TimePoint start_date;
        TimePoint day_date;
    };

    class GetClosestInsideDayResult : public QueryResult
    {
        std::unique_ptr<EventsRecord> record;

      public:
        GetClosestInsideDayResult(std::unique_ptr<EventsRecord> record);
        [[nodiscard]] auto getResult() -> std::unique_ptr<EventsRecord>;

        [[nodiscard]] auto debugInfo() const -> std::string override;
    };

} // namespace db::query::events
