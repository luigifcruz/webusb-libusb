#include <iostream>

extern "C" {
#include "libusb.h"
#include <libairspy/airspy.h>
}

int main() {
    std::cout << "Hello from WASM C++." << std::endl;

    uint64_t serials[8];
    int count = airspy_list_devices(serials, 8);
    if (count < 0) {
        std::cerr << "Error airspy_list_devices()." << std::endl;
        return 1;
    }

    std::cout << "Airspy Device Found: " << count << std::endl;

    std::cout << "AIRSPY SUCCESSFUL" << std::endl;

    return 0;
}