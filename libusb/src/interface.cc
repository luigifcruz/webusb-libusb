#include <iostream>

#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>

#include "libusb.h"
#include "interface.h"

#define DEBUG_TRACE

//
// Helper functions.
//

static _Atomic bool started = false;

void *WUSBThread(void*) {
#ifdef DEBUG_TRACE
    std::cout << "WebUSB Thread Started (" << pthread_self() << ")" << std::endl;
#endif
    started = true;
    emscripten_exit_with_live_runtime();
    return NULL;
}

device_context* dc(libusb_device* dev) {
    return (device_context*)dev;
}

webusb_context* wc(libusb_context* ctx) {
    return (webusb_context*)ctx;
}

webusb_context* wc(libusb_device* dev) {
    return dc(dev)->ctx;
}

webusb_context* wc(libusb_device_handle* dev_handle) {
    return (webusb_context*)dev_handle;
}

//
// Proxied methods.
//

int libusb_init(libusb_context** ctx) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    auto _ctx = (webusb_context*)calloc(1, sizeof(webusb_context));

    if (pthread_create(&_ctx->worker, NULL, WUSBThread, nullptr) != 0)
        return LIBUSB_ERROR_NOT_SUPPORTED;

    while (!started)
        emscripten_sleep(100);

    *ctx = (libusb_context*)_ctx;

    return emscripten_dispatch_to_thread_sync(_ctx->worker, EM_FUNC_SIG_II, _libusb_init, nullptr,
            ctx);
}

void libusb_exit(libusb_context *ctx) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_dispatch_to_thread_sync(wc(ctx)->worker, EM_FUNC_SIG_VI, _libusb_exit, nullptr,
            ctx);
    free(ctx);
}

ssize_t libusb_get_device_list(libusb_context *ctx, libusb_device ***list) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    libusb_device** _list;

    size_t n = emscripten_dispatch_to_thread_sync(wc(ctx)->worker, EM_FUNC_SIG_III, _libusb_get_device_list, nullptr,
            ctx, &_list);

    libusb_device** l = (libusb_device**)malloc(sizeof(libusb_device*) * (n + 2));

    for (int i = 0; i < n; i++) {
        auto dc = (device_context*)malloc(sizeof(device_context));
        dc->ctx = wc(ctx);
        dc->idev = _list[i];
        l[i] = (libusb_device*)dc;
    }

    free(_list);

    l[n] = nullptr;
    l[n+1] = (libusb_device*)ctx;

    *list = l;

    return n;
}

void libusb_free_device_list(libusb_device **list, int unref_devices) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    pthread_t thread;

    int n = 0;
    for (int i = 0; i < 100; i++) {
        if (list[i] == nullptr) {
            thread = ((webusb_context*)list[i+1])->worker;
            break;
        }
        n += 1;
    }

    libusb_device** _list = (libusb_device**)malloc(sizeof(libusb_device*) * n + 1);

    for (int i = 0; i < n; i++) {
        _list[i] = dc(list[i])->idev;
    }

    _list[n] = nullptr;

    free(list);

    emscripten_dispatch_to_thread_sync(thread, EM_FUNC_SIG_VII, _libusb_free_device_list, nullptr,
            _list, unref_devices);
}

int libusb_get_device_descriptor(libusb_device *dev, struct libusb_device_descriptor *desc) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(dev)->worker, EM_FUNC_SIG_III, _libusb_get_device_descriptor, nullptr,
            dc(dev)->idev, desc);
}

int LIBUSB_CALL libusb_open(libusb_device *dev, libusb_device_handle **dev_handle) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    *dev_handle = (libusb_device_handle*)wc(dev);
    return emscripten_dispatch_to_thread_sync(wc(dev)->worker, EM_FUNC_SIG_III, _libusb_open, nullptr,
            dc(dev)->idev, nullptr);
}

void LIBUSB_CALL libusb_close(libusb_device_handle *dev_handle) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_dispatch_to_thread_sync(wc(dev_handle)->worker, EM_FUNC_SIG_VI, _libusb_close, nullptr,
            nullptr);
}

int LIBUSB_CALL libusb_get_string_descriptor_ascii(libusb_device_handle *dev_handle,
    uint8_t desc_index, unsigned char *data, int length) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(dev_handle)->worker, EM_FUNC_SIG_IIIII, _libusb_get_string_descriptor_ascii, nullptr,
            nullptr, desc_index, data, length);
}

int LIBUSB_CALL libusb_set_configuration(libusb_device_handle *dev_handle, int configuration) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(dev_handle)->worker, EM_FUNC_SIG_III, _libusb_set_configuration, nullptr,
            nullptr, configuration);
}

int LIBUSB_CALL libusb_claim_interface(libusb_device_handle *dev_handle, int interface_number) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(dev_handle)->worker, EM_FUNC_SIG_III, _libusb_claim_interface, nullptr,
            nullptr, interface_number);
}

int LIBUSB_CALL libusb_release_interface(libusb_device_handle *dev_handle, int interface_number) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(dev_handle)->worker, EM_FUNC_SIG_III, _libusb_release_interface, nullptr,
            nullptr, interface_number);
}

int LIBUSB_CALL libusb_control_transfer(libusb_device_handle *dev_handle,
    uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
    unsigned char *data, uint16_t wLength, unsigned int timeout) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(dev_handle)->worker, EM_FUNC_SIG_IIIIIIIII, _libusb_control_transfer, nullptr,
            nullptr, request_type, bRequest, wValue, wIndex, data, wLength, timeout);
}

int LIBUSB_CALL libusb_clear_halt(libusb_device_handle *dev_handle, unsigned char endpoint) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(dev_handle)->worker, EM_FUNC_SIG_III, _libusb_clear_halt, nullptr,
            nullptr, endpoint);
}

int LIBUSB_CALL libusb_submit_transfer(struct libusb_transfer *transfer) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(transfer->dev_handle)->worker, EM_FUNC_SIG_II, _libusb_submit_transfer, nullptr,
            transfer);
}

int LIBUSB_CALL libusb_reset_device(libusb_device_handle *dev_handle) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(dev_handle)->worker, EM_FUNC_SIG_II, _libusb_reset_device, nullptr,
            nullptr);
}

int LIBUSB_CALL libusb_kernel_driver_active(libusb_device_handle *dev_handle, int interface_number) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(dev_handle)->worker, EM_FUNC_SIG_III, _libusb_kernel_driver_active, nullptr,
            nullptr, interface_number);
}

int LIBUSB_CALL libusb_cancel_transfer(struct libusb_transfer *transfer) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(transfer->dev_handle)->worker, EM_FUNC_SIG_II, _libusb_cancel_transfer, nullptr,
            transfer);
}

void LIBUSB_CALL libusb_free_transfer(struct libusb_transfer *transfer) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_dispatch_to_thread_sync(wc(transfer->dev_handle)->worker, EM_FUNC_SIG_VI, _libusb_free_transfer, nullptr,
            transfer);
}

int LIBUSB_CALL libusb_handle_events_timeout(libusb_context *ctx, struct timeval *tv) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(ctx)->worker, EM_FUNC_SIG_III, _libusb_handle_events_timeout, nullptr,
            ctx, tv);
}

int LIBUSB_CALL libusb_handle_events_timeout_completed(libusb_context *ctx,
	struct timeval *tv, int *completed) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_dispatch_to_thread_sync(wc(ctx)->worker, EM_FUNC_SIG_IIII, _libusb_handle_events_timeout_completed, nullptr,
            ctx, tv, completed);
}

//
// Not Proxied
//

const struct libusb_version* libusb_get_version(void) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return _libusb_get_version();
};

struct libusb_transfer * LIBUSB_CALL libusb_alloc_transfer(int iso_packets) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return _libusb_alloc_transfer(iso_packets);
}
