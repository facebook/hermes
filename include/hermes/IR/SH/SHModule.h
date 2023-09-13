/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_SH_SHMODULE_H
#define HERMES_IR_SH_SHMODULE_H

#include "hermes/IR/IR.h"

#include "llvh/ADT/DenseMap.h"

namespace hermes::sh {

/// A register class identifies a set of registers with similar properties.
enum class RegClass : uint8_t {
  /// A native local that may be a pointer.
  LocalPtr,
  /// A native local that is guaranteed to not be a pointer.
  LocalNonPtr,
  /// An entry in the VM register stack.
  RegStack,
  /// The last entry.
  _last,
};

/// An alias to make it explicit that a value is a register index.
using RegIndex = uint32_t;

/// This is an instance of a register. It contains a register class and an index
/// within the class. Register is passed by value and must remain a small
/// wrapper around an integer.
class Register {
  /// Marks unused/invalid register.
  static constexpr uint32_t kInvalidRegister = ~(uint32_t)0;
  /// A tombstone used by llvh::DenseMap.
  static constexpr uint32_t kTombstoneRegister = kInvalidRegister - 1;

  /// Bits reserved for the register class.
  static constexpr unsigned kClassWidth = 4;
  /// Remaining bits for the register index.
  static constexpr unsigned kIndexWidth = 32 - kClassWidth;

  static_assert(
      (unsigned)RegClass::_last <= 1u << kClassWidth,
      "not enough bits for RegClass");

  /// The class and the index are packed inside a 32-bit word. We use a union
  /// and a bitfield struct for convenient access.
  union {
    struct {
      /// The register class.
      uint32_t class_ : kClassWidth;
      /// The index withing the register class.
      uint32_t index_ : kIndexWidth;
    };
    /// An opaque 32-bit value holding both the index and the class.
    uint32_t value_;
  };

  explicit constexpr Register(uint32_t value) : value_(value) {}

 public:
  /// Create an invalid register.
  explicit constexpr Register() : Register(kInvalidRegister) {}

  /// Create a register with class and index.
  explicit constexpr Register(RegClass cls, RegIndex index)
      : class_((uint32_t)cls), index_(index) {
    assert(index < (1u << kIndexWidth) && "register index too large");
  }

  /// Create a tombstone register for used by llvh::DenseMap.
  static constexpr Register getTombstoneKey() {
    return Register(kTombstoneRegister);
  }

  /// \returns true if this is a valid result.
  bool isValid() const {
    return value_ != kInvalidRegister;
  }

  /// \returns an opaque value containing all information stored in the
  /// register:
  ///     the index and the class.
  uint32_t getOpaqueValue() const {
    return value_;
  }

  /// \returns the register class.
  RegClass getClass() const {
    assert(isValid());
    return static_cast<RegClass>(class_);
  }

  /// \returns the index within the register class.
  RegIndex getIndex() const {
    assert(isValid());
    return index_;
  }

  bool operator==(Register RHS) const {
    return value_ == RHS.value_;
  }
  bool operator!=(Register RHS) const {
    return !(*this == RHS);
  }

  /// \returns true if the register RHS comes right after this one.
  /// For example, R5 comes after R4.
  bool isConsecutive(Register RHS) const {
    assert(isValid() && RHS.isValid());
    return getClass() == RHS.getClass() && getIndex() + 1 == RHS.getIndex();
  }

  /// \return the n'th consecutive register after the current register.
  Register getConsecutive(uint32_t count = 1) {
    assert(isValid());
    return Register(getClass(), getIndex() + count);
  }

  /// Impose an arbitrary ordering between registers.
  static bool less(Register a, Register b) {
    return a.value_ < b.value_;
  }
};

// Print Register to llvm debug/error streams.
llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, Register reg);

/// An IR representation of a local and the type used to access it.
class SHLocal : public Value {
  Register reg_;

 public:
  SHLocal(Register reg, Type type) : Value(ValueKind::SHLocalKind), reg_(reg) {
    setType(type);
  }

  static bool classof(const Value *v) {
    return v->getKind() == ValueKind::SHLocalKind;
  }

  Register reg() const {
    return reg_;
  }
};

/// All SH-specific IR data is owned by this class.
class SHModule {
  /// A map from register and type to local.
  llvh::DenseMap<std::pair<Register, Type>, SHLocal *> locals_{};

 public:
  SHModule();
  ~SHModule();

  /// \returns a local with the given register and type, creating one if it
  ///     doesn't exist.
  SHLocal *getLocal(Register reg, Type type);
};

} // namespace hermes::sh

namespace llvh {
template <>
struct DenseMapInfo<hermes::sh::Register> {
  static inline hermes::sh::Register getEmptyKey() {
    return hermes::sh::Register();
  }
  static inline hermes::sh::Register getTombstoneKey() {
    return hermes::sh::Register::getTombstoneKey();
  }
  static unsigned getHashValue(hermes::sh::Register val) {
    return val.getOpaqueValue();
  }
  static bool isEqual(hermes::sh::Register LHS, hermes::sh::Register RHS) {
    return LHS == RHS;
  }
};

template <>
struct DenseMapInfo<std::pair<hermes::sh::Register, hermes::Type>> {
  using T = std::pair<hermes::sh::Register, hermes::Type>;
  using Info = DenseMapInfo<hermes::sh::Register>;

  static inline T getEmptyKey() {
    return T(Info::getEmptyKey(), hermes::Type::createNoType());
  }
  static inline T getTombstoneKey() {
    return T(Info::getTombstoneKey(), hermes::Type::createNoType());
  }
  static unsigned getHashValue(const T &Val) {
    return llvh::hash_combine(Info::getHashValue(Val.first), Val.second);
  }
  static bool isEqual(const T &LHS, const T &RHS) {
    return Info::isEqual(LHS.first, RHS.first) && LHS.second == RHS.second;
  }
};
} // namespace llvh

#endif
