/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_RUN_WASM
#include "hermes/Optimizer/Wasm/WasmIntrinsics.h"

#include <cassert>

namespace hermes {

static const char *wasmIntrinsicsName[] = {
#define WASM_INTRINSICS(name, numArgs) "__uasm." #name "_" #numArgs,
#include "hermes/Optimizer/Wasm/WasmIntrinsics.def"
};

const char *getWasmIntrinsicsName(unsigned intrinsics) {
  assert(
      intrinsics < WasmIntrinsics::_count && "invalid Wasm intrinsics index");
  return wasmIntrinsicsName[intrinsics];
}

} // namespace hermes
#endif // HERMES_RUN_WASM
