/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_RUN_WASM
#ifndef HERMES_INST_WASM_INTRINSICS_H
#define HERMES_INST_WASM_INTRINSICS_H

#include <cassert>

namespace hermes {

namespace WasmIntrinsics {
enum Enum : unsigned char {
#define WASM_INTRINSICS(name, numArgs) __uasm##_##name,
#include "WasmIntrinsics.def"
  _count,
};

} // namespace WasmIntrinsics

/// Return a string representation of the Wasm intrinsics.
const char *getWasmIntrinsicsName(unsigned intrinsics);

} // namespace hermes

#endif // HERMES_INST_WASM_INTRINSICS_H
#endif // HERMES_RUN_WASM
