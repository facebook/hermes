/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "hermes/Support/UTF8.h"
#include "hermes/VM/Domain.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StringPrimitive.h"

#include "llvh/Support/Path.h"

namespace hermes {
namespace vm {

CallResult<HermesValue> runRequireCall(
    Runtime &runtime,
    Handle<RequireContext> context,
    Handle<Domain> domain,
    uint32_t cjsModuleOffset) {
  {
    auto cachedExports = domain->getCachedExports(runtime, cjsModuleOffset);
    if (!cachedExports->isEmpty()) {
      // Fast path: require() completed, so just return exports immediately.
      return cachedExports.getHermesValue();
    }
  }

  if (auto module = domain->getModule(runtime, cjsModuleOffset)) {
    assert(module.get() != nullptr);
    // Still initializing, so return the current state of module.exports.
    auto res = JSObject::getNamed_RJS(
        runtime.makeHandle(std::move(module)),
        runtime,
        Predefined::getSymbolID(Predefined::exports));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return res->get();
  }

  GCScope gcScope{runtime};
  // If not initialized yet, start initializing and set the module object.
  Handle<JSObject> module = runtime.makeHandle(JSObject::create(runtime));
  Handle<JSObject> exports = runtime.makeHandle(JSObject::create(runtime));
  if (LLVM_UNLIKELY(
          JSObject::putNamed_RJS(
              module,
              runtime,
              Predefined::getSymbolID(Predefined::exports),
              exports) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  domain->setModule(cjsModuleOffset, runtime, module);

  MutableHandle<JSObject> requireFn{runtime};
  if (context) {
    // Slow path.
    // If the context is provided, then it will be used to resolve string
    // requires in future calls to require().
    auto funcRes = BoundFunction::create(
        runtime,
        Handle<Callable>::vmcast(&runtime.requireFunction),
        1,
        ConstArgIterator(context.unsafeGetPinnedHermesValue() + 1));
    if (LLVM_UNLIKELY(funcRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    requireFn = vmcast<BoundFunction>(*funcRes);

    // Set the require.context property.
    PropertyFlags pf = PropertyFlags::defaultNewNamedPropertyFlags();
    pf.writable = 0;
    pf.configurable = 0;
    if (LLVM_UNLIKELY(
            JSObject::defineNewOwnProperty(
                requireFn,
                runtime,
                Predefined::getSymbolID(Predefined::context),
                pf,
                context) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  } else {
    // Fast path.
    // The context must not be provided, and any actual calls to require()
    // should have been turned into requireFast() calls.
    // Calls to require() should throw.
    requireFn = domain->getThrowingRequire(runtime).get();
  }

  CodeBlock *codeBlock = domain->getRuntimeModule(runtime, cjsModuleOffset)
                             ->getCodeBlockMayAllocate(domain->getFunctionIndex(
                                 runtime, cjsModuleOffset));

  Handle<JSFunction> func = runtime.makeHandle(JSFunction::create(
      runtime,
      domain,
      Handle<JSObject>::vmcast(&runtime.functionPrototype),
      Runtime::makeNullHandle<Environment>(),
      codeBlock));

  if (LLVM_UNLIKELY(
          JSFunction::executeCall3(
              func,
              runtime,
              exports,
              exports.getHermesValue(),
              requireFn.getHermesValue(),
              module.getHermesValue()) == ExecutionStatus::EXCEPTION)) {
    // If initialization of the module throws, reset it so that calling require
    // again may succeed.
    domain->setModule(cjsModuleOffset, runtime, Runtime::getNullValue());
    return ExecutionStatus::EXCEPTION;
  }

  // The module.exports object may have been replaced during initialization,
  // so we have to run getNamed to ensure we pick up the changes.
  auto exportsRes = JSObject::getNamed_RJS(
      module, runtime, Predefined::getSymbolID(Predefined::exports));
  if (LLVM_UNLIKELY(exportsRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  domain->setCachedExports(cjsModuleOffset, runtime, exportsRes->get());
  return domain->getCachedExports(runtime, cjsModuleOffset).getHermesValue();
}

CallResult<HermesValue> requireFast(void *, Runtime &runtime, NativeArgs args) {
  assert(
      runtime.getStackFrames().begin()->getSavedCodeBlock() != nullptr &&
      "requireFast() cannot be called from native");

  // Find the RuntimeModule from which this require was called.
  RuntimeModule *runtimeModule =
      runtime.getStackFrames().begin()->getSavedCodeBlock()->getRuntimeModule();
  auto domain = runtimeModule->getDomain(runtime);

  uint32_t index = args.getArg(0).getNumberAs<uint32_t>();
  OptValue<uint32_t> cjsModuleOffset =
      domain->getCJSModuleOffset(runtime, index);
  if (LLVM_UNLIKELY(!cjsModuleOffset)) {
    return runtime.raiseTypeError(
        TwineChar16("Unable to find module with ID: ") + index);
  }
  return runRequireCall(
      runtime,
      Runtime::makeNullHandle<RequireContext>(),
      domain,
      *cjsModuleOffset);
}

static llvh::SmallString<32> canonicalizePath(
    Runtime &runtime,
    Handle<StringPrimitive> dirname,
    Handle<StringPrimitive> target) {
  // Copy the current path so we can modify it as necessary.
  llvh::SmallString<32> canonicalPath{};

  auto appendToCanonical = [&canonicalPath](
                               Handle<StringPrimitive> strPrim,
                               uint32_t start = 0) {
    SmallU16String<32> u16String{};
    strPrim->appendUTF16String(u16String);
    std::string str{};
    hermes::convertUTF16ToUTF8WithReplacements(
        str, UTF16Ref{u16String}.slice(start));
    llvh::sys::path::append(canonicalPath, llvh::sys::path::Style::posix, str);
  };

  if (target->getStringLength() > 0 && target->at(0) == u'/') {
    // If the dirname is absolute (starts with a '/'), resolve from the module
    // root.
    appendToCanonical(target, 1);
  } else {
    // Else, the dirname is relative. Resolve from the dirname.
    appendToCanonical(dirname);
    appendToCanonical(target);
  }

  // Remove all dots. This is done to get rid of ../ or anything like ././.
  llvh::sys::path::remove_dots(
      canonicalPath, true, llvh::sys::path::Style::posix);

  return canonicalPath;
}

CallResult<HermesValue> require(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  auto requireContext = args.vmcastThis<RequireContext>();
  auto domain =
      runtime.makeHandle(RequireContext::getDomain(runtime, *requireContext));
  auto dirname =
      runtime.makeHandle(RequireContext::getDirname(runtime, *requireContext));
  auto targetRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(targetRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto target = runtime.makeHandle(std::move(*targetRes));

  auto canonicalPath = canonicalizePath(runtime, dirname, target);

  auto targetStrRes = StringPrimitive::create(runtime, canonicalPath);
  if (LLVM_UNLIKELY(targetStrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto cr = stringToSymbolID(
      runtime, createPseudoHandle(vmcast<StringPrimitive>(*targetStrRes)));
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<SymbolID> targetID = *cr;

  // Find the relevant CJS module.
  OptValue<uint32_t> cjsModuleOffset = domain->getCJSModuleOffset(*targetID);
  if (LLVM_UNLIKELY(!cjsModuleOffset)) {
    return runtime.raiseTypeError(
        TwineChar16("Unable to find module: ") + target.get());
  }

  llvh::sys::path::remove_filename(
      canonicalPath, llvh::sys::path::Style::posix);
  auto dirnameRes = StringPrimitive::create(runtime, canonicalPath);
  if (LLVM_UNLIKELY(dirnameRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto dirnameHandle = runtime.makeHandle<StringPrimitive>(*dirnameRes);

  auto newRequireContext =
      RequireContext::create(runtime, domain, dirnameHandle);

  return runRequireCall(runtime, newRequireContext, domain, *cjsModuleOffset);
}

} // namespace vm
} // namespace hermes
