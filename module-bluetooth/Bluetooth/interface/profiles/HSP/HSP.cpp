// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <log/log.hpp>
#include "HSPImpl.hpp"
#include "HSP.hpp"

#include <Bluetooth/Error.hpp>
#include <service-evtmgr/Constants.hpp>
#include <BluetoothWorker.hpp>
#include <service-audio/AudioMessage.hpp>
#include <service-bluetooth/Constants.hpp>
#include <service-bluetooth/messages/AudioVolume.hpp>
#include <service-cellular/service-cellular/CellularServiceAPI.hpp>
#include <service-audio/AudioServiceAPI.hpp>
#include <service-evtmgr/Constants.hpp>

extern "C"
{
#include "btstack.h"
#include "btstack_run_loop_freertos.h"
#include "btstack_stdin.h"
#include <btstack_defines.h>
}

namespace bluetooth
{
    static btstack_cvsd_plc_state_t cvsd_plc_state;

    bool CellularInterfaceImpl::answerIncomingCall(sys::Service *service)
    {
        return CellularServiceAPI::AnswerIncomingCall(service);
    }

    bool CellularInterfaceImpl::hangupCall(sys::Service *service)
    {
        return CellularServiceAPI::HangupCall(service);
    }

    HSP::HSP() : pimpl(std::make_unique<HSPImpl>(HSPImpl()))
    {}

    HSP::HSP(HSP &&other) noexcept : pimpl(std::move(other.pimpl))
    {}

    auto HSP::operator=(HSP &&other) noexcept -> HSP &
    {
        if (&other == this) {
            return *this;
        }
        pimpl = std::move(other.pimpl);
        return *this;
    }

    auto HSP::init() -> Error::Code
    {
        return pimpl->init();
    }

    void HSP::setDeviceAddress(uint8_t *addr)
    {
        pimpl->setDeviceAddress(addr);
    }

    void HSP::setOwnerService(const sys::Service *service)
    {
        ownerService = service;
        pimpl->setOwnerService(service);
    }

    void HSP::connect()
    {
        pimpl->connect();
    }

    void HSP::disconnect()
    {
        pimpl->disconnect();
    }

    void HSP::start()
    {
        pimpl->start();
    }

    void HSP::stop()
    {
        pimpl->stop();
    }

    auto HSP::startRinging() const noexcept -> Error::Code
    {
        pimpl->startRinging();
        return Error::Success;
    }

    auto HSP::stopRinging() const noexcept -> Error::Code
    {
        pimpl->stopRinging();
        return Error::Success;
    }

    auto HSP::initializeCall() const noexcept -> Error::Code
    {
        pimpl->initializeCall();
        return Error::Success;
    }

    HSP::~HSP() = default;

    uint16_t HSP::HSPImpl::scoHandle = HCI_CON_HANDLE_INVALID;
    bd_addr_t HSP::HSPImpl::deviceAddr;
    std::array<char, commandBufferLength> HSP::HSPImpl::ATcommandBuffer;
    std::array<uint8_t, serviceBufferLength> HSP::HSPImpl::serviceBuffer;
    std::unique_ptr<SCO> HSP::HSPImpl::sco;
    std::unique_ptr<CellularInterface> HSP::HSPImpl::cellularInterface = nullptr;
    const sys::Service *HSP::HSPImpl::ownerService;
    std::string HSP::HSPImpl::agServiceName = "PurePhone HSP";
    bool HSP::HSPImpl::isConnected          = false;
    std::shared_ptr<HSPAudioDevice> HSP::HSPImpl::audioDevice;

    void HSP::HSPImpl::sendAudioEvent(audio::EventType event, audio::Event::DeviceState state)
    {
        auto evt       = std::make_shared<audio::Event>(event, state);
        auto msg       = std::make_shared<AudioEventRequest>(std::move(evt));
        auto &busProxy = const_cast<sys::Service *>(ownerService)->bus;
        busProxy.sendUnicast(std::move(msg), service::name::evt_manager);
    }

    static void sco_demo_receive_CVSD(uint8_t *packet, uint16_t size)
    {
        int16_t audio_frame_out[128]; //

        if (size > sizeof(audio_frame_out)) {
            LOG_DEBUG("sco_demo_receive_CVSD: SCO packet larger than local output buffer - dropping data.\n");
            return;
        }

        const int audio_bytes_read = size - 3;
        const int num_samples      = audio_bytes_read / 2;

        // convert into host endian
        int16_t audio_frame_in[128];
        int i;
        for (i = 0; i < num_samples; i++) {
            audio_frame_in[i] = little_endian_read_16(packet, 3 + i * 2);
        }

        // treat packet as bad frame if controller does not report 'all good'
        bool bad_frame = (packet[1] & 0x30) != 0;

        btstack_cvsd_plc_process_data(&cvsd_plc_state, bad_frame, audio_frame_in, num_samples, audio_frame_out);
        // btstack_ring_buffer_write(&audio_output_ring_buffer, (uint8_t *)audio_frame_out, audio_bytes_read);
    }

    void HSP::HSPImpl::packetHandler(uint8_t packetType, uint16_t channel, uint8_t *event, uint16_t eventSize)
    {
        switch (packetType) {
        case HCI_SCO_DATA_PACKET:
            if (READ_SCO_CONNECTION_HANDLE(event) != scoHandle) {
                break;
            }
            if (audioDevice != nullptr) {
                audioDevice->receiveCVSD(audio::AbstractStream::Span{.data = event, .dataSize = eventSize});
            }
            else {
                static uint32_t packets       = 0;
                static uint32_t crc_errors    = 0;
                static uint32_t data_received = 0;
                static uint32_t byte_errors   = 0;

                data_received += eventSize - 3;
                packets++;
                if (data_received > 100000) {
                    LOG_WARN("Summary: data %07u, packets %04u, packet with crc errors %0u, byte errors %04u\n",
                             (unsigned int)data_received,
                             (unsigned int)packets,
                             (unsigned int)crc_errors,
                             (unsigned int)byte_errors);
                    crc_errors    = 0;
                    byte_errors   = 0;
                    data_received = 0;
                    packets       = 0;
                }
            }
            break;

        case HCI_EVENT_PACKET:
            processHCIEvent(event);
            break;
        default:
            break;
        }
    }

    void HSP::HSPImpl::processHCIEvent(uint8_t *event)
    {
        switch (hci_event_packet_get_type(event)) {
        case HCI_EVENT_SCO_CAN_SEND_NOW:
            if (audioDevice != nullptr) {
                LOG_WARN("Audio device is not null");
                audioDevice->onDataSend(scoHandle);
            }
            else {
                LOG_WARN("Audio device is null");
                const auto scoPacketLength  = hci_get_sco_packet_length();
                const auto scoPayloadLength = scoPacketLength - 3;

                hci_reserve_packet_buffer();
                auto *sco_packet = hci_get_outgoing_packet_buffer();
                for (int j = 0; j < scoPayloadLength; j++) {
                    sco_packet[3 + j] = 0x00;
                }
                // memset(sco_packet + 3, 0, scoPayloadLength);
                // set handle + flags
                little_endian_store_16(sco_packet, 0, scoHandle);
                // set len
                sco_packet[2] = scoPacketLength;
                // finally send packet
                hci_send_sco_packet_buffer(scoPacketLength);

                hci_request_sco_can_send_now_event();
            }
            break;
        case HCI_EVENT_HSP_META:
            processHSPEvent(event);
            break;
        default:
            break;
        }
    }

    void HSP::HSPImpl::processHSPEvent(uint8_t *event)
    {
        auto eventDescriptor = event[2];
        switch (eventDescriptor) {
        case HSP_SUBEVENT_RFCOMM_CONNECTION_COMPLETE:
            if (hsp_subevent_rfcomm_connection_complete_get_status(event) != 0u) {
                LOG_DEBUG("RFCOMM connection establishement failed with status %u\n",
                          hsp_subevent_rfcomm_connection_complete_get_status(event));
                break;
            }
            LOG_DEBUG("RFCOMM connection established.\n");
            sendAudioEvent(audio::EventType::BlutoothHSPDeviceState, audio::Event::DeviceState::Connected);
            isConnected = true;
            break;
        case HSP_SUBEVENT_RFCOMM_DISCONNECTION_COMPLETE:
            if (hsp_subevent_rfcomm_disconnection_complete_get_status(event) != 0u) {
                LOG_DEBUG("RFCOMM disconnection failed with status %u.\n",
                          hsp_subevent_rfcomm_disconnection_complete_get_status(event));
            }
            else {
                LOG_DEBUG("RFCOMM disconnected.\n");
                sendAudioEvent(audio::EventType::BlutoothHSPDeviceState, audio::Event::DeviceState::Disconnected);
            }
            isConnected = false;
            break;
        case HSP_SUBEVENT_AUDIO_CONNECTION_COMPLETE:
            if (hsp_subevent_audio_connection_complete_get_status(event) != 0u) {
                LOG_DEBUG("Audio connection establishment failed with status %u\n",
                          hsp_subevent_audio_connection_complete_get_status(event));
            }
            else {
                scoHandle = hsp_subevent_audio_connection_complete_get_handle(event);
                LOG_ERROR("Start audio routing from HSP");
                AudioServiceAPI::RoutingStart(const_cast<sys::Service *>(ownerService));
                LOG_DEBUG("Audio connection established with SCO handle 0x%04x.\n", scoHandle);
                hci_request_sco_can_send_now_event();
                RunLoop::trigger();
            }
            break;
        case HSP_SUBEVENT_AUDIO_DISCONNECTION_COMPLETE:
            LOG_DEBUG("Audio connection released.\n\n");
            sendAudioEvent(audio::EventType::BlutoothHSPDeviceState, audio::Event::DeviceState::Disconnected);
            scoHandle   = HCI_CON_HANDLE_INVALID;
            isConnected = false;
            break;
        case HSP_SUBEVENT_MICROPHONE_GAIN_CHANGED:
            LOG_DEBUG("Received microphone gain change %d\n", hsp_subevent_microphone_gain_changed_get_gain(event));
            break;
        case HSP_SUBEVENT_SPEAKER_GAIN_CHANGED: {
            const auto volume = hsp_subevent_speaker_gain_changed_get_gain(event);
            auto &busProxy    = const_cast<sys::Service *>(ownerService)->bus;
            busProxy.sendUnicast(std::make_shared<message::bluetooth::HSPVolume>(volume), service::name::bluetooth);
            LOG_DEBUG("Received speaker gain change %d\n", hsp_subevent_speaker_gain_changed_get_gain(event));
        } break;
        case HSP_SUBEVENT_HS_CALL_ANSWER:
            LOG_DEBUG("HSP CALL ANSWER");
            cellularInterface->answerIncomingCall(const_cast<sys::Service *>(ownerService));
            break;
        case HSP_SUBEVENT_HS_CALL_HANGUP:
            LOG_DEBUG("HSP CALL HANGUP");
            cellularInterface->hangupCall(const_cast<sys::Service *>(ownerService));
            break;
        case HSP_SUBEVENT_HS_COMMAND: {
            ATcommandBuffer.fill(0);
            auto cmd_length   = hsp_subevent_hs_command_get_value_length(event);
            auto size         = cmd_length <= ATcommandBuffer.size() ? cmd_length : ATcommandBuffer.size();
            auto commandValue = hsp_subevent_hs_command_get_value(event);
            memcpy(ATcommandBuffer.data(), commandValue, size - 1);
            LOG_DEBUG("Received custom command: \"%s\". \nExit code or call hsp_ag_send_result.\n",
                      ATcommandBuffer.data());
            break;
        }
        default:
            LOG_DEBUG("event not handled %u\n", event[2]);
            break;
        }
    }

    void HSP::HSPImpl::establishAudioConnection()
    {
        LOG_DEBUG("Establish Audio connection...\n");
        hsp_ag_establish_audio_connection();
    }

    auto HSP::HSPImpl::init() -> Error::Code
    {
        sco = std::make_unique<SCO>();
        sco->setOwnerService(ownerService);
        sco->init();

        cellularInterface = std::make_unique<CellularInterfaceImpl>();

        Profile::initL2cap();
        btstack_cvsd_plc_init(&cvsd_plc_state);
        Profile::initSdp();

        serviceBuffer.fill(0);
        constexpr uint32_t hspSdpRecordHandle = 0x10004;
        hsp_ag_create_sdp_record(serviceBuffer.data(), hspSdpRecordHandle, rfcommChannelNr, agServiceName.c_str());

        if (const auto status = sdp_register_service(serviceBuffer.data()); status != ERROR_CODE_SUCCESS) {
            LOG_ERROR("Can't register service. Status %x", status);
        }

        rfcomm_init();

        hsp_ag_init(rfcommChannelNr);
        hsp_ag_register_packet_handler(&packetHandler);

        // register for SCO packets
        hci_register_sco_packet_handler(&packetHandler);

        gap_discoverable_control(1);
        gap_set_class_of_device(CLASS_OF_DEVICE);

        LOG_INFO("HSP init done!");

        return bluetooth::Error::Success;
    }

    void HSP::HSPImpl::connect()
    {
        if (isConnected) {
            disconnect();
        }
        hsp_ag_connect(deviceAddr);
    }

    void HSP::HSPImpl::disconnect()
    {
        hsp_ag_disconnect();
    }

    void HSP::HSPImpl::setDeviceAddress(bd_addr_t addr)
    {
        bd_addr_copy(deviceAddr, addr);
        LOG_DEBUG("Address set!");
    }

    void HSP::HSPImpl::setOwnerService(const sys::Service *service)
    {
        ownerService = service;
    }

    auto HSP::HSPImpl::getStreamData() -> std::shared_ptr<BluetoothStreamData>
    {
        return sco->getStreamData();
    }
    void HSP::HSPImpl::start()
    {
        if (!isConnected) {
            connect();
        }
    }
    void HSP::HSPImpl::stop()
    {
        stopRinging();
        hsp_ag_release_audio_connection();
    }

    void HSP::HSPImpl::startRinging() const noexcept
    {
        LOG_DEBUG("Bluetooth ring started");
        hsp_ag_start_ringing();
    }

    void HSP::HSPImpl::stopRinging() const noexcept
    {
        LOG_DEBUG("Bluetooth ring stopped");
        hsp_ag_stop_ringing();
    }

    void HSP::HSPImpl::initializeCall() const noexcept
    {
        stopRinging();
        establishAudioConnection();
    }

    void HSP::setAudioDevice(std::shared_ptr<bluetooth::BluetoothAudioDevice> audioDevice)
    {
        pimpl->setAudioDevice(audioDevice);
    }

    void HSP::HSPImpl::setAudioDevice(std::shared_ptr<bluetooth::BluetoothAudioDevice> audioDevice)
    {
        HSP::HSPImpl::audioDevice = std::static_pointer_cast<HSPAudioDevice>(audioDevice);
    }
} // namespace bluetooth
