#include <iostream>

extern "C" {
#include <rtl-sdr.h>
}

#include "libusb.h"

int main() {
    std::cout << "Hello from WASM C++." << std::endl;

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

    std::cout << "SUCCESSFUL" << std::endl;

    auto r = rtlsdr_get_device_count();

    return 0;
}
