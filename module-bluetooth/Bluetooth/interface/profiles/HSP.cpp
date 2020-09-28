#include "HSP.hpp"
#include "SCO.hpp"
#include <Bluetooth/Device.hpp>
#include <Bluetooth/Error.hpp>
#include <log/log.hpp>
#include <vector>
extern "C"
{
#include "btstack.h"
#include "btstack_run_loop_freertos.h"
#include "btstack_stdin.h"
#include <btstack_defines.h>
};

namespace Bt
{

    class HSP::HSPImpl
    {
      public:
        static void packetHandler(uint8_t packetType, uint16_t channel, uint8_t *event, uint16_t eventSize);
        auto init() -> Error;
        void start();
        void stop();
        void setDeviceAddress(bd_addr_t addr);
      private:
        static uint8_t serviceBuffer[150];
        static constexpr uint8_t rfcommChannelNr = 1;
        static constexpr char agServiceName[]    = "Audio Gateway Test";
        static uint16_t scoHandle;
        static std::unique_ptr<SCO> sco;
        static char hsCmdBuffer[100];
        static bd_addr_t deviceAddr;
    };

    HSP::HSP() : pimpl(std::make_unique<HSPImpl>(HSPImpl()))
    {}
    HSP::HSP(const HSP &other) : pimpl(new HSPImpl(*other.pimpl))
    {}
    auto HSP::operator=(HSP rhs) -> HSP &
    {
        swap(pimpl, rhs.pimpl);
        return *this;
    }
    auto HSP::init() -> Error
    {
        return pimpl->init();
    }
    void HSP::setDeviceAddress(uint8_t *addr)
    {
        pimpl->setDeviceAddress(addr);
    }
    void HSP::start()
    {
        pimpl->start();
    }
    void HSP::stop()
    {
        pimpl->stop();
    }
    HSP::~HSP() = default;

} // namespace Bt

using namespace Bt;

uint16_t HSP::HSPImpl::scoHandle = HCI_CON_HANDLE_INVALID;
bd_addr_t HSP::HSPImpl::deviceAddr;
char HSP::HSPImpl::hsCmdBuffer[100];
uint8_t HSP::HSPImpl::serviceBuffer[150];
std::unique_ptr<SCO> HSP::HSPImpl::sco;

void HSP::HSPImpl::packetHandler(uint8_t packetType, uint16_t channel, uint8_t *event, uint16_t eventSize)
{
    UNUSED(channel);

    switch (packetType) {
    case HCI_SCO_DATA_PACKET:
        if (READ_SCO_CONNECTION_HANDLE(event) != scoHandle)
            break;
        sco->receive(event, eventSize);
        break;

    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(event)) {
#ifndef HAVE_BTSTACK_STDIN
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(event) != HCI_STATE_WORKING)
                break;
            printf("Establish HSP AG service to %s...\n", device_addr_string);
            hsp_ag_connect(device_addr);
            break;
#endif
        case HCI_EVENT_SCO_CAN_SEND_NOW:
            sco->send(scoHandle);
            break;
        case HCI_EVENT_HSP_META:
            switch (event[2]) {
            case HSP_SUBEVENT_RFCOMM_CONNECTION_COMPLETE:
                if (hsp_subevent_rfcomm_connection_complete_get_status(event)) {
                    printf("RFCOMM connection establishement failed with status %u\n",
                           hsp_subevent_rfcomm_connection_complete_get_status(event));
                    break;
                }
                printf("RFCOMM connection established.\n");
                //#ifndef HAVE_BTSTACK_STDIN
                printf("Establish Audio connection to %s...\n", bd_addr_to_str(deviceAddr));
                hsp_ag_establish_audio_connection();
                //#endif
                break;
            case HSP_SUBEVENT_RFCOMM_DISCONNECTION_COMPLETE:
                if (hsp_subevent_rfcomm_disconnection_complete_get_status(event)) {
                    printf("RFCOMM disconnection failed with status %u.\n",
                           hsp_subevent_rfcomm_disconnection_complete_get_status(event));
                }
                else {
                    printf("RFCOMM disconnected.\n");
                }
                break;
            case HSP_SUBEVENT_AUDIO_CONNECTION_COMPLETE:
                if (hsp_subevent_audio_connection_complete_get_status(event)) {
                    printf("Audio connection establishment failed with status %u\n",
                           hsp_subevent_audio_connection_complete_get_status(event));
                }
                else {
                    scoHandle = hsp_subevent_audio_connection_complete_get_handle(event);
                    printf("Audio connection established with SCO handle 0x%04x.\n", scoHandle);
                    hci_request_sco_can_send_now_event();
                    btstack_run_loop_freertos_trigger();
                }
                break;
            case HSP_SUBEVENT_AUDIO_DISCONNECTION_COMPLETE:
                printf("Audio connection released.\n\n");
                scoHandle = HCI_CON_HANDLE_INVALID;
                break;
            case HSP_SUBEVENT_MICROPHONE_GAIN_CHANGED:
                printf("Received microphone gain change %d\n", hsp_subevent_microphone_gain_changed_get_gain(event));
                break;
            case HSP_SUBEVENT_SPEAKER_GAIN_CHANGED:
                printf("Received speaker gain change %d\n", hsp_subevent_speaker_gain_changed_get_gain(event));
                break;
            case HSP_SUBEVENT_HS_COMMAND: {
                memset(hsCmdBuffer, 0, sizeof(hsCmdBuffer));
                unsigned int cmd_length = hsp_subevent_hs_command_get_value_length(event);
                unsigned int size       = cmd_length <= sizeof(hsCmdBuffer) ? cmd_length : sizeof(hsCmdBuffer);
                memcpy(hsCmdBuffer, hsp_subevent_hs_command_get_value(event), size - 1);
                printf("Received custom command: \"%s\". \nExit code or call hsp_ag_send_result.\n", hsCmdBuffer);
                break;
            }
            default:
                printf("event not handled %u\n", event[2]);
                break;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

Error HSP::HSPImpl::init()
{
    sco = std::make_unique<SCO>();
    sco->init();
    sco->setCodec(HFP_CODEC_CVSD);
    l2cap_init();
    sdp_init();

    memset((uint8_t *)serviceBuffer, 0, sizeof(serviceBuffer));
    hsp_ag_create_sdp_record(serviceBuffer, 0x10001, rfcommChannelNr, agServiceName);
    printf("SDP service record size: %u\n", de_get_len(serviceBuffer));
    sdp_register_service(serviceBuffer);

    rfcomm_init();

    hsp_ag_init(rfcommChannelNr);
    hsp_ag_register_packet_handler(&packetHandler);

    // register for SCO packets
    hci_register_sco_packet_handler(&packetHandler);

    gap_set_local_name("PurePhone");
    gap_discoverable_control(1);
    gap_set_class_of_device(0x400204);
    hci_power_control(HCI_POWER_ON);

    return Error();
}

void HSP::HSPImpl::start()
{
    hsp_ag_connect(deviceAddr);
}

void HSP::HSPImpl::stop()
{
    sco->close();
    hsp_ag_release_audio_connection();
    hsp_ag_disconnect();
}
void HSP::HSPImpl::setDeviceAddress(bd_addr_t addr)
{
    bd_addr_copy(deviceAddr, addr);
    LOG_INFO("Address set!");
}