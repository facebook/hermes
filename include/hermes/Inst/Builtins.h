/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_INST_BUILTINS_H
#define HERMES_INST_BUILTINS_H

namespace hermes {
namespace inst {

namespace BuiltinMethod {
enum Enum : unsigned char {
#define BUILTIN_METHOD(object, name) object##_##name,
#include "Builtins.def"
  _count
};

}; // namespace BuiltinMethod

static_assert(BuiltinMethod::_count <= 256, "More than 256 builtin methods");

/// Return a string representation of the builtin method name.
const char *getBuiltinMethodName(int method);

} // namespace inst
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BUILTINS_H
