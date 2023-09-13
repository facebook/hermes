/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/SH/SHModule.h"

#define DEBUG_TYPE "shmodule"

namespace hermes::sh {

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, Register reg) {
  if (!reg.isValid()) {
    OS << "invalid";
  } else {
    switch (reg.getClass()) {
      case RegClass::LocalPtr:
        OS << "loc" << reg.getIndex();
        break;
      case RegClass::LocalNonPtr:
        OS << "np" << reg.getIndex();
        break;
      case RegClass::RegStack:
        OS << "stack[" << reg.getIndex() << ']';
        break;
      case RegClass::_last:
        llvm_unreachable("invalid register class");
    }
  }

  return OS;
}

SHModule::SHModule() = default;

SHModule::~SHModule() {
  for (auto &pair : locals_)
    Value::destroy(pair.second);
}

SHLocal *SHModule::getLocal(hermes::sh::Register reg, hermes::Type type) {
  auto [it, inserted] = locals_.try_emplace(std::make_pair(reg, type), nullptr);
  if (inserted)
    it->second = new SHLocal(reg, type);
  return it->second;
}

} // namespace hermes::sh

#undef DEBUG_TYPE
