#pragma once

#include <Device.hpp>
#include <log/log.hpp>
#include <vector>
#include <Error.hpp>
#include <BtCommand.hpp>
#include <vfs.hpp>
namespace Bt::A2DP_config
{}

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


namespace Bt
{

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

        using AVDTP_sbcConfiguration = struct
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
        };

        Error init() override;
        void start();
        void stop();
        void setDeviceAddress(bd_addr_t addr);
        static void startTimer(mediaContext *context);
        static mediaContext mediaTracker;
        static AVDTP_sbcConfiguration sbcConfiguration;
        static uint8_t mediaSbcCodecCapabilities[];
    };

} // namespace Bt
