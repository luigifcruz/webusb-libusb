#include <iostream>

extern "C" {
#include <libairspy/airspy.h>
}

#include "libusb.h"

#define OPEN_AIRSPY

int main() {
    std::cout << "Hello from WASM C++." << std::endl;

#ifdef TEST_LIBUSB
    const libusb_version* ver = libusb_get_version();
    std::cout
        << "LibUSB Version: "
        << ver->major << "." << ver->minor << "." << ver->micro
        << std::endl;

    libusb_context *ctx;
    if (libusb_init(&ctx) < 0) {
        std::cerr << "Error libusb_init()." << std::endl;
        return 1;
    }

    libusb_device **list;
    int cnt = libusb_get_device_list(ctx, &list);
    if (cnt < 0) {
        std::cerr << "Error libusb_get_device_list()." << std::endl;
        return 1;
    }

    struct libusb_device_descriptor dd;
    for (int i = 0; i < cnt; i++) {
		int res = libusb_get_device_descriptor(list[i], &dd);
        if (res < 0) {
            std::cerr << "Error libusb_get_device_descriptor()." << std::endl;
            return 1;
        }
    }

    libusb_free_device_list(list, 1);

    libusb_exit(ctx);

    std::cout << "LIBUSB SUCCESSFUL" << std::endl;
#endif

#ifdef TEST_AIRSPY
    uint64_t serials[8];
    int count = airspy_list_devices(serials, 8);
    if (count < 0) {
        std::cerr << "Error airspy_list_devices()." << std::endl;
        return 1;
    }

    std::cout << "Airspy Device Found: " << count << std::endl;

    std::cout << "AIRSPY SUCCESSFUL" << std::endl;
#endif

#ifdef OPEN_AIRSPY
    struct airspy_device *device;
    if (airspy_open(&device) != AIRSPY_SUCCESS) {
        std::cerr << "Error airspy_open()." << std::endl;
        return 1;
    }
#endif

    return 0;
}
