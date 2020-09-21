#pragma once

#include <stdint.h>
#include <module-bsp/bsp/keyboard/key_codes.hpp>

extern "C" {
	#include "FreeRTOS.h"
	#include "task.h"
	#include "queue.h"
}

#include "../common.hpp"

namespace bsp {

namespace magnetometer{

	int32_t init(xQueueHandle qHandle);

	bool isPresent(void);
	bsp::Board GetBoard(void);
	float getTemperature();
    int16_t getAxisX();
    bsp::KeyCodes getDiscrete(int16_t raw_measurement);
}

}
