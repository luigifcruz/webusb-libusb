#include <vector>
#include <complex>
#include <iostream>
#include <chrono>
#include <ctime>

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

        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < 100; i++) {
            size_t size = 2048;
            std::vector<std::complex<float>> data(size);
            ASSERT_SUCCESS(device.ReadStream(rx, data.data(), size, 1000));
        }
        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_seconds = end-start;
        std::cout << "Finished 100 cycles in " << elapsed_seconds.count() << "s\n";

        ASSERT_SUCCESS(device.StopStream());
    }

    return 0;
}
