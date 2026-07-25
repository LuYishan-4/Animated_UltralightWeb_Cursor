#!/usr/bin/env bash
#  just for me test
cd "$(dirname "$0")"


if [ ! -d "sdk" ]; then
    mkdir -p sdk
fi

cd sdk
if [ ! -d "ultralight-free-sdk-1.4.0-linux-x64" ]; then
    7z x ultralight-free-sdk-1.4.0-linux-x64.7z
fi
cd ..

cd "$(dirname "$0")"

sudo rm -rf build
cmake -B build \
      -S . \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr

cmake --build build

cd build
sudo cmake --install .
