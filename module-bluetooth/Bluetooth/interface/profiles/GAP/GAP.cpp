// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "GAP.hpp"

#include <log/log.hpp>
#include <service-bluetooth/BluetoothMessage.hpp>
#include "Service/Bus.hpp"

extern "C"
{
#include "btstack.h"
#include "hci.h"
};

btstack_packet_callback_registration_t cb_handler;

static auto startScan() -> int;
static void packetHandler(std::uint8_t packet_type, std::uint16_t channel, std::uint8_t *packet, std::uint16_t size);
static auto getDeviceIndexForAddress(std::vector<Devicei> &devs, bd_addr_t addr) -> int;

using ScanState                       = enum STATE { init, active, done };
ScanState state                       = ScanState::init;
constexpr auto inquiryIntervalSeconds = 5;

namespace bluetooth::GAP
{
    std::vector<Devicei> devices;
    static sys::Service *ownerService = nullptr;

    void setOwnerService(sys::Service *service)
    {
        ownerService = service;
    }

    auto registerScan() -> Error
    {
        LOG_INFO("GAP register scan!");
        /// -> this have to be called prior to power on!
        hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);
        cb_handler.callback = &packetHandler;
        hci_add_event_handler(&cb_handler);
        return Error();
    }

    auto scan() -> Error
    {
        if (hci_get_state() == HCI_STATE_WORKING) {
            if (auto ret = startScan(); ret != 0) {
                LOG_ERROR("Start scan error!: 0x%X", ret);
                return Error(Error::LibraryError, ret);
            }
        }
        else {
            return Error(Error::NotReady);
        }
        return Error();
    }

    void stopScan()
    {
        gap_inquiry_force_stop();
        LOG_INFO("Scan stopped!");
    }

    void setVisibility(bool visibility)
    {
        gap_discoverable_control(static_cast<std::uint8_t>(visibility));
        LOG_INFO("Visibility: %s", visibility ? "true" : "false");
    }

    auto pair(std::uint8_t *addr, std::uint8_t protectionLevel) -> bool
    {
        if (hci_get_state() == HCI_STATE_WORKING) {
            return gap_dedicated_bonding(addr, protectionLevel) == 0;
        }
        return false;
    }

} // namespace bluetooth::GAP

auto getDeviceIndexForAddress(std::vector<Devicei> &devs, bd_addr_t addr) -> int
{
    auto result = std::find_if(
        std::begin(devs), std::end(devs), [&addr](Devicei &device) { return bd_addr_cmp(addr, device.address) == 0; });

    auto index = std::distance(std::begin(devs), result);
    return (result == std::end(devs)) ? -1 : index;
}

void sendDevices()
{
    auto msg = std::make_shared<BluetoothScanResultMessage>(bluetooth::GAP::devices);
    sys::Bus::SendUnicast(msg, "ApplicationSettings", bluetooth::GAP::ownerService);
    sys::Bus::SendUnicast(msg, "ApplicationSettingsNew", bluetooth::GAP::ownerService);
}

auto startScan() -> int
{
    LOG_INFO("Starting inquiry scan..");
    return gap_inquiry_start(inquiryIntervalSeconds);
}

auto remoteNameToFetch() -> bool
{
    auto result = std::find_if(std::begin(bluetooth::GAP::devices),
                               std::end(bluetooth::GAP::devices),
                               [](Devicei &device) { return device.state == REMOTE_NAME_REQUEST; });

    auto index = std::distance(std::end(bluetooth::GAP::devices), result);
    return index != 0;
}

void fetchRemoteName()
{
    std::for_each(std::begin(bluetooth::GAP::devices), std::end(bluetooth::GAP::devices), [](Devicei device) {
        if (device.state == REMOTE_NAME_REQUEST) {
            device.state = REMOTE_NAME_INQUIRED;
            LOG_INFO("Get remote name of %s...", bd_addr_to_str(device.address));
            gap_remote_name_request(device.address, device.pageScanRepetitionMode, device.clockOffset | 0x8000);
            return;
        }
    });
}

void continueScanning()
{
    if (remoteNameToFetch()) {
        fetchRemoteName();
        return;
    }
    startScan();
}
auto updateDeviceName(std::uint8_t *packet, bd_addr_t &addr) -> bool
{
    reverse_bd_addr(&packet[3], addr);
    auto index = getDeviceIndexForAddress(bluetooth::GAP::devices, addr);
    if (index >= 0) {
        if (packet[2] == 0) {
            LOG_INFO("Name: '%s'", &packet[9]);
            bluetooth::GAP::devices[index].state = REMOTE_NAME_FETCHED;
            bluetooth::GAP::devices[index].name  = std::string{reinterpret_cast<const char *>(&packet[9])};
            return true;
        }
        else {
            LOG_INFO("Failed to get name: page timeout");
        }
    }
    return false;
}

void addNewDevice(std::uint8_t *packet, bd_addr_t &addr)
{
    Devicei device;
    device.setAddress(&addr);
    device.pageScanRepetitionMode = gap_event_inquiry_result_get_page_scan_repetition_mode(packet);
    device.clockOffset            = gap_event_inquiry_result_get_clock_offset(packet);

    LOG_INFO("Device found: %s ", bd_addr_to_str(addr));
    LOG_INFO("with COD: 0x%06x, ", static_cast<unsigned int>(gap_event_inquiry_result_get_class_of_device(packet)));
    LOG_INFO("pageScan %d, ", device.pageScanRepetitionMode);
    LOG_INFO("clock offset 0x%04x", device.clockOffset);
    if (gap_event_inquiry_result_get_rssi_available(packet) != 0u) {
        LOG_INFO(", rssi %d dBm", static_cast<int8_t>(gap_event_inquiry_result_get_rssi(packet)));
    }
    if (gap_event_inquiry_result_get_name_available(packet) != 0u) {
        auto name   = gap_event_inquiry_result_get_name(packet);
        device.name = std::string{reinterpret_cast<const char *>(name)};
        LOG_INFO(", name '%s'", device.name.c_str());
        device.state = REMOTE_NAME_FETCHED;
    }
    else {
        device.state = REMOTE_NAME_REQUEST;
    }

    bluetooth::GAP::devices.emplace_back(device);
}

/* @text In ACTIVE, the following events are processed:
 *  - GAP Inquiry result event: BTstack provides a unified inquiry result that contain
 *    Class of Device (CoD), page scan mode, clock offset. RSSI and name (from EIR) are optional.
 *  - Inquiry complete event: the remote name is requested for devices without a fetched
 *    name. The state of a remote name can be one of the following:
 *    REMOTE_NAME_REQUEST, REMOTE_NAME_INQUIRED, or REMOTE_NAME_FETCHED.
 *  - Remote name request complete event: the remote name is stored in the table and the
 *    state is updated to REMOTE_NAME_FETCHED. The query of remote names is continued.
 */
void activeStateHandler(std::uint8_t eventType, std::uint8_t *packet, bd_addr_t &addr)
{
    switch (eventType) {

    case GAP_EVENT_INQUIRY_RESULT: {
        gap_event_inquiry_result_get_bd_addr(packet, addr);
        auto index = getDeviceIndexForAddress(bluetooth::GAP::devices, addr);
        if (index >= 0) {
            break; // already in our list
        }
        addNewDevice(packet, addr);
        sendDevices();
    } break;

    case GAP_EVENT_INQUIRY_COMPLETE:
        for (auto &device : bluetooth::GAP::devices) {
            // retry remote name request
            if (device.state == REMOTE_NAME_INQUIRED) {
                device.state = REMOTE_NAME_REQUEST;
            }
        }
        continueScanning();
        break;

    case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE: {
        if (updateDeviceName(packet, addr)) {
            sendDevices();
        }
        continueScanning();
    } break;
    case GAP_EVENT_DEDICATED_BONDING_COMPLETED: {
        auto result = packet[2];
        auto msg    = std::make_shared<BluetoothPairResultMessage>(result == 0u);
        sys::Bus::SendUnicast(msg, "ApplicationSettings", bluetooth::GAP::ownerService);
        sys::Bus::SendUnicast(msg, "ApplicationSettingsNew", bluetooth::GAP::ownerService);
    } break;
    default:
        break;
    }
}
static void packetHandler(std::uint8_t packet_type, std::uint16_t channel, std::uint8_t *packet, std::uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);

    bd_addr_t addr;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    const auto eventType = hci_event_packet_get_type(packet);
    switch (state) {
    case ScanState::init:
        if (eventType == BTSTACK_EVENT_STATE) {
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                state = ScanState::active;
            }
        }
        break;
    case ScanState::active:
        activeStateHandler(eventType, packet, addr);
        break;
    default:
        break;
    }
}
