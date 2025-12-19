#! /bin/bash

./utils/cmake-dev-build.sh -B ./build_clangd -G Ninja -DCMAKE_BUILD_TYPE=Debug
ln -sf ./build_clangd/compile_commands.json .
