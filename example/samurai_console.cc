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

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>
#include <GL/gl.h>
#include <SDL_opengl.h>

using namespace Samurai;

bool keepgoing = true;

SDL_Window *window = nullptr;
GLuint tex, lut;
GLint loc, loc2;

unsigned char turbo_srgb_bytes[256][3] = {{48,18,59},{50,21,67},{51,24,74},{52,27,81},{53,30,88},{54,33,95},{55,36,102},{56,39,109},{57,42,115},{58,45,121},{59,47,128},{60,50,134},{61,53,139},{62,56,145},{63,59,151},{63,62,156},{64,64,162},{65,67,167},{65,70,172},{66,73,177},{66,75,181},{67,78,186},{68,81,191},{68,84,195},{68,86,199},{69,89,203},{69,92,207},{69,94,211},{70,97,214},{70,100,218},{70,102,221},{70,105,224},{70,107,227},{71,110,230},{71,113,233},{71,115,235},{71,118,238},{71,120,240},{71,123,242},{70,125,244},{70,128,246},{70,130,248},{70,133,250},{70,135,251},{69,138,252},{69,140,253},{68,143,254},{67,145,254},{66,148,255},{65,150,255},{64,153,255},{62,155,254},{61,158,254},{59,160,253},{58,163,252},{56,165,251},{55,168,250},{53,171,248},{51,173,247},{49,175,245},{47,178,244},{46,180,242},{44,183,240},{42,185,238},{40,188,235},{39,190,233},{37,192,231},{35,195,228},{34,197,226},{32,199,223},{31,201,221},{30,203,218},{28,205,216},{27,208,213},{26,210,210},{26,212,208},{25,213,205},{24,215,202},{24,217,200},{24,219,197},{24,221,194},{24,222,192},{24,224,189},{25,226,187},{25,227,185},{26,228,182},{28,230,180},{29,231,178},{31,233,175},{32,234,172},{34,235,170},{37,236,167},{39,238,164},{42,239,161},{44,240,158},{47,241,155},{50,242,152},{53,243,148},{56,244,145},{60,245,142},{63,246,138},{67,247,135},{70,248,132},{74,248,128},{78,249,125},{82,250,122},{85,250,118},{89,251,115},{93,252,111},{97,252,108},{101,253,105},{105,253,102},{109,254,98},{113,254,95},{117,254,92},{121,254,89},{125,255,86},{128,255,83},{132,255,81},{136,255,78},{139,255,75},{143,255,73},{146,255,71},{150,254,68},{153,254,66},{156,254,64},{159,253,63},{161,253,61},{164,252,60},{167,252,58},{169,251,57},{172,251,56},{175,250,55},{177,249,54},{180,248,54},{183,247,53},{185,246,53},{188,245,52},{190,244,52},{193,243,52},{195,241,52},{198,240,52},{200,239,52},{203,237,52},{205,236,52},{208,234,52},{210,233,53},{212,231,53},{215,229,53},{217,228,54},{219,226,54},{221,224,55},{223,223,55},{225,221,55},{227,219,56},{229,217,56},{231,215,57},{233,213,57},{235,211,57},{236,209,58},{238,207,58},{239,205,58},{241,203,58},{242,201,58},{244,199,58},{245,197,58},{246,195,58},{247,193,58},{248,190,57},{249,188,57},{250,186,57},{251,184,56},{251,182,55},{252,179,54},{252,177,54},{253,174,53},{253,172,52},{254,169,51},{254,167,50},{254,164,49},{254,161,48},{254,158,47},{254,155,45},{254,153,44},{254,150,43},{254,147,42},{254,144,41},{253,141,39},{253,138,38},{252,135,37},{252,132,35},{251,129,34},{251,126,33},{250,123,31},{249,120,30},{249,117,29},{248,114,28},{247,111,26},{246,108,25},{245,105,24},{244,102,23},{243,99,21},{242,96,20},{241,93,19},{240,91,18},{239,88,17},{237,85,16},{236,83,15},{235,80,14},{234,78,13},{232,75,12},{231,73,12},{229,71,11},{228,69,10},{226,67,10},{225,65,9},{223,63,8},{221,61,8},{220,59,7},{218,57,7},{216,55,6},{214,53,6},{212,51,5},{210,49,5},{208,47,5},{206,45,4},{204,43,4},{202,42,4},{200,40,3},{197,38,3},{195,37,3},{193,35,2},{190,33,2},{188,32,2},{185,30,2},{183,29,2},{180,27,1},{178,26,1},{175,24,1},{172,23,1},{169,22,1},{167,20,1},{164,19,1},{161,18,1},{158,16,1},{155,15,1},{152,14,1},{149,13,1},{146,11,1},{142,10,1},{139,9,2},{136,8,2},{133,7,2},{129,6,2},{126,5,2},{122,4,3}};

// Shader sources
const GLchar* vertexSource = R"END(#version 300 es
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    layout (location = 2) in vec2 aSpecTexCoord;

    out vec2 TexCoord;
    out vec2 SpecTexCoord;

    void main() {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = vec2(aTexCoord.x, aTexCoord.y);
        SpecTexCoord = vec2(aSpecTexCoord.x, aSpecTexCoord.y);
    }
)END";

const GLchar* fragmentSource = R"END(#version 300 es
    precision highp float;

    in vec2 TexCoord;
    in vec2 SpecTexCoord;
    out vec4 FragColor;

    uniform int view;
    uniform float idx;
    uniform sampler2D lut;
    uniform sampler2D texture1;

    void waterfall() {
        if (TexCoord.y >= 0.995) {
            FragColor = vec4(0,0,0,0);
            return;
        }

        float y = idx - TexCoord.y;

        if (y < 0.0) {
            y = 1.0 - abs(y);
        }

        float mag = texture(texture1, vec2(TexCoord.x, y)).r;

        FragColor = texture(lut, vec2(mag, 0));
    }

    void spectogram() {
        float hits = 0.0;
        for (int i=0; i < 300; i++) {
            float y = float(i) / 300.0;
            float val = texture(texture1, vec2(SpecTexCoord.x, y)).r;
            float tar = SpecTexCoord.y;
            if (val >= (tar - 0.01) && val <= (tar + 0.01)) {
                hits += 2.0 * ((300.0-float(i))/300.0);
            }
        }
        FragColor = texture(lut, vec2(hits / 300.0, 0));
    }

    void main() {
        if (view == 1)
            spectogram();
        else if (view == 2)
            waterfall();
    }
)END";

void draw_sdl(uint8_t* fft, int width, int height, int idx) {
    if (!window) {
        window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                1280, 500, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetSwapInterval(0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        auto glc = SDL_GL_CreateContext(window);

        auto rdr = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

        glEnable(GL_MULTISAMPLE);

        // Create a Vertex Buffer Object and copy the vertex data to it
        GLuint vbo;
        glGenBuffers(1, &vbo);

        GLuint ebo;
        glGenBuffers(1, &ebo);

        float vertices[] = {
            // positions          // texture coords
             1.0f,  1.0f, 0.0f,   0.0f, 0.0f,   1.0f, 1.0f, // top right
             1.0f, -1.0f, 0.0f,   0.0f, 1.0f,   1.0f, 0.0f, // bottom right
            -1.0f, -1.0f, 0.0f,   1.0f, 1.0f,   0.0f, 0.0f, // bottom left
            -1.0f,  1.0f, 0.0f,   1.0f, 0.0f,   0.0f, 1.0f, // top left
        };

        GLuint elements[] = {
            0, 1, 2,
            2, 3, 0
        };

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

        // Create and compile the vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);

        // Create and compile the fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);

        // Link the vertex and fragment shader into a shader program
        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);

        // Specify the layout of the vertex data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // Add LUT
        glGenTextures(1, &lut);
        glBindTexture(GL_TEXTURE_2D, lut);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)&turbo_srgb_bytes);

        glUniform1i(glGetUniformLocation(shaderProgram, "lut"), 0);

        // Add texture
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 1);

        loc = glGetUniformLocation(shaderProgram, "idx");
        loc2 = glGetUniformLocation(shaderProgram, "view");
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, fft);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 300, 1280, 200);

    glUniform1i(loc2, 1);
    glUniform1f(loc, float(idx)/float(height));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lut);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glViewport(0, 0, 1280, 300);

    glUniform1i(loc2, 2);
    glUniform1f(loc, float(idx)/float(height));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lut);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(window);
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

    const size_t h_hei = 300;
    const size_t h_wid = g_len;
    const size_t h_len = h_wid * h_hei;
    auto h_buf = (uint8_t*)malloc(sizeof(uint8_t) * h_len);

    {
        ASSERT_SUCCESS(device.StartStream());

        int count = 0;
        int line = 0;
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

                float max = 0;
                for (int i=0; i < g_len; i++)
                    if (g_buf[i] > max)
                        max = g_buf[i];

                for (int i=0; i < h_wid; i++)
                    h_buf[(line*h_wid)+i] = (uint8_t)(g_buf[i] * (255/max));

                emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIIII, draw_sdl, h_buf, h_wid, h_hei, line);

                if (++line > h_hei)
                    line = 0;
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
    free(h_buf);

    std::cout << "SAMURAI CoNSOLE SUCCESSFUL" << std::endl;

    return 0;
}
