#include "Timer.hpp"
#include "Service.hpp"               // for Service, Service::Timers
#include "Service/TimerMessage.hpp"  // for TimerMessage
#include "log/log.hpp"               // for LOG_ERROR
#include <stdint.h>                  // for uint32_t
#include <Service/Bus.hpp>           // for Bus
#include <limits>                    // for numeric_limits
#include <memory>                    // for make_shared

#ifdef DEBUG_TIMER
#define log_timers(...) LOG_DEBUG(__VA_ARGS__)
#else
#define log_timers(...)
#endif

namespace sys
{

    const ms Timer::timeout_infinite = std::numeric_limits<ms>().max();
    // debug variable
    static uint32_t timer_id;

    auto toName(const std::string &val, uint32_t no) -> std::string
    {
        return val + "_" + std::to_string(no);
    }

    Timer::Timer(const std::string &name, Service *service, ms interval, Type type)
        : cpp_freertos::Timer((toName(name,timer_id)).c_str(), interval, type == Type::Periodic),
          parent(service), type(type), interval(interval), name(toName(name, timer_id))
    {
        if (service != nullptr) {
            service->getTimers().attach(this);
        }
        else {
            LOG_ERROR("Bad timer creation!");
        }
        ++timer_id;
        log_timers("Timer %s created", name.c_str());
    }

    Timer::Timer(Service *parent, ms interval, Type type) : Timer("Timer", parent, interval, type)
    {
    }

    Timer::~Timer()
    {
        stop();
    }

    void Timer::Run()
    {
        auto msg = std::make_shared<TimerMessage>(this);
        if (parent == nullptr) {
            LOG_ERROR("Timer %s error: no parent service", name.c_str());
            return;
        }
        if (!Bus::SendUnicast(msg, parent->GetName(), parent)) {
            LOG_ERROR("Timer error: bus error");
            return;
        }
    }

    void Timer::start()
    {
        log_timers("Timer %s start!", name.c_str());
        Start(0);
    }

    void Timer::reload(ms from_time)
    {
        log_timers("Timer %s reload!", name.c_str());
        Start(from_time);
    }

    void Timer::stop()
    {
        log_timers("Timer %s stop!", name.c_str());
        Stop(0);
    }

    void Timer::setInterval(ms new_interval)
    {
        log_timers("Timer %s set interval to: %d ms!", name.c_str(), interval);
        interval = new_interval;
        SetPeriod(new_interval, 0);
    }

    void Timer::onTimeout()
    {
        if (callback) {
            log_timers("Timer %s callback run!", name.c_str());
            callback(*this);
        }
    }
} // namespace sys
