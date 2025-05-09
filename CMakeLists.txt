cmake_minimum_required(VERSION 3.31)
project(Zephyr-Native-Bridge)

set(CMAKE_CXX_STANDARD 23)

add_library(Zephyr SHARED library.cpp)
