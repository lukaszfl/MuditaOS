//
// Created by bartek on 14.09.2020.
//

#include <Bluetooth/Device.hpp>
#include <log/log.hpp>
#include <Bluetooth/Error.hpp>
#include "A2DP.hpp"

Bt::A2DP::mediaContext Bt::A2DP::mediaTracker;
Bt::A2DP::AVDTP_sbcConfiguration Bt::A2DP::sbcConfiguration;
uint8_t Bt::A2DP::mediaSbcCodecCapabilities[] = {
    (AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
    0xFF, //(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) | AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
    2,
    53};

namespace Bt::A2DP_config
{

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

using namespace Bt;

static bd_addr_t device_addr;

static uint8_t sdp_a2dp_source_service_buffer[150];
static uint8_t sdp_avrcp_target_service_buffer[200];
static uint8_t sdp_avrcp_controller_service_buffer[200];

static btstack_sbc_encoder_state_t sbc_encoder_state;

static uint8_t media_sbc_codec_configuration[4];


// static int sine_phase;
static int sample_rate = 44100;

static int hxcmod_initialized;
static modcontext mod_context;
static tracker_buffer_state trkbuf;


static btstack_packet_callback_registration_t hci_event_callback_registration;

// static const char *device_addr_string = "00:12:6F:E7:9D:05";

/* LISTING_START(MainConfiguration): Setup Audio Source and AVRCP Target services */
static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void a2dp_source_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *event, uint16_t event_size);
static void avrcp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void avrcp_controller_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
#ifdef HAVE_BTSTACK_STDIN
static void stdin_process(char cmd);
#endif

static void a2dp_demo_reconfigure_sample_rate(int new_sample_rate);

static int a2dp_source_and_avrcp_services_init(void)
{
    // request role change on reconnecting headset to always use them in slave mode
    hci_set_master_slave_policy(0);

    l2cap_init();
    // Initialize  A2DP Source.
    a2dp_source_init();
    a2dp_source_register_packet_handler(&a2dp_source_packet_handler);

    // Create stream endpoint.
    avdtp_stream_endpoint_t *local_stream_endpoint =
        a2dp_source_create_stream_endpoint(AVDTP_AUDIO,
                                           AVDTP_CODEC_SBC,
                                           A2DP::mediaSbcCodecCapabilities,
                                           sizeof(A2DP::mediaSbcCodecCapabilities),
                                           media_sbc_codec_configuration,
                                           sizeof(media_sbc_codec_configuration));
    if (!local_stream_endpoint) {
        LOG_INFO("A2DP Source: not enough memory to create local stream endpoint\n");
        return 1;
    }
    A2DP::mediaTracker.local_seid = avdtp_local_seid(local_stream_endpoint);
    avdtp_source_register_delay_reporting_category(A2DP::mediaTracker.local_seid);

    // Initialize AVRCP Service.
    avrcp_init();
    avrcp_register_packet_handler(&avrcp_packet_handler);
    // Initialize AVRCP Target.
    avrcp_target_init();
    avrcp_target_register_packet_handler(&avrcp_target_packet_handler);
    // Initialize AVRCP Controller
    avrcp_controller_init();
    avrcp_controller_register_packet_handler(&avrcp_controller_packet_handler);

    // Initialize SDP,
    sdp_init();

    // Create  A2DP Source service record and register it with SDP.
    memset(sdp_a2dp_source_service_buffer, 0, sizeof(sdp_a2dp_source_service_buffer));
    a2dp_source_create_sdp_record(
        sdp_a2dp_source_service_buffer, 0x10001, AVDTP_SOURCE_FEATURE_MASK_PLAYER, NULL, NULL);
    sdp_register_service(sdp_a2dp_source_service_buffer);

    // Create AVRCP target service record and register it with SDP.
    memset(sdp_avrcp_target_service_buffer, 0, sizeof(sdp_avrcp_target_service_buffer));
    uint16_t supported_features = AVRCP_FEATURE_MASK_CATEGORY_PLAYER_OR_RECORDER;
#ifdef AVRCP_BROWSING_ENABLED
    supported_features |= AVRCP_FEATURE_MASK_BROWSING;
#endif
    avrcp_target_create_sdp_record(sdp_avrcp_target_service_buffer, 0x10002, supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_target_service_buffer);

    // setup AVRCP Controller
    memset(sdp_avrcp_controller_service_buffer, 0, sizeof(sdp_avrcp_controller_service_buffer));
    uint16_t controller_supported_features = AVRCP_FEATURE_MASK_CATEGORY_PLAYER_OR_RECORDER;
    avrcp_controller_create_sdp_record(
        sdp_avrcp_controller_service_buffer, 0x10003, controller_supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_controller_service_buffer);

    // Set local name with a template Bluetooth address, that will be automatically
    // replaced with a actual address once it is available, i.e. when BTstack boots
    // up and starts talking to a Bluetooth module.
    gap_set_local_name("PurePhone");
    gap_discoverable_control(1);
    gap_set_class_of_device(0x200408);

    // Register for HCI events.
    hci_event_callback_registration.callback = &hci_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    a2dp_demo_reconfigure_sample_rate(sample_rate);

#ifdef HAVE_BTSTACK_STDIN
    // Parse human readable Bluetooth address.
    //  sscanf_bd_addr(device_addr_string, device_addr);
    // btstack_stdin_setup(stdin_process);
#endif
    return 0;
}
/* LISTING_END */

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
    A2DP::mediaTracker.sbc_storage_count = 0;
    A2DP::mediaTracker.samples_ready     = 0;
    hxcmod_unload(&mod_context);
    hxcmod_setcfg(&mod_context, sample_rate, 16, 1, 1, 1);
    hxcmod_load(&mod_context, (void *)&mod_data, mod_len);
}

static void a2dp_demo_send_media_packet(void)
{
    int num_bytes_in_frame = btstack_sbc_encoder_sbc_buffer_length();
    int bytes_in_storage   = A2DP::mediaTracker.sbc_storage_count;
    uint8_t num_frames     = bytes_in_storage / num_bytes_in_frame;
    a2dp_source_stream_send_media_payload(A2DP::mediaTracker.a2dp_cid,
                                          A2DP::mediaTracker.local_seid,
                                          A2DP::mediaTracker.sbc_storage,
                                          bytes_in_storage,
                                          num_frames,
                                          0);
    A2DP::mediaTracker.sbc_storage_count = 0;
    A2DP::mediaTracker.sbc_ready_to_send = 0;
}

static void produce_mod_audio(int16_t *pcm_buffer, int num_samples_to_write)
{
    hxcmod_fillbuffer(&mod_context, (unsigned short *)&pcm_buffer[0], num_samples_to_write, &trkbuf);
}

static void produce_audio(int16_t *pcm_buffer, int num_samples)
{
    produce_mod_audio(pcm_buffer, num_samples);

#ifdef VOLUME_REDUCTION
    int i;
    for (i = 0; i < num_samples * 2; i++) {
        if (pcm_buffer[i] > 0) {
            pcm_buffer[i] = pcm_buffer[i] >> VOLUME_REDUCTION;
        }
        else {
            pcm_buffer[i] = -((-pcm_buffer[i]) >> VOLUME_REDUCTION);
        }
    }
#endif
}

static int a2dp_demo_fill_sbc_audio_buffer(A2DP::mediaContext *context)
{
    // perform sbc encodin
    int total_num_bytes_read                      = 0;
    unsigned int num_audio_samples_per_sbc_buffer = btstack_sbc_encoder_num_audio_frames();
    while (context->samples_ready >= num_audio_samples_per_sbc_buffer &&
           (context->max_media_payload_size - context->sbc_storage_count) >= btstack_sbc_encoder_sbc_buffer_length()) {

        int16_t pcm_frame[256 * A2DP::NUM_CHANNELS];

        produce_audio(pcm_frame, num_audio_samples_per_sbc_buffer);
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
    A2DP::mediaContext *context = (A2DP::mediaContext *)btstack_run_loop_get_timer_context(timer);
    btstack_run_loop_set_timer(&context->audio_timer, A2DP::AUDIO_TIMEOUT_MS);
    btstack_run_loop_add_timer(&context->audio_timer);
    uint32_t now = btstack_run_loop_get_time_ms();

    uint32_t update_period_ms = A2DP::AUDIO_TIMEOUT_MS;
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

void A2DP::startTimer(A2DP::mediaContext *context)
{
    LOG_DEBUG("Timer start");

    context->max_media_payload_size =
        btstack_min(a2dp_max_media_payload_size(context->a2dp_cid, context->local_seid), A2DP::SBC_STORAGE_SIZE);
    context->sbc_storage_count = 0;
    context->sbc_ready_to_send = 0;
    context->streaming         = 1;
    btstack_run_loop_remove_timer(&context->audio_timer);
    btstack_run_loop_set_timer_handler(&context->audio_timer, a2dp_demo_audio_timeout_handler);
    btstack_run_loop_set_timer_context(&context->audio_timer, context);
    btstack_run_loop_set_timer(&context->audio_timer, A2DP::AUDIO_TIMEOUT_MS);
    btstack_run_loop_add_timer(&context->audio_timer);
}

static void a2dp_demo_timer_stop(A2DP::mediaContext *context)
{
    LOG_DEBUG("Timer stop");

    context->time_audio_data_sent   = 0;
    context->acc_num_missed_samples = 0;
    context->samples_ready          = 0;
    context->streaming              = 1;
    context->sbc_storage_count      = 0;
    context->sbc_ready_to_send      = 0;
    btstack_run_loop_remove_timer(&context->audio_timer);
}

static void dump_sbc_configuration(A2DP::AVDTP_sbcConfiguration *configuration)
{
    LOG_INFO("Received media codec configuration:");
    LOG_INFO("    - num_channels: %d", configuration->num_channels);
    LOG_INFO("    - sampling_frequency: %d", configuration->sampling_frequency);
    LOG_INFO("    - channel_mode: %d", configuration->channel_mode);
    LOG_INFO("    - block_length: %d", configuration->block_length);
    LOG_INFO("    - subbands: %d", configuration->subbands);
    LOG_INFO("    - allocation_method: %d", configuration->allocation_method);
    LOG_INFO("    - bitpool_value [%d, %d] ", configuration->min_bitpool_value, configuration->max_bitpool_value);
}

void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
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

static void a2dp_source_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
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
                     A2DP::mediaTracker.a2dp_cid);
            A2DP::mediaTracker.a2dp_cid = 0;
            break;
        }
        A2DP::mediaTracker.a2dp_cid = cid;
        A2DP::mediaTracker.volume   = 64;

        LOG_INFO("A2DP Source: Connected to address %s, a2dp cid 0x%02x, local seid %d.\n",
                 bd_addr_to_str(address),
                 A2DP::mediaTracker.a2dp_cid,
                 A2DP::mediaTracker.local_seid);
        break;

    case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_SBC_CONFIGURATION: {
        cid = avdtp_subevent_signaling_media_codec_sbc_configuration_get_avdtp_cid(packet);
        if (cid != A2DP::mediaTracker.a2dp_cid)
            return;
        A2DP::mediaTracker.remote_seid = a2dp_subevent_signaling_media_codec_sbc_configuration_get_acp_seid(packet);

        A2DP::sbcConfiguration.reconfigure =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_reconfigure(packet);
        A2DP::sbcConfiguration.num_channels =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_num_channels(packet);
        A2DP::sbcConfiguration.sampling_frequency =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_sampling_frequency(packet);
        A2DP::sbcConfiguration.channel_mode =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_channel_mode(packet);
        A2DP::sbcConfiguration.block_length =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_block_length(packet);
        A2DP::sbcConfiguration.subbands = a2dp_subevent_signaling_media_codec_sbc_configuration_get_subbands(packet);
        A2DP::sbcConfiguration.allocation_method =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_allocation_method(packet);
        A2DP::sbcConfiguration.min_bitpool_value =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_min_bitpool_value(packet);
        A2DP::sbcConfiguration.max_bitpool_value =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_max_bitpool_value(packet);
        A2DP::sbcConfiguration.frames_per_buffer =
            A2DP::sbcConfiguration.subbands * A2DP::sbcConfiguration.block_length;
        LOG_INFO("A2DP Source: Received SBC codec configuration, sampling frequency %u, a2dp_cid 0x%02x, local seid %d "
                 "(expected %d), remote seid %d .\n",
                 A2DP::sbcConfiguration.sampling_frequency,
                 cid,
                 a2dp_subevent_signaling_media_codec_sbc_configuration_get_int_seid(packet),
                 A2DP::mediaTracker.local_seid,
                 A2DP::mediaTracker.remote_seid);

        // Adapt Bluetooth spec definition to SBC Encoder expected input
        A2DP::sbcConfiguration.allocation_method -= 1;
        A2DP::sbcConfiguration.num_channels = 2;
        switch (A2DP::sbcConfiguration.channel_mode) {
        case AVDTP_SBC_JOINT_STEREO:
            A2DP::sbcConfiguration.channel_mode = 3;
            break;
        case AVDTP_SBC_STEREO:
            A2DP::sbcConfiguration.channel_mode = 2;
            break;
        case AVDTP_SBC_DUAL_CHANNEL:
            A2DP::sbcConfiguration.channel_mode = 1;
            break;
        case AVDTP_SBC_MONO:
            A2DP::sbcConfiguration.channel_mode = 0;
            A2DP::sbcConfiguration.num_channels = 1;
            break;
        }
        dump_sbc_configuration(&A2DP::sbcConfiguration);

        btstack_sbc_encoder_init(&sbc_encoder_state,
                                 SBC_MODE_STANDARD,
                                 A2DP::sbcConfiguration.block_length,
                                 A2DP::sbcConfiguration.subbands,
                                 A2DP::sbcConfiguration.allocation_method,
                                 A2DP::sbcConfiguration.sampling_frequency,
                                 A2DP::sbcConfiguration.max_bitpool_value,
                                 A2DP::sbcConfiguration.channel_mode);
        break;
    }

    case A2DP_SUBEVENT_SIGNALING_DELAY_REPORTING_CAPABILITY:
        LOG_INFO("A2DP Source: remote supports delay report, remote seid %d\n",
                 avdtp_subevent_signaling_delay_reporting_capability_get_remote_seid(packet));
        break;
    case A2DP_SUBEVENT_SIGNALING_CAPABILITIES_DONE:
        LOG_INFO("A2DP Source: All capabilities reported, remote seid %d\n",
                 avdtp_subevent_signaling_capabilities_done_get_remote_seid(packet));
        break;

    case A2DP_SUBEVENT_SIGNALING_DELAY_REPORT:
        LOG_INFO("A2DP Source: Received delay report of %d.%0d ms, local seid %d\n",
                 avdtp_subevent_signaling_delay_report_get_delay_100us(packet) / 10,
                 avdtp_subevent_signaling_delay_report_get_delay_100us(packet) % 10,
                 avdtp_subevent_signaling_delay_report_get_local_seid(packet));
        break;

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
                 A2DP::mediaTracker.a2dp_cid,
                 cid,
                 local_seid,
                 A2DP::mediaTracker.local_seid,
                 a2dp_subevent_stream_established_get_remote_seid(packet),
                 A2DP::mediaTracker.remote_seid);

        if (local_seid != A2DP::mediaTracker.local_seid) {
            LOG_INFO("A2DP Source: Stream failed, wrong local seid %d, expected %d.\n",
                     local_seid,
                     A2DP::mediaTracker.local_seid);
            break;
        }
        LOG_INFO("A2DP Source: Stream established, address %s, a2dp cid 0x%02x, local seid %d, remote seid %d.\n",
                 bd_addr_to_str(address),
                 A2DP::mediaTracker.a2dp_cid,
                 A2DP::mediaTracker.local_seid,
                 a2dp_subevent_stream_established_get_remote_seid(packet));
        A2DP::mediaTracker.stream_opened = 1;
        status = a2dp_source_start_stream(A2DP::mediaTracker.a2dp_cid, A2DP::mediaTracker.local_seid);
        break;

    case A2DP_SUBEVENT_STREAM_RECONFIGURED:
        status     = a2dp_subevent_stream_reconfigured_get_status(packet);
        local_seid = a2dp_subevent_stream_reconfigured_get_local_seid(packet);
        cid        = a2dp_subevent_stream_reconfigured_get_a2dp_cid(packet);

        LOG_INFO("A2DP Source: Reconfigured: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 A2DP::mediaTracker.a2dp_cid,
                 cid,
                 A2DP::mediaTracker.local_seid,
                 local_seid);
        LOG_INFO("Status 0x%02x\n", status);
        break;

    case A2DP_SUBEVENT_STREAM_STARTED:
        local_seid = a2dp_subevent_stream_started_get_local_seid(packet);
        cid        = a2dp_subevent_stream_started_get_a2dp_cid(packet);

        A2DP_config::play_info.status = AVRCP_PLAYBACK_STATUS_PLAYING;
        if (A2DP::mediaTracker.avrcp_cid) {
            avrcp_target_set_now_playing_info(A2DP::mediaTracker.avrcp_cid,
                                              &A2DP_config::tracks[1],
                                              sizeof(A2DP_config::tracks) / sizeof(avrcp_track_t));
            avrcp_target_set_playback_status(A2DP::mediaTracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_PLAYING);
        }
        A2DP::startTimer(&A2DP::mediaTracker);
        LOG_INFO("A2DP Source: Stream started: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 A2DP::mediaTracker.a2dp_cid,
                 cid,
                 A2DP::mediaTracker.local_seid,
                 local_seid);
        break;

    case A2DP_SUBEVENT_STREAMING_CAN_SEND_MEDIA_PACKET_NOW:
        local_seid = a2dp_subevent_streaming_can_send_media_packet_now_get_local_seid(packet);
        cid        = a2dp_subevent_signaling_media_codec_sbc_configuration_get_a2dp_cid(packet);
        // LOG_INFO("A2DP Source: can send media packet: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid
        // [expected %d, received %d]\n", A2DP::mediaTracker.a2dp_cid, cid, A2DP::mediaTracker.local_seid, local_seid);
        a2dp_demo_send_media_packet();
        break;

    case A2DP_SUBEVENT_STREAM_SUSPENDED:
        local_seid = a2dp_subevent_stream_suspended_get_local_seid(packet);
        cid        = a2dp_subevent_stream_suspended_get_a2dp_cid(packet);

        A2DP_config::play_info.status = AVRCP_PLAYBACK_STATUS_PAUSED;
        if (A2DP::mediaTracker.avrcp_cid) {
            avrcp_target_set_playback_status(A2DP::mediaTracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_PAUSED);
        }
        LOG_INFO("A2DP Source: Stream paused: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 A2DP::mediaTracker.a2dp_cid,
                 cid,
                 A2DP::mediaTracker.local_seid,
                 local_seid);

        a2dp_demo_timer_stop(&A2DP::mediaTracker);
        break;

    case A2DP_SUBEVENT_STREAM_RELEASED:
        A2DP_config::play_info.status = AVRCP_PLAYBACK_STATUS_STOPPED;
        cid              = a2dp_subevent_stream_released_get_a2dp_cid(packet);
        local_seid       = a2dp_subevent_stream_released_get_local_seid(packet);

        LOG_INFO("A2DP Source: Stream released: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 A2DP::mediaTracker.a2dp_cid,
                 cid,
                 A2DP::mediaTracker.local_seid,
                 local_seid);

        if (cid == A2DP::mediaTracker.a2dp_cid) {
            A2DP::mediaTracker.stream_opened = 0;
            LOG_INFO("A2DP Source: Stream released.\n");
        }
        if (A2DP::mediaTracker.avrcp_cid) {
            avrcp_target_set_now_playing_info(
                A2DP::mediaTracker.avrcp_cid, NULL, sizeof(A2DP_config::tracks) / sizeof(avrcp_track_t));
            avrcp_target_set_playback_status(A2DP::mediaTracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_STOPPED);
        }

        a2dp_demo_timer_stop(&A2DP::mediaTracker);
        break;
    case A2DP_SUBEVENT_SIGNALING_CONNECTION_RELEASED:
        cid = a2dp_subevent_signaling_connection_released_get_a2dp_cid(packet);
        if (cid == A2DP::mediaTracker.a2dp_cid) {
            A2DP::mediaTracker.avrcp_cid = 0;
            A2DP::mediaTracker.a2dp_cid  = 0;
            LOG_INFO("A2DP Source: Signaling released.\n\n");
        }
        break;
    default:
        break;
    }
}

static void avrcp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    bd_addr_t event_addr;
    uint16_t local_cid;
    uint8_t status = ERROR_CODE_SUCCESS;

    if (packet_type != HCI_EVENT_PACKET)
        return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META)
        return;

    switch (packet[2]) {
    case AVRCP_SUBEVENT_CONNECTION_ESTABLISHED:
        local_cid = avrcp_subevent_connection_established_get_avrcp_cid(packet);
        status    = avrcp_subevent_connection_established_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
            LOG_INFO("AVRCP: Connection failed, local cid 0x%02x, status 0x%02x\n", local_cid, status);
            return;
        }
        A2DP::mediaTracker.avrcp_cid = local_cid;
        avrcp_subevent_connection_established_get_bd_addr(packet, event_addr);

        avrcp_target_set_now_playing_info(
            A2DP::mediaTracker.avrcp_cid, NULL, sizeof(A2DP_config::tracks) / sizeof(avrcp_track_t));
        avrcp_target_set_unit_info(A2DP::mediaTracker.avrcp_cid, AVRCP_SUBUNIT_TYPE_AUDIO, A2DP_config::company_id);
        avrcp_target_set_subunit_info(A2DP::mediaTracker.avrcp_cid,
                                      AVRCP_SUBUNIT_TYPE_AUDIO,
                                      (uint8_t *)A2DP_config::subunit_info,
                                      sizeof(A2DP_config::subunit_info));

        avrcp_controller_get_supported_events(A2DP::mediaTracker.avrcp_cid);

        LOG_INFO("AVRCP: Channel successfully opened:  A2DP::mediaTracker.avrcp_cid 0x%02x\n",
                 A2DP::mediaTracker.avrcp_cid);
        return;

    case AVRCP_SUBEVENT_CONNECTION_RELEASED:
        LOG_INFO("AVRCP Target: Disconnected, avrcp_cid 0x%02x\n",
                 avrcp_subevent_connection_released_get_avrcp_cid(packet));
        A2DP::mediaTracker.avrcp_cid = 0;
        return;
    default:
        break;
    }

    if (status != ERROR_CODE_SUCCESS) {
        LOG_INFO("Responding to event 0x%02x failed with status 0x%02x\n", packet[2], status);
    }
}

static void avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    uint8_t status = ERROR_CODE_SUCCESS;

    if (packet_type != HCI_EVENT_PACKET)
        return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META)
        return;

    switch (packet[2]) {
    case AVRCP_SUBEVENT_NOTIFICATION_VOLUME_CHANGED:
        A2DP::mediaTracker.volume = avrcp_subevent_notification_volume_changed_get_absolute_volume(packet);
        LOG_INFO("AVRCP Target: Volume set to %d%% (%d)\n",
                 A2DP::mediaTracker.volume * 127 / 100,
                 A2DP::mediaTracker.volume);
        break;
    case AVRCP_SUBEVENT_EVENT_IDS_QUERY:
        status = avrcp_target_supported_events(
            A2DP::mediaTracker.avrcp_cid, A2DP_config::events_num, A2DP_config::events, sizeof(A2DP_config::events));
        break;
    case AVRCP_SUBEVENT_COMPANY_IDS_QUERY:
        status = avrcp_target_supported_companies(A2DP::mediaTracker.avrcp_cid,
                                                  A2DP_config::companies_num,
                                                  A2DP_config::companies,
                                                  sizeof(A2DP_config::companies));
        break;
    case AVRCP_SUBEVENT_PLAY_STATUS_QUERY:
        status = avrcp_target_play_status(A2DP::mediaTracker.avrcp_cid,
                                          A2DP_config::play_info.song_length_ms,
                                          A2DP_config::play_info.song_position_ms,
                                          A2DP_config::play_info.status);
        break;
        // case AVRCP_SUBEVENT_NOW_PLAYING_INFO_QUERY:
        //     status = avrcp_target_now_playing_info(avrcp_cid);
        //     break;
    case AVRCP_SUBEVENT_OPERATION: {
        avrcp_operation_id_t operation_id = (avrcp_operation_id_t)avrcp_subevent_operation_get_operation_id(packet);
        switch (operation_id) {
        case AVRCP_OPERATION_ID_PLAY:
            LOG_INFO("AVRCP Target: PLAY\n");
            status = a2dp_source_start_stream(A2DP::mediaTracker.a2dp_cid, A2DP::mediaTracker.local_seid);
            break;
        case AVRCP_OPERATION_ID_PAUSE:
            LOG_INFO("AVRCP Target: PAUSE\n");
            status = a2dp_source_pause_stream(A2DP::mediaTracker.a2dp_cid, A2DP::mediaTracker.local_seid);
            break;
        case AVRCP_OPERATION_ID_STOP:
            LOG_INFO("AVRCP Target: STOP\n");
            status = a2dp_source_disconnect(A2DP::mediaTracker.a2dp_cid);
            break;
        default:
            LOG_INFO("AVRCP Target: operation 0x%2x is not handled\n", operation_id);
            return;
        }
        break;
    }

    default:
        break;
    }

    if (status != ERROR_CODE_SUCCESS) {
        LOG_INFO("Responding to event 0x%02x failed with status 0x%02x\n", packet[2], status);
    }
}

static void avrcp_controller_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    uint8_t status = 0xFF;

    if (packet_type != HCI_EVENT_PACKET)
        return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META)
        return;

    status = packet[5];
    if (!A2DP::mediaTracker.avrcp_cid)
        return;

    // ignore INTERIM status
    if (status == AVRCP_CTYPE_RESPONSE_INTERIM)
        return;

    switch (packet[2]) {
    case AVRCP_SUBEVENT_NOTIFICATION_VOLUME_CHANGED:
        LOG_INFO("AVRCP Controller: notification absolute volume changed %d %%\n",
                 avrcp_subevent_notification_volume_changed_get_absolute_volume(packet) * 100 / 127);
        break;
    case AVRCP_SUBEVENT_GET_CAPABILITY_EVENT_ID:
        LOG_INFO("Remote supports EVENT_ID 0x%02x\n", avrcp_subevent_get_capability_event_id_get_event_id(packet));
        break;
    case AVRCP_SUBEVENT_GET_CAPABILITY_EVENT_ID_DONE:
        LOG_INFO("automatically enable notifications\n");
        avrcp_controller_enable_notification(A2DP::mediaTracker.avrcp_cid, AVRCP_NOTIFICATION_EVENT_VOLUME_CHANGED);
        break;
    default:
        break;
    }
}

void A2DP::start()
{
    LOG_INFO("Starting playback to %s", bd_addr_to_str(device_addr));
    a2dp_source_establish_stream(device_addr, A2DP::mediaTracker.local_seid, &A2DP::mediaTracker.a2dp_cid);
}

void A2DP::stop()
{
    LOG_INFO("Stopping playback");
    a2dp_source_disconnect(A2DP::mediaTracker.a2dp_cid);
    l2cap_unregister_service(1);
};

Error A2DP::init()
{
    a2dp_source_and_avrcp_services_init();
    LOG_INFO("Init done!");
    return Error();
}

void A2DP::setDeviceAddress(bd_addr_t addr)
{
    bd_addr_copy(device_addr, addr);
    LOG_INFO("Address set!");
}
