#include <vector>
#include <complex>
#include <iostream>
#include <fstream>

#include <SDL.h>
#include <fftw3.h>
#include <stdlib.h>
#include <audiocontext.h>
#include <liquid/liquid.h>
#include <samurai/samurai.hpp>
#include <emscripten/threading.h>

using namespace Samurai;

bool keepgoing = true;

SDL_Window *window = nullptr;
SDL_Renderer *renderer;
SDL_Surface *surface;

void draw_sdl(float* fft, int size) {
    if (!window) {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_CreateWindowAndRenderer(1920, 510, 0, &window, &renderer);
        surface = SDL_CreateRGBSurface(0, size, 255, 32, 0, 0, 0, 0);
    }

    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

    Uint8 * pixels = (uint8_t*)surface->pixels;

    float max = 0;
    for (int i=0; i < size; i++)
        if (fft[i] > max)
            max = fft[i];

    for (int i=0; i < size*255*4; i++) {
        pixels[i] = pixels[i]/1.25;
    }

    for (int i=0; i < size; i++) {
        int v = 255 - (uint8_t)(fft[i] * (255/max));
        pixels[(((size*v)+i)*4)+0] = 255;
        pixels[(((size*v)+i)*4)+1] = 255;
        pixels[(((size*v)+i)*4)+2] = 255;
        pixels[(((size*v)+i)*4)+3] = 255;
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

    SDL_Texture *screenTexture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(screenTexture);
}

int main() {
    std::cout << "Hello from WASM C++." << std::endl;

    //////// USER CONFIG
    float freq = 96.9e6;
    float samplerate = 256e3;
    //
    float demod_fs = 240e3;
    float output_fs = 48e3;
    size_t buffer_size = 1024 * 2;
    ////////

    auto device = AirspyHF::Device();

    Device::Config deviceConfig{};
    deviceConfig.sampleRate = samplerate;
    device.Enable(deviceConfig);

    ChannelId rx;
    Channel::Config channelConfig{};
    channelConfig.mode = Mode::RX;
    channelConfig.dataFmt = Format::F32;
    ASSERT_SUCCESS(device.EnableChannel(channelConfig, &rx));

    Channel::State channelState{};
    channelState.enableAGC = true;
    channelState.frequency = freq;
    ASSERT_SUCCESS(device.UpdateChannel(rx, channelState));

    // Copy I/Q samples from SDR.
    const size_t a_len = buffer_size;
    auto a_buf = (std::complex<float>*)malloc(sizeof(std::complex<float>) * a_len);

    // Downsample to intermediate samplerate.
    float brf = demod_fs / samplerate;
    msresamp_crcf b_resamp = msresamp_crcf_create(brf, 60.0);

    const size_t b_len = ceilf(a_len * brf);
    auto b_buf = (std::complex<float>*)malloc(sizeof(std::complex<float>) * b_len);

    // Apply automatic gain control.
    agc_crcf agc = agc_crcf_create();
    agc_crcf_set_bandwidth(agc, 1e-3f);

    // Apply frequency demodulation.
    freqdem dem = freqdem_create(100e3 / demod_fs);

    const size_t c_len = b_len;
    auto c_buf = (float*)malloc(sizeof(float) * c_len);

    // Downsample real into output.
    float orf = output_fs / demod_fs;
    msresamp_rrrf o_resamp = msresamp_rrrf_create(orf, 60.0);

    const size_t d_len = ceilf(c_len * orf);
    auto d_buf = (float*)malloc(sizeof(float) * d_len);

    // Create output buffer.
    audiocontext_config cfg{};
    cfg.sample_rate = output_fs;
    audiocontext_init(&cfg);

    const size_t e_len = 1;
    auto e_buf = (float**)calloc(1, sizeof(float**) * e_len);
    e_buf[0] = d_buf;

    // Plan FFT
    const size_t f_len = b_len;
    auto f_buf = (std::complex<float>*)malloc(sizeof(std::complex<float>) * f_len);
    fftwf_plan plan = fftwf_plan_dft_1d(f_len, reinterpret_cast<fftwf_complex*>(b_buf),
            reinterpret_cast<fftwf_complex*>(f_buf), FFTW_FORWARD, FFTW_ESTIMATE);
    const size_t g_len = f_len;
    auto g_buf = (float*)malloc(sizeof(float) * g_len);

    {
        ASSERT_SUCCESS(device.StartStream());

        int count = 0;
        while (keepgoing) {
            unsigned int len = a_len;

            ASSERT_SUCCESS(device.ReadStream(rx, a_buf, len, 1000));

            msresamp_crcf_execute(b_resamp, a_buf, len, b_buf, &len);

            if ((count++ % 4) == 0) {
                fftwf_execute(plan);
                for (int i = 0; i < f_len; ++i) {
                    if (i < f_len/2) {
                        int fi = i + int((f_len/2) + 0.5);
                        g_buf[i] = sqrt(f_buf[fi].real() * f_buf[fi].real() +
                                        f_buf[fi].imag() * f_buf[fi].imag());
                    }

                    if (i > f_len/2) {
                        int fi = i - int(f_len/2);
                        g_buf[i] = sqrt(f_buf[fi].real() * f_buf[fi].real() +
                                        f_buf[fi].imag() * f_buf[fi].imag());
                    }
                }

                emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VII, draw_sdl, g_buf, g_len);
            }

            for (size_t i = 0; i < len; i++)
                agc_crcf_execute(agc, b_buf[i], &b_buf[i]);

            freqdem_demodulate_block(dem, b_buf, len, c_buf);

            msresamp_rrrf_execute(o_resamp, c_buf, len, d_buf, &len);

            audiocontext_feed(e_buf, e_len, len, output_fs);
        }

        ASSERT_SUCCESS(device.StopStream());
    }

    msresamp_crcf_destroy(b_resamp);
    msresamp_rrrf_destroy(o_resamp);
    agc_crcf_destroy(agc);
    freqdem_destroy(dem);

    fftwf_destroy_plan(plan);

    free(a_buf);
    free(b_buf);
    free(c_buf);
    free(d_buf);
    free(e_buf);
    free(f_buf);
    free(g_buf);

    std::cout << "SAMURAI RADIO SUCCESSFUL" << std::endl;

    return 0;
}
