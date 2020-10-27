#pragma once

extern "C"
{
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stream_buffer.h"
#include "message_buffer.h"
#include "event_groups.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "composite.h"
}

#include <errno.h>
#include <iostream>
#include <log/log.hpp>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERIAL_BUFFER_LEN 512 // this matches the buffer length in rt1051 cdc implementaion

namespace bsp
{
    usb_cdc_vcom_struct_t *usbInit(xQueueHandle);
    void usbCDCReceive(void *ptr);
    int usbCDCSend(std::string *sendMsg);
} // namespace bsp
