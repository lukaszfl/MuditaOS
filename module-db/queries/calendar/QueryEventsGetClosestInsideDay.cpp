#include "QueryEventsGetClosestInsideDay.hpp"

namespace db::query::events
{
    GetClosestInsideDay::GetClosestInsideDay(TimePoint start_date, TimePoint day_date)
        : Query(Query::Type::Read), start_date(start_date), day_date(day_date)
    {}

    auto GetClosestInsideDay::debugInfo() const -> std::string
    {
        return "GetClosestInsideDay";
    }

    GetClosestInsideDayResult::GetClosestInsideDayResult(std::unique_ptr<EventsRecord> record)
        : record(std::move(record))
    {}

    auto GetClosestInsideDayResult::getResult() -> std::unique_ptr<EventsRecord>
    {
        return std::move(record);
    }

    auto GetClosestInsideDayResult::debugInfo() const -> std::string
    {
        return "GetClosestInsideDayResult";
    }
} // namespace db::query::events
