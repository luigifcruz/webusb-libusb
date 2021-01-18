#include <iostream>

#include <emscripten/threading.h>

#include "libusb.h"
#include "interface.h"

const struct libusb_version* libusb_get_version(void) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return _libusb_get_version();
};

int libusb_init(libusb_context** ctx) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_II, _libusb_init,
            ctx);
};

ssize_t libusb_get_device_list(libusb_context *ctx, libusb_device ***list) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_III, _libusb_get_device_list,
            ctx, list);
}

int libusb_get_device_descriptor(libusb_device *dev, struct libusb_device_descriptor *desc) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_III, _libusb_get_device_descriptor,
            dev, desc);
}

void libusb_free_device_list(libusb_device **list, int unref_devices) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VII, _libusb_free_device_list,
            list, unref_devices);
}

int LIBUSB_CALL libusb_open(libusb_device *dev, libusb_device_handle **dev_handle) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_III, _libusb_open,
            dev, dev_handle);
}

void LIBUSB_CALL libusb_close(libusb_device_handle *dev_handle) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_I, _libusb_close,
            dev_handle);
}

int LIBUSB_CALL libusb_get_string_descriptor_ascii(libusb_device_handle *dev_handle,
    uint8_t desc_index, unsigned char *data, int length) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_IIIII, _libusb_get_string_descriptor_ascii,
            dev_handle, desc_index, data, length);
}

int LIBUSB_CALL libusb_set_configuration(libusb_device_handle *dev_handle, int configuration) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_III, _libusb_set_configuration,
            dev_handle, configuration);
}

int LIBUSB_CALL libusb_claim_interface(libusb_device_handle *dev_handle, int interface_number) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_III, _libusb_claim_interface,
            dev_handle, interface_number);
}

int LIBUSB_CALL libusb_release_interface(libusb_device_handle *dev_handle, int interface_number) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_III, _libusb_release_interface,
            dev_handle, interface_number);
}

int LIBUSB_CALL libusb_control_transfer(libusb_device_handle *dev_handle,
    uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
    unsigned char *data, uint16_t wLength, unsigned int timeout) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_IIIIIIIII, _libusb_control_transfer,
            dev_handle, request_type, bRequest, wValue, wIndex, data, wLength, timeout);
}

struct libusb_transfer * LIBUSB_CALL libusb_alloc_transfer(int iso_packets) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return _libusb_alloc_transfer(iso_packets);
}

int LIBUSB_CALL libusb_clear_halt(libusb_device_handle *dev_handle, unsigned char endpoint) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_III, _libusb_clear_halt,
            dev_handle, endpoint);
}

int LIBUSB_CALL libusb_submit_transfer(struct libusb_transfer *transfer) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_II, _libusb_submit_transfer,
            transfer);
}

void libusb_exit(libusb_context *ctx) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_I, _libusb_exit,
            ctx);
}

int LIBUSB_CALL libusb_reset_device(libusb_device_handle *dev_handle) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_II, _libusb_reset_device,
            dev_handle);
}

int LIBUSB_CALL libusb_kernel_driver_active(libusb_device_handle *dev_handle, int interface_number) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_III, _libusb_kernel_driver_active,
            dev_handle, interface_number);
}

int LIBUSB_CALL libusb_cancel_transfer(struct libusb_transfer *transfer) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_II, _libusb_cancel_transfer,
            transfer);
}

void LIBUSB_CALL libusb_free_transfer(struct libusb_transfer *transfer) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_I, _libusb_free_transfer,
            transfer);
}

int LIBUSB_CALL libusb_handle_events_timeout(libusb_context *ctx, struct timeval *tv) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_III, _libusb_handle_events_timeout,
            ctx, tv);
}

int LIBUSB_CALL libusb_handle_events_timeout_completed(libusb_context *ctx,
	struct timeval *tv, int *completed) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    return emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_IIII, _libusb_handle_events_timeout_completed,
            ctx, tv, completed);
}
