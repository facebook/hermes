/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_REGALLOC_H
#define HERMES_BCGEN_REGALLOC_H

#include "hermes/IR/IR.h"
#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/BitVector.h"
#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"

namespace hermes {

using llvh::ArrayRef;
using llvh::BitVector;
using llvh::DenseMap;

/// A register class identifies a set of registers with similar properties.
enum class RegClass : uint8_t {
  /// A "dummy" register used for no-output instructions.
  NoOutput,
  /// A value that is known to be a number.
  Number,
  /// A value that is known to be a non-pointer value.
  NonPtr,
  /// A value that can be anything.
  Other,
  /// The last entry.
  _last,
};

/// An alias to make it explicit that a value is a register index.
using RegIndex = uint32_t;

/// This is an instance of a bytecode register. It's just a wrapper around a
/// simple integer index. Register is passed by value and must remain a small
/// wrapper around an integer.
class Register {
  /// Markes unused/invalid register.
  static const unsigned InvalidRegister = ~0u;
  static const unsigned TombstoneRegister = ~0u - 1;

  /// Bits reserved for the register class.
  static constexpr unsigned kClassWidth = 4;
  /// Remaining bits for the register index.
  static constexpr unsigned kIndexWidth = 32 - kClassWidth;

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

  /// Constructor to set the whole value, only used by special values.
  explicit constexpr Register(uint32_t value) : value_(value) {}

  /// DenseMap needs access to the private value for hashing.
  friend llvh::DenseMapInfo<Register>;

 public:
  explicit constexpr Register() : value_(InvalidRegister) {}

  explicit constexpr Register(RegClass regClass, RegIndex index)
      : class_((uint32_t)regClass), index_(index) {
    assert(index < (1u << kIndexWidth) && "register index too large");
  }

  /// \returns true if this is a valid result.
  bool isValid() const {
    return value_ != InvalidRegister;
  }

  /// \returns the register class of the register.
  RegClass getClass() const {
    return static_cast<RegClass>(class_);
  }

  /// \returns the numeric value of the register within the class.
  /// NOTE: All reg classes index from 0, this is not a global register number,
  /// and isn't to be used directly in the VM. Use getHVMRegisterIndex for that.
  RegIndex getIndexInClass() const {
    return index_;
  }

  /// Marks an empty register in the map.
  static Register getTombstoneKey() {
    return Register(TombstoneRegister);
  }

 public:
  bool operator==(Register RHS) const {
    return value_ == RHS.value_;
  }
  bool operator!=(Register RHS) const {
    return !(*this == RHS);
  }

  /// \returns true if the register RHS comes right after this one in the same
  /// class. For example, (Number,5) comes after (Number,4).
  bool isConsecutive(Register RHS) const {
    return getIndexInClass() + 1 == RHS.getIndexInClass() &&
        getClass() == RHS.getClass();
  }

  /// \return the n'th consecutive register after the current register
  /// in the same class.
  Register getConsecutive(unsigned count = 1) const {
    return Register(getClass(), getIndexInClass() + count);
  }

  static bool compare(const Register &a, const Register &b) {
    return a.value_ < b.value_;
  }
};

/// This class represents the register file. In keeps track of the currently
/// live registers and knows how to recycle registers.
class RegisterFile {
  // Notice that in a few places we rely on the fact that the register file
  // can only grow (and not shrink). This is how we keep track of the max number
  // of allocated register. There is no need to shrink the register file because
  // the compile time wins are negligable.
  llvh::BitVector registers_[(size_t)RegClass::_last]{};

 public:
  RegisterFile(const RegisterFile &) = delete;
  void operator=(const RegisterFile &) = delete;
  RegisterFile() = default;

  /// \returns true if the register \r is used;
  bool isUsed(Register r);

  /// \returns true if the register \r is free;
  bool isFree(Register r);

  /// \returns a register that's currently unused.
  Register allocateRegister(RegClass regClass);

  /// Reserves \p n consecutive registers at the end of the register file.
  /// 'n' consecutive registers will be allocated and the first one is returned.
  Register tailAllocateConsecutive(RegClass regClass, unsigned n);

  /// Free the register \p reg and make it available for re-allocation.
  void killRegister(Register reg);

  /// \returns the number of currently allocated registers.
  unsigned getNumLiveRegisters(RegClass regClass) const {
    return registers(regClass).size() - registers(regClass).count();
  }

  /// \returns the number of registers that were ever created.
  unsigned getMaxRegisterUsage(RegClass regClass) const {
    return registers(regClass).size();
  }

  /// \return the BitVector tracking registers of \p regClass.
  llvh::BitVector &registers(RegClass regClass) {
    return registers_[(size_t)regClass];
  }
  /// \return the BitVector tracking registers of \p regClass.
  const llvh::BitVector &registers(RegClass regClass) const {
    return registers_[(size_t)regClass];
  }

  /// Clear all register files except for the one of RegClass::Other,
  /// and add the number of registers removed from type-specific reg classes to
  /// the Other class.
  /// \post There are no more allocated registers in any classes besides
  /// Other and NoOutput.
  /// Used by SpillRegisters to make everything into Other registers before
  /// reserving and spilling.
  void convertTypeSpecificRegsToOther();

  /// Verify the internal state of the register file.
  void verify();

  /// Dump the state of the register file.
  void dump();
};

class Function;
class Instruction;
class PhiInst;
class CallInst;

/// Segment is a value type that repreents a consecutive half-open interval in
/// the range of [start, end).
struct Segment {
  size_t start_;
  size_t end_;

  explicit Segment(size_t start, size_t end) : start_(start), end_(end) {
    assert(end_ >= start_ && "invalid segment range");
  }

  /// \returns the size represented by the segment.
  size_t size() const {
    return end_ - start_;
  }

  /// \returns true if the segment is unused.
  size_t empty() const {
    return size() == 0;
  }

  /// \returns true if the location \p loc falls inside the current range.
  bool contains(size_t loc) const {
    return loc < end_ && loc >= start_;
  }

  /// \return true if the segment \p other intersects with this segment.
  bool intersects(Segment other) const {
    return !(other.start_ >= end_ || start_ >= other.end_);
  }

  /// \return true if the segment \p other touches this segment.
  bool touches(Segment other) const {
    return other.start_ == end_ || start_ == other.end_;
  }

  /// Join the range of the other interval into the current interval.
  void merge(Segment other) {
    assert(
        (intersects(other) || touches(other)) &&
        "merging non overlapping segment");
    start_ = std::min(start_, other.start_);
    end_ = std::max(end_, other.end_);
  }
};

/// Interval is a collection of segments repreents a non-consecutive half-open
/// range.
struct Interval {
  llvh::SmallVector<Segment, 2> segments_;

  explicit Interval() = default;

  explicit Interval(size_t start, size_t end) {
    add(Segment(start, end));
  }

  /// \return true if this interval intersects \p other.
  bool intersects(Segment other) const {
    for (auto &s : segments_) {
      if (s.intersects(other))
        return true;
    }
    return false;
  }

  /// \return true if this interval intersects \p other.
  bool intersects(const Interval &other) const {
    Segment a(start(), end());
    Segment b(other.start(), other.end());
    return a.intersects(b);
  }

  /// Join the range of the other interval into the current interval.
  void add(const Interval &other) {
    for (auto &S : other.segments_) {
      add(S);
    }
  }

  /// Join the range of the other segment into the current interval.
  void add(Segment other) {
    for (auto &s : segments_) {
      if (s.intersects(other) || s.touches(other)) {
        s.merge(other);
        return;
      }
    }
    segments_.push_back(other);
  }

  /// \returns a new compressed interval.
  Interval compress() const {
    Interval t;
    for (auto &s : segments_) {
      t.add(s);
    }
    return t;
  }

  /// \returns the size represented by the interval.
  size_t size() const {
    if (segments_.size())
      return end() - start();

    return 0;
  }

  size_t start() const {
    assert(segments_.size() && "No segments in interval!");
    size_t start = segments_[0].start_;
    for (auto &S : segments_) {
      start = std::min(start, S.start_);
    }
    return start;
  }

  size_t end() const {
    assert(segments_.size() && "No segments in interval!");
    size_t start = segments_[0].end_;
    for (auto &S : segments_) {
      start = std::max(start, S.end_);
    }
    return start;
  }
};

/// A register allocator that uses livenes information to allocate registers
/// correctly.
class RegisterAllocator {
  /// Represents the liveness info for one block.
  struct BlockLifetimeInfo {
    BlockLifetimeInfo() = default;
    void init(unsigned size) {
      gen_.resize(size);
      kill_.resize(size);
      liveIn_.resize(size);
      liveOut_.resize(size);
      maskIn_.resize(size);
    }
    /// Which live values are used in this block.
    BitVector gen_;
    /// Which live values are defined in this block.
    BitVector kill_;
    /// Which values are marked as live-in, coming into this basic block.
    BitVector liveIn_;
    /// Which values are marked as live-in, coming out of this basic block.
    BitVector liveOut_;
    /// Which values are *masked* as live-in, coming into this basic block. The
    /// mask-in bit vector is used for blocking the flow in specific blocks.
    /// We use this to block the flow of phi values and enforce flow-sensitive
    /// liveness.
    BitVector maskIn_;
  };

  /// Maps active slots (per bit) for each basic block.
  llvh::DenseMap<BasicBlock *, BlockLifetimeInfo> blockLiveness_;

  /// Maps index numbers to instructions.
  llvh::DenseMap<Instruction *, unsigned> instructionNumbers_;
  /// Maps instructions to a index numbers.
  llvh::SmallVector<Instruction *, 32> instructionsByNumbers_;
  /// Holds the live interval of each instruction.
  llvh::SmallVector<Interval, 32> instructionInterval_;

  /// How many registers are numbers.
  /// Allocated regs [0, numberRegCount_).
  /// Populated after reorderRegsByType.
  uint32_t numberRegCount_ = 0;

  /// How many registers are nonPointers.
  /// Allocated regs [numberRegCount_, numberRegCount_ + nonPointerRegCount_).
  /// Populated after reorderRegsByType.
  uint32_t nonPtrRegCount_ = 0;

  /// Returns the last index allocated.
  unsigned getMaxInstrIndex() {
    return instructionsByNumbers_.size();
  }

  /// Computes the liveness information for block \p BB in \p livenessInfo.
  void calculateLocalLiveness(BlockLifetimeInfo &livenessInfo, BasicBlock *BB);

  /// Computes the global liveness across the whole function.
  void calculateGlobalLiveness(ArrayRef<BasicBlock *> order);

  /// Calculates the live intervals for each instruction.
  void calculateLiveIntervals(ArrayRef<BasicBlock *> order);

  /// Coalesce registers by merging the live intervals of multiple instructions
  /// together to take advantage of holes. Updates \p map with mapping between
  /// the coalesced interval and the interval it was merged into and the
  /// register that it will adopt.
  /// The order of the basic blocks is passed in \p order.
  void coalesce(
      DenseMap<Instruction *, Instruction *> &map,
      ArrayRef<BasicBlock *> order);

 protected:
  /// Keeps track of the already allocated values.
  llvh::DenseMap<Value *, Register> allocated{};

  /// The register file.
  RegisterFile file{};

  /// If the function has fewer than this number of instructions,
  /// assign registers sequentially instead of being smart about it.
  unsigned fastPassThreshold = 0;

  /// If allocation is expected to take more than this number of bytes of
  /// memory, use the fast pass instead. This can protect against certain
  /// degenerate cases.
  uint64_t memoryLimit = -1;

  /// Allocate the registers for the instructions in the function in a trivial,
  /// suboptimal, but very fast way.
  void allocateFastPass(ArrayRef<BasicBlock *> order);

  Function *F;

  /// Whether there are any try/catch statements in the function.
  bool hasTry_ = false;

 public:
  using RegisterType = Register;

  /// Dump the status of the allocator in a textual form.
  void dump();

  /// \returns the computed live interval for the instruction \p I.
  Interval &getInstructionInterval(Instruction *I);

  explicit RegisterAllocator(Function *func);

  virtual ~RegisterAllocator() = default;

  Context &getContext() {
    return F->getContext();
  }

  void setFastPassThreshold(unsigned maxInstCount) {
    fastPassThreshold = maxInstCount;
  }

  void setMemoryLimit(uint64_t memoryLimitInBytes) {
    memoryLimit = memoryLimitInBytes;
  }

  /// \returns the index of instruction \p I.
  unsigned getInstructionNumber(Instruction *I);

  /// \returns true if the instruction \p already has a number.
  bool hasInstructionNumber(Instruction *I);

  /// Checks if the instruction \p I is manipulated by the target.
  virtual bool hasTargetSpecificLowering(Instruction *I) {
    return false;
  }

  /// \returns true if the interval for \p I is allocated manually.
  bool isManuallyAllocatedInterval(Instruction *I);

  /// Performs target specific lowering for \p I.
  virtual void handleInstruction(Instruction *I) {}

  /// Lower the PHI nodes in the program into a sequence of MOVs in the
  /// predecessor blocks.
  void lowerPhis(ArrayRef<BasicBlock *> order);

  /// Allocate the registers for the instructions in the function. The blocks
  /// must be in reverse-post-order, and must match the order which we'll use
  /// for instruction selection.
  void allocate(ArrayRef<BasicBlock *> order);

  /// Reserves consecutive registers that will be manually managed by the user.
  /// \p values is a list of values to be assigned consecutive registers.
  ///  nullptr values are also allocated a register but not registered.
  /// \returns the first register in the sequence.
  Register reserve(RegClass regClass, ArrayRef<Value *> values);

  /// Reserves \n count registers that will be manually managed by the user.
  Register reserve(RegClass regClass, unsigned count = 1);

  /// Free a register that was allocated with 'reserve'.
  void free(Register reg);

  /// \return the register allocated for the value \p V.
  Register getRegister(Value *I);

  /// \return the register allocated for the value \p V which may be None.
  hermes::OptValue<Register> getOptionalRegister(Value *I) const;

  /// Marks the value \p as being allocated to \p R.
  void updateRegister(Value *I, Register R);

  /// \return true if the value \p V has been allocated.
  bool isAllocated(Value *I);

  /// \returns the highest number of registers that are used concurrently.
  /// In here we assume that the registers are allocated consecutively
  /// and that allocating this number of registers will cover all of the
  /// registers that were allocated during the lifetime of the program.
  unsigned getMaxRegisterUsage(RegClass regClass) const {
    return file.getMaxRegisterUsage(regClass);
  }

  /// Convert all type-specific registers to RegClass::Other by modifying the
  /// register files. Used when spilling to allow us to have low HVM register
  /// indices by clearing out anything before Other.
  void convertTypeSpecificRegsToOther();

 private:
  /// \return the RegClass in which \p inst should be allocated.
  RegClass getRegClass(Instruction *inst);
};

} // namespace hermes

namespace llvh {
class raw_ostream;

// Print Register to llvm debug/error streams.
raw_ostream &operator<<(raw_ostream &OS, const hermes::Register &reg);
raw_ostream &operator<<(raw_ostream &OS, const hermes::Interval &interval);
raw_ostream &operator<<(raw_ostream &OS, const hermes::Segment &segment);
template <>
struct DenseMapInfo<hermes::Register> {
  static inline hermes::Register getEmptyKey() {
    return hermes::Register();
  }
  static inline hermes::Register getTombstoneKey() {
    return hermes::Register::getTombstoneKey();
  }
  static unsigned getHashValue(hermes::Register Val);
  static bool isEqual(hermes::Register LHS, hermes::Register RHS);
};

} // namespace llvh

#endif
