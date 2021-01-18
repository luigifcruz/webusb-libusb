cmake_minimum_required(VERSION 3.10)

project(audiocontext VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(audiocontext src/audiocontext.cc src/interface.cc)

target_include_directories(audiocontext PUBLIC include)

set_target_properties(audiocontext PROPERTIES PUBLIC_HEADER "include/audiocontext.h")
install(TARGETS audiocontext PUBLIC_HEADER DESTINATION include)
