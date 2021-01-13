#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <bitset>
#include <thread>

#include <emscripten.h>
#include <emscripten/val.h>

#include "threads_posix.h"
#include "libusb.h"
#include "libusbi.h"

using namespace emscripten;

#define LIBUSB_MANUFACTURER_ID  ((uint8_t)1)
#define LIBUSB_PRODUCT_ID       ((uint8_t)2)
#define LIBUSB_SN_ID            ((uint8_t)3)

val _webusb_dev = val::object();
struct pending_transfer * pending_transfers = NULL;
struct pending_transfer {
    struct libusb_transfer * transfer;
    struct pending_transfer * next;
    struct pending_transfer * previous;
};

val create_out_buffer(uint8_t* buffer, size_t size) {
#ifdef __EMSCRIPTEN_PTHREADS__
    auto tmp = val(typed_memory_view(size, buffer));
    auto buf = val::global("Uint8Array").new_(size);
    buf.call<val>("set", tmp);
    return buf;
#else
    return val(typed_memory_view(size, buffer));
#endif
}

const struct libusb_version* libusb_get_version(void) {
    std::cout << "> " << __func__ << std::endl;

    static struct libusb_version info = {1, 0, 24, 0};
    return &info;
};

int libusb_init(libusb_context**) {
    std::cout << "> " << __func__ << std::endl;

    val navigator = val::global("navigator");

    if (!navigator["usb"].as<bool>()) {
        std::cerr << "WebUSB not supported by browser." << std::endl;
        return LIBUSB_ERROR_NOT_SUPPORTED;
    }

    return LIBUSB_SUCCESS;
};

ssize_t libusb_get_device_list(libusb_context *ctx, libusb_device ***list) {
    std::cout << "> " << __func__ << std::endl;

    val filter = val::object();
    filter.set("filters", val::array());

    val usb = val::global("navigator")["usb"];
    _webusb_dev = usb.call<val>("requestDevice", filter).await();

    if (!_webusb_dev.as<bool>())
        return LIBUSB_ERROR_NO_DEVICE;

    struct libusb_device* _dev;
    _dev = (struct libusb_device*)calloc(1, sizeof(struct libusb_device));
    _dev->device = &_webusb_dev;

    libusb_device** l;
    l = (libusb_device**)malloc(sizeof(libusb_device*) * 2);
    l[0] = _dev;
    l[1] = nullptr;
    *list = l;

    return 1;
}

int libusb_get_device_descriptor(libusb_device *dev, struct libusb_device_descriptor *desc) {
    std::cout << "> " << __func__ << std::endl;

    if (dev == nullptr)
        return LIBUSB_ERROR_INVALID_PARAM;

    val device = *dev->device;
    if (!device.as<bool>())
        return LIBUSB_ERROR_INVALID_PARAM;

    desc->bLength = LIBUSB_DT_DEVICE_SIZE;
    desc->bDescriptorType = LIBUSB_DT_DEVICE;
    desc->bcdUSB = device["usbVersionMajor"].as<uint8_t>() << 8 |
                   device["usbVersionMinor"].as<uint8_t>() << 0 ;
    desc->bDeviceClass = device["deviceClass"].as<uint8_t>();
    desc->bDeviceSubClass = device["deviceSubclass"].as<uint8_t>();
    desc->bDeviceProtocol = device["deviceProtocol"].as<uint8_t>();
    desc->bMaxPacketSize0 = 64;
    desc->idVendor = device["vendorId"].as<uint16_t>();
    desc->idProduct = device["productId"].as<uint16_t>();
    desc->bcdDevice = device["deviceVersionMajor"].as<uint8_t>()    << 8 |
                      device["deviceVersionMinor"].as<uint8_t>()    << 4 |
                      device["deviceVersionSubminor"].as<uint8_t>() << 0 ;
    desc->iManufacturer = LIBUSB_MANUFACTURER_ID;
    desc->iProduct = LIBUSB_PRODUCT_ID;
    desc->iSerialNumber = LIBUSB_SN_ID;
    desc->bNumConfigurations = device["configurations"]["length"].as<uint8_t>();

    return LIBUSB_SUCCESS;
}

void libusb_free_device_list(libusb_device **list, int unref_devices) {
    std::cout << "> " << __func__ << std::endl;
    free(list);
}

int LIBUSB_CALL libusb_open(libusb_device *dev, libusb_device_handle **dev_handle) {
    std::cout << "> " << __func__ << std::endl;

    val device = *dev->device;
    if (!device.as<bool>())
        return LIBUSB_ERROR_INVALID_PARAM;

    val::global().set("device", device);
    device.call<val>("open").await();

    struct libusb_device_handle* _dev_handle;
    _dev_handle = (struct libusb_device_handle*)calloc(1, sizeof(*_dev_handle));
    _dev_handle->dev = dev;
    *dev_handle = _dev_handle;

    return LIBUSB_SUCCESS;
}

void LIBUSB_CALL libusb_close(libusb_device_handle *dev_handle) {
    std::cout << "> " << __func__ << std::endl;

    val device = *dev_handle->dev->device;
    if (!device.as<bool>())
        return;

    std::cout << "close" << std::endl;

    device.call<val>("close").await();
}

int LIBUSB_CALL libusb_get_string_descriptor_ascii(libusb_device_handle *dev_handle,
    uint8_t desc_index, unsigned char *data, int length) {
    std::cout << "> " << __func__ << std::endl;

    val device = *dev_handle->dev->device;
    if (!device.as<bool>())
        return LIBUSB_ERROR_INVALID_PARAM;

    std::string str;
    switch (desc_index) {
        case LIBUSB_PRODUCT_ID:
            str = device["productName"].as<std::string>();
            std::copy(str.begin(), str.end(), data);
            return str.size();
        case LIBUSB_MANUFACTURER_ID:
            str = device["manufacturerName"].as<std::string>();
            std::copy(str.begin(), str.end(), data);
            return str.size();
        case LIBUSB_SN_ID:
            str = device["serialNumber"].as<std::string>();
            std::copy(str.begin(), str.end(), data);
            return str.size();
    }

    return LIBUSB_ERROR_INVALID_PARAM;
}

int LIBUSB_CALL libusb_set_configuration(libusb_device_handle *dev_handle, int configuration) {
    std::cout << "> " << __func__ << std::endl;

    val device = *dev_handle->dev->device;
    if (!device.as<bool>())
        return LIBUSB_ERROR_INVALID_PARAM;

    device.call<val>("selectConfiguration", configuration).await();

    return LIBUSB_SUCCESS;
}

int LIBUSB_CALL libusb_claim_interface(libusb_device_handle *dev_handle, int interface_number) {
    std::cout << "> " << __func__ << std::endl;

    val device = *dev_handle->dev->device;
    if (!device.as<bool>())
        return LIBUSB_ERROR_INVALID_PARAM;

    device.call<val>("claimInterface", interface_number).await();

    std::cout << "a" << std::endl;

    return LIBUSB_SUCCESS;
}

int LIBUSB_CALL libusb_release_interface(libusb_device_handle *dev_handle, int interface_number) {
    std::cout << "> " << __func__ << std::endl;

    val device = *dev_handle->dev->device;
    if (!device.as<bool>())
        return LIBUSB_ERROR_INVALID_PARAM;

    device.call<val>("releaseInterface", interface_number).await();

    return LIBUSB_SUCCESS;
}

int LIBUSB_CALL libusb_control_transfer(libusb_device_handle *dev_handle,
    uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
    unsigned char *data, uint16_t wLength, unsigned int timeout) {
    std::cout << "> " << __func__ << std::endl;

    val device = *dev_handle->dev->device;
    if (!device.as<bool>())
        return LIBUSB_ERROR_INVALID_PARAM;

    val setup = val::object();
    setup.set("request", bRequest);
    setup.set("value", wValue);
    setup.set("index", wIndex);

    switch ((request_type & 0x31)) {
        case LIBUSB_RECIPIENT_DEVICE:
            setup.set("recipient", std::string("device"));
            break;
        case LIBUSB_RECIPIENT_INTERFACE:
            setup.set("recipient", std::string("interface"));
            break;
        case LIBUSB_RECIPIENT_ENDPOINT:
            setup.set("recipient", std::string("endpoint"));
            break;
        case LIBUSB_RECIPIENT_OTHER:
            setup.set("recipient", std::string("other"));
            break;
    }

    switch ((request_type & 0x60)) {
        case LIBUSB_REQUEST_TYPE_STANDARD:
            setup.set("requestType", std::string("standard"));
            break;
        case LIBUSB_REQUEST_TYPE_CLASS:
            setup.set("requestType", std::string("class"));
            break;
        case LIBUSB_REQUEST_TYPE_VENDOR:
            setup.set("requestType", std::string("vendor"));
            break;
        case LIBUSB_REQUEST_TYPE_RESERVED:
            return LIBUSB_ERROR_INVALID_PARAM;
    }

    if ((request_type & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) {
        val res = device.call<val>("controlTransferIn", setup, wLength).await();

        if (res["status"].as<std::string>().compare("ok"))
            return LIBUSB_ERROR_IO;

        auto buf = res["data"]["buffer"].as<std::string>();
        std::copy(buf.begin(), buf.end(), data);

        return res["data"]["buffer"]["byteLength"].as<int>();
    }

    if ((request_type & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
        auto buf = create_out_buffer(data, wLength);
        val res = device.call<val>("controlTransferOut", setup, buf).await();

        if (res["status"].as<std::string>().compare("ok"))
            return LIBUSB_ERROR_IO;

        return res["bytesWritten"].as<int>();
    }

    return LIBUSB_ERROR_OTHER;
}

struct libusb_transfer * LIBUSB_CALL libusb_alloc_transfer(int iso_packets) {
    std::cout << "> " << __func__ << std::endl;

    size_t alloc_size =
		sizeof(struct libusb_transfer) +
		(sizeof(struct libusb_iso_packet_descriptor) * (size_t)iso_packets);

    return (struct libusb_transfer*)malloc(alloc_size);
}

int LIBUSB_CALL libusb_clear_halt(libusb_device_handle *dev_handle, unsigned char endpoint) {
    std::cout << "> " << __func__ << std::endl;

    val device = *dev_handle->dev->device;
    if (!device.as<bool>())
        return LIBUSB_ERROR_INVALID_PARAM;

    std::string direction = ((endpoint & LIBUSB_ENDPOINT_DIR_MASK)
            & LIBUSB_ENDPOINT_OUT) ? "out" : "in";
    unsigned char num = endpoint & ~LIBUSB_ENDPOINT_DIR_MASK;

    device.call<val>("clearHalt", direction, num).await();

    return LIBUSB_SUCCESS;
}

int LIBUSB_CALL libusb_submit_transfer(struct libusb_transfer *transfer) {
    std::cout << "> " << __func__ << std::endl;

    MAIN_THREAD_EM_ASM(return device);

    val device = *transfer->dev_handle->dev->device;
    unsigned char num = transfer->endpoint & ~LIBUSB_ENDPOINT_DIR_MASK;

    // TODO: Throw some C++ at this. Code by marcnewlin/webusb-libusb-shim.
    if (pending_transfers == NULL)
        pending_transfers = (struct pending_transfer*)calloc(1, sizeof(struct pending_transfer));

    struct pending_transfer* t = (struct pending_transfer*)calloc(1, sizeof(struct pending_transfer));
    t->transfer = transfer;

    struct pending_transfer * p = pending_transfers;
    while(p->next != NULL) {
        p = p->next;
    }

    t->previous = p;
    p->next = t;

    transfer->status = (enum libusb_transfer_status)-1;

    if (!device.as<bool>())
        return LIBUSB_ERROR_INVALID_PARAM;

    if ((transfer->endpoint & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) {
        switch(transfer->type) {
            case LIBUSB_TRANSFER_TYPE_BULK: {
                val res = device.call<val>("transferIn", num, transfer->length).await();

                if (res["status"].as<std::string>().compare("ok"))
                    return LIBUSB_ERROR_IO;

                auto buf = res["data"]["buffer"].as<std::string>();
                std::copy(buf.begin(), buf.end(), transfer->buffer);

                transfer->actual_length = res["data"]["buffer"]["byteLength"].as<int>();
                if (transfer->status != LIBUSB_TRANSFER_CANCELLED) {
                    transfer->status = LIBUSB_TRANSFER_COMPLETED;
                }

                return LIBUSB_SUCCESS;
            }
            case LIBUSB_TRANSFER_TYPE_BULK_STREAM:
                std::cout << "Not implemented: IN LIBUSB_TRANSFER_TYPE_BULK_STREAM" << std::endl;
                return LIBUSB_ERROR_NOT_SUPPORTED;
            case LIBUSB_TRANSFER_TYPE_CONTROL:
                std::cout << "Not implemented: IN LIBUSB_TRANSFER_TYPE_CONTROL" << std::endl;
                return LIBUSB_ERROR_NOT_SUPPORTED;
            case LIBUSB_TRANSFER_TYPE_INTERRUPT:
                std::cout << "Not implemented: IN LIBUSB_TRANSFER_TYPE_INTERRUPT" << std::endl;
                return LIBUSB_ERROR_NOT_SUPPORTED;
            case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
                std::cout << "Not implemented: IN LIBUSB_TRANSFER_TYPE_ISOCHRONOUS" << std::endl;
                return LIBUSB_ERROR_NOT_SUPPORTED;
        }
    }

    if ((transfer->endpoint & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
        switch(transfer->type) {
            case LIBUSB_TRANSFER_TYPE_BULK:
                std::cout << "Not implemented: OUT LIBUSB_TRANSFER_TYPE_BULK" << std::endl;
                return LIBUSB_ERROR_NOT_SUPPORTED;
            case LIBUSB_TRANSFER_TYPE_BULK_STREAM:
                std::cout << "Not implemented: OUT LIBUSB_TRANSFER_TYPE_BULK_STREAM" << std::endl;
                return LIBUSB_ERROR_NOT_SUPPORTED;
            case LIBUSB_TRANSFER_TYPE_CONTROL:
                std::cout << "Not implemented: OUT LIBUSB_TRANSFER_TYPE_CONTROL" << std::endl;
                return LIBUSB_ERROR_NOT_SUPPORTED;
            case LIBUSB_TRANSFER_TYPE_INTERRUPT:
                std::cout << "Not implemented: OUT LIBUSB_TRANSFER_TYPE_INTERRUPT" << std::endl;
                return LIBUSB_ERROR_NOT_SUPPORTED;
            case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
                std::cout << "Not implemented: OUT LIBUSB_TRANSFER_TYPE_ISOCHRONOUS" << std::endl;
                return LIBUSB_ERROR_NOT_SUPPORTED;
        }
    }

    return LIBUSB_ERROR_OTHER;
}

void libusb_exit(libusb_context *ctx) {
    std::cout << "> " << __func__ << std::endl;
    // nothing to do
}

int LIBUSB_CALL libusb_reset_device(libusb_device_handle *dev_handle) {
    std::cout << "> " << __func__ << std::endl;
}

int LIBUSB_CALL libusb_kernel_driver_active(libusb_device_handle *dev_handle, int interface_number) {
    std::cout << "> " << __func__ << std::endl;
}

int LIBUSB_CALL libusb_cancel_transfer(struct libusb_transfer *transfer) {
    std::cout << "> " << __func__ << std::endl;
}

void LIBUSB_CALL libusb_free_transfer(struct libusb_transfer *transfer) {
    std::cout << "> " << __func__ << std::endl;
}

int LIBUSB_CALL libusb_handle_events_timeout_completed(libusb_context *ctx,
	struct timeval *tv, int *completed) {
    std::cout << "> " << __func__ << std::endl;

    // TODO: Some C++ here too.
    struct pending_transfer * p = pending_transfers;
    while(p->next != NULL) {
        p = p->next;
        if(p->transfer->status == LIBUSB_TRANSFER_COMPLETED ||
            p->transfer->status == LIBUSB_TRANSFER_CANCELLED) {
            p->transfer->callback(p->transfer);
            p->transfer->status = (enum libusb_transfer_status)-1;
        }
    }

    return LIBUSB_SUCCESS;
}
