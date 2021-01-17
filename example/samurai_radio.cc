#include <vector>
#include <complex>
#include <iostream>

#include <samurai/samurai.hpp>

extern "C" {
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <liquid/liquid.h>
}

using namespace Samurai;

int main() {
    std::cout << "Hello from WASM C++." << std::endl;

    auto device = Airspy::Device();

    Device::Config deviceConfig{};
    deviceConfig.sampleRate = 2.5e6;
    device.Enable(deviceConfig);

    ChannelId rx;
    Channel::Config channelConfig{};
    channelConfig.mode = Mode::RX;
    channelConfig.dataFmt = Format::F32;
    ASSERT_SUCCESS(device.EnableChannel(channelConfig, &rx));

    Channel::State channelState{};
    channelState.enableAGC = true;
    channelState.frequency = 96.9e6;
    ASSERT_SUCCESS(device.UpdateChannel(rx, channelState));

    ALCdevice *dev = NULL;
    ALCcontext *ctx = NULL;

    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    std::cout << "Default device: " << defname << std::endl;

    dev = alcOpenDevice(defname);
    ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);


    /* Create buffer to store samples */
    ALuint buf;
    alGenBuffers(1, &buf);
    al_check_error();

    /* Fill buffer with Sine-Wave */
    float freq = 440.f;
    int seconds = 4;
    unsigned sample_rate = 22050;
    size_t buf_size = seconds * sample_rate;

    short *samples;
    samples = new short[buf_size];
    for(int i=0; i<buf_size; ++i) {
        samples[i] = 32760 * sin( (2.f*float(M_PI)*freq)/sample_rate * i );
    }

    /* Download buffer to OpenAL */
    alBufferData(buf, AL_FORMAT_MONO16, samples, buf_size, sample_rate);
    al_check_error();


    /* Set-up sound source and play buffer */
    ALuint src = 0;
    alGenSources(1, &src);
    alSourcei(src, AL_BUFFER, buf);
    alSourcePlay(src);

    /* While sound is playing, sleep */
    al_check_error();
    emscripten_sleep(1000*seconds);

    ALCdevice *dev = NULL;
    ALCcontext *ctx = NULL;
    ctx = alcGetCurrentContext();
    dev = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(dev);

    return 0;
}
