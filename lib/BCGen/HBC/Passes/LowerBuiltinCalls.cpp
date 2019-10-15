/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "builtins"
#include "hermes/BCGen/HBC/Passes/LowerBuiltinCalls.h"

#include "hermes/BCGen/HBC/BackendContext.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Inst/Builtins.h"
#include "hermes/Support/Statistic.h"
#include "hermes/Support/StringTable.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"

STATISTIC(NumLowered, "Number of builtin calls lowered");

namespace hermes {
namespace hbc {

class LowerBuiltinCallsContext {
 public:
  LowerBuiltinCallsContext(StringTable &strTab);

  static LowerBuiltinCallsContext &get(Context &ctx) {
    auto &BEC = BackendContext::get(ctx);
    if (!BEC.lowerBuiltinCallsContext)
      BEC.lowerBuiltinCallsContext =
          std::make_shared<LowerBuiltinCallsContext>(ctx.getStringTable());

    return *BEC.lowerBuiltinCallsContext;
  }

  /// Look for a builtin method \c object.method.
  hermes::OptValue<int> findBuiltinMethod(
      Identifier objectName,
      Identifier methodName);

  Identifier getHermesInternalID() const {
    return hermesInternalID_;
  }

 private:
  /// Identifier of "HermesInternal".
  Identifier hermesInternalID_;

  /// Map from a builtin object to an integer "object index".
  llvm::DenseMap<Identifier, int> objects_;

  /// Map from "object index":identifier to a "method index".
  /// We are avoiding allocating a DenseMap per object.
  llvm::DenseMap<std::pair<int, Identifier>, int> methods_;
};

LowerBuiltinCallsContext::LowerBuiltinCallsContext(StringTable &strTab) {
  hermesInternalID_ = strTab.getIdentifier("HermesInternal");

  // First insert all objects.
  int objIndex = 0;
#define BUILTIN_OBJECT(name) objects_[strTab.getIdentifier(#name)] = objIndex++;
#include "hermes/Inst/Builtins.def"

  // Now insert all methods.
  int methodIndex = 0;
#define BUILTIN_METHOD(object, name)                                           \
  methods_[std::make_pair(                                                     \
      objects_[strTab.getIdentifier(#object)], strTab.getIdentifier(#name))] = \
      methodIndex++;
#include "hermes/Inst/Builtins.def"
}

hermes::OptValue<int> LowerBuiltinCallsContext::findBuiltinMethod(
    Identifier objectName,
    Identifier methodName) {
  auto objIt = objects_.find(objectName);
  if (objIt == objects_.end())
    return llvm::None;

  auto methIt = methods_.find(std::make_pair(objIt->second, methodName));
  if (methIt == methods_.end())
    return llvm::None;

  return methIt->second;
}

static bool run(Function *F) {
  IRBuilder builder{F};
  bool changed = false;

  auto &builtins = LowerBuiltinCallsContext::get(F->getContext());

  for (auto &BB : *F) {
    for (auto it = BB.begin(), e = BB.end(); it != e;) {
      // Get a pointer to the instruction and increment the iterator so we
      // can delete the instruction if we want to.
      auto *inst = &*it++;

      // Look for an instruction sequence of the kind:
      //    (Call
      //        (LoadProperty
      //            (LoadProperty globalObject, "objectLiteral")
      //            Prop)
      //        ...)
      // where \c Object.Prop is a pre-defined builtin.
      // Note: we only want to check for call. Not for construct.
      if (inst->getKind() != ValueKind::CallInstKind)
        continue;
      auto *callInst = cast<CallInst>(inst);
      auto *loadProp = dyn_cast<LoadPropertyInst>(callInst->getCallee());
      if (!loadProp)
        continue;
      auto propLit = dyn_cast<LiteralString>(loadProp->getProperty());
      if (!propLit)
        continue;
      auto *loadGlobalProp = dyn_cast<LoadPropertyInst>(loadProp->getObject());
      if (!loadGlobalProp)
        continue;
      if (!isa<GlobalObject>(loadGlobalProp->getObject()))
        continue;
      LiteralString *objLit =
          dyn_cast<LiteralString>(loadGlobalProp->getProperty());
      if (!objLit)
        continue;

      // Uncomment this to get a dump of all global calls detected during
      // compilation.
      // llvm::outs() << "global call: " << objLit->getValue() << "."
      //             << propLit->getValue() << "\n";

      auto builtinIndex =
          builtins.findBuiltinMethod(objLit->getValue(), propLit->getValue());
      if (!builtinIndex)
        continue;
      // Too many arguments?
      if (callInst->getNumArguments() > HBCCallBuiltinInst::MAX_ARGUMENTS)
        continue;

      LLVM_DEBUG(
          llvm::dbgs() << "Found builtin [" << *builtinIndex << "] "
                       << inst::getBuiltinMethodName(*builtinIndex) << "()\n");

      // Always lower HermesInternal.xxx() calls, but only lower the rest if
      // -fstatic-builtins is enabled.
      if (objLit->getValue() != builtins.getHermesInternalID() &&
          !F->getContext().getOptimizationSettings().staticBuiltins) {
        continue;
      }

      changed = true;
      builder.setInsertionPoint(callInst);
      builder.setLocation(callInst->getLocation());

      llvm::SmallVector<Value *, 8> args{};
      unsigned numArgsExcludingThis = callInst->getNumArguments() - 1;
      args.reserve(numArgsExcludingThis);
      for (unsigned i = 0; i < numArgsExcludingThis; ++i)
        args.push_back(callInst->getArgument(i + 1));

      auto *callBuiltin = builder.createHBCCallBuiltinInst(*builtinIndex, args);
      callInst->replaceAllUsesWith(callBuiltin);
      callInst->eraseFromParent();

      // The property access instructions are not normally optimizable since
      // they have side effects, but in this case it is safe to remove them.
      if (!loadProp->hasUsers())
        loadProp->eraseFromParent();
      if (!loadGlobalProp->hasUsers())
        loadGlobalProp->eraseFromParent();

      ++NumLowered;
    }
  }

  return changed;
}

bool LowerBuiltinCalls::runOnFunction(Function *F) {
  return run(F);
}

} // namespace hbc
} // namespace hermes
