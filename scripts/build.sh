#!/usr/bin/env bash

mkdir -p dist

# eosio.wps
eosio-cpp -abigen \
  ./src/fastevm.cpp \
  -contract fastevm \
  -R ./ricardian \
  -o ./dist/fastevm.wasm \
  -I ./include \
  -I ./external/eos_mem_wrapper \
  -I ./external/evmc \
  -I ./external/sha3iuf \
  -I ./external/bigint 