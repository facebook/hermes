/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/IR.h"

#include "llvh/Support/ErrorHandling.h"

// This file implements
//   Module::hash
// The implementation is hierarchical:
//  * The hash of a module is a function of the hashes of its Functions.
//  * The hash of a Function depends on the hashes of its Instructions.
//  * The hash of an instruction depends on it's kind, etc, and operands.

// When something depends on something else, and a loop may result, the
// entities are referred to by their numerical position.  This applies to
// Functions in a Module, or Instructions in a Function.  Non-instruction Value
// operands have hash functions depending on their contents.

namespace hermes {

namespace {

// There is a useful function in Hashing.h in llvh,
// "hash_combine_range".  This expects an iterator yielding a range of
// some T's, where there is a hash_value(const T&) overload.  In several cases,
// we need a "context" type -- to provide things like caching, or mappings of
// Instructions/Functions to numerical indices.  So we will temporarily make
// vectors of types that pair the Function/Instruction with this context
// information.

/// We will cache expensive-to-compute hashes for some kinds of Values.
using ValueHashCache = llvh::DenseMap<const Value *, llvh::hash_code>;

/// These provide the numerical index mapping discussed above, to avoid
/// cycles in dependencies.
using FunctionIndexMap = llvh::DenseMap<const Function *, unsigned>;
using InstrIndexMap = llvh::DenseMap<const Instruction *, unsigned>;

/// This is the "context" data that is shared for all hashes in a Module.
class ModuleSharedData {
 public:
  ModuleSharedData(const Module *M) : M(M) {}

  const Module *M;

  mutable ValueHashCache valueHashCache;

  /// The function index map is immutable via ModuleSharedData.
  const FunctionIndexMap &getFuncIndexMap() const {
    return funcIndexMap_;
  }
  FunctionIndexMap &getFuncIndexMap() {
    return funcIndexMap_;
  }

 protected:
  FunctionIndexMap funcIndexMap_;
};

/// This is the "context" data that is shared for all hashes in a Function.
class FuncSharedData {
 public:
  /// We capture a reference to the Module's shared data.
  explicit FuncSharedData(const ModuleSharedData &modData) : modData(modData) {}

  const ModuleSharedData &modData;

  /// The instruction index map is immutable via ModuleSharedData.
  const InstrIndexMap &getInstrIndexMap() const {
    return instrIndexMap_;
  }
  InstrIndexMap &getInstrIndexMap() {
    return instrIndexMap_;
  }

 protected:
  InstrIndexMap instrIndexMap_;
};

/// Pairs an Instruction with the FuncSharedData for the containing Function.
struct InstrWithSharedData {
  const Instruction &inst;
  const FuncSharedData &sharedData;

  InstrWithSharedData(const Instruction &inst, const FuncSharedData &sharedData)
      : inst(inst), sharedData(sharedData) {}
};

// Pairs a function with ModuleSharedData for the containing module.
struct FunctionWithSharedData {
  const Function *func;
  const ModuleSharedData &modSharedData;

  FunctionWithSharedData(
      const Function *func,
      const ModuleSharedData &modSharedData)
      : func(func), modSharedData(modSharedData) {}
};

/// The hash_value for a VariableScope.
llvh::hash_code hash_value(
    const VariableScope &varScope,
    const ModuleSharedData &modData) {
  auto iter = modData.valueHashCache.find(&varScope);
  if (iter != modData.valueHashCache.end()) {
    return iter->second;
  }
  VariableScope *par = varScope.getParentScope();
  llvh::hash_code result;
  if (par) {
    result = hash_value(*par, modData);
  }
  for (Variable *v : varScope.getVariables()) {
    result = llvh::hash_combine(result, hash_value(v->getName().str()));
  }
  modData.valueHashCache[&varScope] = result;
  return result;
}

/// Hash of non-Instruction Value that can appear as an operand of an
/// Instruction.
llvh::hash_code hash_value(const Value &val, const FuncSharedData &sharedData) {
  assert(
      !llvh::isa<Instruction>(val) &&
      "Precondition: only for non-instruction values");
  llvh::hash_code hc{0};
  // Set hc to any hash value specific to the kind.  (We will combine
  // the kind value after).
  switch (val.getKind()) {
    case ValueKind::LiteralEmptyKind:
    case ValueKind::LiteralUninitKind:
    case ValueKind::LiteralUndefinedKind:
    case ValueKind::LiteralNullKind:
    case ValueKind::GlobalObjectKind:
    case ValueKind::EmptySentinelKind:
      break;
    case ValueKind::LiteralNumberKind:
      hc = llvh::hash_value(llvh::cast<LiteralNumber>(val).getValue());
      break;
    case ValueKind::LiteralBigIntKind:
      hc = hash_value(llvh::cast<LiteralBigInt>(val).getValue()->str());
      break;
    case ValueKind::LiteralStringKind:
      hc = hash_value(llvh::cast<LiteralString>(val).getValue().str());
      break;
    case ValueKind::LiteralBoolKind:
      hc = (llvh::cast<LiteralBool>(val).getValue() ? 1 : 0);
      break;
    case ValueKind::LiteralBuiltinIdxKind:
      hc = llvh::cast<LiteralBuiltinIdx>(val).getData();
      break;
    case ValueKind::LiteralIRTypeKind:
      hc = llvh::cast<LiteralIRType>(val).getData().hash();
      break;
    case ValueKind::LiteralNativeSignatureKind:
      hc = llvh::cast<LiteralNativeSignature>(val).getData()->hash();
      break;
    case ValueKind::LiteralNativeExternKind: {
      auto &ne = llvh::cast<LiteralNativeExtern>(val);
      hc = llvh::hash_combine(
          ne.getData()->name()->str(), ne.getData()->signature()->hash());
      break;
    }
    case ValueKind::LiteralTypeOfIsTypesKind:
      hc = llvh::cast<LiteralTypeOfIsTypes>(val).getData().getRaw();
      break;
    case ValueKind::LabelKind:
      hc = hash_value(llvh::cast<Label>(val).get().str());
      break;
    case ValueKind::GlobalObjectPropertyKind: {
      const GlobalObjectProperty &gop = llvh::cast<GlobalObjectProperty>(val);
      hc = hash_combine(
          hash_value(gop.getName()->getValue().str()),
          static_cast<size_t>(gop.isDeclared()));
      break;
    }
    case ValueKind::VariableKind: {
      const Variable &v = llvh::cast<Variable>(val);
      hc = llvh::hash_combine(
          hash_value(
              llvh::cast<VariableScope>(*v.getParent()), sharedData.modData),
          llvh::hash_value(v.getName().str()));
      break;
    }
    case ValueKind::JSDynamicParamKind:
      hc = llvh::hash_value(
          llvh::cast<JSDynamicParam>(val).getIndexInParamList());
      break;
    case ValueKind::JSSpecialParamKind: {
      auto &specParam = llvh::cast<JSSpecialParam>(val);
      Function *f = specParam.getParent();
      hc = llvh::hash_value((&specParam == f->getParentScopeParam() ? 0 : 1));
      break;
    }
    case ValueKind::BasicBlockKind:
      // Hash the index of the first instruction of the BB.
      hc = sharedData.getInstrIndexMap().at(
          &*llvh::cast<BasicBlock>(val).begin());
      break;
    case ValueKind::VariableScopeKind:
      hc = hash_value(llvh::cast<VariableScope>(val), sharedData.modData);
      break;
    default:
      /// We handle all the Function subtypes (NormalFunction,
      /// GeneratorFunction, AsyncFunction) the same way: as the
      /// index of the Function in the Module.
      const auto *F = llvh::cast<Function>(&val);
      hc = sharedData.modData.getFuncIndexMap().at(F);
  }
  // Now add in the kind, and any attributes of the value.
  return hash_combine(
      static_cast<unsigned>(val.getKind()),
      hash_combine(hc, val.getAttributes(sharedData.modData.M).hash()));
}

llvh::hash_code hash_value(const InstrWithSharedData &instWithSharedData) {
  const Instruction &inst = instWithSharedData.inst;
  llvh::hash_code hc = llvh::hash_combine(
      inst.getType().hash(), (unsigned)inst.getKind(), inst.getNumOperands());

  // Check operands.
  for (unsigned i = 0, e = inst.getNumOperands(); i != e; ++i) {
    Value *op = inst.getOperand(i);
    if (Instruction *opInst = llvh::dyn_cast<Instruction>(op)) {
      hc = llvh::hash_combine(
          hc, instWithSharedData.sharedData.getInstrIndexMap().at(opInst));
    } else {
      hc = llvh::hash_combine(
          hc, hash_value(*op, instWithSharedData.sharedData));
    }
  }

  return hc;
}

llvh::hash_code hash_value(const FunctionWithSharedData &funcWithSharedData) {
  const Function &F = *funcWithSharedData.func;

  // Note that this hash value is concerned only with aspects of the class
  // that might be changed during optimization passes.
  llvh::hash_code hc = llvh::hash_combine(
      std::string(F.getInternalNameStr()), F.getReturnType().hash());

  FuncSharedData sharedData{funcWithSharedData.modSharedData};
  unsigned index = 0;
  std::vector<InstrWithSharedData> instrs;
  for (const BasicBlock &BB : F) {
    for (const Instruction &I : BB) {
      sharedData.getInstrIndexMap()[&I] = index++;
      instrs.emplace_back(I, sharedData);
    }
  }

  return llvh::hash_combine(
      hc,
      F.getAttributes(F.getParent()).hash(),
      llvh::hash_combine_range(instrs.begin(), instrs.end()));
}

} // namespace

llvh::hash_code Module::hash() const {
  ModuleSharedData modSharedData(this);

  unsigned functionIndex = 0;
  std::vector<FunctionWithSharedData> funcs;
  for (const Function &F : *this) {
    modSharedData.getFuncIndexMap()[&F] = functionIndex++;
    funcs.emplace_back(&F, modSharedData);
  }

  return llvh::hash_combine_range(funcs.begin(), funcs.end());
}

} // namespace hermes
