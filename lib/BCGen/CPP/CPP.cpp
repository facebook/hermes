/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifdef HERMES_CPP_BACKEND

#include "hermes/BCGen/CPP/CPP.h"

#include "hermes/BCGen/HBC/Passes.h"
#include "hermes/BCGen/Lowering.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/UTF8.h"

using namespace hermes;
using namespace cpp;

/// Escapes a string
/// Quotes are added around the string.
/// Special characters are backslash escaped.
/// Non ascii characters are escaped to escaped UTF16 character points.
/// Returns true iff the string only contained ASCII characters.
/// NOTE: Surrogate pairs will lead to generation of uncompileable code.
static std::pair<std::string, bool> escapeStr(llvm::ArrayRef<uint16_t> str) {
  bool isAscii = true;
  std::string out;
  out += "\"";
  // Iterate through UTF16 character points
  for (auto i = str.begin(), e = str.end(); i != e; ++i) {
    uint16_t c = *i;
    if (std::isprint(c) && c != '\\' && c != '"') {
      // Simple character
      out += c;
    } else {
      // Needs escaping
      switch (c) {
        case '"':
          out += "\\\"";
          break;
        case '\\':
          out += "\\\\";
          break;
        case '\t':
          out += "\\t";
          break;
        case '\r':
          out += "\\r";
          break;
        case '\n':
          out += "\\n";
          break;
        default:
          isAscii = false;
          // Convert to escaped UTF16 character point
          char const *const hexdig = "0123456789ABCDEF";
          out += "\\u";
          out += hexdig[c >> 12];
          out += hexdig[(c >> 8) & 0xF];
          out += hexdig[(c >> 4) & 0xF];
          out += hexdig[c & 0xF];
      }
    }
  }
  out += "\"";
  return {out, isAscii};
}

/// Serializes a string into c++ source code that creates either an
/// ASCIIRef or a UTF16Ref.
static std::string serializeStr(StringRef name) {
  // First convert the string to UTF16.
  llvm::SmallVector<uint16_t, 16> str;
  convertUTF8WithSurrogatesToUTF16(
      std::back_inserter(str), name.begin(), name.end());

  auto res = escapeStr(str);
  auto out = res.first;
  auto isAscii = res.second;

  if (isAscii) {
    out = "ASCIIRef(" + out + ", static_cast<size_t>(" +
        std::to_string(str.size()) + "))";
  } else {
    out = "UTF16Ref(u" + out + ", " + std::to_string(str.size()) + ")";
  }
  return out;
}

/// Generates the correct label for BasicBlock \p B based on \p bbMap and
/// outputs it through \p OS.
static void generateBasicBlockLabel(
    BasicBlock *B,
    llvm::raw_ostream &OS,
    const llvm::DenseMap<BasicBlock *, unsigned> &bbMap) {
  OS << "L" << bbMap.find(B)->second;
}

/// Generates the correct label for BasicBlock \p B based on \p bbMap and
/// outputs it through \p OS. If the JS function name is valid c++ then it will
/// be appended to the name for searchability, otherwise just the unique number
/// alone will be used.
static void generateFunctionLabel(
    Function *F,
    llvm::raw_ostream &OS,
    const llvm::DenseMap<Function *, unsigned> &funcMap) {
  OS << "_" << funcMap.find(F)->second;

  auto name = F->getInternalNameStr();
  for (auto c : name) {
    if (!(isalnum(c) || c == '_')) {
      return;
    }
  }

  OS << "_" << name;
}

/// Constructs a map from basic blocks to their catch instruction
static void constructCatchMap(
    llvm::DenseMap<BasicBlock *, CatchInst *> &catchMap,
    Function &F) {
  // Create catchInfoMap entry for every CatchInst
  CatchInfoMap catchInfoMap{};
  for (auto &B : F) {
    for (auto &I : B) {
      if (auto CI = dyn_cast<CatchInst>(&I)) {
        catchInfoMap[CI] = CatchCoverageInfo();
      }
    }
  }

  // Populate catchInfoMap
  llvm::SmallVector<CatchInst *, 4> aliveCatches{};
  llvm::SmallPtrSet<BasicBlock *, 32> visited{};
  hermes::constructCatchMap(catchInfoMap, aliveCatches, visited, &F.front());

  // Invert catchInfoMap into a catchMap from BB to CatchInst
  // Make sure to always invert the CatchInst of highest depth
  llvm::DenseMap<BasicBlock *, uint32_t> maxDepths{};
  for (auto it : catchInfoMap) {
    auto catchInst = it.first;
    auto depth = it.second.depth;
    for (auto BB : it.second.coveredBlockList) {
      auto depthIt = maxDepths.find(BB);
      if (depthIt == maxDepths.end() || depth > depthIt->second) {
        catchMap[BB] = catchInst;
        maxDepths[BB] = depth;
      }
    }
  }
}

namespace {

class InstrGen {
  /// The ostream used to output c++ code
  llvm::raw_ostream &os_;

  /// The register allocator that was created for the current function
  hbc::HVMRegisterAllocator &ra_;

  /// A map from basic blocks to unique numbers for identification
  const llvm::DenseMap<BasicBlock *, unsigned> &bbMap_;

  /// A map from functions to unique numbers for identification
  const llvm::DenseMap<Function *, unsigned> &funcMap_;

  /// Function scope analysis of the current module
  FunctionScopeAnalysis &scopeAnalysis_;

  /// Map from basic blocks to their catch instruction
  llvm::DenseMap<BasicBlock *, CatchInst *> catchMap_;

  /// The size of this functions environment
  unsigned envSize_;

  /// Starts out true and changes to false if an error occurs
  bool &status_;

  /// Starts out at 0 and increments every time a cache index is used
  uint32_t &nextCacheIdx_;

 private:
  /// Outputs double \p d as an expression of type HermesValue
  void generateDouble(double d) {
    os_ << "HermesValue::encodeDoubleValue(makeDouble(";
    uint64_t n;
    ::memcpy(&n, &d, sizeof(n));
    os_ << n;
    os_ << "u))";
  }

  /// Converts Value \p V into a valid c++ expression and outputs it through
  /// the ostream.
  void generateValue(Value *V) {
    if (auto U = dyn_cast<LiteralUndefined>(V)) {
      os_ << "HermesValue::encodeUndefinedValue()";
    } else if (auto N = dyn_cast<LiteralNull>(V)) {
      os_ << "HermesValue::encodeNullValue()";
    } else if (auto B = dyn_cast<LiteralBool>(V)) {
      os_ << "HermesValue::encodeBoolValue(" << B->getValue() << ")";
    } else if (auto LN = dyn_cast<LiteralNumber>(V)) {
      generateDouble(LN->getValue());
    } else if (auto *I = dyn_cast<Instruction>(V)) {
      os_ << "frame.getLocalVarRef(";
      os_ << ra_.getRegister(I).getIndex();
      os_ << ")";
    } else {
      os_ << "XXX";
      status_ = false;
    }
  }

  /// Converts Value \p V into a valid c++ expression of Handle-like type.
  void generateValueOrVar(Value *V) {
    os_ << "&";
    generateValue(V);
  }

  /// Outputs \p var verbatim, type of var should be Handle-like.
  void generateValueOrVar(const char *var) {
    os_ << var;
  }

  void generateHandleException(Instruction &I) {
    auto it = catchMap_.find(I.getParent());
    if (it == catchMap_.end()) {
      os_ << "return ExecutionStatus::EXCEPTION";
    } else {
      os_ << "goto ";
      generateBasicBlockLabel(it->second->getParent(), os_, bbMap_);
    }
    os_ << ";";
  }

  /// Checks if variable \p var has exceptional status and throws if it does.
  void generateCheckResult(Instruction &I, int indent, const char *var) {
    os_.indent(indent) << "if (LLVM_UNLIKELY(" << var
                       << " == ExecutionStatus::EXCEPTION)) {\n";
    os_.indent(indent + 2);
    generateHandleException(I);
    os_ << "\n";
    os_.indent(indent) << "}\n";
  }

  /// Assigns var \p var to string \p LS and throws on exceptional status
  void generateString(
      Instruction &I,
      int indent,
      const char *var,
      LiteralString *LS) {
    os_.indent(indent) << "auto " << var
                       << " = StringPrimitive::create(runtime, ";
    os_ << serializeStr(LS->getValue().str());
    os_ << ");\n";
    generateCheckResult(I, indent, var);
  }

  void generateInstr(Instruction &I) {
    os_.indent(2);
    os_ << "XXX: ";
    I.dump();
    status_ = false;
  }

  void generateInstr(HBCLoadConstInst &I) {
    auto V = I.getSingleOperand();
    if (auto LS = dyn_cast<LiteralString>(V)) {
      os_.indent(2) << "{\n";
      generateString(I, 4, "str", LS);
      os_.indent(4);
      generateValue(&I);
      os_ << " = *str;\n";
      os_.indent(2) << "}\n";
    } else {
      os_.indent(2);
      generateValue(&I);
      os_ << " = ";
      generateValue(V);
      os_ << ";\n";
    }
  }

  void generateInstr(HBCCreateEnvironmentInst &I) {
    os_.indent(2) << "{\n";
    os_.indent(4)
        << "auto env = Environment::create(runtime, "
        << "runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), "
        << envSize_ << ");\n";
    generateCheckResult(I, 4, "env");
    os_.indent(4);
    generateValue(&I);
    os_ << " = *env;\n";
    os_.indent(2) << "}\n";
    os_.indent(2) << "marker.flush();\n";
  }

  /// Uses getNamedOrIndex to put to \p obj value \p val at key \p name
  void generateGetById(
      Instruction &I,
      Value *obj,
      Identifier name,
      bool isGlobalProperty) {
    bool isObject = obj->getType().isObjectType();
    bool canBeObjectSubtype = obj->getType().canBeObjectSubtype();

    if (isObject) {
      os_.indent(2) << "{\n";
    } else if (canBeObjectSubtype) {
      os_.indent(2) << "if (LLVM_LIKELY(";
      generateValue(obj);
      os_ << ".isObject())) {\n";
    }

    if (canBeObjectSubtype) {
      os_.indent(4) << "auto obj = Handle<JSObject>::vmcast(&";
      generateValue(obj);
      os_ << ");\n";

      auto cacheIdx = nextCacheIdx_;
      ++nextCacheIdx_;

      os_.indent(4) << "auto clazz = obj->getClass();\n";
      os_.indent(4) << "auto *cacheEntry = runtime->getCppPropertyCacheEntry("
                    << cacheIdx << ");\n";

      os_.indent(4) << "if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {\n";

      os_.indent(6);
      generateValue(&I);
      os_ << " = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);\n";

      os_.indent(4) << "} else {\n";

      os_.indent(6)
          << "auto id = runtime->getIdentifierTable().registerLazyIdentifier("
          << serializeStr(name.str()) << ");\n";

      os_.indent(6) << "NamedPropertyDescriptor desc;\n";

      os_.indent(6) << "if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast("
                    << "*obj, id, desc) && !desc.flags.accessor)) {\n";

      os_.indent(8) << "if (LLVM_LIKELY(!clazz->isDictionary())) {\n";
      os_.indent(10) << "cacheEntry->clazz = clazz;\n";
      os_.indent(10) << "cacheEntry->slot = desc.slot;\n";
      os_.indent(8) << "}\n";

      os_.indent(8);
      generateValue(&I);
      os_ << " = JSObject::getNamedSlotValue(*obj, desc);\n";

      os_.indent(6) << "} else {\n";

      os_.indent(8)
          << "auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags";
      if (isGlobalProperty)
        os_ << ".plusMustExist()";
      os_ << ");\n";
      generateCheckResult(I, 8, "res");
      os_.indent(8);

      generateValue(&I);
      os_ << " = *res;\n";

      os_.indent(6) << "}\n";

      os_.indent(4) << "}\n";
    }

    if (!canBeObjectSubtype) {
      os_.indent(2) << "{\n";
    } else if (!isObject) {
      os_.indent(2) << "} else {\n";
    }

    if (!isObject) {
      os_.indent(4)
          << "auto id = runtime->getIdentifierTable().registerLazyIdentifier("
          << serializeStr(name.str()) << ");\n";

      os_.indent(4) << "auto res = Interpreter::getByIdTransient(\n";
      os_.indent(8) << "runtime,\n";
      os_.indent(8) << "Handle<>(&";
      generateValue(obj);
      os_ << "),\n";
      os_.indent(8) << "id);\n";
      generateCheckResult(I, 4, "res");
      os_.indent(4);
      generateValue(&I);
      os_ << " = *res;\n";
    }

    os_.indent(2) << "}\n";
  }

  void generateGetByVal(Instruction &I, Value *obj, Value *property) {
    if (auto LS = dyn_cast<LiteralString>(property)) {
      generateGetById(I, obj, LS->getValue(), false);
    } else {
      bool isObject = obj->getType().isObjectType();
      bool canBeObjectSubtype = obj->getType().canBeObjectSubtype();

      if (isObject) {
        os_.indent(2) << "{\n";
      } else if (canBeObjectSubtype) {
        os_.indent(2) << "if (LLVM_LIKELY(";
        generateValue(obj);
        os_ << ".isObject())) {\n";
      }

      if (canBeObjectSubtype) {
        os_.indent(4)
            << "auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&";
        generateValue(obj);
        os_ << "), runtime, Handle<>(runtime, ";
        generateValue(property);
        os_ << "));\n";
        generateCheckResult(I, 4, "res");
        os_.indent(4);
        generateValue(&I);
        os_ << " = *res;\n";
      }

      if (!canBeObjectSubtype) {
        os_.indent(2) << "{\n";
      } else if (!isObject) {
        os_.indent(2) << "} else {\n";
      }

      if (!isObject) {
        os_.indent(4)
            << "auto res = Interpreter::getByValTransient(runtime, Handle<>(&";
        generateValue(obj);
        os_ << "), Handle<>(runtime, ";
        generateValue(property);
        os_ << "));\n";
        generateCheckResult(I, 4, "res");
        os_.indent(4);
        generateValue(&I);
        os_ << " = *res;\n";
      }

      os_.indent(2) << "}\n";
    }
    os_.indent(2) << "marker.flush();\n";
  }

  /// Uses putNamedOrIndex to put to \p obj value \p val at key \p name
  void generatePutById(
      Instruction &I,
      Value *obj,
      Identifier name,
      Value *val,
      bool isGlobalProperty) {
    bool isObject = obj->getType().isObjectType();
    bool canBeObjectSubtype = obj->getType().canBeObjectSubtype();

    if (isObject) {
      os_.indent(2) << "{\n";
    } else if (canBeObjectSubtype) {
      os_.indent(2) << "if (LLVM_LIKELY(";
      generateValue(obj);
      os_ << ".isObject())) {\n";
    }

    if (canBeObjectSubtype) {
      os_.indent(4) << "auto obj = Handle<JSObject>::vmcast(&";
      generateValue(obj);
      os_ << ");\n";

      auto cacheIdx = nextCacheIdx_;
      ++nextCacheIdx_;

      os_.indent(4) << "auto clazz = obj->getClass();\n";
      os_.indent(4) << "auto *cacheEntry = runtime->getCppPropertyCacheEntry("
                    << cacheIdx << ");\n";

      os_.indent(4) << "if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {\n";

      os_.indent(6) << "JSObject::setNamedSlotValue<PropStorage::Inline::Yes>("
                    << "*obj, runtime, cacheEntry->slot, ";
      generateValue(val);
      os_ << ");\n";

      os_.indent(4) << "} else {\n";

      os_.indent(6)
          << "auto id = runtime->getIdentifierTable().registerLazyIdentifier("
          << serializeStr(name.str()) << ");\n";

      os_.indent(6) << "NamedPropertyDescriptor desc;\n";

      os_.indent(6)
          << "if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc)"
          << " && !desc.flags.accessor && desc.flags.writable"
          << " && !desc.flags.internalSetter)) {\n";

      os_.indent(8) << "if (LLVM_LIKELY(!clazz->isDictionary())) {\n";
      os_.indent(10) << "cacheEntry->clazz = clazz;\n";
      os_.indent(10) << "cacheEntry->slot = desc.slot;\n";
      os_.indent(8) << "}\n";

      os_.indent(8) << "JSObject::setNamedSlotValue(*obj, runtime, desc.slot, ";
      generateValue(val);
      os_ << ");\n";

      os_.indent(6) << "} else {\n";

      os_.indent(8) << "auto res = JSObject::putNamed(\n";
      os_.indent(12) << "obj,\n";
      os_.indent(12) << "runtime,\n";
      os_.indent(12) << "id,\n";
      os_.indent(12) << "Handle<>(runtime, ";
      generateValue(val);
      os_ << "),\n";
      os_.indent(12) << isGlobalProperty << " && strictMode ? ";
      os_ << "defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);\n";
      generateCheckResult(I, 8, "res");

      os_.indent(6) << "}\n";

      os_.indent(4) << "}\n";
    }

    if (!canBeObjectSubtype) {
      os_.indent(2) << "{\n";
    } else if (!isObject) {
      os_.indent(2) << "} else {\n";
    }

    if (!isObject) {
      os_.indent(4)
          << "auto id = runtime->getIdentifierTable().registerLazyIdentifier("
          << serializeStr(name.str()) << ");\n";

      os_.indent(4) << "auto res = Interpreter::putByIdTransient(\n";
      os_.indent(8) << "runtime,\n";
      os_.indent(8) << "Handle<>(&";
      generateValue(obj);
      os_ << "),\n";
      os_.indent(8) << "id,\n";
      os_.indent(8) << "Handle<>(&";
      generateValue(val);
      os_ << "),\n";
      os_.indent(8) << "strictMode);\n";
      generateCheckResult(I, 4, "res");
    }

    os_.indent(2) << "}\n";
  }

  /// Uses defineNewOwnProperty to define value \p val at key \p index on object
  /// \p obj
  /// Equivalent to interpreter PutOwnByIndex
  template <typename T>
  void generatePutOwnByIndex(
      Instruction &I,
      T obj,
      uint32_t index,
      Value *val,
      int indent) {
    if (auto LS = dyn_cast<LiteralString>(val)) {
      os_.indent(indent) << "{\n";
      generateString(I, indent + 2, "str", LS);
      os_.indent(indent + 2) << "(void)JSObject::defineOwnComputedPrimitive(\n";
      os_.indent(indent + 6) << "Handle<JSObject>::vmcast(";
      generateValueOrVar(obj);
      os_ << "),\n";
      os_.indent(indent + 6) << "runtime,\n";
      os_.indent(indent + 6) << "Handle<>(runtime, ";
      generateDouble(index);
      os_ << "),\n";
      os_.indent(indent + 6)
          << "DefinePropertyFlags::getDefaultNewPropertyFlags(),\n";
      os_.indent(indent + 6) << "Handle<>(runtime, *str));\n";
      os_.indent(indent) << "}\n";
    } else {
      os_.indent(indent) << "(void)JSObject::defineOwnComputedPrimitive(\n";
      os_.indent(indent + 4) << "Handle<JSObject>::vmcast(";
      generateValueOrVar(obj);
      os_ << "),\n";
      os_.indent(indent + 4) << "runtime,\n";
      os_.indent(indent + 4) << "Handle<>(runtime, ";
      generateDouble(index);
      os_ << "),\n";
      os_.indent(indent + 4)
          << "DefinePropertyFlags::getDefaultNewPropertyFlags(),\n";
      os_.indent(indent + 4) << "Handle<>(runtime, ";
      generateValue(val);
      os_ << "));\n";
    }
  }

  void generateInstr(HBCStoreToEnvironmentInst &I) {
    auto name = I.getResolvedName();
    if (name->inGlobalScope()) {
      generatePutById(
          I,
          I.getScope(),
          name->getName(),
          I.getStoredValue(),
          isa<GlobalProperty>(name));
    } else {
      os_.indent(2);
      os_ << "vmcast<Environment>(";
      generateValue(I.getScope());
      os_ << ")->slot(";
      os_ << name->getIndexInVariableList();
      os_ << ").set(";
      generateValue(I.getStoredValue());
      os_ << ", &runtime->getHeap());\n";
    }
    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(HBCLoadFromEnvironmentInst &I) {
    auto name = I.getResolvedName();
    if (name->inGlobalScope()) {
      generateGetById(
          I, I.getScope(), name->getName(), isa<GlobalProperty>(name));
    } else {
      os_.indent(2);
      generateValue(&I);
      os_ << " = vmcast<Environment>(";
      generateValue(I.getScope());
      os_ << ")->slot(" << name->getIndexInVariableList() << ");\n";
    }
    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(ReturnInst &I) {
    os_.indent(2);
    os_ << "return ";
    generateValue(I.getValue());
    os_ << ";\n";
  }

  void generateInstr(BranchInst &I) {
    os_.indent(2);
    os_ << "goto ";
    generateBasicBlockLabel(I.getBranchDest(), os_, bbMap_);
    os_ << ";\n";
  }

  void generateInstr(CondBranchInst &I) {
    os_.indent(2);
    os_ << "if (toBoolean(";
    generateValue(I.getCondition());
    os_ << ")) { goto ";
    generateBasicBlockLabel(I.getTrueDest(), os_, bbMap_);
    os_ << "; } else { goto ";
    generateBasicBlockLabel(I.getFalseDest(), os_, bbMap_);
    os_ << "; }\n";
  }

  void generateInstr(AllocObjectInst &I) {
    os_.indent(2);
    generateValue(&I);
    os_ << " = JSObject::create(runtime).getHermesValue();\n";
    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(StorePropertyInst &I) {
    auto V = I.getProperty();
    if (auto LS = dyn_cast<LiteralString>(V)) {
      generatePutById(
          I, I.getObject(), LS->getValue(), I.getStoredValue(), false);
    } else {
      bool isObject = I.getObject()->getType().isObjectType();
      bool canBeObjectSubtype = I.getObject()->getType().canBeObjectSubtype();

      if (isObject) {
        os_.indent(2) << "{\n";
      } else if (canBeObjectSubtype) {
        os_.indent(2) << "if (LLVM_LIKELY(";
        generateValue(I.getObject());
        os_ << ".isObject())) {\n";
      }

      if (canBeObjectSubtype) {
        os_.indent(4)
            << "auto res = JSObject::putComputed(Handle<JSObject>::vmcast(&";
        generateValue(I.getObject());
        os_ << "), runtime, Handle<>(runtime, ";
        generateValue(I.getProperty());
        os_ << "), Handle<>(runtime, ";
        generateValue(I.getStoredValue());
        os_ << "), defaultPropOpFlags);\n";
        generateCheckResult(I, 4, "res");
      }

      if (!canBeObjectSubtype) {
        os_.indent(2) << "{\n";
      } else if (!isObject) {
        os_.indent(2) << "} else {\n";
      }

      if (!isObject) {
        os_.indent(4)
            << "auto res = Interpreter::putByValTransient(runtime, Handle<>(&";
        generateValue(I.getObject());
        os_ << "), Handle<>(runtime, ";
        generateValue(I.getProperty());
        os_ << "), Handle<>(runtime, ";
        generateValue(I.getStoredValue());
        os_ << "), strictMode);\n";
        generateCheckResult(I, 4, "res");
      }

      os_.indent(2) << "}\n";
    }
    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(StoreOwnPropertyInst &I) {
    auto V = I.getProperty();
    if (auto LS = dyn_cast<LiteralString>(V)) {
      bool dontUseOwn = LS->getValue() ==
          I.getParent()->getParent()->getContext().getIdentifier("__proto__");
      if (dontUseOwn) {
        generatePutById(
            I, I.getObject(), LS->getValue(), I.getStoredValue(), false);
      } else {
        os_.indent(2) << "{\n";
        os_.indent(4) << "auto res = toObject(runtime, Handle<>(&";
        generateValue(I.getObject());
        os_ << "));\n";
        generateCheckResult(I, 4, "res");
        os_.indent(4) << "JSObject::defineNewOwnProperty(\n";
        os_.indent(8) << "runtime->makeHandle<JSObject>(res.getValue()),\n";
        os_.indent(8) << "runtime,\n";
        os_.indent(8)
            << "runtime->getIdentifierTable().registerLazyIdentifier(";
        os_ << serializeStr(LS->getValue().str());
        os_ << "),\n";
        os_.indent(8) << "PropertyFlags::defaultNewNamedPropertyFlags(),\n";
        os_.indent(8) << "Handle<>(&";
        generateValue(I.getStoredValue());
        os_ << "));\n";
        os_.indent(2) << "}\n";
      }
    } else {
      // For LiteralNumber index, we know this is from array initializations.
      auto *litNum = dyn_cast<LiteralNumber>(V);
      generatePutOwnByIndex(
          I, I.getObject(), litNum->asUInt32(), I.getStoredValue(), 2);
    }
    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(LoadPropertyInst &I) {
    generateGetByVal(I, I.getObject(), I.getProperty());
  }

  void generateInstr(HBCGetGlobalObjectInst &I) {
    os_.indent(2);
    generateValue(&I);
    os_ << " = runtime->getGlobal().getHermesValue();\n";
  }

  void generateCall(CallInst &I, bool constructor) {
    os_.indent(2) << "{\n";

    os_.indent(4) << "auto callee = ";
    generateValue(I.getCallee());
    os_ << ";\n";

    os_.indent(4) << "if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {\n";
    os_.indent(6) << "runtime->raiseTypeErrorForValue("
                  << "Handle<>(runtime, callee), \" is not a function\");\n";
    os_.indent(6);
    generateHandleException(I);
    os_ << "\n";
    os_.indent(4) << "}\n";

    os_.indent(4) << "auto func = Handle<Callable>::vmcast(&callee);\n";

    // `this` is considered an argument, but we handle it separately.
    // So we only care about `getNumArguments - 1` arguments, and we offset
    // `getArgument` by 1.

    os_.indent(4);
    if (I.getNumArguments() > 1)
      os_ << "auto newFrame = ";
    os_ << "runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(";
    os_ << I.getNumArguments() - 1;
    os_ << ", *func, ";
    generateValue(I.getThis());
    os_ << ");\n";

    for (unsigned i = 0; i < I.getNumArguments() - 1; ++i) {
      os_.indent(4);
      os_ << "newFrame->getArgRef(" << i << ") = ";
      generateValue(I.getArgument(i + 1));
      os_ << ";\n";
    }

    os_.indent(4);
    os_ << "auto res = Callable::call(func, runtime, " << constructor << ");\n";

    generateCheckResult(I, 4, "res");

    os_.indent(4);
    generateValue(&I);
    os_ << " = *res;\n";

    os_.indent(4);
    os_ << "runtime->popStack(StackFrameLayout::callerOutgoingRegisters(";
    os_ << I.getNumArguments() - 1;
    os_ << "));\n";

    os_.indent(2) << "}\n";
  }

  void generateInstr(CallInst &I) {
    generateCall(I, false);
    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(HBCConstructInst &I) {
    generateCall(I, true);
    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(AllocStackInst &I) {
    // no-op: Used to reserve a register during register allocation.
  }

  void generateInstr(PhiInst &I) {
    // PhiInst has been translated into a sequence of MOVs in RegAlloc
    // Nothing to do here.
  }

  void generateInstr(DebuggerInst &I) {
    // nope
  }

  void generateInstr(HBCProfilePointInst &I) {
    // nah
  }

  void generateMov(Value *dst, Value *src) {
    if (dst != src) {
      os_.indent(2);
      generateValue(dst);
      os_ << " = ";
      generateValue(src);
      os_ << ";\n";
    }
  }

  void generateInstr(MovInst &I) {
    generateMov(&I, I.getSingleOperand());
  }

  void generateInstr(LoadStackInst &I) {
    generateMov(&I, I.getSingleOperand());
  }

  void generateInstr(HBCCreateFunctionInst &I) {
    auto F = I.getFunctionCode();

    os_.indent(2);
    os_ << "{\n";

    os_.indent(4)
        << "auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);\n";
    os_.indent(4)
        << "auto prototypeObjectHandle = "
        << "toHandle(runtime, JSObject::create(runtime, objProto));\n";

    os_.indent(4);
    os_ << "auto symbol = runtime->getIdentifierTable().getSymbolHandle(\n";
    os_.indent(8) << "runtime,\n";
    os_.indent(8);
    os_ << serializeStr(F->getOriginalOrInferredName().str());
    os_ << ")->get();\n";

    os_.indent(4)
        << "auto func = toHandle(runtime, NativeConstructor::create(\n";
    os_.indent(8) << "runtime,\n";
    os_.indent(8) << "Handle<JSObject>::vmcast(&runtime->functionPrototype),\n";
    os_.indent(8);
    os_ << "Handle<Environment>::vmcast(&";
    generateValue(I.getScope());
    os_ << "),\n";
    os_.indent(8) << "nullptr,\n";
    os_.indent(8);
    generateFunctionLabel(F, os_, funcMap_);
    os_ << ",\n";
    os_.indent(8) << "JSObject::createWithException,\n";
    os_.indent(8) << "CellKind::ObjectKind));\n";

    os_.indent(4) << "(void)Callable::defineNameLengthAndPrototype(\n";
    os_.indent(8) << "func,\n";
    os_.indent(8) << "runtime,\n";
    os_.indent(8) << "symbol,\n";
    os_.indent(8) << F->getParameters().size() << ",\n";
    os_.indent(8) << "prototypeObjectHandle,\n";
    os_.indent(8) << "false,\n";
    os_.indent(8) << F->isStrictMode() << ");\n";

    os_.indent(4);
    generateValue(&I);
    os_ << " = func.getHermesValue();\n";

    os_.indent(2);
    os_ << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(HBCResolveEnvironment &I) {
    llvm::Optional<int32_t> instrScopeDepth =
        scopeAnalysis_.getScopeDepth(I.getScope());
    llvm::Optional<int32_t> curScopeDepth =
        scopeAnalysis_.getScopeDepth(I.getParent()->getParent());
    int32_t delta = curScopeDepth.getValue() - instrScopeDepth.getValue() - 1;

    os_.indent(2) << "{\n";

    // Repeatedly moves up curEnv `delta` times to get to the desired
    // environment
    os_.indent(4);
    os_ << "Environment *curEnv = frame.getCalleeClosure()->getEnvironment();\n";
    os_.indent(4);
    os_ << "for (unsigned level = " << delta << "; level; --level) {\n";
    os_.indent(6) << "curEnv = curEnv->getParentEnvironment();\n";
    os_.indent(4) << "}\n";
    os_.indent(4);
    generateValue(&I);
    os_ << " = HermesValue::encodeObjectValue(curEnv);\n";

    os_.indent(2) << "}\n";
  }

  void generateInstr(HBCLoadParamInst &I) {
    int32_t idx = I.getIndex()->asInt32() - 1;
    os_.indent(2);
    generateValue(&I);
    if (idx >= 0) {
      os_ << " = args.getArg(" << idx << ");\n";
    } else {
      assert(idx == -1 && "HBCLoadParamInst operand should not be negative");
      os_ << " = args.getThisArg();\n";
    }
  }

  void generateInstr(HBCGetArgumentsPropByValInst &I) {
    bool strictMode = I.getParent()->getParent()->isStrictMode();

    os_.indent(2) << "{\n";

    os_.indent(4) << "bool done = 0;\n";
    os_.indent(4) << "if (";
    generateValue(I.getLazyRegister());
    os_ << ".isUndefined()) {\n";
    os_.indent(6) << "if (auto index = toArrayIndexFastPath(";
    generateValue(I.getIndex());
    os_ << ")) {\n";
    os_.indent(8) << "if (*index < frame.getArgCount()) {\n";
    os_.indent(10);
    generateValue(&I);
    os_ << " = frame.getArgRef(*index);\n";
    os_.indent(10);
    os_ << "done = true;\n";
    os_.indent(8) << "}\n";
    os_.indent(6) << "}\n";
    os_.indent(4) << "}\n";

    os_.indent(4) << "if (!done) {\n";

    os_.indent(6) << "auto res = Interpreter::getArgumentsPropByValSlowPath(\n";
    os_.indent(10) << "runtime,\n";
    os_.indent(10) << "&";
    generateValue(I.getLazyRegister());
    os_ << ",\n";
    os_.indent(10) << "&";
    generateValue(I.getIndex());
    os_ << ",\n";
    os_.indent(10) << "frame.getCalleeClosureHandle(),\n";
    os_.indent(10) << strictMode << ");\n";

    generateCheckResult(I, 6, "res");

    os_.indent(6);
    generateValue(&I);
    os_ << " = *res;\n";

    os_.indent(4) << "}\n";

    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(HBCGetArgumentsLengthInst &I) {
    os_.indent(2);
    os_ << "if (";
    generateValue(I.getLazyRegister());
    os_ << ".isUndefined()) {\n";
    os_.indent(4);
    generateValue(&I);
    os_ << " = HermesValue::encodeNumberValue(frame.getArgCount());\n";
    os_.indent(2) << "} else {\n";

    os_.indent(4) << "auto res = JSObject::getNamedOrIndexed(\n";
    os_.indent(8) << "Handle<JSObject>::vmcast(&";
    generateValue(I.getLazyRegister());
    os_ << "),\n";
    os_.indent(8) << "runtime,\n";
    os_.indent(8) << "runtime->getPredefinedSymbolID(Predefined::length));\n";

    generateCheckResult(I, 4, "res");

    os_.indent(4);
    generateValue(&I);
    os_ << " = *res;\n";

    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(HBCReifyArgumentsInst &I) {
    os_.indent(2) << "if (";
    generateValue(I.getLazyRegister());
    os_ << ".isUndefined()) {\n";

    os_.indent(4) << "auto argRes = Interpreter::reifyArgumentsSlowPath("
                  << "runtime, frame.getCalleeClosureHandle(), strictMode);\n";

    generateCheckResult(I, 4, "argRes");
    os_.indent(4);
    generateValue(I.getLazyRegister());
    os_ << " = *argRes;\n";

    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(TryStartInst &I) {
    os_.indent(2);
    os_ << "goto ";
    generateBasicBlockLabel(I.getTryBody(), os_, bbMap_);
    os_ << ";\n";
  }

  void generateInstr(TryEndInst &I) {
    // no-op
  }

  void generateInstr(CatchInst &I) {
    os_.indent(2);
    generateValue(&I);
    os_ << " = runtime->getThrownValue();\n";

    os_.indent(2);
    os_ << "runtime->clearThrownValue();\n";
  }

  void generateInstr(ThrowInst &I) {
    os_.indent(2);
    os_ << "runtime->setThrownValue(";
    generateValue(I.getThrownValue());
    os_ << ");\n";

    os_.indent(2);
    generateHandleException(I);
    os_ << "\n";
  }

  void generateInstr(StoreGetterSetterInst &I) {
    os_.indent(2) << "{\n";

    os_.indent(4)
        << "auto res = putGetterSetter(runtime, Handle<JSObject>::vmcast(&";
    generateValue(I.getObject());
    os_ << "), runtime->getIdentifierTable().registerLazyIdentifier("
        << serializeStr(I.getProperty()->getValue().str())
        << "), Handle<>::vmcast(&";
    generateValue(I.getStoredGetter());
    os_ << "), Handle<>::vmcast(&";
    generateValue(I.getStoredSetter());
    os_ << "));\n";

    generateCheckResult(I, 4, "res");

    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  /// Calls \p oper on the two operands, after converting them to numbers if
  /// needed. If \p isBothNumber is true they are assumed to be numbers.
  /// If \p isFunction is true then \p oper must be a function and is called
  /// with the two operands as arguments.
  /// The result is then assigned to the instruction's register.
  void generateBinOp(
      BinaryOperatorInst &I,
      const char *oper,
      const bool isFunction = false) {
    bool isBothNumber = I.getLeftHandSide()->getType().isNumberType() &&
        I.getRightHandSide()->getType().isNumberType();
    if (isBothNumber) {
      os_.indent(2) << "{\n";
    } else {
      os_.indent(2) << "if (LLVM_LIKELY(";
      generateValue(I.getLeftHandSide());
      os_ << ".isNumber() && ";
      generateValue(I.getRightHandSide());
      os_ << ".isNumber())) {\n";
    }

    os_.indent(4);
    generateValue(&I);
    os_ << " = HermesValue::encodeDoubleValue(";
    if (isFunction) {
      os_ << oper << "(";
      generateValue(I.getLeftHandSide());
      os_ << ".getNumber(), ";
      generateValue(I.getRightHandSide());
      os_ << ".getNumber())";
    } else {
      generateValue(I.getLeftHandSide());
      os_ << ".getNumber() " << oper << " ";
      generateValue(I.getRightHandSide());
      os_ << ".getNumber()";
    }
    os_ << ");\n";

    if (!isBothNumber) {
      os_.indent(2) << "} else {\n";

      os_.indent(4) << "auto leftRes = toNumber(runtime, Handle<>(&";
      generateValue(I.getLeftHandSide());
      os_ << "));\n";
      generateCheckResult(I, 4, "leftRes");

      os_.indent(4) << "auto rightRes = toNumber(runtime, Handle<>(&";
      generateValue(I.getRightHandSide());
      os_ << "));\n";
      generateCheckResult(I, 4, "rightRes");

      os_.indent(4);
      generateValue(&I);
      os_ << " = HermesValue::encodeDoubleValue(";
      if (isFunction) {
        os_ << oper << "(leftRes->getDouble(), rightRes->getDouble()));\n";
      } else {
        os_ << "leftRes->getDouble() " << oper << " rightRes->getDouble());\n";
      }

      os_.indent(4) << "marker.flush();\n";
    }

    os_.indent(2) << "}\n";
  }

  /// Calls \p oper on the two operands, after converting them to numbers if
  /// needed. If \p isBothNumber is true they are assumed to be numbers.
  /// The input numbers are also converted to Int32's
  /// The result is then assigned to the instruction's register.
  void generateBitwiseBinOp(BinaryOperatorInst &I, const char *oper) {
    bool isBothNumber = I.getLeftHandSide()->getType().isNumberType() &&
        I.getRightHandSide()->getType().isNumberType();
    if (isBothNumber) {
      os_.indent(2) << "{\n";
    } else {
      os_.indent(2) << "if (LLVM_LIKELY(";
      generateValue(I.getLeftHandSide());
      os_ << ".isNumber() && ";
      generateValue(I.getRightHandSide());
      os_ << ".isNumber())) {\n";
    }

    os_.indent(4);
    generateValue(&I);
    os_ << " = HermesValue::encodeDoubleValue(hermes::truncateToInt32(";
    generateValue(I.getLeftHandSide());
    os_ << ".getNumber()) " << oper << " hermes::truncateToInt32(";
    generateValue(I.getRightHandSide());
    os_ << ".getNumber()));\n";

    if (!isBothNumber) {
      os_.indent(2) << "} else {\n";

      os_.indent(4) << "auto leftRes = toInt32(runtime, Handle<>(&";
      generateValue(I.getLeftHandSide());
      os_ << "));\n";
      generateCheckResult(I, 4, "leftRes");

      os_.indent(4) << "auto rightRes = toInt32(runtime, Handle<>(&";
      generateValue(I.getRightHandSide());
      os_ << "));\n";
      generateCheckResult(I, 4, "rightRes");

      os_.indent(4);
      generateValue(&I);
      os_ << " = HermesValue::encodeDoubleValue("
          << "leftRes->getNumberAs<int32_t>() " << oper
          << " rightRes->getNumberAs<int32_t>());\n";

      os_.indent(4) << "marker.flush();\n";
    }

    os_.indent(2) << "}\n";
  }

  /// Calls \p oper on the two operands, after converting them to numbers if
  /// needed. If \p isBothNumber is true they are assumed to be numbers.
  /// The input numbers are converted and casted according to \p lConv \p lType
  /// and \p retType.
  /// The result is then assigned to the instruction's register.
  void generateShiftOp(
      BinaryOperatorInst &I,
      const char *oper,
      const char *lConv,
      const char *lType,
      const char *retType) {
    bool isBothNumber = I.getLeftHandSide()->getType().isNumberType() &&
        I.getRightHandSide()->getType().isNumberType();
    if (isBothNumber) {
      os_.indent(2) << "{\n";
    } else {
      os_.indent(2) << "if (LLVM_LIKELY(";
      generateValue(I.getLeftHandSide());
      os_ << ".isNumber() && ";
      generateValue(I.getRightHandSide());
      os_ << ".isNumber())) {\n";
    }

    os_.indent(4) << "auto lnum = static_cast<" << lType
                  << ">(hermes::truncateToInt32(";
    generateValue(I.getLeftHandSide());
    os_ << ".getNumber()));\n";

    os_.indent(4)
        << "auto rnum = static_cast<uint32_t>(hermes::truncateToInt32(";
    generateValue(I.getRightHandSide());
    os_ << ".getNumber())) & 0x1f;\n";

    os_.indent(4);
    generateValue(&I);
    os_ << " = HermesValue::encodeDoubleValue(static_cast<" << retType
        << ">(lnum " << oper << " rnum));\n";

    if (!isBothNumber) {
      os_.indent(2) << "} else {\n";

      os_.indent(4);
      os_ << "auto leftRes = " << lConv << "(runtime, Handle<>(&";
      generateValue(I.getLeftHandSide());
      os_ << "));\n";
      generateCheckResult(I, 4, "leftRes");
      os_.indent(4) << "auto left = static_cast<" << lType
                    << ">(leftRes->getNumber());\n";

      os_.indent(4) << "auto rightRes = toUInt32(runtime, Handle<>(&";
      generateValue(I.getRightHandSide());
      os_ << "));\n";
      generateCheckResult(I, 4, "rightRes");
      os_.indent(4)
          << "auto right = static_cast<uint32_t>(rightRes->getNumber()) & 0x1f;\n";

      os_.indent(4);
      generateValue(&I);
      os_ << " = HermesValue::encodeDoubleValue(static_cast<" << retType
          << ">(left " << oper << " right));\n";

      os_.indent(4) << "marker.flush();\n";
    }

    os_.indent(2) << "}\n";
  }

  /// Calls \p func with `runtime` as the first argument, and the two operands
  /// as the second and third arguments.
  /// The function must return a `CallResult` of some kind, which is then
  /// checked, throwing if the status was exceptional.
  /// The result is then assigned to the instruction's register, prepending
  /// \p preRes and appending \p postRes to the `res` variable to allow for
  /// further modification.
  void generateBinCall(
      BinaryOperatorInst &I,
      const char *func,
      const char *preRes,
      const char *postRes) {
    os_.indent(2) << "{\n";

    os_.indent(4) << "auto res = " << func << "(runtime, Handle<>(&";
    generateValue(I.getLeftHandSide());
    os_ << "), Handle<>(&";
    generateValue(I.getRightHandSide());
    os_ << "));\n";

    generateCheckResult(I, 4, "res");

    os_.indent(4);
    generateValue(&I);
    os_ << " = " << preRes << "res" << postRes << ";\n";

    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  /// Calls \p oper with the two operands as the two arguments if they are both
  /// numbers, otherwise calls \p func with `runtime` as the first argument,
  /// and the two operands as the second and third arguments.
  /// The function must return a `CallResult` of some kind, which is then
  /// checked, throwing if the status was exceptional.
  /// The result is then assigned to the instruction's register.
  void
  generateCompOp(BinaryOperatorInst &I, const char *oper, const char *func) {
    bool isBothNumber = I.getLeftHandSide()->getType().isNumberType() &&
        I.getRightHandSide()->getType().isNumberType();
    if (isBothNumber) {
      os_.indent(2) << "{\n";
    } else {
      os_.indent(2) << "if (LLVM_LIKELY(";
      generateValue(I.getLeftHandSide());
      os_ << ".isNumber() && ";
      generateValue(I.getRightHandSide());
      os_ << ".isNumber())) {\n";
    }

    os_.indent(4);
    generateValue(&I);
    os_ << " = HermesValue::encodeBoolValue(";
    generateValue(I.getLeftHandSide());
    os_ << ".getNumber() " << oper << " ";
    generateValue(I.getRightHandSide());
    os_ << ".getNumber());\n";

    if (!isBothNumber) {
      os_.indent(2) << "} else {\n";

      os_.indent(4) << "auto res = " << func << "(runtime, Handle<>(&";
      generateValue(I.getLeftHandSide());
      os_ << "), Handle<>(&";
      generateValue(I.getRightHandSide());
      os_ << "));\n";

      generateCheckResult(I, 4, "res");

      os_.indent(4);
      generateValue(&I);
      os_ << " = res.getValue();\n";

      os_.indent(4) << "marker.flush();\n";
    }

    os_.indent(2) << "}\n";
  }

  void generateStrictEq(BinaryOperatorInst &I, bool invert) {
    os_.indent(2);
    generateValue(&I);
    os_ << " = HermesValue::encodeBoolValue(";
    if (invert)
      os_ << "!";
    os_ << "strictEqualityTest(";
    generateValue(I.getLeftHandSide());
    os_ << ", ";
    generateValue(I.getRightHandSide());
    os_ << "));\n";
  }

  void generateInstr(BinaryOperatorInst &I) {
    bool isBothNumber = I.getLeftHandSide()->getType().isNumberType() &&
        I.getRightHandSide()->getType().isNumberType();
    switch (I.getOperatorKind()) {
      case BinaryOperatorInst::OpKind::EqualKind:
        generateBinCall(I, "abstractEqualityTest", "", ".getValue()");
        break;
      case BinaryOperatorInst::OpKind::NotEqualKind:
        generateBinCall(
            I,
            "abstractEqualityTest",
            "HermesValue::encodeBoolValue(!",
            "->getBool())");
        break;

      case BinaryOperatorInst::OpKind::StrictlyEqualKind:
        generateStrictEq(I, false);
        break;
      case BinaryOperatorInst::OpKind::StrictlyNotEqualKind:
        generateStrictEq(I, true);
        break;

      case BinaryOperatorInst::OpKind::LessThanKind:
        generateCompOp(I, "<", "lessOp");
        break;
      case BinaryOperatorInst::OpKind::LessThanOrEqualKind:
        generateCompOp(I, "<=", "lessEqualOp");
        break;
      case BinaryOperatorInst::OpKind::GreaterThanKind:
        generateCompOp(I, ">", "greaterOp");
        break;
      case BinaryOperatorInst::OpKind::GreaterThanOrEqualKind:
        generateCompOp(I, ">=", "greaterEqualOp");
        break;

      case BinaryOperatorInst::OpKind::LeftShiftKind:
        generateShiftOp(I, "<<", "toUInt32", "uint32_t", "int32_t");
        break;
      case BinaryOperatorInst::OpKind::RightShiftKind:
        generateShiftOp(I, ">>", "toInt32", "int32_t", "int32_t");
        break;
      case BinaryOperatorInst::OpKind::UnsignedRightShiftKind:
        generateShiftOp(I, ">>", "toUInt32", "uint32_t", "uint32_t");
        break;

      case BinaryOperatorInst::OpKind::AddKind:
        if (isBothNumber) {
          os_.indent(2) << "{\n";
        } else {
          os_.indent(2) << "if (LLVM_LIKELY(";
          generateValue(I.getLeftHandSide());
          os_ << ".isNumber() && ";
          generateValue(I.getRightHandSide());
          os_ << ".isNumber())) {\n";
        }
        os_.indent(4);
        generateValue(&I);
        os_ << " = HermesValue::encodeDoubleValue(";
        generateValue(I.getLeftHandSide());
        os_ << ".getNumber() + ";
        generateValue(I.getRightHandSide());
        os_ << ".getNumber());\n";
        if (!isBothNumber) {
          os_.indent(2) << "} else {\n";
          os_.indent(4) << "auto res = addOp(runtime, "
                        << "Handle<>(&";
          generateValue(I.getLeftHandSide());
          os_ << "), Handle<>(&";
          generateValue(I.getRightHandSide());
          os_ << "));\n";
          generateCheckResult(I, 4, "res");
          os_.indent(4);
          generateValue(&I);
          os_ << " = *res;\n";
          os_.indent(4) << "marker.flush();\n";
        }
        os_.indent(2) << "}\n";
        break;

      case BinaryOperatorInst::OpKind::SubtractKind:
        generateBinOp(I, "-");
        break;
      case BinaryOperatorInst::OpKind::MultiplyKind:
        generateBinOp(I, "*");
        break;
      case BinaryOperatorInst::OpKind::DivideKind:
        generateBinOp(I, "/");
        break;
      case BinaryOperatorInst::OpKind::ModuloKind:
        generateBinOp(I, "std::fmod", true);
        break;

      case BinaryOperatorInst::OpKind::OrKind:
        generateBitwiseBinOp(I, "|");
        break;
      case BinaryOperatorInst::OpKind::XorKind:
        generateBitwiseBinOp(I, "^");
        break;
      case BinaryOperatorInst::OpKind::AndKind:
        generateBitwiseBinOp(I, "&");
        break;

      case BinaryOperatorInst::OpKind::InKind:
        os_.indent(2) << "{\n";
        os_.indent(4) << "auto res = getIsIn(runtime, Handle<>(&";
        generateValue(I.getLeftHandSide());
        os_ << "), Handle<>(&";
        generateValue(I.getRightHandSide());
        os_ << "));\n";
        generateCheckResult(I, 4, "res");
        os_.indent(4);
        generateValue(&I);
        os_ << " = *res;\n";
        os_.indent(2) << "}\n";
        os_.indent(2) << "marker.flush();\n";
        break;

      case BinaryOperatorInst::OpKind::InstanceOfKind:
        generateBinCall(I, "instanceOf", "HermesValue::encodeBoolValue(*", ")");
        break;

      default:
        os_.indent(2);
        os_ << "XXX: ";
        I.dump();
        status_ = false;
    }
  }

  /// Calls \p func with `runtime` as the first argument, and the two operands
  /// as the second and third arguments.
  /// The function must return a `CallResult` of some kind, which is then
  /// checked, throwing if the status was exceptional.
  /// The result is then converted to a boolean and the appropriate destination
  /// is jumped to, inverting the destination if \p invert is true.
  void
  generateBinCallBranch(CompareBranchInst &I, const char *func, bool invert) {
    os_.indent(2) << "{\n";

    os_.indent(4) << "auto res = " << func << "(runtime, Handle<>(&";
    generateValue(I.getLeftHandSide());
    os_ << "), Handle<>(&";
    generateValue(I.getRightHandSide());
    os_ << "));\n";

    generateCheckResult(I, 4, "res");

    os_.indent(4) << "if (";
    if (invert)
      os_ << "!";
    os_ << "toBoolean(*res)) { goto ";
    generateBasicBlockLabel(I.getTrueDest(), os_, bbMap_);
    os_ << "; } else { goto ";
    generateBasicBlockLabel(I.getFalseDest(), os_, bbMap_);
    os_ << "; }\n";
    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  /// Calls \p oper with the two operands as the two arguments if they are both
  /// numbers, otherwise calls \p func with `runtime` as the first argument,
  /// and the two operands as the second and third arguments.
  /// The function must return a `CallResult` of some kind, which is then
  /// checked, throwing if the status was exceptional.
  /// The operator must return a `bool`.
  /// The result is then converted to a boolean and the appropriate destination
  /// is jumped to, inverting the destination if \p invert is true.
  void generateBinCallBranchFast(
      CompareBranchInst &I,
      const char *oper,
      const char *func,
      const bool invert) {
    bool isBothNumber = I.getLeftHandSide()->getType().isNumberType() &&
        I.getRightHandSide()->getType().isNumberType();
    if (isBothNumber) {
      os_.indent(2) << "{\n";
    } else {
      os_.indent(2) << "if (LLVM_LIKELY(";
      generateValue(I.getLeftHandSide());
      os_ << ".isNumber() && ";
      generateValue(I.getRightHandSide());
      os_ << ".isNumber())) {\n";
    }

    os_.indent(4) << "if (";
    if (invert)
      os_ << "!";
    os_ << "(";
    generateValue(I.getLeftHandSide());
    os_ << ".getNumber() " << oper << " ";
    generateValue(I.getRightHandSide());
    os_ << ".getNumber())) { goto ";
    generateBasicBlockLabel(I.getTrueDest(), os_, bbMap_);
    os_ << "; } else { goto ";
    generateBasicBlockLabel(I.getFalseDest(), os_, bbMap_);
    os_ << "; }\n";

    if (!isBothNumber) {
      os_.indent(2) << "} else {\n";

      os_.indent(4) << "auto res = " << func << "(runtime, Handle<>(&";
      generateValue(I.getLeftHandSide());
      os_ << "), Handle<>(&";
      generateValue(I.getRightHandSide());
      os_ << "));\n";

      generateCheckResult(I, 4, "res");

      os_.indent(4) << "if (";
      if (invert)
        os_ << "!";
      os_ << "toBoolean(*res)) { goto ";
      generateBasicBlockLabel(I.getTrueDest(), os_, bbMap_);
      os_ << "; } else { goto ";
      generateBasicBlockLabel(I.getFalseDest(), os_, bbMap_);
      os_ << "; }\n";
      os_.indent(4) << "marker.flush();\n";
    }

    os_.indent(2) << "}\n";
  }

  /// Calls strictEqualityTest with the two operands as the two arguments.
  /// The result is then used to jump to the appropriate destination, inverting
  /// the destination if \p invert is true.
  void generateStrictEqBranch(CompareBranchInst &I, bool invert) {
    os_.indent(2) << "if (";
    if (invert)
      os_ << "!";
    os_ << "strictEqualityTest(";
    generateValue(I.getLeftHandSide());
    os_ << ", ";
    generateValue(I.getRightHandSide());
    os_ << ")) {\n";
    os_.indent(4) << "goto ";
    generateBasicBlockLabel(I.getTrueDest(), os_, bbMap_);
    os_ << ";\n";
    os_.indent(2) << "} else {\n";
    os_.indent(4) << "goto ";
    generateBasicBlockLabel(I.getFalseDest(), os_, bbMap_);
    os_ << ";\n";
    os_.indent(2) << "}\n";
  }

  void generateInstr(CompareBranchInst &I) {
    switch (I.getOperatorKind()) {
      case BinaryOperatorInst::OpKind::EqualKind:
        generateBinCallBranch(I, "abstractEqualityTest", false);
        break;
      case BinaryOperatorInst::OpKind::NotEqualKind:
        generateBinCallBranch(I, "abstractEqualityTest", true);
        break;

      case BinaryOperatorInst::OpKind::StrictlyEqualKind:
        generateStrictEqBranch(I, false);
        break;
      case BinaryOperatorInst::OpKind::StrictlyNotEqualKind:
        generateStrictEqBranch(I, true);
        break;

      case BinaryOperatorInst::OpKind::LessThanKind:
        generateBinCallBranchFast(I, "<", "lessOp", false);
        break;
      case BinaryOperatorInst::OpKind::LessThanOrEqualKind:
        generateBinCallBranchFast(I, "<=", "lessEqualOp", false);
        break;
      case BinaryOperatorInst::OpKind::GreaterThanKind:
        generateBinCallBranchFast(I, ">", "greaterOp", false);
        break;
      case BinaryOperatorInst::OpKind::GreaterThanOrEqualKind:
        generateBinCallBranchFast(I, ">=", "greaterEqualOp", false);
        break;

      default:
        os_.indent(2);
        os_ << "XXX: ";
        I.dump();
        status_ = false;
    }
  }

  void generateInstr(UnaryOperatorInst &I) {
    bool isNumber = I.getSingleOperand()->getType().isNumberType();
    switch (I.getOperatorKind()) {
      case UnaryOperatorInst::OpKind::VoidKind:
        os_.indent(2);
        generateValue(&I);
        os_ << " = HermesValue::encodeUndefinedValue();\n";
        break;

      case UnaryOperatorInst::OpKind::TypeofKind:
        os_.indent(2);
        generateValue(&I);
        os_ << " = typeOf(runtime, Handle<>(&";
        generateValue(I.getSingleOperand());
        os_ << "));\n";
        os_.indent(2) << "marker.flush();\n";
        break;

      case UnaryOperatorInst::OpKind::MinusKind:
        if (isNumber) {
          os_.indent(2) << "{\n";
        } else {
          os_.indent(2) << "if (LLVM_LIKELY(";
          generateValue(I.getSingleOperand());
          os_ << ".isNumber())) {\n";
        }
        os_.indent(4);
        generateValue(&I);
        os_ << " = HermesValue::encodeDoubleValue(-";
        generateValue(I.getSingleOperand());
        os_ << ".getNumber());\n";
        if (!isNumber) {
          os_.indent(2) << "} else {\n";
          os_.indent(4) << "auto res = toNumber(runtime, Handle<>(&";
          generateValue(I.getSingleOperand());
          os_ << "));\n";
          generateCheckResult(I, 4, "res");
          os_.indent(4);
          generateValue(&I);
          os_ << " = HermesValue::encodeDoubleValue(-res->getNumber());\n";
          os_.indent(4) << "marker.flush();\n";
        }
        os_.indent(2) << "}\n";
        break;

      case UnaryOperatorInst::OpKind::TildeKind:
        if (isNumber) {
          os_.indent(2) << "{\n";
        } else {
          os_.indent(2) << "if (LLVM_LIKELY(";
          generateValue(I.getSingleOperand());
          os_ << ".isNumber())) {\n";
        }
        os_.indent(4);
        generateValue(&I);
        os_ << " = HermesValue::encodeDoubleValue(~hermes::truncateToInt32(";
        generateValue(I.getSingleOperand());
        os_ << ".getNumber()));\n";
        if (!isNumber) {
          os_.indent(2) << "} else {\n";
          os_.indent(4) << "auto res = toInt32(runtime, Handle<>(&";
          generateValue(I.getSingleOperand());
          os_ << "));\n";
          generateCheckResult(I, 4, "res");
          os_.indent(4);
          generateValue(&I);
          os_ << " = HermesValue::encodeDoubleValue("
              << "~res->getNumberAs<int32_t>());\n";
          os_.indent(4) << "marker.flush();\n";
        }
        os_.indent(2) << "}\n";
        break;

      case UnaryOperatorInst::OpKind::BangKind:
        os_.indent(2);
        generateValue(&I);
        os_ << " = HermesValue::encodeBoolValue(!toBoolean(";
        generateValue(I.getSingleOperand());
        os_ << "));\n";
        break;

      default:
        os_.indent(2);
        os_ << "XXX: ";
        I.dump();
        status_ = false;
    }
  }

  void generateInstr(AsNumberInst &I) {
    os_.indent(2) << "if (LLVM_LIKELY(";
    generateValue(I.getSingleOperand());
    os_ << ".isNumber())) {\n";

    os_.indent(4);
    generateValue(&I);
    os_ << " = ";
    generateValue(I.getSingleOperand());
    os_ << ";\n";

    os_.indent(2) << "} else {\n";

    os_.indent(4) << "auto res = toNumber(runtime, Handle<>(&";
    generateValue(I.getSingleOperand());
    os_ << "));\n";
    generateCheckResult(I, 4, "res");
    os_.indent(4);
    generateValue(&I);
    os_ << " = res.getValue();\n";

    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(DeletePropertyInst &I) {
    bool isObject = I.getObject()->getType().isObjectType();
    bool canBeObjectSubtype = I.getObject()->getType().canBeObjectSubtype();

    if (isObject) {
      os_.indent(2) << "{\n";
    } else if (canBeObjectSubtype) {
      os_.indent(2) << "if (LLVM_LIKELY(";
      generateValue(I.getObject());
      os_ << ".isObject())) {\n";
    }

    if (canBeObjectSubtype) {
      os_.indent(4);
      if (auto *LS = dyn_cast<LiteralString>(I.getProperty())) {
        os_ << "auto status = JSObject::deleteNamed(\n";
      } else {
        os_ << "auto status = JSObject::deleteComputed(\n";
      }
      os_.indent(8) << "Handle<JSObject>::vmcast(&";
      generateValue(I.getObject());
      os_ << "),\n";
      os_.indent(8) << "runtime,\n";
      os_.indent(8);
      if (auto *LS = dyn_cast<LiteralString>(I.getProperty())) {
        os_ << "runtime->getIdentifierTable().registerLazyIdentifier("
            << serializeStr(LS->getValue().str()) << "),\n";
      } else {
        os_ << "Handle<>(&";
        generateValue(I.getProperty());
        os_ << "),\n";
      }
      os_.indent(8) << "defaultPropOpFlags);\n";

      generateCheckResult(I, 4, "status");

      os_.indent(4);
      generateValue(&I);
      os_ << " = HermesValue::encodeBoolValue(status.getValue());\n";
    }

    if (!canBeObjectSubtype) {
      os_.indent(2) << "{\n";
    } else if (!isObject) {
      os_.indent(2) << "} else {\n";
    }

    if (!isObject) {
      os_.indent(4) << "auto res = toObject(runtime, Handle<>(&";
      generateValue(I.getObject());
      os_ << "));\n";

      if (auto *LS = dyn_cast<LiteralString>(I.getProperty())) {
        // If an exception is thrown, likely we are trying to convert
        // undefined/null to an object. Passing over the name of the property
        // so that we could emit more meaningful error messages.
        os_.indent(4)
            << "if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {\n";
        os_.indent(6)
            << "(void)amendPropAccessErrorMsgWithPropName(runtime, Handle<>(&";
        generateValue(I.getObject());
        os_ << "), \"delete\", runtime->getIdentifierTable().registerLazyIdentifier("
            << serializeStr(LS->getValue().str()) << "));\n";
        os_.indent(4);
        generateHandleException(I);
        os_ << "\n";
        os_.indent(4) << "}\n";
      } else {
        generateCheckResult(I, 4, "res");
      }

      os_.indent(4);
      if (auto *LS = dyn_cast<LiteralString>(I.getProperty())) {
        os_ << "auto status = JSObject::deleteNamed(\n";
      } else {
        os_ << "auto status = JSObject::deleteComputed(\n";
      }
      os_.indent(8) << "runtime->makeHandle<JSObject>(res.getValue()),\n";
      os_.indent(8) << "runtime,\n";
      os_.indent(8);
      if (auto *LS = dyn_cast<LiteralString>(I.getProperty())) {
        os_ << "runtime->getIdentifierTable().registerLazyIdentifier("
            << serializeStr(LS->getValue().str()) << "),\n";
      } else {
        os_ << "Handle<>(&";
        generateValue(I.getProperty());
        os_ << "),\n";
      }
      os_.indent(8) << "defaultPropOpFlags);\n";

      generateCheckResult(I, 4, "status");

      os_.indent(4);
      generateValue(&I);
      os_ << " = HermesValue::encodeBoolValue(status.getValue());\n";
    }

    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(HBCGetThisNSInst &I) {
    os_.indent(2) << "if (LLVM_LIKELY(frame.getThisArgRef().isObject())) {\n";
    os_.indent(4);
    generateValue(&I);
    os_ << " = frame.getThisArgRef();\n";

    os_.indent(2) << "} else if (\n";
    os_.indent(6) << "frame.getThisArgRef().isNull() ||\n";
    os_.indent(6) << "frame.getThisArgRef().isUndefined()) {\n";
    os_.indent(4);
    generateValue(&I);
    os_ << " = runtime->getGlobal().getHermesValue();\n";

    os_.indent(2) << "} else {\n";
    os_.indent(4)
        << "auto res = toObject(runtime, Handle<>::vmcast(&frame.getThisArgRef()));\n";
    generateCheckResult(I, 4, "res");
    os_.indent(4);
    generateValue(&I);
    os_ << " = res.getValue();\n";

    os_.indent(2);
    os_ << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(AllocArrayInst &I) {
    os_.indent(2) << "{\n";

    auto sizeHint = std::min((unsigned)UINT16_MAX, I.getSizeHint()->asUInt32());

    os_.indent(4) << "auto res = JSArray::create(runtime, ";
    os_ << sizeHint << ", " << sizeHint << ");\n";
    generateCheckResult(I, 4, "res");
    os_.indent(4) << "auto obj = toHandle(runtime, std::move(*res));\n";

    auto elementCount = I.getElementCount();
    if (elementCount > 0) {
      for (unsigned i = 0, e = elementCount; i < e; ++i) {
        generatePutOwnByIndex(I, "obj", i, I.getArrayElement(i), 4);
      }
    }

    os_.indent(4);
    generateValue(&I);
    os_ << " = obj.getHermesValue();\n";

    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(HBCCreateThisInst &I) {
    os_.indent(2) << "{\n";

    os_.indent(4) << "if (LLVM_UNLIKELY(!vmisa<Callable>(";
    generateValue(I.getClosure());
    os_ << "))) {\n";
    os_.indent(6)
        << "runtime->raiseTypeError(\"constructor is not callable\");\n";
    os_.indent(6);
    generateHandleException(I);
    os_ << "\n";
    os_.indent(4) << "}\n";

    os_.indent(4) << "auto crtRes = Callable::newObject(\n";
    os_.indent(8) << "Handle<Callable>::vmcast(&";
    generateValue(I.getClosure());
    os_ << "),\n";
    os_.indent(8) << "runtime,\n";
    os_.indent(8) << "Handle<JSObject>::vmcast(";
    generateValue(I.getPrototype());
    os_ << ".isObject() ? &";
    generateValue(I.getPrototype());
    os_ << " : &runtime->objectPrototype));\n";

    generateCheckResult(I, 4, "crtRes");

    os_.indent(4);
    generateValue(&I);
    os_ << " = *crtRes;\n";

    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(HBCGetConstructedObjectInst &I) {
    os_.indent(2);
    generateValue(&I);
    os_ << " = ";
    generateValue(I.getConstructorReturnValue());
    os_ << ".isObject() ? ";
    generateValue(I.getConstructorReturnValue());
    os_ << " : ";
    generateValue(I.getThisValue());
    os_ << ";\n";
  }

  void generateInstr(GetPNamesInst &I) {
    os_.indent(2) << "if (";
    generateValue(I.getBase());
    os_ << ".isUndefined() || ";
    generateValue(I.getBase());
    os_ << ".isNull()) {\n";

    os_.indent(4);
    generateValue(I.getIterator());
    os_ << " = HermesValue::encodeUndefinedValue();\n";

    os_.indent(2) << "} else {\n";

    os_.indent(4) << "auto res = toObject(runtime, Handle<>(&";
    generateValue(I.getBase());
    os_ << "));\n";
    generateCheckResult(I, 4, "res");
    os_.indent(4);
    generateValue(I.getBase());
    os_ << " = res.getValue();\n";

    os_.indent(4)
        << "auto obj = runtime->makeMutableHandle(vmcast<JSObject>(res.getValue()));\n";
    os_.indent(4) << "auto cr = getAllPropertyNames(obj, runtime);\n";
    generateCheckResult(I, 4, "cr");
    os_.indent(4) << "auto arr = *cr;\n";

    os_.indent(4);
    generateValue(I.getIterator());
    os_ << " = arr.getHermesValue();\n";
    os_.indent(4);
    generateValue(I.getIndex());
    os_ << " = HermesValue::encodeNumberValue(0);\n";
    os_.indent(4);
    generateValue(I.getSize());
    os_ << " = HermesValue::encodeNumberValue(JSArray::getLength(*arr));\n";

    os_.indent(2) << "}\n";

    os_.indent(2) << "if (";
    generateValue(I.getIterator());
    os_ << ".isUndefined()) goto ";
    generateBasicBlockLabel(I.getOnEmptyDest(), os_, bbMap_);
    os_ << ";\n";

    os_.indent(2) << "goto ";
    generateBasicBlockLabel(I.getOnSomeDest(), os_, bbMap_);
    os_ << ";\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(GetNextPNameInst &I) {
    os_.indent(2) << "{\n";

    os_.indent(4) << "MutableHandle<> tmpHandle{runtime};\n";
    os_.indent(4) << "auto obj = Handle<JSObject>::vmcast(&";
    generateValue(I.getBaseAddr());
    os_ << ");\n";
    os_.indent(4) << "auto arr = Handle<JSArray>::vmcast(&";
    generateValue(I.getIteratorAddr());
    os_ << ");\n";
    os_.indent(4) << "uint32_t idx = ";
    generateValue(I.getIndexAddr());
    os_ << ".getNumber();\n";
    os_.indent(4) << "uint32_t size = ";
    generateValue(I.getSizeAddr());
    os_ << ".getNumber();\n";
    os_.indent(4) << "MutableHandle<JSObject> propObj{runtime};\n";

    os_.indent(4) << "while (idx < size) {\n";
    os_.indent(6) << "tmpHandle = arr->at(idx);\n";
    os_.indent(6) << "ComputedPropertyDescriptor desc;\n";
    os_.indent(6) << "JSObject::getComputedPrimitiveDescriptor("
                  << "obj, runtime, tmpHandle, propObj, desc);\n";
    os_.indent(6) << "if (LLVM_LIKELY(propObj)) break;\n";
    os_.indent(6) << "++idx;\n";
    os_.indent(4) << "}\n";

    os_.indent(4) << "if (idx < size) {\n";
    os_.indent(6) << "if (tmpHandle->isNumber()) {\n";
    os_.indent(8) << "auto status = toString(runtime, tmpHandle);\n";
    os_.indent(8) << "tmpHandle = status.getValue();\n";
    os_.indent(6) << "}\n";
    os_.indent(6);
    generateValue(I.getPropertyAddr());
    os_ << " = tmpHandle.get();\n";
    os_.indent(6);
    generateValue(I.getIndexAddr());
    os_ << " = HermesValue::encodeNumberValue(idx + 1);\n";
    os_.indent(4) << "} else {\n";
    os_.indent(6);
    generateValue(I.getPropertyAddr());
    os_ << " = HermesValue::encodeUndefinedValue();\n";
    os_.indent(4) << "}\n";

    os_.indent(4) << "if (";
    generateValue(I.getPropertyAddr());
    os_ << ".isUndefined()) goto ";
    generateBasicBlockLabel(I.getOnLastDest(), os_, bbMap_);
    os_ << ";\n";

    os_.indent(4) << "goto ";
    generateBasicBlockLabel(I.getOnSomeDest(), os_, bbMap_);
    os_ << ";\n";

    os_.indent(2) << "}\n";

    os_.indent(2) << "marker.flush();\n";
  }

  void generateInstr(GetByPNameInst &I) {
    generateGetByVal(I, I.getBaseAddr(), I.getPropertyAddr());
  }

  void generateInstr(CreateRegExpInst &I) {
    if (auto regexp = CompiledRegExp::tryCompile(
            I.getPattern()->getValue().str(), I.getFlags()->getValue().str())) {
      auto bytecode = regexp->getBytecode();

      os_.indent(2) << "{\n";

      os_.indent(4)
          << "auto regRes = JSRegExp::create(runtime, "
          << "Handle<JSObject>::vmcast(&runtime->regExpPrototype));\n";
      generateCheckResult(I, 4, "regRes");
      os_.indent(4) << "auto re = runtime->makeHandle<JSRegExp>(*regRes);\n";

      generateString(I, 4, "patternRes", I.getPattern());
      os_.indent(4)
          << "auto pattern = runtime->makeHandle<StringPrimitive>(*patternRes);\n";

      generateString(I, 4, "flagsRes", I.getFlags());
      os_.indent(4)
          << "auto flags = runtime->makeHandle<StringPrimitive>(*flagsRes);\n";

      os_.indent(4)
          << "auto bytecode = reinterpret_cast<const unsigned char*>(\"";
      for (size_t i = 0; i < bytecode.size(); ++i) {
        os_ << "\\x";
        os_.write_hex(bytecode[i]);
      }
      os_ << "\");\n";

      os_.indent(4) << "auto res = JSRegExp::initialize(\n";
      os_.indent(8) << "re,\n";
      os_.indent(8) << "runtime,\n";
      os_.indent(8) << "pattern,\n";
      os_.indent(8) << "flags,\n";
      os_.indent(8) << "ArrayRef<uint8_t>(bytecode, " << bytecode.size()
                    << "));\n";
      generateCheckResult(I, 4, "res");

      os_.indent(4);
      generateValue(&I);
      os_ << " = re.getHermesValue();\n";

      os_.indent(2) << "}\n";

      os_.indent(2) << "marker.flush();\n";
    }
  }

 public:
  /// \p os is the output stream
  /// \p ra is the pre-ran register allocator for the current function
  /// \p envSize is the environment size of the current function
  /// \p status is only false if an error has occurred
  InstrGen(
      llvm::raw_ostream &os,
      hbc::HVMRegisterAllocator &ra,
      const llvm::DenseMap<BasicBlock *, unsigned> &bbMap,
      const llvm::DenseMap<Function *, unsigned> &funcMap,
      Function &F,
      FunctionScopeAnalysis &scopeAnalysis,
      llvm::DenseMap<BasicBlock *, CatchInst *> &catchMap,
      unsigned envSize,
      bool &status,
      uint32_t &nextCacheIdx)
      : os_(os),
        ra_(ra),
        bbMap_(bbMap),
        funcMap_(funcMap),
        scopeAnalysis_(scopeAnalysis),
        catchMap_(catchMap),
        envSize_(envSize),
        status_(status),
        nextCacheIdx_(nextCacheIdx) {}

  /// Converts Instruction \p I into valid c++ code and outputs it through the
  /// ostream.
  void generate(Instruction &I) {
    switch (I.getKind()) {
#define INCLUDE_HBC_INSTRS
#define DEF_VALUE(CLASS, PARENT) \
  case ValueKind::CLASS##Kind:   \
    return generateInstr(*cast<CLASS>(&I));
#include "hermes/IR/Instrs.def"
      default:
        llvm_unreachable("Invalid kind");
    }
  }
};

} // namespace

static void generateFunctionStub(
    Function &F,
    llvm::raw_ostream &OS,
    const llvm::DenseMap<Function *, unsigned> funcMap) {
  OS << "static CallResult<HermesValue> ";
  generateFunctionLabel(&F, OS, funcMap);
  OS << "(void *, Runtime *runtime, NativeArgs args);\n\n";
}

/// Converts Function \p F into valid c++ code and outputs it through \p OS
/// setting status to false if an error occurs.
static void generateFunction(
    Function &F,
    llvm::raw_ostream &OS,
    const llvm::DenseMap<Function *, unsigned> funcMap,
    FunctionScopeAnalysis &scopeAnalysis,
    bool &status,
    uint32_t &nextCacheIdx,
    BytecodeGenerationOptions options) {
  PostOrderAnalysis PO(&F);

  llvm::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());

  hbc::HVMRegisterAllocator RA(&F);

  RA.allocate(order);

  PassManager PM;
  PM.addPass(new LowerStoreInstrs(RA));
  PM.addPass(new hbc::LowerCalls(RA));
  if (options.optimizationEnabled) {
    PM.addPass(new MovElimination(RA));
    PM.addPass(new hbc::RecreateCheapValues(RA));
    PM.addPass(new hbc::LoadConstantValueNumbering(RA));
  }
  PM.run(&F);

  if (options.format == DumpRA) {
    RA.dump();
    return;
  }

  OS << "static CallResult<HermesValue> ";
  generateFunctionLabel(&F, OS, funcMap);
  OS << "(void *, Runtime *runtime, NativeArgs args) {\n"
     << "  StackFramePtr frame = runtime->getCurrentFrame();\n"
     << "  constexpr bool strictMode = " << F.isStrictMode() << ";\n"
     << "  const PropOpFlags defaultPropOpFlags = "
     << "strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();\n"
     << "  (void)defaultPropOpFlags;\n"
     << "  runtime->checkAndAllocStack(" << RA.getMaxRegisterUsage()
     << " + StackFrameLayout::CalleeExtraRegistersAtStart, "
     << "HermesValue::encodeUndefinedValue());\n"
     << "  GCScopeMarkerRAII marker{runtime};\n";

  if (F.isGlobalScope()) {
    OS << "  {\n"
          "    DefinePropertyFlags dpf{};\n"
          "    dpf.setWritable = 1;\n"
          "    dpf.setConfigurable = 1;\n"
          "    dpf.setEnumerable = 1;\n"
          "    dpf.writable = 1;\n"
          "    dpf.enumerable = 1;\n"
          "    dpf.configurable = 0;\n";
    for (auto var : F.getVariables()) {
      if (isa<GlobalProperty>(var))
        continue;
      OS << "    if (JSObject::defineOwnProperty(\n"
         << "            runtime->getGlobal(),\n"
         << "            runtime,\n"
         << "            runtime->getIdentifierTable().registerLazyIdentifier("
         << serializeStr(var->getName().str()) << "),\n"
         << "            dpf,\n"
         << "            runtime->getUndefinedValue(),\n"
         << "            PropOpFlags().plusThrowOnError()) ==\n"
         << "        ExecutionStatus::EXCEPTION) {\n"
         << "      return ExecutionStatus::EXCEPTION;\n"
         << "    }\n"
         << "    marker.flush();\n";
    }
    OS << "  }\n";
  }

  unsigned bbCounter = 0;
  llvm::DenseMap<BasicBlock *, unsigned> bbMap;
  for (auto &B : order) {
    bbMap[B] = bbCounter;
    bbCounter++;
  }

  unsigned envSize = F.getVariables().size();

  llvm::DenseMap<BasicBlock *, CatchInst *> catchMap{};
  constructCatchMap(catchMap, F);

  InstrGen instrGen(
      OS,
      RA,
      bbMap,
      funcMap,
      F,
      scopeAnalysis,
      catchMap,
      envSize,
      status,
      nextCacheIdx);

  for (auto &B : order) {
    generateBasicBlockLabel(B, OS, bbMap);
    OS << ":\n";

    OS << "  (void)&&";
    generateBasicBlockLabel(B, OS, bbMap);
    OS << ";\n";

    for (auto &I : *B) {
      instrGen.generate(I);
    }
  }

  OS << "}\n\n";
}

/// Converts Module \p M into valid c++ code and outputs it through \p OS.
/// Returns the cache size necessary to store all the cache indexes used.
static uint32_t generateModule(
    Module *M,
    llvm::raw_ostream &OS,
    const BytecodeGenerationOptions &options) {
  PassManager PM;
  PM.addPass(new LowerNumericProperties());
  PM.addPass(new hbc::LowerConstruction());
  PM.addPass(new hbc::LowerArgumentsArray());
  PM.addPass(new LimitAllocArray(UINT16_MAX));
  PM.addPass(new hbc::DedupReifyArguments());
  // TODO Consider supporting LowerSwitchIntoJumpTables for optimization
  PM.addPass(new SwitchLowering());
  PM.addPass(new hbc::LoadConstants());
  PM.addPass(new hbc::LoadParameters());
  PM.addPass(new hbc::LowerLoadStoreFrameInst());
  if (options.optimizationEnabled) {
    // Lowers AllocObjects and its sequential literal properties into a single
    // Reduce comparison and conditional jump to single comparison jump
    PM.addPass(new LowerCondBranch(true));
    // Move loads to child blocks if possible.
    PM.addCodeMotion();
    // Eliminate common HBCLoadConstInsts.
    PM.addCSE();
    // Drop unused HBCLoadParamInsts.
    PM.addDCE();
  }
  PM.run(M);

  bool status = true;
  uint32_t nextCacheIdx = 0;

  OS << "\n";

  auto topLevelFunc = M->getTopLevelFunction();
  llvm::DenseMap<Function *, unsigned> funcMap;
  funcMap[topLevelFunc] = 0;
  unsigned funcCounter = 1;
  for (auto &F : *M) {
    if (&F != topLevelFunc) {
      funcMap[&F] = funcCounter;
      funcCounter++;
    }
  }

  FunctionScopeAnalysis scopeAnalysis{topLevelFunc};

  if (options.format != DumpRA) {
    for (auto &F : *M) {
      generateFunctionStub(F, OS, funcMap);
    }
  }

  for (auto &F : *M) {
    generateFunction(
        F, OS, funcMap, scopeAnalysis, status, nextCacheIdx, options);
  }

  if (!status) {
    llvm::errs() << "ERROR: Compilation failed!\n";
    llvm::errs() << "Output marked with XXX indicates unsupported operations\n";
    llvm::errs() << "\n";
  }

  return nextCacheIdx;
}

/// Converts Module \p M into valid c++ code and outputs it through \p OS
/// If \p standalone is true the generated c++ contains `main` and should be
/// directly compilable.
void cpp::generateCpp(
    Module *M,
    llvm::raw_ostream &OS,
    bool standalone,
    const BytecodeGenerationOptions &options) {
  if (options.format != DumpRA) {
    OS << "#include \"hermes/VM/Runtime.h\"\n"
          "#include \"hermes/VM/Interpreter.h\"\n"
          "#include \"hermes/VM/ArrayStorage.h\"\n"
          "#include \"hermes/VM/Callable.h\"\n"
          "#include \"hermes/VM/JSArray.h\"\n"
          "#include \"hermes/VM/PrimitiveBox.h\"\n"
          "#include \"hermes/VM/StackFrame-inline.h\"\n"
          "#include \"hermes/VM/StringPrimitive.h\"\n"
          "#include \"hermes/VM/Operations.h\"\n"
          "#include \"hermes/VM/IdentifierTable.h\"\n"
          "#include \"hermes/VM/Interpreter.h\"\n"
          "#include \"hermes/VM/CPPUtil/CPPUtil.h\"\n"
          "#include \"hermes/VM/JSRegExp.h\"\n"
          "\n"
          "using namespace hermes::vm;\n";
  }

  uint32_t cacheSize = generateModule(M, OS, options);

  if (standalone && options.format != DumpRA) {
    OS << "int main() {\n"
          "  Runtime runtime(RuntimeConfig::Builder()\n"
          "      .withGCConfig(\n"
          "          GCConfig::Builder()\n"
          "              .withInitHeapSize(1024 * 1024)\n"
          "              .withMaxHeapSize(1024 * 1024 * 1024)\n"
          "              .build())\n"
          "      .build());\n"
          "\n"
          "  runtime.setCppPropertyCacheSize("
       << cacheSize
       << ");\n"
          "\n"
          "  GCScope gcScope{&runtime, \"main gcscope\"};\n"
          "\n"
          "  auto symbol = runtime.getIdentifierTable().getSymbolHandleFromPrimitive(\n"
          "      &runtime,\n"
          "      StringPrimitive::createNoThrow(&runtime, \"_0\"))->get();\n"
          "\n"
          "  auto func = runtime.makeHandle<NativeFunction>(*NativeFunction::create(\n"
          "      &runtime,\n"
          "      Handle<JSObject>::vmcast(&runtime.functionPrototype),\n"
          "      nullptr,\n"
          "      _0,\n"
          "      symbol,\n"
          "      0,\n"
          "      runtime.makeNullHandle<JSObject>()));\n"
          "\n"
          "  *Callable::executeCall0(func, &runtime, runtime.getGlobal());\n"
          "\n"
          "  return 0;\n"
          "}\n";
  }
}

#endif
