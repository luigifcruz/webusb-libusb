all: libusb external examples

PWD_DIR := $(shell pwd)
BUILD_DIR := $(PWD_DIR)/build

EM_DEBUG := -g4 --profiling
EM_FLAGS := $(EM_DEBUG) -O3 -s USE_PTHREADS=1 -s WASM=1 -s 'ASYNCIFY_IMPORTS=["emscripten_receive_on_main_thread_js"]' -s ASYNCIFY_STACK_SIZE=10000 -s INITIAL_MEMORY=64MB -s PROXY_TO_PTHREAD=1
EM_OPTS := -I./build/include/ -L./build/lib/ -lusb-1.0 --bind -s ASYNCIFY $(EM_FLAGS)

CMAKE_EM_OPTS := -DCMAKE_CXX_FLAGS="$(EM_FLAGS)" -DCMAKE_C_FLAGS="$(EM_FLAGS)"
CMAKE_LIBUSB_OPTS := -DLIBUSB_INCLUDE_DIR=$(BUILD_DIR)/include -DLIBUSB_LIBRARIES=$(BUILD_DIR)/lib
CMAKE_INSTALL_OPTS := -DCMAKE_INSTALL_PREFIX=$(BUILD_DIR)

build_dir:
	mkdir -p build

clean:
	sudo rm -fr build
	sudo rm -fr external/airspyone_host/build
	sudo rm -fr external/samurai/build
	sudo rm -fr external/liquid-dsp/build
	sudo rm -fr libusb/build
	sudo rm -fr audiocontext/build

#
# Build Libraries
#

audiocontext: build_dir
	mkdir -p audiocontext/build
	cd audiocontext/build && emcmake cmake $(CMAKE_INSTALL_OPTS) $(CMAKE_EM_OPTS) ..
	cd audiocontext/build && emmake make -j8
	cd audiocontext/build && sudo emmake make install

libusb: build_dir
	mkdir -p libusb/build
	cd libusb/build && emcmake cmake $(CMAKE_INSTALL_OPTS) $(CMAKE_EM_OPTS) ..
	cd libusb/build && emmake make -j8
	cd libusb/build && sudo emmake make install

libs: audiocontext libusb

#
# Build External
#

liquid: build_dir
	cd external/liquid-dsp && bash bootstrap.sh
	cd external/liquid-dsp && emconfigure ./configure --build=x86_64 --target=x86_64 --host=x86_64 -C CFLAGS='-O3' --prefix=$(BUILD_DIR) ..
	cd external/liquid-dsp && emmake make -j8
	cd external/liquid-dsp && sudo emmake make install

samurai: build_dir airspy
	sudo rm -fr external/samurai/build
	cd external/samurai && meson . build --cross-file target/emscripten.txt --prefix $(BUILD_DIR)
	cd external/samurai/build && ninja
	cd external/samurai/build && sudo ninja install

airspy: build_dir
	mkdir -p external/airspyone_host/build
	cd external/airspyone_host/build && emcmake cmake $(CMAKE_LIBUSB_OPTS) $(CMAKE_INSTALL_OPTS) $(CMAKE_EM_OPTS) ..
	cd external/airspyone_host/build && emmake make -j8
	cd external/airspyone_host/build && sudo emmake make install

external: airspy samurai #liquid

#
# Build Examples
#

example_dir: build_dir
	mkdir -p build/example

libusb_list_devices: libusb example_dir
	em++ $(EM_OPTS) example/libusb_list_devices.cc -o build/example/libusb_list_devices.html

airspy_list_devices: libusb airspy example_dir
	em++ $(EM_OPTS) -lairspy example/airspy_list_devices.cc -o build/example/airspy_list_devices.html

airspy_stream: libusb airspy example_dir
	em++ $(EM_OPTS) -lairspy example/airspy_stream.cc -o build/example/airspy_stream.html

samurai_stream: libusb airspy samurai
	em++ $(EM_OPTS) --std=c++17 -lairspy -lsamurai example/samurai_stream.cc -o build/example/samurai_stream.html

samurai_radio: libusb airspy samurai audiocontext #liquid
	em++ $(EM_OPTS) --std=c++17 -laudiocontext -lairspy -lsamurai -lliquid example/samurai_radio.cc -o build/example/samurai_radio.html

audiocontext_test: audiocontext
	em++ $(EM_OPTS) --std=c++17 -laudiocontext example/audiocontext_test.cc -o build/example/audiocontext_test.html

examples: libusb_list_devices airspy_list_devices airspy_stream samurai_stream samurai_radio audiocontext_test
