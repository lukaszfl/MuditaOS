#pragma once

extern "C"
{
#include <FreeRTOS.h>
#include <portmacro.h>
}

namespace cpp_freertos
{
    class CriticalSectionGuard
    {
      public:
        CriticalSectionGuard();
        ~CriticalSectionGuard();
        CriticalSectionGuard(const CriticalSectionGuard &) = delete;
        CriticalSectionGuard(CriticalSectionGuard &&)      = delete;

        CriticalSectionGuard &operator=(const CriticalSectionGuard &) = delete;
        CriticalSectionGuard &operator=(CriticalSectionGuard &&) = delete;

      private:
        UBaseType_t savedInterruptStatus;
    };
}; // namespace cpp_freertos
