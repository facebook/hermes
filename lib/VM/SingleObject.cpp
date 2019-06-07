/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/SingleObject.h"
#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {
void MathBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

void JSONBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}
} // namespace vm
} // namespace hermes
