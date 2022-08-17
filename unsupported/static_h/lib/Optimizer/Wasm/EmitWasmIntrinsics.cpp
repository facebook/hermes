/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_RUN_WASM
#define DEBUG_TYPE "EmitWasmIntrinsics"
#include "hermes/Optimizer/Wasm/EmitWasmIntrinsics.h"

#include "hermes/IR/IRBuilder.h"
#include "hermes/Optimizer/Wasm/WasmIntrinsics.h"
#include "hermes/Support/Statistic.h"
#include "hermes/Support/StringTable.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/Debug.h"

STATISTIC(NumWasmIntrinsics, "Number of Wasm intrinsics recognized");

namespace hermes {

class EmitWasmIntrinsicsContext {
 public:
  explicit EmitWasmIntrinsicsContext(StringTable &strTab);

  static EmitWasmIntrinsicsContext &get(Context &ctx) {
    if (!ctx.getWasmIntrinsicsContext()) {
      auto wasmIntrinsicsContext =
          std::make_shared<EmitWasmIntrinsicsContext>(ctx.getStringTable());
      ctx.setWasmIntrinsicsContext(wasmIntrinsicsContext);
    }

    return *ctx.getWasmIntrinsicsContext();
  }

  /// Look for unsafe compiler intrinsics.
  hermes::OptValue<WasmIntrinsics::Enum> findWasmIntrinsics(
      Identifier intrinsicsName);

  /// Lookup the expected number of arguments for a specific intrinsic.
  unsigned getWasmIntrinsicsNumArgs(WasmIntrinsics::Enum index) {
    return intrinsicsNumArgs_[index];
  }

 private:
  /// Map from an intrinisc name to an integer "intrinsic index".
  llvh::DenseMap<Identifier, WasmIntrinsics::Enum> intrinsics_;

  /// Map from an integer "intrinsic index" to its expected number of
  /// arguments.
  unsigned intrinsicsNumArgs_[WasmIntrinsics::_count];
};

EmitWasmIntrinsicsContext::EmitWasmIntrinsicsContext(StringTable &strTab) {
  // Insert all intrinsics.
  int intrinsicsIndex = 0;
#define WASM_INTRINSICS(name, numArgs)       \
  intrinsics_[strTab.getIdentifier(#name)] = \
      (WasmIntrinsics::Enum)(intrinsicsIndex++);
#include "hermes/Optimizer/Wasm/WasmIntrinsics.def"

  // Insert number of arguments.
  intrinsicsIndex = 0;
#define WASM_INTRINSICS(name, numArgs) \
  intrinsicsNumArgs_[intrinsicsIndex++] = numArgs;
#include "hermes/Optimizer/Wasm/WasmIntrinsics.def"
}

hermes::OptValue<WasmIntrinsics::Enum>
EmitWasmIntrinsicsContext::findWasmIntrinsics(Identifier intrinsicsName) {
  auto intrinsicsIt = intrinsics_.find(intrinsicsName);
  if (intrinsicsIt == intrinsics_.end()) {
    return llvh::None;
  }

  return intrinsicsIt->second;
}

bool EmitWasmIntrinsics::runOnFunction(Function *F) {
  IRBuilder builder{F};
  bool changed = false;

  auto &wasmIntrinsics = EmitWasmIntrinsicsContext::get(F->getContext());

  for (auto &BB : *F) {
    for (auto it = BB.begin(), e = BB.end(); it != e;) {
      auto *inst = &*it++;
      // Look for an instruction sequence of the kind:
      //    (Call
      //        (LoadProperty
      //            (LoadProperty globalObject, "__uasm")
      //            Prop)
      //        ...)
      if (inst->getKind() != ValueKind::CallInstKind)
        continue;
      auto *callInst = cast<CallInst>(inst);
      auto *loadProp = llvh::dyn_cast<LoadPropertyInst>(callInst->getCallee());
      if (!loadProp)
        continue;
      auto propLit = llvh::dyn_cast<LiteralString>(loadProp->getProperty());
      if (!propLit)
        continue;
      auto *loadGlobalProp =
          llvh::dyn_cast<LoadPropertyInst>(loadProp->getObject());
      if (!loadGlobalProp)
        continue;
      if (!llvh::isa<GlobalObject>(loadGlobalProp->getObject()))
        continue;
      LiteralString *objLit =
          llvh::dyn_cast<LiteralString>(loadGlobalProp->getProperty());
      if (!objLit || (objLit->getValue().str() != "__uasm"))
        continue;

      // Check if the intrinsic is defined.
      auto wasmIntrinsicsIndex =
          wasmIntrinsics.findWasmIntrinsics(propLit->getValue());
      if (!wasmIntrinsicsIndex) {
        // Undefined intrinsics in __uasm namesapce.
        F->getContext().getSourceErrorManager().error(
            F->getSourceRange(),
            Twine("the intrinsic \"") + propLit->getValue().str() +
                "\" is undefined.");
        continue;
      }

      LLVM_DEBUG(
          llvh::dbgs() << "__uasm call: " << objLit->getValue() << "."
                       << propLit->getValue() << "\n");
      LLVM_DEBUG(
          llvh::dbgs() << "Wasm intriniscs found [" << (int)*wasmIntrinsicsIndex
                       << "] " << getWasmIntrinsicsName(*wasmIntrinsicsIndex)
                       << "()\n");

      // Check if the intrinsic call has the correct number of arguments.
      unsigned numArgsExcludingThis = callInst->getNumArguments() - 1;
      unsigned numExpectedArgs =
          wasmIntrinsics.getWasmIntrinsicsNumArgs(*wasmIntrinsicsIndex);
      if (numArgsExcludingThis != numExpectedArgs) {
        F->getContext().getSourceErrorManager().error(
            F->getSourceRange(),
            Twine("the intrinsic \"") + propLit->getValue().str() +
                "\" is called with incorrect number of arguments. Expecting " +
                llvh::Twine(numExpectedArgs) + " but got " +
                llvh::Twine(numArgsExcludingThis) + ".");
        continue;
      }

      changed = true;
      builder.setInsertionPoint(callInst);
      builder.setLocation(callInst->getLocation());

      llvh::SmallVector<Value *, 2> args{};
      args.reserve(numArgsExcludingThis);
      for (unsigned i = 0; i < numArgsExcludingThis; i++) {
        args.push_back(callInst->getArgument(i + 1));
      }

      auto *callIntrinsic =
          builder.createCallIntrinsicInst(*wasmIntrinsicsIndex, args);
      callInst->replaceAllUsesWith(callIntrinsic);
      callInst->eraseFromParent();

      // Remove property access intructions.
      if (!loadProp->hasUsers())
        loadProp->eraseFromParent();
      if (!loadGlobalProp->hasUsers())
        loadGlobalProp->eraseFromParent();

      NumWasmIntrinsics++;
    }
  }

  return changed;
}

} // namespace hermes

#undef DEBUG_TYPE
#endif // HERMES_RUN_WASM
