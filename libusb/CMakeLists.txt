cmake_minimum_required(VERSION 3.10)

project(usb-1.0 VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(usb-1.0 src/libusb.cc)

target_include_directories(usb-1.0 PUBLIC include)

set_target_properties(usb-1.0 PROPERTIES PUBLIC_HEADER "include/libusb.h")
install(TARGETS usb-1.0 PUBLIC_HEADER DESTINATION include)