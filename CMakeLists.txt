cmake_minimum_required(VERSION 3.5)
project(FastEVM VERSION 1.0.0)

include_directories(./include)
include_directories(./external/evmc)
include_directories(./external/eos_mem_wrapper/)

find_package(eosio.cdt)

add_contract( FastEVM FastEVM ./src/fastevm.cpp ./include/fastevm/fastevm.hpp)
