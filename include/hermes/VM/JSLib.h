/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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

void initGlobalObject(Runtime *runtime);

std::shared_ptr<RuntimeCommonStorage> createRuntimeCommonStorage();

/// eval() entry point. Evaluate the given source \p utf8code within the given
/// \p environment, using the given \p scopeChain to resolve identifiers.
/// \p thisArg is the initial "this" value of the function being evaluated.
/// If \p singleFunction is set, require that the output be only a single
/// function. \return the result of evaluation.
CallResult<HermesValue> evalInEnvironment(
    Runtime *runtime,
    llvm::StringRef utf8code,
    Handle<Environment> environment,
    const ScopeChain &scopeChain,
    Handle<> thisArg,
    bool singleFunction);

/// If the target CJS module is not initialized, execute it.
/// \param thisArg the "this" argument to call require() with.
/// \param fast true if we are doing a requireFast call and don't pass "this".
/// \return the resultant module.exports object.
CallResult<HermesValue> runRequireCall(
    Runtime *runtime,
    Handle<> thisArg,
    Handle<Domain> domain,
    uint32_t cjsModuleOffset,
    bool fast);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSLIB_H
