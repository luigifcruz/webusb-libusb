#include <vector>
#include <complex>
#include <iostream>

#include <samurai/samurai.hpp>

using namespace Samurai;

int main() {
    std::cout << "Hello from WASM C++." << std::endl;

    auto device = Airspy::Device();

    Device::Config deviceConfig{};
    deviceConfig.sampleRate = 10e6;
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

        size_t size = 2048;
        std::vector<std::complex<float>> data(size);
        ASSERT_SUCCESS(device.ReadStream(rx, data.data(), size, 1000));
        std::cout << data[1] << std::endl;

        ASSERT_SUCCESS(device.StopStream());
    }

    return 0;
}
