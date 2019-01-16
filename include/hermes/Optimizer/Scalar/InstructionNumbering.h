#ifndef HERMES_OPTIMIZER_SCALAR_INSTRUCTION_NUMBERING_H
#define HERMES_OPTIMIZER_SCALAR_INSTRUCTION_NUMBERING_H

#include "hermes/IR/IR.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"

#include <cassert>
#include <iterator>
#include <utility>

namespace hermes {

/// Analysis that numbers instructions within a basic block.
///
/// This class iterates over a range of instructions within a basic block and
/// produces an Expression object for each instruction. An expression is like an
/// instruction, except its operands are stored differently. There are three
/// kinds of operands supported:
///
///   1. Internal operands are references to other instructions in the input
///      range. We store these by their offset in the range.
///   2. External operands are references to instructions defined outside the
///      input range. We store an index for these, ordered by first appearance.
///   3. Value operands are any other kind of operands, including literals,
///      labels, variables, parameters, basic blocks, functions, and others. We
///      store these directly as Value pointers.
///
/// Internal operands are only used when the operand is defined earlier. For
/// example, a PHI node that references a later instruction in the range would
/// store it as an External operand, not Internal.
///
/// Instruction numbering is useful for comparing the structure of different
/// sequences of instructions. If it produces the same sequence of expressions
/// for two different blocks, then those blocks are equivalent -- they always
/// produce the same values when given the same external inputs.
///
/// Here is an example, with IR instructions on the left and corresponding
/// expressions on the right (I = Internal, E = External, V = Value):
///
/// \code
///     %4 = BinaryOperatorInst '+', %2, %3    |    (I0) E0 + E1
///     %5 = BinaryOperatorInst '*', %4, 50    |    (I1) I0 * V50
///     %6 = BinaryOperatorInst '-', %4, %5    |    (I2) I0 - I1
///     %7 = BinaryOperatorInst '-', %6, %2    |    (I3) I2 - E0
/// \endcode
///
/// By default, External operands are only used for instructions. However, using
/// the ExternalFlags option, clients can specify other ValueKinds that should
/// be treated as External instead of Value operands. This makes it possible to
/// tune the definition of equivalence when comparing expressions. For example,
/// an ExternalFlags option of Instructions | Literals would mean that the
/// second expression in the example above, IO * V50, becomes I0 * E2.
///
/// This class provides an iterator interface to generate expressions one at a
/// time. For example, you can use it with a range-based for loop:
///
/// \code
///     InstructionNumbering numbering(*BB);
///     for (const Expression &expr : numbering) {
///       // Do something with expr ...
///     }
/// \endcode
///
/// It uses input iterators, so you can only traverse the sequence once.
class InstructionNumbering {
 public:
  /// Bitmask for specifying what ValueKinds are treated as External operands
  /// rather than Value operands.
  using ExternalFlags = unsigned;
  static constexpr ExternalFlags Instructions = 1 << 0;
  static constexpr ExternalFlags Parameters = 1 << 1;
  static constexpr ExternalFlags Literals = 1 << 2;

  /// Create an InstructionNumbering for the given range.
  ///
  /// \param range The input range.
  /// \param flags Bitwise OR of ExternalFlags constants.
  explicit InstructionNumbering(
      BasicBlock::range range,
      ExternalFlags flags = Instructions);

  /// Kinds of operands.
  enum class OperandKind {
    /// A reference to an earlier instruction in the input range.
    Internal,
    /// A value considered as an external input (type present in ExternalFlags).
    External,
    /// A value considered as a constant (type not present in ExternalFlags).
    Value,
  };

  /// An operand in an Expression.
  struct Operand {
    /// The operand's kind.
    OperandKind kind;
    /// If kind is Internal or External, the index; otherwise -1.
    unsigned index;
    /// If kind is Value, the Value; otherwise nullptr.
    Value *valuePtr;

    explicit Operand(OperandKind kind, unsigned index)
        : kind(kind), index(index), valuePtr(nullptr) {}
    explicit Operand(OperandKind kind, Value *valuePtr)
        : kind(kind), index(-1), valuePtr(valuePtr) {}

    bool operator==(const Operand &other) const {
      return kind == other.kind && index == other.index &&
          valuePtr == other.valuePtr;
    };
    bool operator!=(const Operand &other) const {
      return !(*this == other);
    }
  };

  /// A representation of an Instruction that stores Operands instead of Values.
  struct Expression {
    using OperandVec = llvm::SmallVector<Operand, 2>;
    Instruction::Variety variety;
    OperandVec operands;

    explicit Expression(Instruction *inst, OperandVec operands)
        : variety(inst->getVariety()), operands(std::move(operands)) {}

    bool operator==(const Expression &other) const {
      return variety == other.variety && operands == other.operands;
    }
    bool operator!=(const Expression &other) const {
      return !(*this == other);
    }
  };

  /// Input iterator over the Expressions for the input range.
  class iterator : public std::iterator<std::input_iterator_tag, Expression> {
    InstructionNumbering *numbering_;
    bool isEnd_;
    friend class InstructionNumbering;
    iterator(InstructionNumbering *numbering, bool isEnd)
        : numbering_(numbering), isEnd_(isEnd) {}

   public:
    /// Advance to the next instruction in the input range.
    iterator &operator++() {
      assert(!isEnd_ && "Cannot increment end iterator!");
      ++numbering_->currentIter_;
      isEnd_ = numbering_->currentIter_ == numbering_->endIter_;
      numbering_->processInstruction();
      return *this;
    }

    /// This iterator does not support post-increment.
    iterator operator++(int) = delete;

    /// \return the current instruction.
    Instruction *getInstruction() const {
      assert(!isEnd_ && "Cannot dereference end iterator!");
      return &*numbering_->currentIter_;
    }

    /// \return The Expression for the current instruction.
    ///
    /// NOTE: This reference is invalidated by incrementing the iterator.
    Expression &operator*() const {
      assert(!isEnd_ && "Cannot dereference end iterator!");
      assert(numbering_->expression_.hasValue() && "Expression is not set!");
      return numbering_->expression_.getValue();
    }

    Expression *operator->() const {
      return &operator*();
    }

    bool operator==(const iterator &other) const {
      return numbering_ == other.numbering_ && isEnd_ == other.isEnd_;
    }
    bool operator!=(const iterator &other) const {
      return !(*this == other);
    }
  };

  /// \return An iterator for generating expressions.
  ///
  /// NOTE: This can only be called once.
  iterator begin() {
    assert(!startedIteration_ && "Can only call begin once!");
    startedIteration_ = true;
    return iterator(this, currentIter_ == endIter_);
  }

  /// \return An iterator for the end of the range.
  iterator end() {
    return iterator(this, true);
  }

 private:
  /// Process the instruction at currentIter_ and set expression_. Set it to
  /// None if the iterator is at the end.
  void processInstruction();

  /// \return True if \p value should be considered as an External operand
  /// according to externalFlags_.
  bool isExternalKind(Value *value) const;

  /// Current location in the input range.
  BasicBlock::iterator currentIter_;
  /// End of the input range.
  const BasicBlock::iterator endIter_;

  /// Flags specifying what types to treat as External operands.
  const ExternalFlags externalFlags_;

  /// Flag to ensure the client calls begin() only once.
  bool startedIteration_{false};

  /// Expression for the instruction at currentIter_.
  llvm::Optional<Expression> expression_;

  /// Inline size for SmallDenseMap. Use this to avoid allocations in the common
  /// case of comparing two ranges, where only the first few expressions are
  /// generated before there is a mismatch.
  static constexpr size_t SMALL_SIZE = 16;

  /// Map from instructions in the input range to their offset within it.
  llvm::SmallDenseMap<Instruction *, unsigned, SMALL_SIZE> internalMap_;
  /// Map from External values to their index ordered by first use as an operand
  /// of an instruction in the range.
  llvm::SmallDenseMap<Value *, unsigned, SMALL_SIZE> externalMap_;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_INSTRUCTION_NUMBERING_H
