/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_REGEX_REGEXBYTECODE_H
#define HERMES_REGEX_REGEXBYTECODE_H

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/Casting.h"

#include <cstdint>
#include <vector>

namespace hermes {
namespace regex {

/// Define the enum class of regex opcodes.
#define REOP(code) code,
enum class Opcode : uint8_t {
#include "hermes/Regex/RegexOpcodes.def"
};

/// Type representing a jump location, as a 32 bit value.
using JumpTarget32 = uint32_t;

/// Type representing a set of MatchConstraint flags as a bitmask.
using MatchConstraintSet = uint8_t;

/// The list of Instructions corresponding to our Opcodes.
/// Our instructions are packed with byte alignment, beacuse they need ton be
/// serializable directly.
LLVM_PACKED_START

/// Base instruction type. All instructions derive from this. Note that Insn and
/// its subclasses support LLVM RTTI (isa, cast, dyn_cast, etc).
struct Insn {
  Opcode opcode;
};

struct GoalInsn : public Insn {};
struct LeftAnchorInsn : public Insn {};
struct RightAnchorInsn : public Insn {};
struct MatchAnyInsn : public Insn {};
struct U16MatchAnyInsn : public Insn {};
struct MatchAnyButNewlineInsn : public Insn {};
struct U16MatchAnyButNewlineInsn : public Insn {};
struct MatchChar8Insn : public Insn {
  char c;
};

// Matches a 16 bit character without attempting to interpret surrogate pairs.
struct MatchChar16Insn : public Insn {
  char16_t c;
};

// Matches a code point, decoding a surrogate pair if necessary.
struct U16MatchChar32Insn : public Insn {
  uint32_t c;
};

// Instructions for case-insensitive matching. c is already case-folded.
struct MatchCharICase8Insn : public Insn {
  char c;
};

// Matches a 16 bit character without attempting to interpret surrogate pairs.
struct MatchCharICase16Insn : public Insn {
  char16_t c;
};

// Matches a code point (case insensitive), decoding a surrogate pair if
// necessary.
struct U16MatchCharICase32Insn : public Insn {
  uint32_t c;
};

struct AlternationInsn : public Insn {
  /// The primary branch is the Insn following the alternation, while the
  /// secondary branch is at the secondaryBranch jump target. Both branches have
  /// constraints which determine whether they are viable.
  JumpTarget32 secondaryBranch;
  MatchConstraintSet primaryConstraints;
  MatchConstraintSet secondaryConstraints;
};
struct Jump32Insn : public Insn {
  JumpTarget32 target;
};

struct BackRefInsn : public Insn {
  uint16_t mexp;
};

/// A BracketRange represents an inclusive range of characters in a bracket,
/// such as /[a-z]/. Singletons like /[a]/ are represented as the range a-a.
struct BracketRange32 {
  uint32_t start;
  uint32_t end;
};

/// BracketInsn is a variable-width instruction. Each BracketInsn is followed by
/// a sequence of BracketRange32 in the bytecode stream.
struct BracketInsn : public Insn {
  /// Number of BracketRange32s following this instruction.
  uint32_t rangeCount;
  /// Whether the bracket is negated (leading ^).
  uint8_t negate : 1;

  /// A bitmask containing the three positive character classes \d \s \w, and a
  /// negative companion for their inverts \D \S \W. See CharacterClass::Type
  /// for the flag values.
  uint8_t positiveCharClasses : 3;
  uint8_t negativeCharClasses : 3;

  /// \return the width of this instruction plus its bracket ranges.
  uint32_t totalWidth() const {
    return sizeof(*this) + rangeCount * sizeof(BracketRange32);
  }
};

/// U16BracketInsn is a variant of BracketInsn used in Unicode regular
/// expressions. It differs in that surrogate characters are decoded.
struct U16BracketInsn : public BracketInsn {};

struct MatchNChar8Insn : public Insn {
  // number of 8-byte char following this instruction.
  uint8_t charCount;

  /// \return the width of this instruction plus its characters.
  uint32_t totalWidth() const {
    return sizeof(*this) + charCount * sizeof(char);
  }
};

struct MatchNCharICase8Insn : public Insn {
  // number of 8-byte char following this instruction.
  uint8_t charCount;

  /// \return the width of this instruction plus its characters.
  uint32_t totalWidth() const {
    return sizeof(*this) + charCount * sizeof(char);
  }
};

// See BytecodeFileFormatTest for details about bit field layouts.
static_assert(
    sizeof(BracketInsn) == 6,
    "BracketInsn should take up 6 byte total");

struct WordBoundaryInsn : public Insn {
  /// Whether the boundary is inverted (\B instead of \b).
  bool invert;
};

/// Begin/EndMarkedSubexpression stores the index of the marked subexpression.
/// Note that the first marked subexpression has index 1 (0 is reserved for
/// the entire match).
struct BeginMarkedSubexpressionInsn : public Insn {
  uint16_t mexp;
};
struct EndMarkedSubexpressionInsn : public Insn {
  uint16_t mexp;
};

/// A LookaroundInsn is immediately followed by bytecode for its contained
/// expression. It has a jump target to its continuation.
struct LookaroundInsn : public Insn {
  /// Whether we are inverted: (?!...) instead of (?=...).
  bool invert;
  /// Whether we are forwards: (?=...) instead of (?<=...).
  bool forwards;
  /// Constraints on what can match the contained expression.
  MatchConstraintSet constraints;
  // The subexpression marked regions we want to be able to backtrack.
  uint16_t mexpBegin;
  uint16_t mexpEnd;
  /// Jump target if the lookahead matches.
  JumpTarget32 continuation;
};

/// An instruction for entering a loop. This supports all loop constructs
/// available in regexp, and includes optional nongreedy support, min/max
/// counts, and enclosed capture groups. The body of the loop is the Insn
/// following the BeginLoopInsn, while the not-taken target is stored following
/// the loop body.
struct BeginLoopInsn : public Insn {
  /// The LoopID is used to index into the state to count iterations and track
  /// the entry position.
  uint32_t loopId;

  /// Minimum and maximum iterations.
  /// For an unbounded loop (including Kleene star), max is UINT32_MAX
  uint32_t min;
  uint32_t max;

  /// Range of marked subexpressions enclosed by the loop, as [begin, end).
  uint16_t mexpBegin;
  uint16_t mexpEnd;

  /// Whether the loop is greedy (i.e. * instead of *?)
  bool greedy;

  /// Set of constraints on what can match the loop body.
  MatchConstraintSet loopeeConstraints;

  /// The not-taken target for the loop.
  JumpTarget32 notTakenTarget;
};

/// An instruction that closes a loop, appearing after the loop body.
/// The target is always a BeginLoopInsn.
struct EndLoopInsn : public Insn {
  JumpTarget32 target;
};

/// An instruction for entering a simple loop. This supports only loops that:
/// - have no minimum or maximum,
/// - do not contain any capture groups,
/// - are greedy, and
/// - have a body that cannot match the empty string.
struct BeginSimpleLoopInsn : public Insn {
  /// We don't need a loop ID like BeginLoopInsn because we don't need to
  /// track the iteration count or the entry position.

  /// Set of constraints on what can match the loop body.
  MatchConstraintSet loopeeConstraints;

  /// The not-taken target for the loop.
  JumpTarget32 notTakenTarget;
};

/// An instruction that closes a simple loop, appearing after the loop body.
/// The target is always a BeginSimpleLoopInsn.
struct EndSimpleLoopInsn : public Insn {
  JumpTarget32 target;
};

/// An instruction for entering a loop whose body always matches one character
/// and does not contain any capture groups.
struct Width1LoopInsn : public Insn {
  /// The LoopID is used to index into the state to count iterations and track
  /// the entry position.
  uint32_t loopId;

  /// Minimum and maximum iterations.
  /// For an unbounded loop (including Kleene star), max is UINT32_MAX
  uint32_t min;
  uint32_t max;

  /// Whether the loop is greedy (i.e. * instead of *?)
  bool greedy;

  /// The not-taken target for the loop.
  JumpTarget32 notTakenTarget;
};

/// A header that appears at the beginning of a bytecode stream.
struct RegexBytecodeHeader {
  /// Number of capture groups.
  uint16_t markedCount;

  /// Number of loops.
  uint16_t loopCount;

  /// Syntax flags used to construct the regex.
  uint8_t syntaxFlags;

  /// Constraints on what strings can match this regex.
  MatchConstraintSet constraints;
};

LLVM_PACKED_END;

/// OpcodeFor<Instruction>::value is the opcode for the Instruction.
template <typename Instruction>
struct OpcodeFor {};

#define REOP(Code)                                \
  template <>                                     \
  struct OpcodeFor<Code##Insn> {                  \
    static constexpr Opcode value = Opcode::Code; \
  };
#include "hermes/Regex/RegexOpcodes.def"

/// A class representing a regex compiled to a bytecode stream.
class RegexBytecodeStream {
  /// The stream of instructions encoded as bytes.
  std::vector<uint8_t> bytes_;

  /// Whether our bytecode has been acquired.
  bool acquired_ = false;

 public:
  /// Type acting as a reallocation-safe pointer to an instruction.
  /// This stores a pointer to the vector and an offset, rather than a pointer
  /// into the vector contents.
  template <typename Instruction>
  class InstructionWrapper {
    std::vector<uint8_t> *const bytes_;
    const uint32_t offset_;

   public:
    Instruction *operator->() {
      Insn *base = reinterpret_cast<Insn *>(&bytes_->at(offset_));
      return llvh::cast<Instruction>(base);
    }

    InstructionWrapper(std::vector<uint8_t> *bytes, uint32_t offset)
        : bytes_(bytes), offset_(offset) {}
  };

  /// Emit an instruction.
  /// \return a dereferenceable "pointer" to the instruction in the bytecode
  /// stream.
  template <typename Instruction>
  InstructionWrapper<Instruction> emit() {
    size_t startSize = bytes_.size();
    bytes_.resize(startSize + sizeof(Instruction), 0);
    Insn *insn = reinterpret_cast<Insn *>(&bytes_[startSize]);
    insn->opcode = OpcodeFor<Instruction>::value;
    return InstructionWrapper<Instruction>(&bytes_, startSize);
  }

  /// Emit a BracketRange32.
  void emitBracketRange(BracketRange32 range) {
    const uint8_t *rangeBytes = reinterpret_cast<const uint8_t *>(&range);
    bytes_.insert(bytes_.end(), rangeBytes, rangeBytes + sizeof(range));
  }

  /// Emit a Char8 for use inside a MatchNChar8Insn or MatchNCharICase8Insn.
  void emitChar8(char c) {
    bytes_.push_back((uint8_t)c);
  }

  /// \return the current offset in the stream, which is where the next
  /// instruction will be emitted. Note the header is omitted.
  uint32_t currentOffset() const {
    return bytes_.size() - sizeof(RegexBytecodeHeader);
  }

  /// \return the bytecode, transferring ownership of it to the caller.
  std::vector<uint8_t> acquireBytecode() {
    assert(!acquired_ && "Bytecode already acquired");
    acquired_ = true;
    return std::move(bytes_);
  }

  /// Construct a RegexBytecodeStream starting with a header.
  RegexBytecodeStream(const RegexBytecodeHeader &header) {
    const uint8_t *headerBytes = reinterpret_cast<const uint8_t *>(&header);
    bytes_.insert(bytes_.end(), headerBytes, headerBytes + sizeof header);
  }
};

} // namespace regex
} // namespace hermes

namespace llvh {
/// LLVM RTTI implementation for regex instructions. Rather than defining
/// classof() for each instruction struct, which would require a lot of
/// error-prone boilerplate, we take the Casting.h header's suggestion of
/// specializing isa_impl for the case where From is just Insn and To is one of
/// its subclasses.
template <typename To, typename From>
struct isa_impl<
    To,
    From,
    typename std::enable_if<
        std::is_same<hermes::regex::Insn, From>::value &&
        std::is_base_of<hermes::regex::Insn, To>::value>::type> {
  static inline bool doit(const From &val) {
    return val.opcode == hermes::regex::OpcodeFor<To>::value;
  }
};
} // namespace llvh
#endif // HERMES_REGEX_REGEXBYTECODE_H
