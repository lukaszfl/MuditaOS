#pragma once
#include <Device.hpp>
#include <log/log.hpp>
#include <vector>
#include <Error.hpp>
#include <BtCommand.hpp>
#include <vfs.hpp>

extern "C"
{
#include "module-bluetooth/lib/btstack/src/btstack.h"
#include <lib/btstack/3rd-party/hxcmod-player/hxcmod.h>
#include <lib/btstack/3rd-party/hxcmod-player/mods/mod.h>
#include <classic/avdtp.h>
#include "classic/avrcp.h"
#include "classic/avrcp_browsing_controller.h"
#include "classic/avrcp_browsing_target.h"
#include "classic/avrcp_controller.h"
#include "classic/avrcp_media_item_iterator.h"
#include "classic/avrcp_target.h"
#include <btstack_defines.h>
};

namespace Bt::A2DP_config
{
    constexpr int NUM_CHANNELS           = 2;
    constexpr int BYTES_PER_AUDIO_SAMPLE = (2 * NUM_CHANNELS);
    constexpr int AUDIO_TIMEOUT_MS       = 10;
    constexpr int TABLE_SIZE_441HZ       = 100;

    constexpr int SBC_STORAGE_SIZE = 1030;

    typedef enum
    {
        STREAM_SINE = 0,
        STREAM_MOD,
        STREAM_PTS_TEST
    } stream_data_source_t;

    typedef struct
    {
        uint16_t a2dp_cid;
        uint8_t local_seid;
        uint8_t remote_seid;
        uint8_t stream_opened;
        uint16_t avrcp_cid;

        uint32_t time_audio_data_sent; // ms
        uint32_t acc_num_missed_samples;
        uint32_t samples_ready;
        btstack_timer_source_t audio_timer;
        uint8_t streaming;
        int max_media_payload_size;

        uint8_t sbc_storage[SBC_STORAGE_SIZE];
        uint16_t sbc_storage_count;
        uint8_t sbc_ready_to_send;

        uint16_t volume;
    } a2dp_media_sending_context_t;

    typedef struct
    {
        int reconfigure;
        int num_channels;
        int sampling_frequency;
        int channel_mode;
        int block_length;
        int subbands;
        int allocation_method;
        int min_bitpool_value;
        int max_bitpool_value;
        int frames_per_buffer;
    } avdtp_media_codec_configuration_sbc_t;

    static uint8_t media_sbc_codec_capabilities[] = {
        (AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
        0xFF, //(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) | AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
        2,
        53};

    /* AVRCP Target context START */
    static const uint8_t subunit_info[] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
                                           4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7};

    static uint32_t company_id   = 0x112233;
    static uint8_t companies_num = 1;
    static uint8_t companies[]   = {
        0x00, 0x19, 0x58 // BT SIG registered CompanyID
    };

    static uint8_t events_num = 6;
    static uint8_t events[]   = {AVRCP_NOTIFICATION_EVENT_PLAYBACK_STATUS_CHANGED,
                               AVRCP_NOTIFICATION_EVENT_TRACK_CHANGED,
                               AVRCP_NOTIFICATION_EVENT_PLAYER_APPLICATION_SETTING_CHANGED,
                               AVRCP_NOTIFICATION_EVENT_NOW_PLAYING_CONTENT_CHANGED,
                               AVRCP_NOTIFICATION_EVENT_AVAILABLE_PLAYERS_CHANGED,
                               AVRCP_NOTIFICATION_EVENT_ADDRESSED_PLAYER_CHANGED};
    // AVRCP
    typedef struct
    {
        uint8_t track_id[8];
        uint32_t song_length_ms;
        avrcp_playback_status_t status;
        uint32_t song_position_ms; // 0xFFFFFFFF if not supported
    } avrcp_play_status_info_t;

    avrcp_track_t tracks[] = {
        {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
         1,
         (char *)"Sine",
         (char *)"Generated",
         (char *)"A2DP Source Demo",
         (char *)"monotone",
         12345},
        {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
         2,
         (char *)"Nao-deceased",
         (char *)"Decease",
         (char *)"A2DP Source Demo",
         (char *)"vivid",
         12345},
        {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
         3,
         (char *)"aaaa",
         (char *)"Decease",
         (char *)"A2DP Source Demo",
         (char *)"vivid",
         12345},
    };
    int current_track_index;
    avrcp_play_status_info_t play_info;

} // namespace Bt::A2DP_config
namespace Bt
{

    class A2DP
    {
      public:
        Error init();
        void start();
        void stop();
        void set_addr(bd_addr_t addr);
        static void source_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
        static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    };

} // namespace Bt
