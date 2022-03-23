/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_INST_BUILTINS_H
#define HERMES_INST_BUILTINS_H

#include <cassert>

namespace hermes {

namespace BuiltinMethod {
enum Enum : unsigned char {
#define BUILTIN_METHOD(object, name) object##_##name,
#define PRIVATE_BUILTIN(name) BUILTIN_METHOD(HermesBuiltin, name)
#define MARK_FIRST_PRIVATE_BUILTIN(name) _firstPrivate = PRIVATE_BUILTIN(name)
#define JS_BUILTIN(name) PRIVATE_BUILTIN(name)
#define MARK_FIRST_JS_BUILTIN(name) _firstJS = JS_BUILTIN(name)
#include "Builtins.def"
  _count,
  _publicCount = _firstPrivate,
  _privateCount = _firstJS - _firstPrivate,
  _jsCount = _count - _firstJS,
};

} // namespace BuiltinMethod

static_assert(BuiltinMethod::_count <= 256, "More than 256 builtin methods");

/// Return a string representation of the builtin method name.
const char *getBuiltinMethodName(int method);

/// Return whether a builtin method is a Native builtin.
/// Otherwise, it is a JS builtin (as of the only two cases today).
/// This is more efficient than consulting the GCKind. It only need to perform
/// a simple comparison between the method index and the JS section offset.
inline bool isNativeBuiltin(unsigned method) {
  assert(method < BuiltinMethod::_count && "out of bound BuiltinMethod index");
  return method < BuiltinMethod::_firstJS;
};

} // namespace hermes

#endif // HERMES_BCGEN_HBC_BUILTINS_H
