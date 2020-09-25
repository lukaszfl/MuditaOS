#pragma once

#include <BtCommand.hpp>
#include <Device.hpp>
#include <Error.hpp>
#include <log/log.hpp>
#include <vector>
#include <vfs.hpp>

extern "C"
{
#include "classic/avrcp.h"
#include "classic/avrcp_browsing_controller.h"
#include "classic/avrcp_browsing_target.h"
#include "classic/avrcp_controller.h"
#include "classic/avrcp_media_item_iterator.h"
#include "classic/avrcp_target.h"
#include "module-bluetooth/lib/btstack/src/btstack.h"
#include <btstack_defines.h>
#include <classic/avdtp.h>
#include <lib/btstack/3rd-party/hxcmod-player/hxcmod.h>
#include <lib/btstack/3rd-party/hxcmod-player/mods/mod.h>
};

namespace Bt
{
    class AVRCP
    {
      public:
        constexpr static const uint8_t subunitInfo[] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
                                                        4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7};

        constexpr static uint32_t companyId   = 0x112233;
        constexpr static uint8_t companiesNum = 1;
        constexpr static uint8_t companies[]  = {
            0x00, 0x19, 0x58 // BT SIG registered CompanyID
        };

        constexpr static uint8_t eventsNum = 6;
        constexpr static uint8_t events[]  = {AVRCP_NOTIFICATION_EVENT_PLAYBACK_STATUS_CHANGED,
                                             AVRCP_NOTIFICATION_EVENT_TRACK_CHANGED,
                                             AVRCP_NOTIFICATION_EVENT_PLAYER_APPLICATION_SETTING_CHANGED,
                                             AVRCP_NOTIFICATION_EVENT_NOW_PLAYING_CONTENT_CHANGED,
                                             AVRCP_NOTIFICATION_EVENT_AVAILABLE_PLAYERS_CHANGED,
                                             AVRCP_NOTIFICATION_EVENT_ADDRESSED_PLAYER_CHANGED};

        using playbackStatusInfo = struct
        {
            uint8_t track_id[8];
            uint32_t song_length_ms;
            avrcp_playback_status_t status;
            uint32_t song_position_ms; // 0xFFFFFFFF if not supported
        };
        static uint8_t sdpTargetServiceBuffer[200];
        static uint8_t sdpControllerServiceBuffer[200];

        static avrcp_track_t tracks[3];
        static int currentTrackIndex;
        static playbackStatusInfo playInfo;

        static void packetHandler(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size);
        static void targetPacketHandler(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size);
        static void controllerPacketHandler(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size);
    };

    class AVDTP
    {
      public:
        using sbcConfiguration = struct
        {
            int reconfigure;
            int numChannels;
            int samplingFrequency;
            int channelMode;
            int blockLength;
            int subbands;
            int allocationMethod;
            int minBitpoolValue;
            int maxBitpoolValue;
            int framesPerBuffer;
        };
        static sbcConfiguration sbcConfig;
        static btstack_sbc_encoder_state_t sbcEncoderState;
        static uint8_t sbcCodecConfiguration[4];
        static constexpr int defaultSampleRate = 44100;
        static int sampleRate;

        static void dumpSbcConfiguration();
    };

    class Player
    {
      public:
        static int hxcmodInitialized;
        static modcontext modContext;
        static tracker_buffer_state trkbuf;
        static void reconfigureSampleRate(int newSampleRate);
        static void produceModAudio(int16_t *pcmBuffer, int numSamplesToWrite);
        static void produceAudio(int16_t *pcmBuffer, int numSamples);
    };

    class A2DP : public Profile
    {
      public:
        static constexpr int NUM_CHANNELS           = 2;
        static constexpr int BYTES_PER_AUDIO_SAMPLE = (2 * NUM_CHANNELS);
        static constexpr int AUDIO_TIMEOUT_MS       = 10;
        static constexpr int TABLE_SIZE_441HZ       = 100;
        static constexpr int SBC_STORAGE_SIZE       = 1030;
        using mediaContext                          = struct
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

            uint8_t sbc_storage[A2DP::SBC_STORAGE_SIZE];
            uint16_t sbc_storage_count;
            uint8_t sbc_ready_to_send;

            uint16_t volume;
        };
        static uint8_t sdpSourceServiceBuffer[150];
        static bd_addr_t deviceAddr;
        static btstack_packet_callback_registration_t hciEventCallbackRegistration;
        static mediaContext mediaTracker;
        static uint8_t mediaSbcCodecCapabilities[];

        Error init() override;
        void start();
        void stop();
        void setDeviceAddress(bd_addr_t addr);
        static void startTimer(mediaContext *context);
        static void stopTimer(A2DP::mediaContext *context);

        static void audioTimeoutHandler(btstack_timer_source_t *timer);

        static void sourcePacketHandler(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size);
        static void hciPacketHandler(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size);
        static void sendMediaPacket(void);
        static auto fillSbcAudioBuffer(A2DP::mediaContext *context) -> int;
    };

} // namespace Bt
