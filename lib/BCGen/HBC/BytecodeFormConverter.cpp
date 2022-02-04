/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/BytecodeFormConverter.h"

#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/Inst/Inst.h"
#include "hermes/Inst/InstDecode.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/Support/MemoryBuffer.h"

using namespace hermes;
using namespace hermes::hbc;
using namespace hermes::inst;
using llvh::MutableArrayRef;

namespace {

/// A helper class to convert between bytecode forms.
template <BytecodeForm TargetForm>
class BytecodeFormConverter {
  /// A helper type that converts unsigned ints either to or from relativized
  /// form.
  struct Adjuster {
    /// The current (last-seen) value. When converting to delta form, adjusting
    /// means storing every value as a delta from the previous; thus we
    /// subtracting this from the incoming value. When converting to execution
    /// form, we do the opposite: add this to each value.
    uint64_t current_{0};

    /// Adjust the given value \p value.
    /// \return the adjusted new value.
    template <typename T>
    T apply(T value) {
      static_assert(
          std::is_unsigned<T>::value, "Can only process unsigned types");
      uint64_t result;
      if (TargetForm == BytecodeForm::Delta) {
        result = value - current_;
        current_ = value;
      } else {
        result = value + current_;
        current_ = result;
      }
      return result;
    }

    /// Adjust the given value \p value, in place.
    template <typename T>
    void operator()(T &value) {
      value = apply(value);
    }
  };

  /// The bytes containing the bytecode file contents.
  MutableArrayRef<uint8_t> bytes_;

  /// Fields pointing into the bytecode file contents.
  MutableBytecodeFileFields fields_;

  /// Bytecode provider for the input file.
  std::unique_ptr<BCProviderFromBuffer> bcProvider_;

  /// Relativize fields in the instructions.
  void processInstructions() {
    // We adjust a few instructions; the set of instructions was determined
    // empirically by comparing the delta sizes.
    Adjuster adjDeclareGlobalVar;
    Adjuster adjCreateClosure;
    Adjuster adjCreateClosureLong;
    Adjuster adjNewArrayWithBuffer;
    Adjuster adjNewArrayWithBufferLong;
    // We cannot pass references to the fields in instructions as they may be
    // unaligned. Use this macro to avoid having to write each adjuster twice.
#define ADJUST(adjuster, field) field = (adjuster).apply(field)
    for (uint32_t func = 0; func < bcProvider_->getFunctionCount(); ++func) {
      RuntimeFunctionHeader header = bcProvider_->getFunctionHeader(func);
      // Find the bytecode start and end for each function.
      uint8_t *bytecodeStart = &bytes_[header.offset()];
      uint8_t *bytecodeEnd = bytecodeStart + header.bytecodeSizeInBytes();
      uint8_t *cursor = bytecodeStart;
      while (cursor < bytecodeEnd) {
        auto *ip = reinterpret_cast<inst::Inst *>(cursor);
        switch (ip->opCode) {
          case OpCode::NewArrayWithBuffer:
            ADJUST(adjNewArrayWithBuffer, ip->iNewArrayWithBuffer.op4);
            break;
          case OpCode::NewArrayWithBufferLong:
            ADJUST(adjNewArrayWithBufferLong, ip->iNewArrayWithBufferLong.op4);
            break;
          case OpCode::CreateClosure:
            ADJUST(adjCreateClosure, ip->iCreateClosure.op3);
            break;
          case OpCode::CreateClosureLongIndex:
            ADJUST(adjCreateClosureLong, ip->iCreateClosureLongIndex.op3);
            break;
          case OpCode::DeclareGlobalVar:
            ADJUST(adjDeclareGlobalVar, ip->iDeclareGlobalVar.op1);
            break;
          default:
            break;
        }
        cursor += getInstSize(ip->opCode);
      }
#undef ADJUST
    }
  }

  /// Relativize fields in function overflow headers.
  void processOverflowFunctionHeaders() {
    // TODO: this logic of finding overflow headers is duplicated here and in
    // BytecodeDataProvider.h as well. Consider centralizing this logic in
    // BytecodeFileFields.
    Adjuster overflowOffsetAdj;
    for (SmallFuncHeader &sfh : fields_.functionHeaders) {
      if (sfh.flags.overflowed) {
        FunctionHeader *fh = reinterpret_cast<FunctionHeader *>(
            &bytes_[sfh.getLargeHeaderOffset()]);
        overflowOffsetAdj(fh->offset);
      }
    }
  }

  /// Relativize fields in function headers.
  void processFunctionHeaders() {
    Adjuster offsetAdj;
    for (SmallFuncHeader &sfh : fields_.functionHeaders) {
      sfh.offset = offsetAdj.apply(sfh.offset);
    }
  }

  /// Relativize the contents of the string table.
  void processStringTable() {
    Adjuster offsetAdj;
    for (SmallStringTableEntry &entry : fields_.stringTableEntries) {
      entry.offset = offsetAdj.apply(entry.offset);
    }
  }

  /// Relativize the contents of the overflow string table.
  void processOverflowStringTable() {
    Adjuster offsetAdj;
    for (OverflowStringTableEntry &entry : fields_.stringTableOverflowEntries) {
      entry.offset = offsetAdj.apply(entry.offset);
    }
  }

  /// Set the magic number.
  void processMagicNumber() {
    fields_.header->magic =
        (TargetForm == BytecodeForm::Delta ? DELTA_MAGIC : MAGIC);
  }

 public:
  /// Construct the a converter given \p bytes and \p fields pointing into those
  /// bytes. The bytes are modified in-place.
  explicit BytecodeFormConverter(
      MutableArrayRef<uint8_t> bytes,
      MutableBytecodeFileFields &fields,
      BytecodeForm sourceForm)
      : bytes_(bytes), fields_(fields) {
    auto res = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
        std::make_unique<Buffer>(bytes.data(), bytes.size()), sourceForm);
    if (!res.first) {
      hermes_fatal(res.second.c_str());
    }
    bcProvider_ = std::move(res.first);
  }

  /// Perform the conversion.
  void perform() {
    // The list of steps, as member function pointers.
    using Step = void (BytecodeFormConverter::*)(void);
    Step steps[] = {
        &BytecodeFormConverter::processMagicNumber,
        &BytecodeFormConverter::processInstructions,
        &BytecodeFormConverter::processOverflowFunctionHeaders,
        &BytecodeFormConverter::processFunctionHeaders,
        &BytecodeFormConverter::processStringTable,
        &BytecodeFormConverter::processOverflowStringTable

    };
    // Some steps adjust values that are used in computing other values. For
    // example, a function's info offset is adjusted, but is needed to find the
    // overflow function header. Therefore we run the steps in reverse order
    // when converting from delta form back to execution form.
    if (TargetForm == BytecodeForm::Execution) {
      std::reverse(std::begin(steps), std::end(steps));
    }
    for (auto step : steps) {
      (this->*step)();
    }
    hbc::BCProviderFromBuffer::updateBytecodeHash(bytes_);
  }
};

} // namespace

bool hermes::hbc::convertBytecodeToForm(
    MutableArrayRef<uint8_t> buffer,
    BytecodeForm targetForm,
    std::string *outError) {
  BytecodeForm sourceForm =
      (targetForm == BytecodeForm::Delta ? BytecodeForm::Execution
                                         : BytecodeForm::Delta);
  MutableBytecodeFileFields fields;
  if (!fields.populateFromBuffer(buffer, outError, sourceForm)) {
    return false;
  }

  if (targetForm == BytecodeForm::Delta) {
    BytecodeFormConverter<BytecodeForm::Delta> conv(buffer, fields, sourceForm);
    conv.perform();
  } else {
    BytecodeFormConverter<BytecodeForm::Execution> conv(
        buffer, fields, sourceForm);
    conv.perform();
  }
  return true;
}
