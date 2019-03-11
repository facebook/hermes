/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_PREDEFINED_H
#define HERMES_VM_PREDEFINED_H

namespace hermes {
namespace vm {

enum class Predefined {
#define STR(name, string) name,
#include "PredefinedStrings.def"
#define SYM(name, string) name,
#include "PredefinedSymbols.def"
  _PREDEFINED_COUNT
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PREDEFINED_H
