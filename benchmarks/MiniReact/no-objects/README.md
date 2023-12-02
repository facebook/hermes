This is the `no-deps` MiniReact benchmark with objects and unsupported generic
features removed.

# Setup

```
(cd ~/builds; cmake -S ~/fbsource/xplat/static_h/benchmarks/ -B benchmarksdebug -G Ninja -DHERMES_BUILD=~/builds/shdebug -DHERMES_SRC=~/fbsource/xplat/static_h) && (cd benchmarks/build-helpers/flow-bundler; yarn install)
```

# Run

```
./benchmarks/build-helpers/flow-bundler/bin/flow-bundler --root benchmarks/MiniReact/no-objects/src/ --out benchmarks/MiniReact/no-objects/MiniReact.js benchmarks/MiniReact/no-objects/src/index.js && (cd ~/builds/shdebug/; ninja hermesvm shermes && ./bin/shermes -typed -exec -g -source-map ~/fbsource/xplat/static_h/benchmarks/MiniReact/no-objects/MiniReact.js.map ~/fbsource/xplat/static_h/benchmarks/MiniReact/no-objects/MiniReact.js)
```
