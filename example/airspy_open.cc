#include <iostream>

#include <libairspy/airspy.h>
#include <emscripten.h>

int callback(airspy_transfer_t* transfer) {
    std::cout << "Out Samples: " << transfer->sample_count << std::endl;
    return 0;
}

int main() {
    int r = AIRSPY_SUCCESS;
    std::cout << "Hello from WASM C++." << std::endl;

    struct airspy_device *device;
    if (airspy_open(&device) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_open()." << std::endl;
        return 1;
    }

    if (airspy_set_samplerate(device, 10e6) != AIRSPY_SUCCESS) {
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

    if ((r = airspy_start_rx(device, callback, NULL)) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_start_rx(): " << r << std::endl;
        return 1;
    }

    emscripten_sleep(10000);

    std::cout << "AIRSPY SUCCESSFUL" << std::endl;

    return 0;
}
