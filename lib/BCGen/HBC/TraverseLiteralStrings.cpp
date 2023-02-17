/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/TraverseLiteralStrings.h"
#include "hermes/IR/Instrs.h"

#include <functional>

namespace {

using namespace hermes;

/// \return true if and only if a literal string operand at index \p idx of
/// instruction \p I is to be treated as an Identifier during Hermes Bytecode
/// generation.
bool isIdOperand(Instruction *I, unsigned idx) {
#define CASE_WITH_PROP_IDX(INSN) \
  case ValueKind::INSN##Kind:    \
    return idx == INSN::PropertyIdx

  switch (I->getKind()) {
    CASE_WITH_PROP_IDX(DeletePropertyInst);
    CASE_WITH_PROP_IDX(LoadPropertyInst);
    CASE_WITH_PROP_IDX(StoreNewOwnPropertyInst);
    CASE_WITH_PROP_IDX(StorePropertyInst);
    CASE_WITH_PROP_IDX(TryLoadGlobalPropertyInst);
    CASE_WITH_PROP_IDX(TryStoreGlobalPropertyInst);

    case ValueKind::HBCAllocObjectFromBufferInstKind:
      // AllocObjectFromBuffer stores the keys and values as alternating
      // operands starting from FirstKeyIdx.
      return (idx - HBCAllocObjectFromBufferInst::FirstKeyIdx) % 2 == 0;

    default:
      return false;
  }
#undef CASE
}

} // namespace

namespace hermes {
namespace hbc {

void traverseFunctions(
    Module *M,
    std::function<bool(Function *)> shouldVisitFunction,
    std::function<void(llvh::StringRef)> traversal,
    bool stripFunctionNames) {
  for (auto &F : *M) {
    if (!shouldVisitFunction(&F)) {
      continue;
    }
    if (!stripFunctionNames) {
      traversal(F.getOriginalOrInferredName().str());
    }
    // The source visibility of the global function indicate the presence of
    // top-level source visibility directives, but we should not preserve the
    // source code of the global function.
    if (!F.isGlobalScope()) {
      // Only add non-default source representation to the string table.
      if (auto source = F.getSourceRepresentationStr()) {
        traversal(*source);
      }
    }
  }
}

void traverseCJSModuleNames(
    Module *M,
    std::function<bool(Function *)> shouldVisitFunction,
    std::function<void(llvh::StringRef)> traversal) {
  for (auto &F : *M) {
    if (!shouldVisitFunction(&F)) {
      continue;
    }

    if (auto *cjsModule = M->findCJSModule(&F)) {
      traversal(cjsModule->filename.str());
    }
  }
}

void traverseLiteralStrings(
    Module *M,
    std::function<bool(Function *)> shouldVisitFunction,
    std::function<void(llvh::StringRef, bool)> traversal) {
  // Walk declared global properties.
  for (auto *prop : M->getGlobalProperties()) {
    if (prop->isDeclared()) {
      traversal(prop->getName()->getValue().str(), /* isIdentifier */ true);
    }
  }

  // Walk functions.
  for (auto &F : *M) {
    if (!shouldVisitFunction(&F)) {
      continue;
    }

    for (auto &BB : F) {
      // Walk instruction operands.
      for (auto &I : BB) {
        for (int i = 0, e = I.getNumOperands(); i < e; i++) {
          auto *op = I.getOperand(i);
          if (auto *str = llvh::dyn_cast<LiteralString>(op)) {
            traversal(str->getValue().str(), isIdOperand(&I, i));
          }
        }
      }
    }
  }
}

} // namespace hbc
} // namespace hermes
