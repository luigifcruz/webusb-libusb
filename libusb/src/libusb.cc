#include <iostream>
#include <string>
#include <vector>
#include <iterator>

#include <emscripten/val.h>

#include "libusb.h"

using namespace emscripten;

#define LIBUSB_DUMMY_DEVICE		((libusb_device*)0xca)
#define LIBUSB_DUMMY_HANDLE		((libusb_device_handle*)0xfe)
#define LIBUSB_MANUFACTURER_ID	((uint8_t)1)
#define LIBUSB_PRODUCT_ID		((uint8_t)2)
#define LIBUSB_SN_ID			((uint8_t)3)

val device = val::object();

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

	val usb = val::global("navigator")["usb"];

	val filter = val::object();
	filter.set("filters", val::array());
	device = usb.call<val>("requestDevice", filter).await();

	if (!device.as<bool>())
		return LIBUSB_ERROR_NO_DEVICE;

	libusb_device** l = (libusb_device**)malloc(sizeof(libusb_device*) * 2);
	l[0] = LIBUSB_DUMMY_DEVICE;
	l[1] = nullptr;
	*list = l;

	return 1;
}

int libusb_get_device_descriptor(libusb_device *dev, struct libusb_device_descriptor *desc) {
	std::cout << "> " << __func__ << std::endl;

	if (dev != LIBUSB_DUMMY_DEVICE)
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

	if (dev != LIBUSB_DUMMY_DEVICE)
		return LIBUSB_ERROR_INVALID_PARAM;

	device.call<val>("open").await();

	*dev_handle = LIBUSB_DUMMY_HANDLE;

	return LIBUSB_SUCCESS;
}

void LIBUSB_CALL libusb_close(libusb_device_handle *dev_handle) {
	std::cout << "> " << __func__ << std::endl;

	if (dev_handle)
		return;

	device.call<val>("close").await();
}

int LIBUSB_CALL libusb_get_string_descriptor_ascii(libusb_device_handle *dev_handle,
	uint8_t desc_index, unsigned char *data, int length) {
	std::cout << "> " << __func__ << std::endl;

	if (dev_handle != LIBUSB_DUMMY_HANDLE)
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

void libusb_exit(libusb_context *ctx) {
	std::cout << "> " << __func__ << std::endl;
	// nothing to do
}

int LIBUSB_CALL libusb_submit_transfer(struct libusb_transfer *transfer) {
	std::cout << "> " << __func__ << std::endl;
}

int LIBUSB_CALL libusb_reset_device(libusb_device_handle *dev_handle) {
	std::cout << "> " << __func__ << std::endl;
}

int LIBUSB_CALL libusb_release_interface(libusb_device_handle *dev_handle, int interface_number) {
	std::cout << "> " << __func__ << std::endl;
}

int LIBUSB_CALL libusb_kernel_driver_active(libusb_device_handle *dev_handle, int interface_number) {
	std::cout << "> " << __func__ << std::endl;
}

