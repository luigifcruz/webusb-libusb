#include <complex>
#include <iostream>

#include <pthread.h>

#include <audiocontext.h>
#include <liquid/liquid.h>
#include <samurai/samurai.hpp>

#include <emscripten.h>
#include <emscripten/threading.h>
#include <emscripten/bind.h>

//#define DEBUG_TRACE

using namespace Samurai;
using namespace emscripten;

typedef struct {
    AirspyHF::Device device;
    ChannelId rx;
    audiocontext_config cfg{};
    msresamp_rrrf o_resamp;
    agc_crcf agc;
    freqdem dem;
    msresamp_crcf b_resamp;

    size_t a_len, b_len, c_len, d_len, e_len;

    std::complex<float> *a_buf, *b_buf;
    float *c_buf, *d_buf, **e_buf;
} core_ctx;

pthread_t core_thread;
core_ctx ctx;

//////// CONFIG
static _Atomic float samplerate = 256e3;
static _Atomic float demod_fs = 240e3;
static _Atomic float output_fs = 48e3;
static _Atomic size_t buffer_size = 1024 * 8;
static _Atomic float volume = 1.0;
static _Atomic float frequency = 96.9e6;
//////// SIGNALS
static _Atomic bool core = false;
static _Atomic bool safed = true;
static _Atomic bool started = false;
static _Atomic bool keepgoing = true;
static _Atomic bool update = false;
////////

void _open_device() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    if (started)
        return;

    ctx.device = AirspyHF::Device();

    Device::Config deviceConfig{};
    deviceConfig.sampleRate = samplerate;
    ctx.device.Enable(deviceConfig);

    Channel::Config channelConfig{};
    channelConfig.mode = Mode::RX;
    channelConfig.dataFmt = Format::F32;
    ASSERT_SUCCESS(ctx.device.EnableChannel(channelConfig, &ctx.rx));

    // Copy I/Q samples from SDR.
    ctx.a_len = buffer_size;
    ctx.a_buf = (std::complex<float>*)malloc(sizeof(std::complex<float>) * ctx.a_len);

    // Downsample to intermediate samplerate.
    float brf = demod_fs / samplerate;
    ctx.b_resamp = msresamp_crcf_create(brf, 60.0);

    ctx.b_len = ceilf(ctx.a_len * brf);
    ctx.b_buf = (std::complex<float>*)malloc(sizeof(std::complex<float>) * ctx.b_len);

    // Apply automatic gain control.
    ctx.agc = agc_crcf_create();
    agc_crcf_set_bandwidth(ctx.agc, 1e-3f);

    // Apply frequency demodulation.
    ctx.dem = freqdem_create(85e3 / demod_fs);

    ctx.c_len = ctx.b_len;
    ctx.c_buf = (float*)malloc(sizeof(float) * ctx.c_len);

    // Downsample real into output.
    float orf = output_fs / demod_fs;
    ctx.o_resamp = msresamp_rrrf_create(orf, 60.0);

    ctx.d_len = ceilf(ctx.c_len * orf);
    ctx.d_buf = (float*)malloc(sizeof(float) * ctx.d_len);

    // Create output buffer.
    ctx.cfg.sample_rate = output_fs;
    audiocontext_init(&ctx.cfg);

    ctx.e_len = 1;
    ctx.e_buf = (float**)calloc(1, sizeof(float**) * ctx.e_len);
    ctx.e_buf[0] = ctx.d_buf;

    started = true;
}

void _update_device() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    while (!started)
        emscripten_sleep(250);

    Channel::State channelState{};
    channelState.enableAGC = true;
    channelState.frequency = frequency;
    ASSERT_SUCCESS(ctx.device.UpdateChannel(ctx.rx, channelState));
}

void _start_stream() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    if (!safed)
        return;

    while (!started)
        emscripten_sleep(250);

    safed = false;
    {
        _update_device();
        ASSERT_SUCCESS(ctx.device.StartStream());

        while (keepgoing) {
            if (update) {
                _update_device();
                update = false;
            }

            unsigned int len = ctx.a_len;

            ASSERT_SUCCESS(ctx.device.ReadStream(ctx.rx, ctx.a_buf, len, 1000));

            msresamp_crcf_execute(ctx.b_resamp, ctx.a_buf, len, ctx.b_buf, &len);

            for (size_t i = 0; i < len; i++)
                agc_crcf_execute(ctx.agc, ctx.b_buf[i], &ctx.b_buf[i]);

            freqdem_demodulate_block(ctx.dem, ctx.b_buf, len, ctx.c_buf);

            msresamp_rrrf_execute(ctx.o_resamp, ctx.c_buf, len, ctx.d_buf, &len);

            for (int i=0; i < len; i++)
                ctx.d_buf[i] *= volume;

            audiocontext_feed(ctx.e_buf, ctx.e_len, len, output_fs);
        }

        ASSERT_SUCCESS(ctx.device.StopStream());
    }
    safed = true;
}

void _close_device() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    while (!safed)
        emscripten_sleep(100);

    started = false;

    msresamp_crcf_destroy(ctx.b_resamp);
    msresamp_rrrf_destroy(ctx.o_resamp);
    agc_crcf_destroy(ctx.agc);
    freqdem_destroy(ctx.dem);

    free(ctx.a_buf);
    free(ctx.b_buf);
    free(ctx.c_buf);
    free(ctx.d_buf);
    free(ctx.e_buf);
}

void* core_main(void*) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    std::cout << "CyberRadio WASM based DSP core started." << std::endl;
    core = true;
    emscripten_exit_with_live_runtime();
    return NULL;
}

int main() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    std::cout << "CyberRadio WASM interface started." << std::endl;
    emscripten_exit_with_live_runtime();
}

//
// Interface
//

int start_core() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    if (pthread_create(&core_thread, NULL, core_main, nullptr) != 0)
        return 1;

    while (!core)
        emscripten_sleep(100);

    return 0;
}

void open_device() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_dispatch_to_thread_async(core_thread, EM_FUNC_SIG_V, _open_device, nullptr);
}

void start_stream() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    keepgoing = true;
    emscripten_dispatch_to_thread_async(core_thread, EM_FUNC_SIG_V, _start_stream, nullptr);
}

void stop_stream() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    keepgoing = false;
}

void close_device() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    stop_stream();
    emscripten_dispatch_to_thread_async(core_thread, EM_FUNC_SIG_V, _close_device, nullptr);
}

void update_demod(float vol) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    volume = vol;
}

void update_device(float freq) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    frequency = freq;
    update = true;
}

//
// Bindings
//

EMSCRIPTEN_BINDINGS(wasm) {
    function("start_core", &start_core);
    function("open_device", &open_device);
    function("close_device", &close_device);
    function("start_stream", &start_stream);
    function("stop_stream", &stop_stream);
    function("update_demod", &update_demod);
    function("update_device", &update_device);
}
