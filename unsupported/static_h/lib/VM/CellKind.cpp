/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/CellKind.h"

#include <cassert>

namespace hermes {
namespace vm {

static const char *cellKinds[] = {
#define CELL_KIND(name) #name,
#include "hermes/VM/CellKinds.def"
};

const char *cellKindStr(CellKind kind) {
  assert(
      static_cast<size_t>(kind) < sizeof(cellKinds) / sizeof(cellKinds[0]) &&
      "Invalid CellKind");
  return cellKinds[static_cast<size_t>(kind)];
}

} // namespace vm
} // namespace hermes
