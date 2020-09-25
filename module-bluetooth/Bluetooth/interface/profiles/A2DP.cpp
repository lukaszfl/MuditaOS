//
// Created by bartek on 14.09.2020.
//

#include "A2DP.hpp"
#include <Bluetooth/Device.hpp>
#include <Bluetooth/Error.hpp>
#include <log/log.hpp>

using namespace Bt;

A2DP::mediaContext A2DP::mediaTracker;
AVDTP::sbcConfiguration AVDTP::sbcConfig;
uint8_t A2DP::mediaSbcCodecCapabilities[] = {
    (AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
    0xFF, //(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) | AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
    2,
    53};

AVRCP::playbackStatusInfo AVRCP::playInfo;
int AVRCP::currentTrackIndex;

avrcp_track_t AVRCP::tracks[3] = {
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

uint8_t A2DP::sdpSourceServiceBuffer[150];
uint8_t AVRCP::sdpTargetServiceBuffer[200];
uint8_t AVRCP::sdpControllerServiceBuffer[200];

bd_addr_t A2DP::deviceAddr;

btstack_sbc_encoder_state_t AVDTP::sbcEncoderState;
uint8_t AVDTP::sbcCodecConfiguration[4];
int AVDTP::sampleRate = AVDTP::defaultSampleRate;

int Player::hxcmodInitialized;
modcontext Player::modContext;
tracker_buffer_state Player::trkbuf;

btstack_packet_callback_registration_t A2DP::hciEventCallbackRegistration;

/* LISTING_START(MainConfiguration): Setup Audio Source and AVRCP Target services */

Error A2DP::init(void)
{
    // request role change on reconnecting headset to always use them in slave mode
    hci_set_master_slave_policy(0);

    l2cap_init();
    // Initialize  A2DP Source.
    a2dp_source_init();
    a2dp_source_register_packet_handler(&sourcePacketHandler);

    // Create stream endpoint.
    avdtp_stream_endpoint_t *local_stream_endpoint =
        a2dp_source_create_stream_endpoint(AVDTP_AUDIO,
                                           AVDTP_CODEC_SBC,
                                           mediaSbcCodecCapabilities,
                                           sizeof(mediaSbcCodecCapabilities),
                                           AVDTP::sbcCodecConfiguration,
                                           sizeof(AVDTP::sbcCodecConfiguration));
    if (!local_stream_endpoint) {
        LOG_INFO("A2DP Source: not enough memory to create local stream endpoint\n");
        return Error(Bt::Error::SystemError);
    }
    mediaTracker.local_seid = avdtp_local_seid(local_stream_endpoint);
    avdtp_source_register_delay_reporting_category(mediaTracker.local_seid);

    // Initialize AVRCP Service.
    avrcp_init();
    avrcp_register_packet_handler(&AVRCP::packetHandler);
    // Initialize AVRCP Target.
    avrcp_target_init();
    avrcp_target_register_packet_handler(&AVRCP::targetPacketHandler);
    // Initialize AVRCP Controller
    avrcp_controller_init();
    avrcp_controller_register_packet_handler(&AVRCP::controllerPacketHandler);

    // Initialize SDP,
    sdp_init();

    // Create  A2DP Source service record and register it with SDP.
    memset(sdpSourceServiceBuffer, 0, sizeof(sdpSourceServiceBuffer));
    a2dp_source_create_sdp_record(sdpSourceServiceBuffer, 0x10001, AVDTP_SOURCE_FEATURE_MASK_PLAYER, NULL, NULL);
    sdp_register_service(sdpSourceServiceBuffer);

    // Create AVRCP target service record and register it with SDP.
    memset(AVRCP::sdpTargetServiceBuffer, 0, sizeof(AVRCP::sdpTargetServiceBuffer));
    uint16_t supportedFeatures = AVRCP_FEATURE_MASK_CATEGORY_PLAYER_OR_RECORDER;
#ifdef AVRCP_BROWSING_ENABLED
    supported_features |= AVRCP_FEATURE_MASK_BROWSING;
#endif
    avrcp_target_create_sdp_record(AVRCP::sdpTargetServiceBuffer, 0x10002, supportedFeatures, NULL, NULL);
    sdp_register_service(AVRCP::sdpTargetServiceBuffer);

    // setup AVRCP Controller
    memset(AVRCP::sdpControllerServiceBuffer, 0, sizeof(AVRCP::sdpControllerServiceBuffer));
    uint16_t controllerSupportedFeatures = AVRCP_FEATURE_MASK_CATEGORY_PLAYER_OR_RECORDER;
    avrcp_controller_create_sdp_record(
        AVRCP::sdpControllerServiceBuffer, 0x10003, controllerSupportedFeatures, NULL, NULL);
    sdp_register_service(AVRCP::sdpControllerServiceBuffer);

    // Set local name with a template Bluetooth address, that will be automatically
    // replaced with a actual address once it is available, i.e. when BTstack boots
    // up and starts talking to a Bluetooth module.
    gap_set_local_name("PurePhone");
    gap_discoverable_control(1);
    gap_set_class_of_device(0x200408);

    // Register for HCI events.
    hciEventCallbackRegistration.callback = &hciPacketHandler;
    hci_add_event_handler(&hciEventCallbackRegistration);

    Player::reconfigureSampleRate(AVDTP::sampleRate);
    LOG_INFO("Init done!");

    return Error();
}

void Player::reconfigureSampleRate(int newSampleRate)
{
    if (!hxcmodInitialized) {
        hxcmodInitialized = hxcmod_init(&modContext);
        if (!hxcmodInitialized) {
            LOG_INFO("could not initialize hxcmod\n");
            return;
        }
    }
    AVDTP::sampleRate                    = newSampleRate;
    A2DP::mediaTracker.sbc_storage_count = 0;
    A2DP::mediaTracker.samples_ready     = 0;
    hxcmod_unload(&modContext);
    hxcmod_setcfg(&modContext, AVDTP::sampleRate, 16, 1, 1, 1);
    hxcmod_load(&modContext, (void *)&mod_data, mod_len);
}

void A2DP::sendMediaPacket(void)
{
    int numBytesInFrame = btstack_sbc_encoder_sbc_buffer_length();
    int bytesInStorage  = A2DP::mediaTracker.sbc_storage_count;
    uint8_t numFrames   = bytesInStorage / numBytesInFrame;
    a2dp_source_stream_send_media_payload(A2DP::mediaTracker.a2dp_cid,
                                          A2DP::mediaTracker.local_seid,
                                          A2DP::mediaTracker.sbc_storage,
                                          bytesInStorage,
                                          numFrames,
                                          0);
    A2DP::mediaTracker.sbc_storage_count = 0;
    A2DP::mediaTracker.sbc_ready_to_send = 0;
}

void Player::produceModAudio(int16_t *pcmBuffer, int numSamplesToWrite)
{
    hxcmod_fillbuffer(&modContext, (unsigned short *)&pcmBuffer[0], numSamplesToWrite, &trkbuf);
}

void Player::produceAudio(int16_t *pcmBuffer, int numSamples)
{
    produceModAudio(pcmBuffer, numSamples);

#ifdef VOLUME_REDUCTION
    int i;
    for (i = 0; i < numSamples * 2; i++) {
        if (pcmBuffer[i] > 0) {
            pcmBuffer[i] = pcmBuffer[i] >> VOLUME_REDUCTION;
        }
        else {
            pcmBuffer[i] = -((-pcmBuffer[i]) >> VOLUME_REDUCTION);
        }
    }
#endif
}

int A2DP::fillSbcAudioBuffer(A2DP::mediaContext *context)
{
    // perform sbc encodin
    int totalNumBytesRead                    = 0;
    unsigned int numAudioSamplesPerSbcBuffer = btstack_sbc_encoder_num_audio_frames();
    while (context->samples_ready >= numAudioSamplesPerSbcBuffer &&
           (context->max_media_payload_size - context->sbc_storage_count) >= btstack_sbc_encoder_sbc_buffer_length()) {

        int16_t pcmFrame[256 * A2DP::NUM_CHANNELS];

        Player::produceAudio(pcmFrame, numAudioSamplesPerSbcBuffer);
        btstack_sbc_encoder_process_data(pcmFrame);

        uint16_t sbcFrameSize = btstack_sbc_encoder_sbc_buffer_length();
        uint8_t *sbcFrame     = btstack_sbc_encoder_sbc_buffer();

        totalNumBytesRead += numAudioSamplesPerSbcBuffer;
        memcpy(&context->sbc_storage[context->sbc_storage_count], sbcFrame, sbcFrameSize);
        context->sbc_storage_count += sbcFrameSize;
        context->samples_ready -= numAudioSamplesPerSbcBuffer;
    }
    return totalNumBytesRead;
}

void A2DP::audioTimeoutHandler(btstack_timer_source_t *timer)
{
    mediaContext *context = (mediaContext *)btstack_run_loop_get_timer_context(timer);
    btstack_run_loop_set_timer(&context->audio_timer, AUDIO_TIMEOUT_MS);
    btstack_run_loop_add_timer(&context->audio_timer);
    uint32_t now = btstack_run_loop_get_time_ms();

    uint32_t updatePeriodMs = AUDIO_TIMEOUT_MS;
    if (context->time_audio_data_sent > 0) {
        updatePeriodMs = now - context->time_audio_data_sent;
    }

    uint32_t numSamples = (updatePeriodMs * AVDTP::sampleRate) / 1000;
    context->acc_num_missed_samples += (updatePeriodMs * AVDTP::sampleRate) % 1000;

    while (context->acc_num_missed_samples >= 1000) {
        numSamples++;
        context->acc_num_missed_samples -= 1000;
    }
    context->time_audio_data_sent = now;
    context->samples_ready += numSamples;

    if (context->sbc_ready_to_send)
        return;

    fillSbcAudioBuffer(context);

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
    btstack_run_loop_set_timer_handler(&context->audio_timer, audioTimeoutHandler);
    btstack_run_loop_set_timer_context(&context->audio_timer, context);
    btstack_run_loop_set_timer(&context->audio_timer, A2DP::AUDIO_TIMEOUT_MS);
    btstack_run_loop_add_timer(&context->audio_timer);
}

void A2DP::stopTimer(A2DP::mediaContext *context)
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

void AVDTP::dumpSbcConfiguration()
{
    LOG_INFO("Received media codec configuration:");
    LOG_INFO("    - numChannels: %d", sbcConfig.numChannels);
    LOG_INFO("    - samplingFrequency: %d", sbcConfig.samplingFrequency);
    LOG_INFO("    - channelMode: %d", sbcConfig.channelMode);
    LOG_INFO("    - blockLength: %d", sbcConfig.blockLength);
    LOG_INFO("    - subbands: %d", sbcConfig.subbands);
    LOG_INFO("    - allocationMethod: %d", sbcConfig.allocationMethod);
    LOG_INFO("    - bitpool_value [%d, %d] ", sbcConfig.minBitpoolValue, sbcConfig.maxBitpoolValue);
}

void A2DP::hciPacketHandler(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    if (packetType != HCI_EVENT_PACKET)
        return;

    if (hci_event_packet_get_type(packet) == HCI_EVENT_PIN_CODE_REQUEST) {
        bd_addr_t address;
        LOG_INFO("Pin code request - using '0000'\n");
        hci_event_pin_code_request_get_bd_addr(packet, address);
        gap_pin_code_response(address, "0000");
    }
}

void A2DP::sourcePacketHandler(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    uint8_t status;
    uint8_t local_seid;
    bd_addr_t address;
    uint16_t cid;

    if (packetType != HCI_EVENT_PACKET)
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
                     mediaTracker.a2dp_cid);
            mediaTracker.a2dp_cid = 0;
            break;
        }
        mediaTracker.a2dp_cid = cid;
        mediaTracker.volume   = 64;

        LOG_INFO("A2DP Source: Connected to address %s, a2dp cid 0x%02x, local seid %d.\n",
                 bd_addr_to_str(address),
                 mediaTracker.a2dp_cid,
                 mediaTracker.local_seid);
        break;

    case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_SBC_CONFIGURATION: {
        cid = avdtp_subevent_signaling_media_codec_sbc_configuration_get_avdtp_cid(packet);
        if (cid != mediaTracker.a2dp_cid)
            return;
        mediaTracker.remote_seid = a2dp_subevent_signaling_media_codec_sbc_configuration_get_acp_seid(packet);

        AVDTP::sbcConfig.reconfigure = a2dp_subevent_signaling_media_codec_sbc_configuration_get_reconfigure(packet);
        AVDTP::sbcConfig.numChannels = a2dp_subevent_signaling_media_codec_sbc_configuration_get_num_channels(packet);
        AVDTP::sbcConfig.samplingFrequency =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_sampling_frequency(packet);
        AVDTP::sbcConfig.channelMode = a2dp_subevent_signaling_media_codec_sbc_configuration_get_channel_mode(packet);
        AVDTP::sbcConfig.blockLength = a2dp_subevent_signaling_media_codec_sbc_configuration_get_block_length(packet);
        AVDTP::sbcConfig.subbands    = a2dp_subevent_signaling_media_codec_sbc_configuration_get_subbands(packet);
        AVDTP::sbcConfig.allocationMethod =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_allocation_method(packet);
        AVDTP::sbcConfig.minBitpoolValue =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_min_bitpool_value(packet);
        AVDTP::sbcConfig.maxBitpoolValue =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_max_bitpool_value(packet);
        AVDTP::sbcConfig.framesPerBuffer = AVDTP::sbcConfig.subbands * AVDTP::sbcConfig.blockLength;
        LOG_INFO("A2DP Source: Received SBC codec configuration, sampling frequency %u, a2dp_cid 0x%02x, local seid %d "
                 "(expected %d), remote seid %d .\n",
                 AVDTP::sbcConfig.samplingFrequency,
                 cid,
                 a2dp_subevent_signaling_media_codec_sbc_configuration_get_int_seid(packet),
                 mediaTracker.local_seid,
                 mediaTracker.remote_seid);

        // Adapt Bluetooth spec definition to SBC Encoder expected input
        AVDTP::sbcConfig.allocationMethod -= 1;
        AVDTP::sbcConfig.numChannels = 2;
        switch (AVDTP::sbcConfig.channelMode) {
        case AVDTP_SBC_JOINT_STEREO:
            AVDTP::sbcConfig.channelMode = 3;
            break;
        case AVDTP_SBC_STEREO:
            AVDTP::sbcConfig.channelMode = 2;
            break;
        case AVDTP_SBC_DUAL_CHANNEL:
            AVDTP::sbcConfig.channelMode = 1;
            break;
        case AVDTP_SBC_MONO:
            AVDTP::sbcConfig.channelMode = 0;
            AVDTP::sbcConfig.numChannels = 1;
            break;
        }
        AVDTP::dumpSbcConfiguration();

        btstack_sbc_encoder_init(&AVDTP::sbcEncoderState,
                                 SBC_MODE_STANDARD,
                                 AVDTP::sbcConfig.blockLength,
                                 AVDTP::sbcConfig.subbands,
                                 AVDTP::sbcConfig.allocationMethod,
                                 AVDTP::sbcConfig.samplingFrequency,
                                 AVDTP::sbcConfig.maxBitpoolValue,
                                 AVDTP::sbcConfig.channelMode);
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
                 mediaTracker.a2dp_cid,
                 cid,
                 local_seid,
                 mediaTracker.local_seid,
                 a2dp_subevent_stream_established_get_remote_seid(packet),
                 mediaTracker.remote_seid);

        if (local_seid != mediaTracker.local_seid) {
            LOG_INFO(
                "A2DP Source: Stream failed, wrong local seid %d, expected %d.\n", local_seid, mediaTracker.local_seid);
            break;
        }
        LOG_INFO("A2DP Source: Stream established, address %s, a2dp cid 0x%02x, local seid %d, remote seid %d.\n",
                 bd_addr_to_str(address),
                 mediaTracker.a2dp_cid,
                 mediaTracker.local_seid,
                 a2dp_subevent_stream_established_get_remote_seid(packet));
        mediaTracker.stream_opened = 1;
        status                     = a2dp_source_start_stream(mediaTracker.a2dp_cid, mediaTracker.local_seid);
        break;

    case A2DP_SUBEVENT_STREAM_RECONFIGURED:
        status     = a2dp_subevent_stream_reconfigured_get_status(packet);
        local_seid = a2dp_subevent_stream_reconfigured_get_local_seid(packet);
        cid        = a2dp_subevent_stream_reconfigured_get_a2dp_cid(packet);

        LOG_INFO("A2DP Source: Reconfigured: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 mediaTracker.a2dp_cid,
                 cid,
                 mediaTracker.local_seid,
                 local_seid);
        LOG_INFO("Status 0x%02x\n", status);
        break;

    case A2DP_SUBEVENT_STREAM_STARTED:
        local_seid = a2dp_subevent_stream_started_get_local_seid(packet);
        cid        = a2dp_subevent_stream_started_get_a2dp_cid(packet);

        AVRCP::playInfo.status = AVRCP_PLAYBACK_STATUS_PLAYING;
        if (mediaTracker.avrcp_cid) {
            avrcp_target_set_now_playing_info(
                mediaTracker.avrcp_cid, &AVRCP::tracks[1], sizeof(AVRCP::tracks) / sizeof(avrcp_track_t));
            avrcp_target_set_playback_status(mediaTracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_PLAYING);
        }
        startTimer(&mediaTracker);
        LOG_INFO("A2DP Source: Stream started: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 mediaTracker.a2dp_cid,
                 cid,
                 mediaTracker.local_seid,
                 local_seid);
        break;

    case A2DP_SUBEVENT_STREAMING_CAN_SEND_MEDIA_PACKET_NOW:
        local_seid = a2dp_subevent_streaming_can_send_media_packet_now_get_local_seid(packet);
        cid        = a2dp_subevent_signaling_media_codec_sbc_configuration_get_a2dp_cid(packet);
        // LOG_INFO("A2DP Source: can send media packet: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid
        // [expected %d, received %d]\n", mediaTracker.a2dp_cid, cid, mediaTracker.local_seid, local_seid);
        sendMediaPacket();
        break;

    case A2DP_SUBEVENT_STREAM_SUSPENDED:
        local_seid = a2dp_subevent_stream_suspended_get_local_seid(packet);
        cid        = a2dp_subevent_stream_suspended_get_a2dp_cid(packet);

        AVRCP::playInfo.status = AVRCP_PLAYBACK_STATUS_PAUSED;
        if (mediaTracker.avrcp_cid) {
            avrcp_target_set_playback_status(mediaTracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_PAUSED);
        }
        LOG_INFO("A2DP Source: Stream paused: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 mediaTracker.a2dp_cid,
                 cid,
                 mediaTracker.local_seid,
                 local_seid);

        stopTimer(&mediaTracker);
        break;

    case A2DP_SUBEVENT_STREAM_RELEASED:
        AVRCP::playInfo.status = AVRCP_PLAYBACK_STATUS_STOPPED;
        cid                    = a2dp_subevent_stream_released_get_a2dp_cid(packet);
        local_seid             = a2dp_subevent_stream_released_get_local_seid(packet);

        LOG_INFO("A2DP Source: Stream released: a2dp_cid [expected 0x%02x, received 0x%02x], local_seid [expected %d, "
                 "received %d]\n",
                 mediaTracker.a2dp_cid,
                 cid,
                 mediaTracker.local_seid,
                 local_seid);

        if (cid == mediaTracker.a2dp_cid) {
            mediaTracker.stream_opened = 0;
            LOG_INFO("A2DP Source: Stream released.\n");
        }
        if (mediaTracker.avrcp_cid) {
            avrcp_target_set_now_playing_info(
                mediaTracker.avrcp_cid, NULL, sizeof(AVRCP::tracks) / sizeof(avrcp_track_t));
            avrcp_target_set_playback_status(mediaTracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_STOPPED);
        }

        stopTimer(&mediaTracker);
        break;
    case A2DP_SUBEVENT_SIGNALING_CONNECTION_RELEASED:
        cid = a2dp_subevent_signaling_connection_released_get_a2dp_cid(packet);
        if (cid == mediaTracker.a2dp_cid) {
            mediaTracker.avrcp_cid = 0;
            mediaTracker.a2dp_cid  = 0;
            LOG_INFO("A2DP Source: Signaling released.\n\n");
        }
        break;
    default:
        break;
    }
}

void AVRCP::packetHandler(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    bd_addr_t event_addr;
    uint16_t local_cid;
    uint8_t status = ERROR_CODE_SUCCESS;

    if (packetType != HCI_EVENT_PACKET)
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
            A2DP::mediaTracker.avrcp_cid, NULL, sizeof(AVRCP::tracks) / sizeof(avrcp_track_t));
        avrcp_target_set_unit_info(A2DP::mediaTracker.avrcp_cid, AVRCP_SUBUNIT_TYPE_AUDIO, AVRCP::companyId);
        avrcp_target_set_subunit_info(A2DP::mediaTracker.avrcp_cid,
                                      AVRCP_SUBUNIT_TYPE_AUDIO,
                                      (uint8_t *)AVRCP::subunitInfo,
                                      sizeof(AVRCP::subunitInfo));

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

void AVRCP::targetPacketHandler(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    uint8_t status = ERROR_CODE_SUCCESS;

    if (packetType != HCI_EVENT_PACKET)
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
        status = avrcp_target_supported_events(A2DP::mediaTracker.avrcp_cid,
                                               AVRCP::eventsNum,
                                               const_cast<uint8_t *>(AVRCP::events),
                                               sizeof(AVRCP::events));
        break;
    case AVRCP_SUBEVENT_COMPANY_IDS_QUERY:
        status = avrcp_target_supported_companies(A2DP::mediaTracker.avrcp_cid,
                                                  AVRCP::companiesNum,
                                                  const_cast<uint8_t *>(AVRCP::companies),
                                                  sizeof(AVRCP::companies));
        break;
    case AVRCP_SUBEVENT_PLAY_STATUS_QUERY:
        status = avrcp_target_play_status(A2DP::mediaTracker.avrcp_cid,
                                          AVRCP::playInfo.song_length_ms,
                                          AVRCP::playInfo.song_position_ms,
                                          AVRCP::playInfo.status);
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

void AVRCP::controllerPacketHandler(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    uint8_t status = 0xFF;

    if (packetType != HCI_EVENT_PACKET)
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
    LOG_INFO("Starting playback to %s", bd_addr_to_str(deviceAddr));
    a2dp_source_establish_stream(deviceAddr, A2DP::mediaTracker.local_seid, &A2DP::mediaTracker.a2dp_cid);
}

void A2DP::stop()
{
    LOG_INFO("Stopping playback");
    a2dp_source_disconnect(A2DP::mediaTracker.a2dp_cid);
    l2cap_unregister_service(1);
};


void A2DP::setDeviceAddress(bd_addr_t addr)
{
    bd_addr_copy(deviceAddr, addr);
    LOG_INFO("Address set!");
}
