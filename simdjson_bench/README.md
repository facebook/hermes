# Replicating results

First, install dependencies: https://github.com/facebook/hermes/blob/main/doc/BuildingAndRunning.md

```
mkdir hermes_workingdir
cd hermes_workingdir
git clone https://github.com/radex/hermes.git
cd hermes
git checkout simdjson
cd ..
cmake -S hermes -B build_release -G Ninja -DCMAKE_BUILD_TYPE=Release -DHERMES_BUILD_NODE_HERMES=true
cmake --build ./build_release
build_release/bin/node-hermes hermes/simdjson_bench/test.js
```
