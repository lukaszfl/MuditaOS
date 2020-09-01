#include "QueryEventsGetFiltered.hpp"

namespace db::query::events
{
    GetFiltered::GetFiltered(TimePoint date_filter) : Query(Query::Type::Read), date_filter(date_filter)
    {}

    auto GetFiltered::debugInfo() const -> std::string
    {
        return "GetFiltered";
    }

    GetFilteredResult::GetFilteredResult(std::unique_ptr<std::vector<EventsRecord>> records)
        : records(std::move(records))
    {}

    auto GetFilteredResult::getResult() -> std::unique_ptr<std::vector<EventsRecord>>
    {
        return std::move(records);
    }

    auto GetFilteredResult::debugInfo() const -> std::string
    {
        return "GetFilteredResult";
    }
} // namespace db::query::events
