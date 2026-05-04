# Node-API Conformance Test Suite (`node-api-cts`)

A test suite for Node-API implementors across different JS engines and runtimes.
Written in ECMAScript & C/C++ with an implementor customizable harness.

> [!IMPORTANT]  
> This repository is currently a work-in-progress and shouldn't yet be relied on by anyone.

## Overview

### Tests

The tests are divided into three buckets:

- `tests/harness/*` exercising the implementor's test harness.

and two parts based on the two header files declaring the Node-API functions:

- `tests/js-native-api/*` testing the engine-specific part of Node-API defined in the [`js_native_api.h`](https://github.com/nodejs/node-api-headers/blob/main/include/js_native_api.h) header.
- `tests/node-api/*` testing the runtime-specific part of Node-API defined in the [`node_api.h`](https://github.com/nodejs/node-api-headers/blob/main/include/node_api.h) header.

### Implementors

This repository offers an opportunity for implementors of Node-API to maintain (parts of) their implementor-specific harness inside this repository, in a sub-directory of the `implementors` directory. We do this in hope of increased velocity from faster iteration and potentials for reuse of code across the harnesses.

We maintain a list of other runtimes implementing Node-API in [doc/node-api-engine-bindings.md](https://github.com/nodejs/abi-stable-node/blob/doc/node-api-engine-bindings.md#node-api-bindings-in-other-runtimes) of the `nodejs/abi-stable-node` repository.
