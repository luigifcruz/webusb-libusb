all: libusb external examples

PWD_DIR := $(shell pwd)
BUILD_DIR := $(PWD_DIR)/build
CMAKE_LIBUSB_OPTS := -DLIBUSB_INCLUDE_DIR=$(BUILD_DIR)/include -DLIBUSB_LIBRARIES=$(BUILD_DIR)/lib
CMAKE_INSTALL_OPTS := -DCMAKE_INSTALL_PREFIX=$(BUILD_DIR)
WASM_OPTS := -I./build/include/ -L./build/lib/ -lusb-1.0 --bind -s SINGLE_FILE=1 -s ASYNCIFY -s WASM=1

build_dir:
	mkdir -p build

clean:
	sudo rm -fr build
	sudo rm -fr external/airspyone_host/build
	sudo rm -fr external/rtl-sdr-blog/build
	sudo rm -fr libusb/build

#
# Build LibUSB Shim
#

libusb: build_dir
	mkdir -p libusb/build
	cd libusb/build && emcmake cmake $(CMAKE_INSTALL_OPTS) ..
	cd libusb/build && emmake make -j8
	cd libusb/build && sudo emmake make install

#
# Build External
#

airspy: build_dir
	mkdir -p external/airspyone_host/build
	cd external/airspyone_host/build && emcmake cmake $(CMAKE_LIBUSB_OPTS) $(CMAKE_INSTALL_OPTS) ..
	cd external/airspyone_host/build && emmake make -j8
	cd external/airspyone_host/build && sudo emmake make install

rtlsdr: build_dir
	mkdir -p external/rtl-sdr-blog/build
	cd external/rtl-sdr-blog/build && emcmake cmake $(CMAKE_LIBUSB_OPTS) $(CMAKE_INSTALL_OPTS) ..
	cd external/rtl-sdr-blog/build && emmake make -j8
	cd external/rtl-sdr-blog/build && sudo emmake make install

external: airspy

#
# Build Examples
#

example_dir: build_dir
	mkdir -p build/example

libusb_list_devices: libusb example_dir
	em++ $(WASM_OPTS) example/libusb_list_devices.cc -o build/example/libusb_list_devices.html

airspy_list_devices: libusb airspy example_dir
	em++ $(WASM_OPTS) -lairspy example/airspy_list_devices.cc -o build/example/airspy_list_devices.html

airspy_open: libusb airspy example_dir
	em++ $(WASM_OPTS) -lairspy example/airspy_open.cc -o build/example/airspy_open.html

examples: libusb_list_devices airspy_list_devices airspy_open
