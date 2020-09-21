#include <FreeRTOS.h>
#include <task.h>
#include <stdint.h>
#include "btstack_debug.h"

uint32_t hal_time_ms(void)
{
    return xTaskGetTickCount();
}
