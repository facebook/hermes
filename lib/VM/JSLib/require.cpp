#include "JSLibInternal.h"

#include "hermes/Support/UTF8.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/StringPrimitive.h"

#include "llvm/Support/Path.h"

namespace hermes {
namespace vm {

/// If the target CJS module is not initialized, execute it.
/// \param thisArg the "this" argument to call require() with.
/// \param fast true if we are doing a requireFast call and don't pass "this".
/// \return the resultant module.exports object.
static CallResult<HermesValue> runRequireCall(
    Runtime *runtime,
    Handle<> thisArg,
    RuntimeModule *runtimeModule,
    RuntimeModule::CJSModule *cjsModule,
    bool fast) {
  if (!cjsModule->cachedExports.isEmpty()) {
    // Fast path: require() completed, so just return exports immediately.
    return cjsModule->cachedExports;
  }

  if (cjsModule->module != nullptr) {
    // Still initializing, so return the current state of module.exports.
    return JSObject::getNamed(
        runtime->makeHandle<JSObject>(cjsModule->module),
        runtime,
        runtime->getPredefinedSymbolID(Predefined::exports));
  }

  assert(
      cjsModule->module == nullptr &&
      "Attempt to run require() on initialized module object");

  GCScope gcScope{runtime};
  // If not initialized yet, start initializing and set the module object.
  Handle<JSObject> module = toHandle(runtime, JSObject::create(runtime));
  Handle<JSObject> exports = toHandle(runtime, JSObject::create(runtime));
  if (LLVM_UNLIKELY(
          JSObject::putNamed(
              module,
              runtime,
              runtime->getPredefinedSymbolID(Predefined::exports),
              exports) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  cjsModule->module = *module;

  MutableHandle<> requireFn{runtime};
  if (!fast) {
    auto funcRes = BoundFunction::create(
        runtime,
        Handle<Callable>::vmcast(&runtime->requireFunction),
        1,
        thisArg.unsafeGetPinnedHermesValue());
    if (LLVM_UNLIKELY(funcRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    requireFn = *funcRes;
  } else {
    requireFn = runtime->throwInvalidRequire;
  }

  CodeBlock *codeBlock = runtimeModule->getCodeBlock(cjsModule->functionIndex);

  auto funcRes = JSFunction::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime->functionPrototype),
      runtime->makeNullHandle<Environment>(),
      codeBlock);
  if (LLVM_UNLIKELY(funcRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto func = runtime->makeHandle<JSFunction>(*funcRes);

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
    cjsModule->module = nullptr;
    return ExecutionStatus::EXCEPTION;
  }

  // The module.exports object may have been replaced during initialization,
  // so we have to run getNamed to ensure we pick up the changes.
  auto exportsRes = JSObject::getNamed(
      module, runtime, runtime->getPredefinedSymbolID(Predefined::exports));
  if (LLVM_UNLIKELY(exportsRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  cjsModule->cachedExports = *exportsRes;
  return cjsModule->cachedExports;
}

CallResult<HermesValue> requireFast(void *, Runtime *runtime, NativeArgs args) {
  assert(
      runtime->getStackFrames().begin()->getSavedCodeBlock() != nullptr &&
      "requireFast() cannot be called from native");

  // Find the RuntimeModule from which this require was called.
  RuntimeModule *runtimeModule = runtime->getStackFrames()
                                     .begin()
                                     ->getSavedCodeBlock()
                                     ->getRuntimeModule();

  uint32_t index = args.getArg(0).getNumberAs<uint32_t>();
  auto *cjsModule = runtimeModule->getCJSModule(index);
  if (LLVM_UNLIKELY(!cjsModule)) {
    return runtime->raiseTypeError(
        TwineChar16("Unable to find module with ID: ") + index);
  }
  return runRequireCall(
      runtime, args.getThisHandle(), runtimeModule, cjsModule, true);
}

static llvm::SmallString<32> canonicalizePath(
    Runtime *runtime,
    RuntimeModule *runtimeModule,
    Handle<StringPrimitive> dirname,
    Handle<StringPrimitive> target) {
  // Copy the current path so we can modify it as necessary.
  llvm::SmallString<32> canonicalPath{};

  auto appendToCanonical = [&canonicalPath](Handle<StringPrimitive> strPrim) {
    SmallU16String<32> u16String{};
    strPrim->copyUTF16String(u16String);
    std::string str{};
    hermes::convertUTF16ToUTF8WithReplacements(str, u16String);
    llvm::sys::path::append(canonicalPath, str);
  };

  appendToCanonical(dirname);
  appendToCanonical(target);

  // Remove all dots. This is done to get rid of ../ or anything like ././.
  llvm::sys::path::remove_dots(canonicalPath, true);

  if (canonicalPath[0] != '/') {
    // Prepend ./ in relative filepaths, because the `./` would've been
    // removed by remove_dots.
    canonicalPath.insert(canonicalPath.begin(), {'.', '/'});
  }

  return canonicalPath;
}

CallResult<HermesValue> require(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // Find the RuntimeModule from which this require was called.
  auto stack = runtime->getStackFrames();
  CodeBlock *curCodeBlock = nullptr;
  for (auto stackIt = stack.begin(), stackEnd = stack.end();
       stackIt != stackEnd;
       ++stackIt) {
    curCodeBlock = stackIt->getSavedCodeBlock();
    if (curCodeBlock) {
      break;
    }
  }
  assert(curCodeBlock && "no valid CodeBlock on callstack");
  RuntimeModule *runtimeModule = curCodeBlock->getRuntimeModule();

  auto strRes = toString(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto dirname = toHandle(runtime, std::move(*strRes));

  auto targetRes = toString(runtime, args.getArgHandle(runtime, 0));
  if (LLVM_UNLIKELY(targetRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto target = toHandle(runtime, std::move(*targetRes));

  auto canonicalPath =
      canonicalizePath(runtime, runtimeModule, dirname, target);

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
  auto *cjsModule = runtimeModule->getCJSModule(*targetID);
  if (LLVM_UNLIKELY(!cjsModule)) {
    return runtime->raiseTypeError(
        TwineChar16("Unable to find module: ") + target.get());
  }

  llvm::sys::path::remove_filename(canonicalPath);
  auto dirnameRes = StringPrimitive::create(runtime, canonicalPath);
  if (LLVM_UNLIKELY(dirnameRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto dirnameHandle = runtime->makeHandle(*dirnameRes);

  return runRequireCall(
      runtime, dirnameHandle, runtimeModule, cjsModule, false);
}

} // namespace vm
} // namespace hermes
