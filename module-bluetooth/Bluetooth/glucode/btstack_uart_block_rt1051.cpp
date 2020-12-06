// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <bsp/bluetooth/Bluetooth.hpp>
#include <log/log.hpp>
#include <module-bluetooth/Bluetooth/BluetoothWorker.hpp>

using namespace bsp;

#include "btstack_uart_block_rt1051.h"
#include <hci_transport.h>
#include <btstack_run_loop.h>
#include <btstack_uart_block.h>
#include <stddef.h> // for null
#include <btstack_run_loop_freertos.h>
static btstack_data_source_t test_datasource;

#if DEBUG_BLUETOOTH_HCI_COMS >= 1
#include <sstream>
#define logHciEvt(...) LOG_DEBUG(__VA_ARGS__)
#else
#define logHciEvt(...)
#endif

static void btstack_uart_embedded_process(btstack_data_source_t *_ds, btstack_data_source_callback_type_t callback_type)
{
    switch (callback_type) {
    case DATA_SOURCE_CALLBACK_POLL: {
        auto bt = BlueKitchen::getInstance();

        Bt::Message notification = Bt::Message::EvtErrorRec;
        if (xQueueReceive(bt->qHandle, &notification, 0) != pdTRUE) {
            LOG_ERROR("Queue receive failure!");
            break;
        }
        switch (notification) {
        case Bt::Message::EvtSending:
            logHciEvt("[evt] sending");
            break;
        case Bt::Message::EvtSent:
            logHciEvt("[evt] sent");
            if (bt->write_done_cb) {
                bt->write_done_cb();
            }
            break;
        case Bt::Message::EvtReceiving:
            logHciEvt("[evt] receiving");
            break;
        case Bt::Message::EvtReceived: {
#if DEBUG_BLUETOOTH_HCI_COMS >= 3
            std::stringstream ss;
            for (int i = 0; i < bt->read_len; ++i) {
                ss << " 0x" << std::hex << (int)*(bt->read_buff + i);
            }
            logHciEvt("[evt] BT DMA received <-- [%ld]>%s<", bt->read_len, ss.str().c_str());
#endif
            bt->read_len = 0;

            if (bt->read_ready_cb) {
                bt->read_ready_cb();
            }
        } break;
        case Bt::Message::EvtSendingError:
        case Bt::Message::EvtReceivingError:
        case Bt::Message::EvtUartError:
        case Bt::Message::EvtRecUnwanted:
            LOG_ERROR("Uart error [%d]: %s", notification, Bt::MessageCstr(notification));
            break;
        default:
            LOG_ERROR("ERROR");
        }
        break;
    }
    default:
        break;
    }
}
    // #define DEBUG_UART

    // and it's hci_transport_config_uart_t which is a bit different...
    static int uart_rt1051_init(const btstack_uart_config_t *config)
    {
        LOG_INFO("Create BlueKitchen interface");
        BlueKitchen::getInstance();
        return 0;
    }

    static int uart_rt1051_open()
    {
        LOG_INFO("BlueKitchen uart open");
        BlueKitchen::getInstance()->open();
        btstack_run_loop_set_data_source_handler(&test_datasource, &btstack_uart_embedded_process);
        btstack_run_loop_enable_data_source_callbacks(&test_datasource, DATA_SOURCE_CALLBACK_POLL);
        btstack_run_loop_add_data_source(&test_datasource);
        return 0;
    }

    static int uart_rt1051_close()
    {
        LOG_INFO("BlueKitchen uart close");
        btstack_run_loop_disable_data_source_callbacks(&test_datasource, DATA_SOURCE_CALLBACK_POLL);
        btstack_run_loop_remove_data_source(&test_datasource);
        BlueKitchen::getInstance()->close();

        return 0;
    }

    static void uart_rt1051_set_block_received(void (*handler)(void))
    {
        BlueKitchen::getInstance()->read_ready_cb = handler;
    }

    static void uart_rt1051_set_block_sent(void (*handler)(void))
    {
        BlueKitchen::getInstance()->write_done_cb = handler;
    }

    static int uart_rt1051_set_baudrate(uint32_t baudrate)
    {
        BlueKitchen::getInstance()->set_baudrate(baudrate);
        return 0;
    }

    static int uart_rt1051_set_parity(int pairity)
    {
        // Not implemented
        LOG_INFO("BlueKitchen set pairity: %d", pairity);
        return 0;
    }

    static int uart_rt1051_set_flowcontrol(int flowcontrol)
    {
        LOG_INFO("BlueKitchen set flowcontrol: %d", flowcontrol);
        // BlueKitchen::getInstance()->set_rts(); ??
        return 0;
    }

    static void uart_rt1051_receive_block(uint8_t *buffer, uint16_t len)
    {
#ifdef DEBUG_UART
        LOG_INFO("<-- read: %d", len);
#endif
        BlueKitchen::getInstance()->read(buffer, len);
    }

    static void uart_rt1051_send_block(const uint8_t *buffer, uint16_t length)
    {
#ifdef DEBUG_UART
        LOG_INFO("--> write: %d", length);
#endif
        BlueKitchen::getInstance()->write(buffer, length);
    }

    static const btstack_uart_block_t btstack_uart_posix = {
        /* int  (*init)(hci_transport_config_uart_t * config); */ uart_rt1051_init,
        /* int  (*open)(void); */ uart_rt1051_open,
        /* int  (*close)(void); */ uart_rt1051_close,
        /* void (*set_block_received)(void (*handler)(void)); */ uart_rt1051_set_block_received,
        /* void (*set_block_sent)(void (*handler)(void)); */ uart_rt1051_set_block_sent,
        /* int  (*set_baudrate)(uint32_t baudrate); */ uart_rt1051_set_baudrate,
        /* int  (*set_parity)(int parity); */ uart_rt1051_set_parity,
        /* int  (*set_flowcontrol)(int flowcontrol); */ NULL, // uart_rt1051_set_flowcontrol,
        /* void (*receive_block)(uint8_t *buffer, uint16_t len); */ uart_rt1051_receive_block,
        /* void (*send_block)(const uint8_t *buffer, uint16_t length); */ uart_rt1051_send_block,
        /* int (*get_supported_sleep_modes); */ NULL,
        /* void (*set_sleep)(btstack_uart_sleep_mode_t sleep_mode); */ NULL,
        /* void (*set_wakeup_handler)(void (*handler)(void)); */ NULL,
    };

    const btstack_uart_block_t *btstack_uart_block_rt1051_instance()
    {
        LOG_INFO("btstack_uart_block_rt1051_instance");
        return &btstack_uart_posix;
    }
