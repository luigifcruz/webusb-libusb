#include <vector>
#include <complex>
#include <iostream>

#include <samurai/samurai.hpp>

extern "C" {
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



    {
        ASSERT_SUCCESS(device.StartStream());

        while (1) {
            size_t size = 2048;
            std::vector<std::complex<float>> data(size);
            ASSERT_SUCCESS(device.ReadStream(rx, data.data(), size, 1000));
        }

        ASSERT_SUCCESS(device.StopStream());
    }

    return 0;
}
