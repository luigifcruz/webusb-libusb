#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <emscripten/val.h>

#include "libusb.h"

using namespace emscripten;

#define LIBUSB_DUMMY_DEVICE ((libusb_device*)0xca)
val device = val::object();

const struct libusb_version* libusb_get_version(void) {
	static struct libusb_version info = {1, 0, 24, 0};
	return &info;
};

int libusb_init(libusb_context**) {
	val navigator = val::global("navigator");

	if (!navigator["usb"].as<bool>()) {
		std::cerr << "WebUSB not supported by browser." << std::endl;
		return LIBUSB_ERROR_NOT_SUPPORTED;
	}

	return LIBUSB_SUCCESS;
};

ssize_t libusb_get_device_list(libusb_context *ctx, libusb_device ***list) {
	val usb = val::global("navigator")["usb"];

	val filter = val::object();
	filter.set("filters", val::array());
	device = usb.call<val>("requestDevice", filter).await();

	if (!device.as<bool>())
		return LIBUSB_ERROR_NO_DEVICE;

	libusb_device** l = (libusb_device**)malloc(sizeof(libusb_device*));
	l[0] = LIBUSB_DUMMY_DEVICE;
	*list = l;

	return 1;
}

int libusb_get_device_descriptor(libusb_device *dev, struct libusb_device_descriptor *desc) {
	if (dev != LIBUSB_DUMMY_DEVICE)
		return LIBUSB_ERROR_INVALID_PARAM;

	desc = (libusb_device_descriptor*)malloc(sizeof(libusb_device_descriptor));

	desc->bLength = LIBUSB_DT_DEVICE_SIZE;
	desc->bDescriptorType = LIBUSB_DT_DEVICE;
	desc->bcdUSB = device["usbVersionMajor"].as<uint8_t>() << 8 |
				   device["usbVersionMinor"].as<uint8_t>() << 0 ;
	desc->bDeviceClass = device["deviceClass"].as<uint8_t>();
	desc->bDeviceSubClass = device["deviceSubclass"].as<uint8_t>();
	desc->bDeviceProtocol = device["deviceProtocol"].as<uint8_t>();
	desc->bMaxPacketSize0 = 128;
	desc->idVendor = device["vendorId"].as<uint16_t>();
	desc->idProduct = device["productId"].as<uint16_t>();
	desc->bcdDevice = device["deviceVersionMajor"].as<uint8_t>()    << 8 |
					  device["deviceVersionMinor"].as<uint8_t>()    << 4 |
					  device["deviceVersionSubminor"].as<uint8_t>() << 0 ;
	desc->bNumConfigurations = device["configurations"]["length"].as<uint8_t>();

	return LIBUSB_SUCCESS;
}

void libusb_free_device_list(libusb_device **list, int unref_devices) {
	free(list);
}

void libusb_exit(libusb_context *ctx) {
	// nop
}


