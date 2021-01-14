#include <iostream>

#include <rtl-sdr.h>
#include <emscripten.h>

int main() {
    std::cout << "Hello from WASM C++." << std::endl;

    static rtlsdr_dev_t  *device;
    if (rtlsdr_open(&device, 0) < 0) {
        std::cerr << "Error rtlsdr_open()." << std::endl;
        return 1;
    }

    std::cout << "AIRSPY SUCCESSFUL" << std::endl;

    return 0;
}
