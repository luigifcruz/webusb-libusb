all: usb airspy wasm

airspy:
	mkdir -p build
	mkdir -p airspyone_host/build
	cd airspyone_host/build && emcmake cmake -DLIBUSB_INCLUDE_DIR=$(shell pwd)/build/include -DLIBUSB_LIBRARIES=$(shell pwd)/build/lib -DCMAKE_INSTALL_PREFIX=$(shell pwd)/build ..
	cd airspyone_host/build && emmake make -j8
	cd airspyone_host/build && sudo emmake make install

rtlsdr:
	mkdir -p build
	mkdir -p rtl-sdr-blog/build
	cd rtl-sdr-blog/build && emcmake cmake -DLIBUSB_INCLUDE_DIR=$(shell pwd)/build/include -DLIBUSB_LIBRARIES=$(shell pwd)/build/lib -DCMAKE_INSTALL_PREFIX=$(shell pwd)/build ..
	cd rtl-sdr-blog/build && emmake make -j8
	cd rtl-sdr-blog/build && sudo emmake make install

usb:
	mkdir -p build
	mkdir -p libusb/build
	cd libusb/build && emcmake cmake -DCMAKE_INSTALL_PREFIX=$(shell pwd)/build ..
	cd libusb/build && emmake make -j8
	cd libusb/build && sudo emmake make install

wasm:
	em++ -I./build/include/ -L./build/lib/ -lairspy -lusb-1.0 --bind -s ASYNCIFY -s WASM=1 main.cc -o main.html
