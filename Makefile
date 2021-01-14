all: libusb external examples

PWD_DIR := $(shell pwd)
BUILD_DIR := $(PWD_DIR)/build

EM_FLAGS := -s USE_PTHREADS=1 -s WASM=1 -fsanitize=undefined -g4 -s 'ASYNCIFY_IMPORTS=["emscripten_receive_on_main_thread_js"]' -s ASYNCIFY_STACK_SIZE=1048576
EM_OPTS := -I./build/include/ -L./build/lib/ -lusb-1.0 --bind -s ASYNCIFY $(EM_FLAGS)

CMAKE_EM_OPTS := -DCMAKE_CXX_FLAGS="$(EM_FLAGS)" -DCMAKE_C_FLAGS="$(EM_FLAGS)"
CMAKE_LIBUSB_OPTS := -DLIBUSB_INCLUDE_DIR=$(BUILD_DIR)/include -DLIBUSB_LIBRARIES=$(BUILD_DIR)/lib
CMAKE_INSTALL_OPTS := -DCMAKE_INSTALL_PREFIX=$(BUILD_DIR)

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
	cd libusb/build && emcmake cmake $(CMAKE_INSTALL_OPTS) $(CMAKE_EM_OPTS) ..
	cd libusb/build && emmake make -j8 VERBOSE=1
	cd libusb/build && sudo emmake make install

#
# Build External
#

liquiddsp: build_dir
	mkdir -p external/liquid-dsp/build
	cd external/liquid-dsp/build && emcmake cmake $(CMAKE_LIBUSB_OPTS) $(CMAKE_INSTALL_OPTS) $(CMAKE_EM_OPTS) ..
	cd external/liquid-dsp/build && emmake make -j8
	cd external/liquid-dsp/build && sudo emmake make install

airspy: build_dir
	mkdir -p external/airspyone_host/build
	cd external/airspyone_host/build && emcmake cmake $(CMAKE_LIBUSB_OPTS) $(CMAKE_INSTALL_OPTS) $(CMAKE_EM_OPTS) ..
	cd external/airspyone_host/build && emmake make -j8
	cd external/airspyone_host/build && sudo emmake make install

external: airspy

#
# Build Examples
#

example_dir: build_dir
	mkdir -p build/example

libusb_list_devices: libusb example_dir
	em++ $(EM_OPTS) example/libusb_list_devices.cc -o build/example/libusb_list_devices.html

airspy_list_devices: libusb airspy example_dir
	em++ $(EM_OPTS) -lairspy example/airspy_list_devices.cc -o build/example/airspy_list_devices.html

airspy_open: libusb airspy example_dir
	em++ $(EM_OPTS) -lairspy example/airspy_open.cc -o build/example/airspy_open.html

rtlsdr_open: libusb example_dir
	em++ $(EM_OPTS) -lrtlsdr example/rtlsdr_open.cc -o build/example/rtlsdr_open.html

examples: libusb_list_devices airspy_list_devices airspy_open
