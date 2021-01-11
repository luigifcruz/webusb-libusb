all:
	em++ -I./libusb/ -I./rtl-sdr-blog/include/ --bind -s ASYNCIFY -s WASM=1 main.cc ./libusb/*.cc ./rtl-sdr-blog/src/*.c -o main.html
