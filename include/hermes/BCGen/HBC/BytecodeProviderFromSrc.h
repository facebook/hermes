/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_BCGEN_HBC_BYTECODEPROVIDERFROMSRC_H
#define HERMES_BCGEN_HBC_BYTECODEPROVIDERFROMSRC_H

#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/BytecodeDataProvider.h"

namespace hermes {
namespace hbc {

/// Flags passed to createBCProviderFromSrc to set parameters on execution.
struct CompileFlags {
  bool optimize{false};
  bool debug{false};
  bool lazy{false};
  bool strict{false};
  bool staticBuiltins{false};
  bool verifyIR{false};
};

#ifndef HERMESVM_LEAN
/// BCProviderFromSrc is used when we are construction the bytecode from
/// source compilation, i.e. we generate BytecodeModule/BytecodeFunction
/// in this code path, and all the data are stored in those classes.
/// We should only depend on this when running from eval code path to
/// make sure maximum efficiency when loading from bytecode file.
class BCProviderFromSrc final : public BCProviderBase {
  /// The BytecodeModule that provides the bytecode data.
  std::unique_ptr<hbc::BytecodeModule> module_;

  explicit BCProviderFromSrc(std::unique_ptr<hbc::BytecodeModule> module);

  /// No need to do anything since it's already created as part of
  /// BytecodeModule and set to the local member.
  void createDebugInfo() {}

 public:
  static std::unique_ptr<BCProviderFromSrc> createBCProviderFromSrc(
      std::unique_ptr<hbc::BytecodeModule> module) {
    return std::unique_ptr<BCProviderFromSrc>(
        new BCProviderFromSrc(std::move(module)));
  }

  /// Creates a BCProviderFromSrc by compiling the given JavaScript in \p buffer
  /// named as \p sourceURL, according to the flags in \p compileFlags. \return
  /// a BCProvider and an empty error, or a null BCProvider and an error
  /// message.
  static std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
  createBCProviderFromSrc(
      std::unique_ptr<Buffer> buffer,
      llvm::StringRef sourceURL,
      const CompileFlags &compileFlags);

  RuntimeFunctionHeader getFunctionHeader(uint32_t functionID) const {
    return RuntimeFunctionHeader(&module_->getFunction(functionID).getHeader());
  }

  StringTableEntry getStringTableEntry(uint32_t index) const {
    assert(index < stringCount_ && "invalid string table index");
    return module_->getStringTable()[index];
  }

  const uint8_t *getBytecode(uint32_t functionID) const {
    return module_->getFunction(functionID).getOpcodeArray().data();
  }

  llvm::ArrayRef<hbc::HBCExceptionHandlerInfo> getExceptionTable(
      uint32_t functionID) const {
    return module_->getFunction(functionID).getExceptionHandlers();
  }

  const hbc::DebugOffsets *getDebugOffsets(uint32_t functionID) const {
    return module_->getFunction(functionID).getDebugOffsets();
  }

  bool isFunctionLazy(uint32_t functionID) const {
    return module_->getFunction(functionID).isLazy();
  }

  bool isLazy() const {
    return false;
  }

  hbc::BytecodeModule *getBytecodeModule() {
    return module_.get();
  }
};

/// BCProviderLazy is used during lazy compilation. When a function is created
/// to be lazily compiled later, we create a BCProviderLazy object with
/// a pointer to such BytecodeFunction.
class BCProviderLazy final : public BCProviderBase {
  /// Pointer to the BytecodeFunction.
  hbc::BytecodeFunction *bytecodeFunction_;

  explicit BCProviderLazy(hbc::BytecodeFunction *bytecodeFunction);

  /// No debug information will be available without compiling it.
  void createDebugInfo() {
    llvm_unreachable("Accessing debug info from a lazy module");
  }

 public:
  static std::unique_ptr<BCProviderBase> createBCProviderLazy(
      hbc::BytecodeFunction *bytecodeFunction) {
    return std::unique_ptr<BCProviderBase>(
        new BCProviderLazy(bytecodeFunction));
  }

  RuntimeFunctionHeader getFunctionHeader(uint32_t) const {
    return RuntimeFunctionHeader(&bytecodeFunction_->getHeader());
  }

  StringTableEntry getStringTableEntry(uint32_t index) const {
    llvm_unreachable("Accessing string table from a lazy module");
  }

  const uint8_t *getBytecode(uint32_t) const {
    llvm_unreachable("Accessing bytecode from a lazy module");
  }

  llvm::ArrayRef<hbc::HBCExceptionHandlerInfo> getExceptionTable(
      uint32_t) const {
    llvm_unreachable("Accessing exception info from a lazy module");
  }

  const hbc::DebugOffsets *getDebugOffsets(uint32_t) const {
    llvm_unreachable("Accessing debug offsets from a lazy module");
  }

  bool isFunctionLazy(uint32_t) const {
    return true;
  }

  bool isLazy() const {
    return true;
  }

  /// \return the pointer to the BytecodeFunction.
  hbc::BytecodeFunction *getBytecodeFunction() {
    return bytecodeFunction_;
  }
};
#endif // HERMESVM_LEAN
} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BYTECODEPROVIDERFROMSRC_H
