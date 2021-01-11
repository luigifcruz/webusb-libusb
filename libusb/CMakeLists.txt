cmake_minimum_required(VERSION 3.10)

project(LibUSB VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(LibUSB src/libusb.cc)

target_include_directories(LibUSB PUBLIC include)

set_target_properties(LibUSB PROPERTIES PUBLIC_HEADER "include/libusb.h")
install(TARGETS LibUSB PUBLIC_HEADER DESTINATION include)
