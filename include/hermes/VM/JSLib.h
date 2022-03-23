/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// JavaScript standard library implementation.
//===----------------------------------------------------------------------===//
#ifndef HERMES_VM_JSLIB_H
#define HERMES_VM_JSLIB_H

#include "hermes/Support/ScopeChain.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/Domain.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/HermesValue.h"

#include <memory>

namespace hermes {
namespace vm {

// External forward declarations.
class Runtime;
struct RuntimeCommonStorage;

/// Flags controlling the initialization of the global object.
class JSLibFlags {
 public:
  /// If true, the HermesInternal object will be populated, otherwise it will be
  /// empty.
  bool enableHermesInternal = true;
  bool enableHermesInternalTestMethods = false;
};

void initGlobalObject(Runtime &runtime, const JSLibFlags &jsLibFlags);

/// Populate the builtin function entries in \b builtins using the corresponding
/// \c BuiltinMethod::xxx index.
void createHermesBuiltins(
    Runtime &runtime,
    llvh::MutableArrayRef<Callable *> builtins);

std::shared_ptr<RuntimeCommonStorage> createRuntimeCommonStorage(
    bool shouldTrace);

/// eval() entry point. Evaluate the given source \p utf8code within the given
/// \p environment, using the given \p scopeChain to resolve identifiers.
/// \p thisArg is the initial "this" value of the function being evaluated.
/// If \p singleFunction is set, require that the output be only a single
/// function. \return the result of evaluation.
CallResult<HermesValue> evalInEnvironment(
    Runtime &runtime,
    llvh::StringRef utf8code,
    Handle<Environment> environment,
    const ScopeChain &scopeChain,
    Handle<> thisArg,
    bool singleFunction);

/// If the target CJS module is not initialized, execute it.
/// \param context the RequireContext to pass through the require.
///        If null, use a fast require instead of the slow path.
/// \return the resultant module.exports object.
CallResult<HermesValue> runRequireCall(
    Runtime &runtime,
    Handle<RequireContext> context,
    Handle<Domain> domain,
    uint32_t cjsModuleOffset);

/// The [[ThrowTypeError]] internal function.
CallResult<HermesValue>
throwTypeError(void *, Runtime &runtime, NativeArgs args);

enum class TypeErrorKind {
  NonStrictOnly,
  InvalidDynamicRequire,
  // change this if more errors are added
  NumKinds
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSLIB_H
