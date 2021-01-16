#include <vector>
#include <complex>
#include <iostream>

#include <libairspy/airspy.h>
#include <emscripten.h>

struct airspy_device *device;

int callback(airspy_transfer_t* transfer) {
    size_t size = transfer->sample_count;
    auto buf = static_cast<std::complex<float>*>(transfer->samples);
    std::vector<std::complex<float>> data(buf, buf + size);

    std::cout << "Data Size: " <<  data.size() << " | First Sample: " << data[0] << std::endl;

    return 0;
}

int main() {
    int r = AIRSPY_SUCCESS;
    std::cout << "Hello from WASM C++." << std::endl;

    if (airspy_open(&device) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_open()." << std::endl;
        return 1;
    }

    if (airspy_set_samplerate(device, 2.5e6) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_set_samplerate()." << std::endl;
        return 1;
    }

    if (airspy_set_lna_agc(device, 1) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_set_lna_agc()." << std::endl;
        return 1;
    }

    if (airspy_set_mixer_agc(device, 1) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_set_mixer_agc()." << std::endl;
        return 1;
    }

    if (airspy_set_freq(device, 96900000) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_set_freq()." << std::endl;
        return 1;
    }

    if (airspy_set_sample_type(device, AIRSPY_SAMPLE_FLOAT32_IQ) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_set_sample_type()." << std::endl;
        return 1;
    }

    if ((r = airspy_start_rx(device, callback, nullptr)) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_start_rx(): " << r << std::endl;
        return 1;
    }

    emscripten_sleep(1000*5);

    std::cout << "release" << std::endl;

    if (airspy_stop_rx(device) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_stop_rx()."<< std::endl;
        return 1;
    }

    airspy_close(device);

    std::cout << "AIRSPY SUCCESSFUL" << std::endl;

    return 0;
}
