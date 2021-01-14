#include <iostream>

#include <libairspy/airspy.h>
#include <emscripten/bind.h>

//#define DJ_FFT_IMPLEMENTATION
#include "dj_fft.h"

int callback(airspy_transfer_t* transfer) {
    //std::cout << "Out Samples: " << transfer->sample_count << std::endl;

    size_t size = transfer->sample_count * 2;

    std::complex<float>* buf = (std::complex<float>*)malloc(size * sizeof(float));
    memcpy(buf, transfer->samples, size * sizeof(float));
    free(buf);

    std::cout << buf[0] << std::endl;

    return 0;
}

struct airspy_device *device;

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

    if ((r = airspy_start_rx(device, callback, NULL)) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_start_rx(): " << r << std::endl;
        return 1;
    }

    return 0;
}

void stop() {
    if (airspy_stop_rx(device) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_stop_rx()."<< std::endl;
        return;
    }

    std::cout << "AIRSPY SUCCESSFUL" << std::endl;
}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("stop", &stop);
}
