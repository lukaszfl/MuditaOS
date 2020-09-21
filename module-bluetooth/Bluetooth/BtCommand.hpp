#pragma once
#include <FreeRTOS.h>
#include <task.h>

#include "BluetoothWorker.hpp"
#include "Error.hpp"
#include <functional>

namespace Bt
{
    Error initialize_stack();
    Error register_hw_error_callback(std::function<void(uint8_t)> new_callback = nullptr);
    Error set_name(std::string &name);
    Error run_stack(TaskHandle_t *handle);
    namespace GAP
    {
        /// THIS have to be called prior to Bt system start!
        Error register_scan();
        Error scan();
        Error set_visibility(bool visibility);
    }; // namespace GAP
    namespace PAN
    {
        Error bnep_start();
        Error bnep_setup();
    } // namespace PAN

    namespace A2DP
    {
        Error init();
        void start();
        void stop();
        static void source_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
        static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    } // namespace A2DP

    namespace HSP
    {
        void init();
        void start();
        void stop();
    } // namespace HSP
};    // namespace Bt
