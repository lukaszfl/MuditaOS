//
// Created by bartek on 14.09.2020.
//

#include <Bluetooth/Device.hpp>
#include <log/log.hpp>
#include <vector>
#include <Bluetooth/Error.hpp>
#include <BtCommand.hpp>
#include <vfs.hpp>

extern "C"
{
#include "btstack.h"
#include <lib/btstack/3rd-party/hxcmod-player/hxcmod.h>
#include <lib/btstack/3rd-party/hxcmod-player/mods/mod.h>
#include <classic/avdtp.h>
#include <classic/avrcp.h>
#include <btstack_defines.h>
};

using namespace Bt;

#define NUM_CHANNELS           2
#define BYTES_PER_AUDIO_SAMPLE (2 * NUM_CHANNELS)
#define AUDIO_TIMEOUT_MS       10
#define TABLE_SIZE_441HZ       100

#define SBC_STORAGE_SIZE 512

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

static bd_addr_t device_addr;

static uint8_t sdp_a2dp_source_service_buffer[150];
static uint8_t sdp_avrcp_target_service_buffer[200];
// static uint8_t sdp_avrcp_controller_service_buffer[200];

static avdtp_media_codec_configuration_sbc_t sbc_configuration;
static btstack_sbc_encoder_state_t sbc_encoder_state;

static uint8_t media_sbc_codec_configuration[4];
static a2dp_media_sending_context_t media_tracker;

static stream_data_source_t data_source;

// static int sine_phase;
static int sample_rate = 44100;

static int hxcmod_initialized;
static modcontext mod_context;
static tracker_buffer_state trkbuf;

/* AVRCP Target context START */
static const uint8_t subunit_info[] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
                                       4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7};

// static uint32_t company_id = 0x112233;
// static uint8_t  companies_num = 1;
// static uint8_t  companies[] = {
//    0x00, 0x19, 0x58 //BT SIG registered CompanyID
//};

// static uint8_t events_num = 6;
// static uint8_t events[] = {
//    AVRCP_NOTIFICATION_EVENT_PLAYBACK_STATUS_CHANGED,
//    AVRCP_NOTIFICATION_EVENT_TRACK_CHANGED,
//    AVRCP_NOTIFICATION_EVENT_PLAYER_APPLICATION_SETTING_CHANGED,
//    AVRCP_NOTIFICATION_EVENT_NOW_PLAYING_CONTENT_CHANGED,
//    AVRCP_NOTIFICATION_EVENT_AVAILABLE_PLAYERS_CHANGED,
//    AVRCP_NOTIFICATION_EVENT_ADDRESSED_PLAYER_CHANGED
//};

typedef struct
{
    uint8_t track_id[8];
    uint32_t song_length_ms;
    avrcp_playback_status_t status;
    uint32_t song_position_ms; // 0xFFFFFFFF if not supported
} avrcp_play_status_info_t;

// avrcp_track_t tracks[] = {
//    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 1, "Sine", "Generated", "A2DP Source Demo", "monotone", 12345},
//    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02}, 2, "Nao-deceased", "Decease", "A2DP Source Demo", "vivid",
//    12345},
//    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03}, 3, (char *)"aaaa", "Decease", "A2DP Source Demo", "vivid",
//    12345},
//};
int current_track_index;
avrcp_play_status_info_t play_info;

static btstack_packet_callback_registration_t hci_event_callback_registration;

static void a2dp_demo_reconfigure_sample_rate(int new_sample_rate)
{
    if (!hxcmod_initialized) {
        hxcmod_initialized = hxcmod_init(&mod_context);
        if (!hxcmod_initialized) {
            LOG_INFO("could not initialize hxcmod\n");
            return;
        }
    }
    sample_rate                     = new_sample_rate;
    media_tracker.sbc_storage_count = 0;
    media_tracker.samples_ready     = 0;
    hxcmod_unload(&mod_context);
    hxcmod_setcfg(&mod_context, sample_rate, 16, 1, 1, 1);
    ////////////////////////////////////////////////////////////
    const char *musicFolder = USER_PATH("music");
    std::vector<std::string> musicFiles;
    LOG_INFO("Scanning music folder: %s", musicFolder);
    auto dirList = vfs.listdir(musicFolder, ".mod");
    //    auto mod_buffer = malloc(7600);
    //    auto file = vfs.fopen(dirList[0].fileName.c_str(),"r");
    //    vfs.fread(mod_buffer,7600,1,file);
    //    auto modlen = 7600;
    //
    //    hxcmod_load(&mod_context, (void *) &mod_buffer, modlen);
}

static void produce_mod_audio(int16_t *pcm_buffer, int num_samples_to_write)
{
    hxcmod_fillbuffer(&mod_context, (unsigned short *)&pcm_buffer[0], num_samples_to_write, &trkbuf);
}

static int a2dp_demo_fill_sbc_audio_buffer(a2dp_media_sending_context_t *context)
{
    // perform sbc encodin
    int total_num_bytes_read                      = 0;
    unsigned int num_audio_samples_per_sbc_buffer = btstack_sbc_encoder_num_audio_frames();
    while (context->samples_ready >= num_audio_samples_per_sbc_buffer &&
           (context->max_media_payload_size - context->sbc_storage_count) >= btstack_sbc_encoder_sbc_buffer_length()) {

        int16_t pcm_frame[256 * NUM_CHANNELS];

        produce_mod_audio(pcm_frame, num_audio_samples_per_sbc_buffer);
        btstack_sbc_encoder_process_data(pcm_frame);

        uint16_t sbc_frame_size = btstack_sbc_encoder_sbc_buffer_length();
        uint8_t *sbc_frame      = btstack_sbc_encoder_sbc_buffer();

        total_num_bytes_read += num_audio_samples_per_sbc_buffer;
        memcpy(&context->sbc_storage[context->sbc_storage_count], sbc_frame, sbc_frame_size);
        context->sbc_storage_count += sbc_frame_size;
        context->samples_ready -= num_audio_samples_per_sbc_buffer;
    }
    return total_num_bytes_read;
}

static void a2dp_demo_audio_timeout_handler(btstack_timer_source_t *timer)
{
    a2dp_media_sending_context_t *context = (a2dp_media_sending_context_t *)btstack_run_loop_get_timer_context(timer);
    btstack_run_loop_set_timer(&context->audio_timer, AUDIO_TIMEOUT_MS);
    btstack_run_loop_add_timer(&context->audio_timer);
    uint32_t now = btstack_run_loop_get_time_ms();

    uint32_t update_period_ms = AUDIO_TIMEOUT_MS;
    if (context->time_audio_data_sent > 0) {
        update_period_ms = now - context->time_audio_data_sent;
    }

    uint32_t num_samples = (update_period_ms * sample_rate) / 1000;
    context->acc_num_missed_samples += (update_period_ms * sample_rate) % 1000;

    while (context->acc_num_missed_samples >= 1000) {
        num_samples++;
        context->acc_num_missed_samples -= 1000;
    }
    context->time_audio_data_sent = now;
    context->samples_ready += num_samples;

    if (context->sbc_ready_to_send)
        return;

    a2dp_demo_fill_sbc_audio_buffer(context);

    if ((context->sbc_storage_count + btstack_sbc_encoder_sbc_buffer_length()) > context->max_media_payload_size) {
        // schedule sending
        context->sbc_ready_to_send = 1;
        a2dp_source_stream_endpoint_request_can_send_now(context->a2dp_cid, context->local_seid);
    }
}

static void a2dp_demo_timer_start(a2dp_media_sending_context_t *context)
{
    context->max_media_payload_size =
        btstack_min(a2dp_max_media_payload_size(context->a2dp_cid, context->local_seid), SBC_STORAGE_SIZE);
    context->sbc_storage_count = 0;
    context->sbc_ready_to_send = 0;
    context->streaming         = 1;
    btstack_run_loop_remove_timer(&context->audio_timer);
    btstack_run_loop_set_timer_handler(&context->audio_timer, a2dp_demo_audio_timeout_handler);
    btstack_run_loop_set_timer_context(&context->audio_timer, context);
    btstack_run_loop_set_timer(&context->audio_timer, AUDIO_TIMEOUT_MS);
    btstack_run_loop_add_timer(&context->audio_timer);
}

static void a2dp_demo_timer_stop(a2dp_media_sending_context_t *context)
{
    context->time_audio_data_sent   = 0;
    context->acc_num_missed_samples = 0;
    context->samples_ready          = 0;
    context->streaming              = 1;
    context->sbc_storage_count      = 0;
    context->sbc_ready_to_send      = 0;
    btstack_run_loop_remove_timer(&context->audio_timer);
}

void A2DP::hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    if (packet_type != HCI_EVENT_PACKET)
        return;

    if (hci_event_packet_get_type(packet) == HCI_EVENT_PIN_CODE_REQUEST) {
        bd_addr_t address;
        LOG_INFO("Pin code request - using '0000'\n");
        hci_event_pin_code_request_get_bd_addr(packet, address);
        gap_pin_code_response(address, "0000");
    }
}

Error A2DP::init()
{
    hci_set_master_slave_policy(0);
    l2cap_init();
    a2dp_source_init();
    a2dp_source_register_packet_handler(&A2DP::source_packet_handler);

    avdtp_stream_endpoint_t *local_stream_endpoint =
        a2dp_source_create_stream_endpoint(AVDTP_AUDIO,
                                           AVDTP_CODEC_SBC,
                                           media_sbc_codec_capabilities,
                                           sizeof(media_sbc_codec_capabilities),
                                           media_sbc_codec_configuration,
                                           sizeof(media_sbc_codec_configuration));
    if (!local_stream_endpoint) {
        LOG_ERROR("A2DP Source: not enough memory to create local stream endpoint");
        return Error::SystemError;
    }

    media_tracker.local_seid = avdtp_local_seid(local_stream_endpoint);
    avdtp_source_register_delay_reporting_category(media_tracker.local_seid);

    sdp_init();

    // Create  A2DP Source service record and register it with SDP.
    memset(sdp_a2dp_source_service_buffer, 0, sizeof(sdp_a2dp_source_service_buffer));
    a2dp_source_create_sdp_record(sdp_a2dp_source_service_buffer, 0x10001, AVDTP_SOURCE_SF_Player, NULL, NULL);
    sdp_register_service(sdp_a2dp_source_service_buffer);

    memset(sdp_avrcp_target_service_buffer, 0, sizeof(sdp_avrcp_target_service_buffer));
    uint16_t supported_features = AVRCP_SUBUNIT_TYPE_AUDIO;

    avrcp_target_create_sdp_record(sdp_avrcp_target_service_buffer, 0x10002, supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_target_service_buffer);

    GAP::set_visibility(true);
    gap_set_class_of_device(0x200408);

    hci_event_callback_registration.callback = &hci_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    a2dp_demo_reconfigure_sample_rate(sample_rate);

    return Error();
}

static void a2dp_demo_send_media_packet(void)
{
    int num_bytes_in_frame = btstack_sbc_encoder_sbc_buffer_length();
    int bytes_in_storage   = media_tracker.sbc_storage_count;
    uint8_t num_frames     = bytes_in_storage / num_bytes_in_frame;
    a2dp_source_stream_send_media_payload(
        media_tracker.a2dp_cid, media_tracker.local_seid, media_tracker.sbc_storage, bytes_in_storage, num_frames, 0);
    media_tracker.sbc_storage_count = 0;
    media_tracker.sbc_ready_to_send = 0;
}

void A2DP::source_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    uint8_t status;
    uint8_t local_seid;
    bd_addr_t address;
    uint16_t cid;

    if (packet_type != HCI_EVENT_PACKET)
        return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_A2DP_META)
        return;

    switch (hci_event_a2dp_meta_get_subevent_code(packet)) {
    case A2DP_SUBEVENT_SIGNALING_CONNECTION_ESTABLISHED:
        a2dp_subevent_signaling_connection_established_get_bd_addr(packet, address);
        cid    = a2dp_subevent_signaling_connection_established_get_a2dp_cid(packet);
        status = a2dp_subevent_signaling_connection_established_get_status(packet);

        if (status != ERROR_CODE_SUCCESS) {
            LOG_INFO("A2DP Source: Connection failed, status 0x%02x, cid 0x%02x, a2dp_cid 0x%02x \n",
                     status,
                     cid,
                     media_tracker.a2dp_cid);
            media_tracker.a2dp_cid = 0;
            break;
        }
        media_tracker.a2dp_cid = cid;
        media_tracker.volume   = 64;

        LOG_INFO("A2DP Source: Connected to address %s, a2dp cid 0x%02x, local seid %d.\n",
                 bd_addr_to_str(address),
                 media_tracker.a2dp_cid,
                 media_tracker.local_seid);
        break;

    case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_SBC_CONFIGURATION: {
        cid = avdtp_subevent_signaling_media_codec_sbc_configuration_get_avdtp_cid(packet);
        if (cid != media_tracker.a2dp_cid)
            return;
        media_tracker.remote_seid = a2dp_subevent_signaling_media_codec_sbc_configuration_get_acp_seid(packet);

        sbc_configuration.reconfigure  = a2dp_subevent_signaling_media_codec_sbc_configuration_get_reconfigure(packet);
        sbc_configuration.num_channels = a2dp_subevent_signaling_media_codec_sbc_configuration_get_num_channels(packet);
        sbc_configuration.sampling_frequency =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_sampling_frequency(packet);
        sbc_configuration.channel_mode = a2dp_subevent_signaling_media_codec_sbc_configuration_get_channel_mode(packet);
        sbc_configuration.block_length = a2dp_subevent_signaling_media_codec_sbc_configuration_get_block_length(packet);
        sbc_configuration.subbands     = a2dp_subevent_signaling_media_codec_sbc_configuration_get_subbands(packet);
        sbc_configuration.allocation_method =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_allocation_method(packet);
        sbc_configuration.min_bitpool_value =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_min_bitpool_value(packet);
        sbc_configuration.max_bitpool_value =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_max_bitpool_value(packet);
        sbc_configuration.frames_per_buffer = sbc_configuration.subbands * sbc_configuration.block_length;
        LOG_INFO("A2DP Source: Received SBC codec configuration, sampling frequency %u, a2dp_cid 0x%02x, local seid %d "
                 "(expected %d), remote seid %d .\n",
                 sbc_configuration.sampling_frequency,
                 cid,
                 a2dp_subevent_signaling_media_codec_sbc_configuration_get_int_seid(packet),
                 media_tracker.local_seid,
                 media_tracker.remote_seid);

        // Adapt Bluetooth spec definition to SBC Encoder expected input
        sbc_configuration.allocation_method -= 1;
        sbc_configuration.num_channels = 2;
        switch (sbc_configuration.channel_mode) {
        case AVDTP_SBC_JOINT_STEREO:
            sbc_configuration.channel_mode = 3;
            break;
        case AVDTP_SBC_STEREO:
            sbc_configuration.channel_mode = 2;
            break;
        case AVDTP_SBC_DUAL_CHANNEL:
            sbc_configuration.channel_mode = 1;
            break;
        case AVDTP_SBC_MONO:
            sbc_configuration.channel_mode = 0;
            sbc_configuration.num_channels = 1;
            break;
        }
        // dump_sbc_configuration(&sbc_configuration);

        btstack_sbc_encoder_init(&sbc_encoder_state,
                                 SBC_MODE_STANDARD,
                                 sbc_configuration.block_length,
                                 sbc_configuration.subbands,
                                 sbc_configuration.allocation_method,
                                 sbc_configuration.sampling_frequency,
                                 sbc_configuration.max_bitpool_value,
                                 sbc_configuration.channel_mode);
        break;
    }

    case A2DP_SUBEVENT_STREAM_ESTABLISHED:
        a2dp_subevent_stream_established_get_bd_addr(packet, address);
        status = a2dp_subevent_stream_established_get_status(packet);
        if (status) {
            LOG_INFO("A2DP Source: Stream failed, status 0x%02x.\n", status);
            break;
        }

        local_seid = a2dp_subevent_stream_established_get_local_seid(packet);
        cid        = a2dp_subevent_stream_established_get_a2dp_cid(packet);
        LOG_INFO("A2DP_SUBEVENT_STREAM_ESTABLISHED:  a2dp_cid [expected 0x%02x, received 0x%02x], local_seid %d "
                 "(expected %d), remote_seid %d (expected %d)\n",
                 media_tracker.a2dp_cid,
                 cid,
                 local_seid,
                 media_tracker.local_seid,
                 a2dp_subevent_stream_established_get_remote_seid(packet),
                 media_tracker.remote_seid);

        if (local_seid != media_tracker.local_seid) {
            LOG_INFO("A2DP Source: Stream failed, wrong local seid %d, expected %d.\n",
                     local_seid,
                     media_tracker.local_seid);
            break;
        }
        LOG_INFO("A2DP Source: Stream established, address %s, a2dp cid 0x%02x, local seid %d, remote seid %d.\n",
                 bd_addr_to_str(address),
                 media_tracker.a2dp_cid,
                 media_tracker.local_seid,
                 a2dp_subevent_stream_established_get_remote_seid(packet));
        media_tracker.stream_opened = 1;
        data_source                 = STREAM_MOD;
        status                      = a2dp_source_start_stream(media_tracker.a2dp_cid, media_tracker.local_seid);
        break;

    case A2DP_SUBEVENT_STREAM_RECONFIGURED:
        status     = a2dp_subevent_stream_reconfigured_get_status(packet);
        local_seid = a2dp_subevent_stream_reconfigured_get_local_seid(packet);
        cid        = a2dp_subevent_stream_reconfigured_get_a2dp_cid(packet);

        LOG_INFO("A2DP Source: Reconfigured: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 media_tracker.a2dp_cid,
                 cid,
                 media_tracker.local_seid,
                 local_seid);
        LOG_INFO("Status 0x%02x\n", status);
        break;

    case A2DP_SUBEVENT_STREAM_STARTED:
        local_seid = a2dp_subevent_stream_started_get_local_seid(packet);
        cid        = a2dp_subevent_stream_started_get_a2dp_cid(packet);

        play_info.status = AVRCP_PLAYBACK_STATUS_PLAYING;
        if (media_tracker.avrcp_cid) {
            //            avrcp_target_set_now_playing_info(media_tracker.avrcp_cid, &tracks[data_source],
            //            sizeof(tracks)/sizeof(avrcp_track_t));
            //            avrcp_target_set_playback_status(media_tracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_PLAYING);
        }
        a2dp_demo_timer_start(&media_tracker);
        LOG_INFO("A2DP Source: Stream started: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 media_tracker.a2dp_cid,
                 cid,
                 media_tracker.local_seid,
                 local_seid);
        break;

    case A2DP_SUBEVENT_STREAMING_CAN_SEND_MEDIA_PACKET_NOW:
        local_seid = a2dp_subevent_streaming_can_send_media_packet_now_get_local_seid(packet);
        cid        = a2dp_subevent_signaling_media_codec_sbc_configuration_get_a2dp_cid(packet);
        // LOG_INFO("A2DP Source: can send media packet: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid
        // [expected %d, received %d]\n", media_tracker.a2dp_cid, cid, media_tracker.local_seid, local_seid);
        a2dp_demo_send_media_packet();
        break;

    case A2DP_SUBEVENT_STREAM_SUSPENDED:
        local_seid = a2dp_subevent_stream_suspended_get_local_seid(packet);
        cid        = a2dp_subevent_stream_suspended_get_a2dp_cid(packet);

        play_info.status = AVRCP_PLAYBACK_STATUS_PAUSED;
        if (media_tracker.avrcp_cid) {
            avrcp_target_set_playback_status(media_tracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_PAUSED);
        }
        LOG_INFO("A2DP Source: Stream paused: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 media_tracker.a2dp_cid,
                 cid,
                 media_tracker.local_seid,
                 local_seid);

        a2dp_demo_timer_stop(&media_tracker);
        break;

    case A2DP_SUBEVENT_STREAM_RELEASED:
        play_info.status = AVRCP_PLAYBACK_STATUS_STOPPED;
        cid              = a2dp_subevent_stream_released_get_a2dp_cid(packet);
        local_seid       = a2dp_subevent_stream_released_get_local_seid(packet);

        LOG_INFO("A2DP Source: Stream released: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 media_tracker.a2dp_cid,
                 cid,
                 media_tracker.local_seid,
                 local_seid);

        if (cid == media_tracker.a2dp_cid) {
            media_tracker.stream_opened = 0;
            LOG_INFO("A2DP Source: Stream released.\n");
        }
        if (media_tracker.avrcp_cid) {
            //            avrcp_target_set_now_playing_info(media_tracker.avrcp_cid, NULL,
            //            sizeof(tracks)/sizeof(avrcp_track_t));
            //            avrcp_target_set_playback_status(media_tracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_STOPPED);
        }

        a2dp_demo_timer_stop(&media_tracker);
        break;
    case A2DP_SUBEVENT_SIGNALING_CONNECTION_RELEASED:
        cid = a2dp_subevent_signaling_connection_released_get_a2dp_cid(packet);
        if (cid == media_tracker.a2dp_cid) {
            media_tracker.avrcp_cid = 0;
            media_tracker.a2dp_cid  = 0;
            LOG_INFO("A2DP Source: Signaling released.\n\n");
        }
        break;
    default:
        break;
    }
}

void A2DP::start()
{
    LOG_INFO("Starting playback");
    a2dp_source_establish_stream(device_addr, media_tracker.local_seid, &media_tracker.a2dp_cid);
}

void A2DP::stop()
{
    LOG_INFO("Stopping playback");
    a2dp_source_disconnect(media_tracker.a2dp_cid);
};
