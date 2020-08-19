#include "QueryEventsGetAllLimited.hpp"

namespace db::query::events
{
    GetAllLimited::GetAllLimited(const uint32_t &offset, const uint32_t &limit)
        : Query(Query::Type::Read), offset(offset), limit(limit)
    {}

    auto GetAllLimited::debugInfo() const -> std::string
    {
        return "GetAllLimited";
    }

    GetAllLimitedResult::GetAllLimitedResult(std::unique_ptr<std::vector<EventsRecord>> records)
        : records(std::move(records))
    {}

    auto GetAllLimitedResult::getResult() -> std::unique_ptr<std::vector<EventsRecord>>
    {
        return std::move(records);
    }

    auto GetAllLimitedResult::debugInfo() const -> std::string
    {
        return "GetAllLimitedResult";
    }
} // namespace db::query::events
