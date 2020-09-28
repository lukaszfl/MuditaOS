/*
 * Copyright (C) 2016 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at
 * contact@profiles-gmbh.com
 *
 */


/*
 * sco_demo_util.c - send/receive test data via SCO, used by hfp_*_demo and hsp_*_demo
 */

#include "SCO.hpp"
#include "wavWriter.hpp"
#include <cstdio>
#include <vfs.hpp>

extern "C"
{
#include "btstack_audio.h"
#include "btstack_debug.h"
#include "btstack_ring_buffer.h"
#include "classic/btstack_cvsd_plc.h"
#include "classic/btstack_sbc.h"
#include "classic/hfp.h"
#include "classic/hfp_msbc.h"
#include "hci.h"
};

namespace Bt
{

    class SCO::SCOImpl
    {
      public:
        static void close();
        static void setCodec(uint8_t codec);
        static void init();
        static void send(hci_con_handle_t sco_handle);
        static void receive(uint8_t *packet, uint16_t size);

      private:
        // constants
        constexpr static int NUM_CHANNELS     = 1;
        constexpr static int CVSD_SAMPLE_RATE = 8000;
        constexpr static int MSBC_SAMPLE_RATE = 16000;
        constexpr static int BYTES_PER_FRAME  = 2;

        constexpr static int SCO_CVSD_PA_PREBUFFER_MS = 50;
        constexpr static int SCO_MSBC_PA_PREBUFFER_MS = 50;
        constexpr static int CVSD_PA_PREBUFFER_BYTES =
            (SCO_CVSD_PA_PREBUFFER_MS * CVSD_SAMPLE_RATE / 1000 * BYTES_PER_FRAME);
        constexpr static int MSBC_PA_PREBUFFER_BYTES =
            (SCO_MSBC_PA_PREBUFFER_MS * MSBC_SAMPLE_RATE / 1000 * BYTES_PER_FRAME);

        // output
        static int audio_output_paused;
        static uint8_t audio_output_ring_buffer_storage[2 * MSBC_PA_PREBUFFER_BYTES];
        static btstack_ring_buffer_t audio_output_ring_buffer;

        static int audio_input_paused;
        static uint8_t audio_input_ring_buffer_storage[2 * CVSD_SAMPLE_RATE]; // full second input buffer
        static btstack_ring_buffer_t audio_input_ring_buffer;

        static int dump_data;
        static int count_sent;
        static int count_received;
        static int negotiated_codec;

        static btstack_sbc_decoder_state_t decoder_state;
        static btstack_cvsd_plc_state_t cvsd_plc_state;

        constexpr static int MAX_NUM_MSBC_SAMPLES = (16 * 8);

        static int num_samples_to_write;
        static int num_audio_frames;
        static unsigned int phase;

        static void sineWave_Int16_At8000Hz_LittleEndian(unsigned int numSamples, uint8_t *data);
        static void sineWave_Int16_At16000Hz_HostEndian(unsigned int numSamples, int16_t *data);
        static void msbcFillSineAudioFrame();
        static void playbackCallback(int16_t *buffer, uint16_t numSamples);
        static void recordingCallback(const int16_t *buffer, uint16_t numSamples);
        static auto audioInitialize(int sampleRate) -> int;
        static void audioTerminate();
        static void handlePcmData(int16_t *data, int numSamples, int numChannels, int sampleRate, void *context);
        static void initMsbc();
        static void receiveMsbc(uint8_t *packet, uint16_t size);
        static void initCvsd();
        static void receiveCvsd(uint8_t *packet, uint16_t size);
    };

    SCO::SCO() : pimpl(std::make_unique<SCOImpl>(SCOImpl()))
    {}
    SCO::SCO(const SCO &other) : pimpl(new SCOImpl(*other.pimpl))
    {}
    auto SCO::operator=(SCO rhs) -> SCO &
    {
        swap(pimpl, rhs.pimpl);
        return *this;
    }
    void SCO::close()
    {
        pimpl->close();
    }
    void SCO::setCodec(uint8_t codec)
    {
        pimpl->setCodec(codec);
    }
    void SCO::init()
    {
        pimpl->init();
    }
    void SCO::send(hci_con_handle_t sco_handle)
    {
        pimpl->send(sco_handle);
    }
    void SCO::receive(uint8_t *packet, uint16_t size)
    {
        pimpl->receive(packet, size);
    }
    SCO::~SCO() = default;
} // namespace Bt

// #define ENABLE_SCO_STEREO_PLAYBACK

#define SCO_WAV_DURATION_IN_SECONDS 30
#define SCO_WAV_FILENAME            USER_PATH("sco_input.wav")

using namespace Bt;

int SCO::SCOImpl::audio_output_paused = 0;
uint8_t SCO::SCOImpl::audio_output_ring_buffer_storage[2 * SCO::SCOImpl::MSBC_PA_PREBUFFER_BYTES];
btstack_ring_buffer_t SCO::SCOImpl::audio_output_ring_buffer;
int SCO::SCOImpl::audio_input_paused = 0;
uint8_t SCO::SCOImpl::audio_input_ring_buffer_storage[2 * SCO::SCOImpl::CVSD_SAMPLE_RATE];
btstack_ring_buffer_t SCO::SCOImpl::audio_input_ring_buffer;

static const int16_t sine_int16_at_16000hz[] = {
    0,      3135,   6237,   9270,   12202,  14999,  17633,  20073,  22294,  24270,  25980,  27406,
    28531,  29344,  29835,  30000,  29835,  29344,  28531,  27406,  25980,  24270,  22294,  20073,
    17633,  14999,  12202,  9270,   6237,   3135,   0,      -3135,  -6237,  -9270,  -12202, -14999,
    -17633, -20073, -22294, -24270, -25980, -27406, -28531, -29344, -29835, -30000, -29835, -29344,
    -28531, -27406, -25980, -24270, -22294, -20073, -17633, -14999, -12202, -9270,  -6237,  -3135,
};

int SCO::SCOImpl::dump_data        = 1;
int SCO::SCOImpl::count_sent       = 0;
int SCO::SCOImpl::count_received   = 0;
int SCO::SCOImpl::negotiated_codec = -1;

btstack_sbc_decoder_state_t SCO::SCOImpl::decoder_state;
btstack_cvsd_plc_state_t SCO::SCOImpl::cvsd_plc_state;
unsigned int SCO::SCOImpl::phase;
int SCO::SCOImpl::num_samples_to_write;
int SCO::SCOImpl::num_audio_frames;

// 8 kHz samples for CVSD/SCO packets in little endian
void SCO::SCOImpl::sineWave_Int16_At8000Hz_LittleEndian(unsigned int numSamples, uint8_t *data)
{
    unsigned int i;
    for (i = 0; i < numSamples; i++) {
        int16_t sample = sine_int16_at_16000hz[phase];
        little_endian_store_16(data, i * 2, sample);
        // ony use every second sample from 16khz table to get 8khz
        phase += 2;
        if (phase >= (sizeof(sine_int16_at_16000hz) / sizeof(int16_t))) {
            phase = 0;
        }
    }
}

// 16 kHz samples for mSBC encoder in host endianess
void SCO::SCOImpl::sineWave_Int16_At16000Hz_HostEndian(unsigned int numSamples, int16_t *data)
{
    unsigned int i;
    for (i = 0; i < numSamples; i++) {
        data[i] = sine_int16_at_16000hz[phase++];
        if (phase >= (sizeof(sine_int16_at_16000hz) / sizeof(int16_t))) {
            phase = 0;
        }
    }
}

void SCO::SCOImpl::msbcFillSineAudioFrame()
{
    if (hfp_msbc_can_encode_audio_frame_now() == 0)
        return;
    int num_samples = hfp_msbc_num_audio_samples_per_frame();
    if (num_samples > MAX_NUM_MSBC_SAMPLES) {
        return;
    }
    int16_t sample_buffer[MAX_NUM_MSBC_SAMPLES];
    sineWave_Int16_At16000Hz_HostEndian(num_samples, sample_buffer);
    hfp_msbc_encode_audio_frame(sample_buffer);
    num_audio_frames++;
}

void SCO::SCOImpl::playbackCallback(int16_t *buffer, uint16_t numSamples)
{

    uint32_t prebuffer_bytes;
    switch (negotiated_codec) {
    case HFP_CODEC_MSBC:
        prebuffer_bytes = MSBC_PA_PREBUFFER_BYTES;
        break;
    case HFP_CODEC_CVSD:
    default:
        prebuffer_bytes = CVSD_PA_PREBUFFER_BYTES;
        break;
    }

    // fill with silence while paused
    if (audio_output_paused != 0) {
        if (btstack_ring_buffer_bytes_available(&audio_output_ring_buffer) < prebuffer_bytes) {
#ifdef ENABLE_SCO_STEREO_PLAYBACK
            memset(buffer, 0, num_samples * BYTES_PER_FRAME * 2);
#else
            memset(buffer, 0, numSamples * BYTES_PER_FRAME);
#endif
            return;
        }
        else {
            // resume playback
            audio_output_paused = 0;
        }
    }

    // get data from ringbuffer
    uint32_t bytes_read = 0;
#ifdef ENABLE_SCO_STEREO_PLAYBACK
    while (num_samples) {
        int16_t temp[16];
        unsigned int bytes_to_read = btstack_min(num_samples * BYTES_PER_FRAME, sizeof(temp));
        btstack_ring_buffer_read(&audio_output_ring_buffer, (uint8_t *)&temp[0], bytes_to_read, &bytes_read);
        if (bytes_read == 0)
            break;
        unsigned int i;
        for (i = 0; i < bytes_read / BYTES_PER_FRAME; i++) {
            *buffer++ = temp[i];
            *buffer++ = temp[i];
            num_samples--;
        }
    }
#else
    btstack_ring_buffer_read(&audio_output_ring_buffer, (uint8_t *)buffer, numSamples * BYTES_PER_FRAME, &bytes_read);
    numSamples -= bytes_read / BYTES_PER_FRAME;
    buffer += bytes_read / BYTES_PER_FRAME;
#endif

    // fill with 0 if not enough
    if (numSamples != 0u) {
#ifdef ENABLE_SCO_STEREO_PLAYBACK
        memset(buffer, 0, num_samples * BYTES_PER_FRAME * 2);
#else
        memset(buffer, 0, numSamples * BYTES_PER_FRAME);
#endif
        audio_output_paused = 1;
    }
}

void SCO::SCOImpl::recordingCallback(const int16_t *buffer, uint16_t numSamples)
{
    btstack_ring_buffer_write(&audio_input_ring_buffer, (uint8_t *)buffer, numSamples * 2);
}

// return 1 if ok
auto SCO::SCOImpl::audioInitialize(int sampleRate) -> int
{

    // -- output -- //

    // init buffers
    memset(audio_output_ring_buffer_storage, 0, sizeof(audio_output_ring_buffer_storage));
    btstack_ring_buffer_init(
        &audio_output_ring_buffer, audio_output_ring_buffer_storage, sizeof(audio_output_ring_buffer_storage));
    // config and setup audio playback
    const btstack_audio_sink_t *audio_sink = btstack_audio_sink_get_instance();
    if (audio_sink == nullptr) {
        return 0;
    }

#ifdef ENABLE_SCO_STEREO_PLAYBACK
    audio_sink->init(2, sampleRate, &playback_callback);
#else
    audio_sink->init(1, sampleRate, &playbackCallback);
#endif
    audio_sink->start_stream();

    audio_output_paused = 1;

    // -- input -- //

    // init buffers
    memset(audio_input_ring_buffer_storage, 0, sizeof(audio_input_ring_buffer_storage));
    btstack_ring_buffer_init(
        &audio_input_ring_buffer, audio_input_ring_buffer_storage, sizeof(audio_input_ring_buffer_storage));

    // config and setup audio recording
    const btstack_audio_source_t *audio_source = btstack_audio_source_get_instance();
    if (audio_source == nullptr) {
        return 0;
    }

    audio_source->init(1, sampleRate, &recordingCallback);
    audio_source->start_stream();

    audio_input_paused = 1;

    return 1;
}

void SCO::SCOImpl::audioTerminate()
{
    const btstack_audio_sink_t *audio_sink = btstack_audio_sink_get_instance();
    if (audio_sink == nullptr) {
        return;
    }
    audio_sink->close();

    const btstack_audio_source_t *audio_source = btstack_audio_source_get_instance();
    if (audio_source == nullptr) {
        return;
    }
    audio_source->close();
}

void SCO::SCOImpl::handlePcmData(int16_t *data, int numSamples, int numChannels, int sampleRate, void *context)
{
    UNUSED(context);
    UNUSED(sampleRate);
    UNUSED(data);
    UNUSED(numSamples);
    UNUSED(numChannels);

    printf("handle_pcm_data num samples %u, sample rate %d\n", numSamples, numChannels);

    // samples in callback in host endianess, ready for playback
    btstack_ring_buffer_write(&audio_output_ring_buffer, (uint8_t *)data, numSamples * numChannels * 2);

    if (num_samples_to_write == 0) {
        return;
    }
    numSamples = btstack_min(numSamples, num_samples_to_write);
    num_samples_to_write -= numSamples;
    wav_writer_write_int16(numSamples, data);
    if (num_samples_to_write == 0) {
        wav_writer_close();
    }

}

void SCO::SCOImpl::initMsbc(void)
{
    printf("SCO Demo: Init mSBC\n");

    btstack_sbc_decoder_init(&decoder_state, SBC_MODE_mSBC, &handlePcmData, nullptr);
    hfp_msbc_init();

    num_samples_to_write = MSBC_SAMPLE_RATE * SCO_WAV_DURATION_IN_SECONDS;
    wav_writer_open(SCO_WAV_FILENAME, 1, MSBC_SAMPLE_RATE);

    msbcFillSineAudioFrame();

    audioInitialize(MSBC_SAMPLE_RATE);
}

void SCO::SCOImpl::receiveMsbc(uint8_t *packet, uint16_t size)
{
    btstack_sbc_decoder_process_data(&decoder_state, (packet[1] >> 4) & 3, packet + 3, size - 3);
}

void SCO::SCOImpl::initCvsd()
{
    printf("SCO Demo: Init CVSD\n");

    btstack_cvsd_plc_init(&cvsd_plc_state);

    num_samples_to_write = CVSD_SAMPLE_RATE * SCO_WAV_DURATION_IN_SECONDS;
    wav_writer_open(SCO_WAV_FILENAME, 1, CVSD_SAMPLE_RATE);

    audioInitialize(CVSD_SAMPLE_RATE);
}

void SCO::SCOImpl::receiveCvsd(uint8_t *packet, uint16_t size)
{

    int16_t audio_frame_out[128]; //

    if (size > sizeof(audio_frame_out)) {
        printf("sco_demo_receive_CVSD: SCO packet larger than local output buffer - dropping data.\n");
        return;
    }

    const int audio_bytes_read = size - 3;
    const int num_samples      = audio_bytes_read / BYTES_PER_FRAME;

    // convert into host endian
    int16_t audio_frame_in[128];
    int i;
    for (i = 0; i < num_samples; i++) {
        audio_frame_in[i] = little_endian_read_16(packet, 3 + i * 2);
    }

    // treat packet as bad frame if controller does not report 'all good'
    bool bad_frame = (packet[1] & 0x30) != 0;

    btstack_cvsd_plc_process_data(&cvsd_plc_state, bad_frame, audio_frame_in, num_samples, audio_frame_out);

    // Samples in CVSD SCO packet are in little endian, ready for wav files (take shortcut)
    const int samples_to_write = btstack_min(num_samples, num_samples_to_write);
    wav_writer_write_le_int16(samples_to_write, audio_frame_out);
    num_samples_to_write -= samples_to_write;
    if (num_samples_to_write == 0) {
        wav_writer_close();
    }

    btstack_ring_buffer_write(&audio_output_ring_buffer, (uint8_t *)audio_frame_out, audio_bytes_read);
}

void SCO::SCOImpl::close()
{
    printf("SCO demo close\n");

    printf("SCO demo statistics: ");

    if (negotiated_codec == HFP_CODEC_MSBC) {
        printf("Used mSBC with PLC, number of processed frames: \n - %d good frames, \n - %d zero frames, \n - %d bad "
               "frames.\n",
               decoder_state.good_frames_nr,
               decoder_state.zero_frames_nr,
               decoder_state.bad_frames_nr);
    }
    else
    {
        printf("Used CVSD with PLC, number of proccesed frames: \n - %d good frames, \n - %d bad frames.\n",
               cvsd_plc_state.good_frames_nr,
               cvsd_plc_state.bad_frames_nr);
    }

    negotiated_codec = -1;
    wav_writer_close();
    audioTerminate();
}

void SCO::SCOImpl::setCodec(uint8_t codec)
{
    if (negotiated_codec == codec) {
        return;
    }
    negotiated_codec = codec;

    if (negotiated_codec == HFP_CODEC_MSBC) {
        initMsbc();
    }
    else {
        initCvsd();
    }
}

void SCO::SCOImpl::init(void)
{
    if (btstack_audio_sink_get_instance() != nullptr) {
        printf("SCO Demo: Sending sine wave, audio output via btstack_audio.\n");
    }
    else {
        printf("SCO Demo: Sending sine wave, hexdump received data.\n");
    }

    hci_set_sco_voice_setting(0x60); // linear, unsigned, 16-bit, CVSD
}

void SCO::SCOImpl::send(hci_con_handle_t sco_handle)
{

    if (sco_handle == HCI_CON_HANDLE_INVALID) {
        return;
    }

    int sco_packet_length  = hci_get_sco_packet_length();
    int sco_payload_length = sco_packet_length - 3;

    hci_reserve_packet_buffer();
    uint8_t *sco_packet = hci_get_outgoing_packet_buffer();

    if (negotiated_codec == HFP_CODEC_MSBC) {

        if (hfp_msbc_num_bytes_in_stream() < sco_payload_length) {
            log_error("mSBC stream is empty.");
        }
        hfp_msbc_read_from_stream(sco_packet + 3, sco_payload_length);
        msbcFillSineAudioFrame();
    }
    else
    {
        const int audio_samples_per_packet = sco_payload_length / BYTES_PER_FRAME;
        sineWave_Int16_At8000Hz_LittleEndian(audio_samples_per_packet, &sco_packet[3]);
    }

// microphone stuff
#if 0
    if (btstack_audio_source_get_instance()) {

        if (negotiated_codec == HFP_CODEC_MSBC) {
            // MSBC

            if (audio_input_paused) {
                if (btstack_ring_buffer_bytes_available(&audio_input_ring_buffer) >= MSBC_PA_PREBUFFER_BYTES) {
                    // resume sending
                    audio_input_paused = 0;
                }
            }

            if (!audio_input_paused) {
                int num_samples = hfp_msbc_num_audio_samples_per_frame();
                if (num_samples > MAX_NUM_MSBC_SAMPLES)
                    return; // assert
                if (hfp_msbc_can_encode_audio_frame_now() &&
                    btstack_ring_buffer_bytes_available(&audio_input_ring_buffer) >=
                        (unsigned int)(num_samples * BYTES_PER_FRAME)) {
                    int16_t sample_buffer[MAX_NUM_MSBC_SAMPLES];
                    uint32_t bytes_read;
                    btstack_ring_buffer_read(
                        &audio_input_ring_buffer, (uint8_t *)sample_buffer, num_samples * BYTES_PER_FRAME, &bytes_read);
                    hfp_msbc_encode_audio_frame(sample_buffer);
                    num_audio_frames++;
                }
                if (hfp_msbc_num_bytes_in_stream() < sco_payload_length) {
                    log_error("mSBC stream should not be empty.");
                }
            }

            if (audio_input_paused || hfp_msbc_num_bytes_in_stream() < sco_payload_length) {
                memset(sco_packet + 3, 0, sco_payload_length);
                audio_input_paused = 1;
            }
            else {
                hfp_msbc_read_from_stream(sco_packet + 3, sco_payload_length);
            }
        }
        else {
            // CVSD

            log_debug("send: bytes avail %u, free %u",
                      btstack_ring_buffer_bytes_available(&audio_input_ring_buffer),
                      btstack_ring_buffer_bytes_free(&audio_input_ring_buffer));
            // fill with silence while paused
            int bytes_to_copy = sco_payload_length;
            if (audio_input_paused) {
                if (btstack_ring_buffer_bytes_available(&audio_input_ring_buffer) >= CVSD_PA_PREBUFFER_BYTES) {
                    // resume sending
                    audio_input_paused = 0;
                }
            }

            // get data from ringbuffer
            uint16_t pos         = 0;
            uint8_t *sample_data = &sco_packet[3];
            if (!audio_input_paused) {
                uint32_t bytes_read = 0;
                btstack_ring_buffer_read(&audio_input_ring_buffer, sample_data, bytes_to_copy, &bytes_read);
                // flip 16 on big endian systems
                // @note We don't use (uint16_t *) casts since all sample addresses are odd which causes crahses on some
                // systems
                if (btstack_is_big_endian()) {
                    unsigned int i;
                    for (i = 0; i < bytes_read; i += 2) {
                        uint8_t tmp            = sample_data[i * 2];
                        sample_data[i * 2]     = sample_data[i * 2 + 1];
                        sample_data[i * 2 + 1] = tmp;
                    }
                }
                bytes_to_copy -= bytes_read;
                pos += bytes_read;
            }

            // fill with 0 if not enough
            if (bytes_to_copy) {
                memset(sample_data + pos, 0, bytes_to_copy);
                audio_input_paused = 1;
            }
        }
    }
    else {
        // just send '0's
        memset(sco_packet + 3, 0, sco_payload_length);
    }
#endif

    // set handle + flags
    little_endian_store_16(sco_packet, 0, sco_handle);
    // set len
    sco_packet[2] = sco_payload_length;
    // finally send packet
    hci_send_sco_packet_buffer(sco_packet_length);

    // request another send event
    hci_request_sco_can_send_now_event();
}

void SCO::SCOImpl::receive(uint8_t *packet, uint16_t size)
{
    dump_data = 1;
    count_received++;
    static uint32_t packets       = 0;
    static uint32_t crc_errors    = 0;
    static uint32_t data_received = 0;
    static uint32_t byte_errors   = 0;

    data_received += size - 3;
    packets++;
    if (data_received > 100000) {
        printf("Summary: data %07u, packets %04u, packet with crc errors %0u, byte errors %04u\n",
               (unsigned int)data_received,
               (unsigned int)packets,
               (unsigned int)crc_errors,
               (unsigned int)byte_errors);
        crc_errors    = 0;
        byte_errors   = 0;
        data_received = 0;
        packets       = 0;
    }

    switch (negotiated_codec) {
    case HFP_CODEC_MSBC:
        receiveMsbc(packet, size);
        break;

    case HFP_CODEC_CVSD:
        receiveCvsd(packet, size);
        break;
    default:
        break;
    }
    dump_data = 0;
}
