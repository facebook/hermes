/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Regex/Executor.h"
#include "hermes/Regex/RegexTraits.h"
#include "hermes/Support/ErrorHandling.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/TrailingObjects.h"

// This file contains the machinery for executing a regexp compiled to bytecode.

namespace hermes {
namespace regex {

template <class Traits>
struct State;

/// \return whether a regex match performed using the given \p flags can
/// possibly match the given \p constraints.
inline bool flagsSatisfyConstraints(
    constants::MatchFlagType flags,
    MatchConstraintSet constraints) {
  if ((constraints & MatchConstraintNonASCII) &&
      (flags & constants::matchInputAllAscii))
    return false;

  if ((constraints & MatchConstraintAnchoredAtStart) &&
      (flags & constants::matchPreviousCharAvailable))
    return false;

  return true;
}

/// The kind of error that occurred when trying to find a match.
enum class MatchRuntimeErrorType {
  /// No error occurred.
  None,

  /// Reached maximum stack depth while searching for match.
  MaxStackDepth,

};

/// An enum describing Width1 opcodes. This is the set of regex opcodes which
/// always match exactly one character (or fail). This is broken out from Opcode
/// to get exhaustiveness checking in switch statements. Note that conversions
/// can be performed via static_cast.
enum class Width1Opcode : uint8_t {
  MatchChar8 = (uint8_t)Opcode::MatchChar8,
  MatchChar16 = (uint8_t)Opcode::MatchChar16,
  MatchCharICase8 = (uint8_t)Opcode::MatchCharICase8,
  MatchCharICase16 = (uint8_t)Opcode::MatchCharICase16,
  MatchAnyButNewline = (uint8_t)Opcode::MatchAnyButNewline,
  Bracket = (uint8_t)Opcode::Bracket,
};

/// A CapturedRange represents a range of the input string captured by a capture
/// group. A CaptureGroup may also not have matched, in which case its start is
/// set to kNotMatched. Note that an unmatched capture group is different than a
/// capture group that matched an empty string.
static constexpr uint32_t kNotMatched = UINT32_MAX;
struct CapturedRange {
  /// Index of the first captured character, or kNotMatched if not matched.
  uint32_t start;

  /// One past the index of the last captured character.
  uint32_t end;
};

/// LoopData tracks information about a loop during a match attempt. Each State
/// has one LoopData per loop.
struct LoopData {
  /// The number of times that this loop has executed in this state.
  uint32_t iterations;

  /// The input position where we entered the loop.
  uint32_t entryPosition;
};

/// A Context records global information about a match attempt.
template <class Traits>
struct Context {
  using CodeUnit = typename Traits::CodeUnit;
  using CodePoint = typename Traits::CodePoint;

  /// The set of backtracking opcodes. These are interpreted by the backtrack()
  /// function.
  enum class BacktrackOp : uint8_t {
    /// Set the value of a capture group to a stored value.
    SetCaptureGroup,

    /// Set the value of a loop data to a stored value.
    SetLoopData,

    /// Set the IP and position in the input string to a stored value.
    SetPosition,

    /// Backtrack by entering the body of a non-greedy loop.
    EnterNonGreedyLoop,

    /// Backtrack a greedy loop whose body matches exactly one character, such
    /// as /.*/.
    GreedyWidth1Loop,

    /// Backtrack a nongreedy loop whose body matches exactly one character,
    /// such as /.*?/.
    NongreedyWidth1Loop,
  };

  /// An instruction describing how to backtrack.
  union BacktrackInsn {
    /// The operation to perform.
    BacktrackOp op;

    /// List of instruction-specific fields. Note that the opcode is reproduced
    /// in every struct; this avoids padding between the opcode and the
    /// following field.

    /// Fields used by setCaptureGroup instruction.
    struct {
      BacktrackOp op;
      uint16_t mexp; /// Which capture group to set.
      CapturedRange range; /// Value to set.
    } setCaptureGroup;

    /// Fields used by SetLoopData instruction.
    struct {
      BacktrackOp op;
      uint16_t loopId; /// Which loop to set.
      LoopData loopData; /// Value to set.
    } setLoopData;

    /// Fields used by SetPosition instruction.
    struct {
      BacktrackOp op;
      uint32_t ip; /// Instruction pointer to set.
      const CodeUnit *value; /// Input string position to set.
    } setPosition;

    /// Fields used by EnterNonGreedyLoop instruction.
    struct {
      BacktrackOp op;
      uint32_t bodyIp; /// The IP of the loop body.
      LoopData loopData; /// Data for the loop to set.
      const BeginLoopInsn *loopInsn; /// The loop instruction.
    } enterNonGreedyLoop;

    /// Fields used by GreedyWidth1Loop and NongreedyWidth1Loop.
    struct {
      BacktrackOp op; /// The opcode.
      uint32_t continuation; /// The ip for the not-taken branch of the loop.
      const CodeUnit *min; /// The minimum possible match position.
      const CodeUnit *max; /// The maximum possible match position.
    } width1Loop;

    /* implicit */ BacktrackInsn(BacktrackOp op) : op(op) {}

    /// \return a SetCaptureGroup instruction.
    static BacktrackInsn makeSetCaptureGroup(
        uint16_t mexp,
        CapturedRange range) {
      BacktrackInsn result{BacktrackOp::SetCaptureGroup};
      result.setCaptureGroup.mexp = mexp;
      result.setCaptureGroup.range = range;
      return result;
    }

    /// \return a SetLoopData instruction.
    static BacktrackInsn makeSetLoopData(uint16_t loopId, LoopData loopData) {
      BacktrackInsn result{BacktrackOp::SetLoopData};
      result.setLoopData.loopId = loopId;
      result.setLoopData.loopData = loopData;
      return result;
    }

    /// \return a SetPosition instruction.
    static BacktrackInsn makeSetPosition(
        uint32_t ip,
        const CodeUnit *inputPos) {
      BacktrackInsn result = BacktrackOp::SetPosition;
      result.setPosition.ip = ip;
      result.setPosition.value = inputPos;
      return result;
    }

    /// \return an EnterNonGreedyLoop instruction.
    static BacktrackInsn makeEnterNonGreedyLoop(
        const BeginLoopInsn *loopInsn,
        uint32_t bodyIp,
        LoopData loopData) {
      BacktrackInsn result = BacktrackOp::EnterNonGreedyLoop;
      result.enterNonGreedyLoop.bodyIp = bodyIp;
      result.enterNonGreedyLoop.loopInsn = loopInsn;
      result.enterNonGreedyLoop.loopData = loopData;
      return result;
    }
  };

  /// Our stack of backtrack instructions.
  using BacktrackStack = llvm::SmallVector<BacktrackInsn, 64>;

  /// The maximum depth of our backtracking stack. Beyond this we return a stack
  /// overflow error.
  static constexpr size_t kMaxBacktrackDepth = 1u << 24;

  /// The stream of bytecode instructions, including the header.
  llvm::ArrayRef<uint8_t> bytecodeStream_;

  /// The flags associated with the match attempt.
  constants::MatchFlagType flags_;

  /// Syntax flags associated with the regex.
  constants::SyntaxFlags syntaxFlags_;

  /// The first character in the input string.
  const CodeUnit *first_;

  /// The end of the input string (one-past the last).
  const CodeUnit *last_;

  /// Count of submatches.
  uint32_t markedCount_;

  /// Count of loops.
  uint32_t loopCount_;

  /// Traits used for canonicalization.
  Traits traits_;

  /// Whether an error occurred during the regex matching.
  MatchRuntimeErrorType error_ = MatchRuntimeErrorType::None;

  Context(
      llvm::ArrayRef<uint8_t> bytecodeStream,
      constants::MatchFlagType flags,
      constants::SyntaxFlags syntaxFlags,
      const CodeUnit *first,
      const CodeUnit *last,
      uint32_t markedCount,
      uint32_t loopCount)
      : bytecodeStream_(bytecodeStream),
        flags_(flags),
        syntaxFlags_(syntaxFlags),
        first_(first),
        last_(last),
        markedCount_(markedCount),
        loopCount_(loopCount) {}

  /// Run the given State \p state, by starting at \p pos and acting on its
  /// ip_ until the match succeeds or fails. If \p onlyAtStart is set, only
  /// test the match at \pos; otherwise test all successive input positions from
  /// pos_ through last_.
  /// \return a pointer to the start of the match if the match succeeds, nullptr
  /// if it fails. If the match succeeds, populates \p state with the state of
  /// the successful match; on failure the state's contents are undefined.
  /// Note the end of the match can be recovered as state->current_.
  const CodeUnit *
  match(State<Traits> *state, const CodeUnit *pos, bool onlyAtStart);

  /// Backtrack the given state \p s with the backtrack stack \p bts.
  /// \return true if we backatracked, false if we exhausted the stack.
  bool backtrack(BacktrackStack &bts, State<Traits> *s);

  /// Set the state's position to the body of a non-greedy loop.
  bool performEnterNonGreedyLoop(
      State<Traits> *s,
      const BeginLoopInsn *loop,
      uint32_t bodyIp,
      LoopData loopData,
      BacktrackStack &backtrackStack);

  /// Add a backtrack instruction to the backtrack stack \p bts.
  /// On overflow, set error_ to Overflow.
  /// \return true on success, false if we overflow.
  bool pushBacktrack(BacktrackStack &bts, BacktrackInsn insn) {
    bts.push_back(insn);
    if (bts.size() > kMaxBacktrackDepth) {
      error_ = MatchRuntimeErrorType::MaxStackDepth;
      return false;
    }
    return true;
  }

  /// Run the given Width1Loop \p insn on the given state \p s with the
  /// backtrack stack \p bts.
  /// \return true on success, false if we should backtrack.
  bool matchWidth1Loop(
      const Width1LoopInsn *insn,
      State<Traits> *s,
      BacktrackStack &bts);

 private:
  /// Do initialization of the given state before it enters the loop body
  /// described by the LoopInsn \p loop, including setting up any backtracking
  /// state.
  /// \return true if backtracking was prepared, false if it overflowed.
  bool prepareToEnterLoopBody(
      State<Traits> *state,
      const BeginLoopInsn *loop,
      BacktrackStack &bts);

  /// Given a Width1Opcode \p w1opcode, return true if the given char \p c
  /// matches the instruction \p insn (with that opcode).
  template <Width1Opcode w1opcode>
  inline bool matchWidth1(const Insn *insn, CodeUnit c) const;
  /// \return true if all chars, stored in contiguous memory after \p insn,
  /// match the chars in state \p s in the same order, case insensitive. Note
  /// the count of chars is given in \p insn.
  inline bool matchesNCharICase8(
      const MatchNCharICase8Insn *insn,
      State<Traits> &s);

  /// Execute the given Width1 instruction \p loopBody on string \p pos up to \p
  /// max times. \return the number of matches made, not to exceed \p max.
  template <Width1Opcode w1opcode>
  inline uint32_t
  matchWidth1LoopBody(const Insn *loopBody, const CodeUnit *pos, uint32_t max);
};

/// We store loop and captured range data contiguously in a single allocation at
/// the end of the State. Use this union to simplify the use of
/// llvm::TrailingObjects.
union LoopOrCapturedRange {
  struct LoopData loopData;
  struct CapturedRange capturedRange;
};

/// State represents a set of in-flight capture groups and loop datas, along
/// with the IP and input position.
template <typename Traits>
struct State {
  using CharT = typename Traits::CodeUnit;

  /// The current character in the input string.
  const CharT *current_ = nullptr;

  /// The instruction pointer position in the bytecode stream.
  uint32_t ip_ = 0;

  /// List of captured ranges. This has size equal to the number of marked
  /// subexpressions for the regex.
  llvm::SmallVector<CapturedRange, 16> capturedRanges_;

  /// List of loop datas. This has size equal to the number of loops for the
  /// regex.
  llvm::SmallVector<LoopData, 16> loopDatas_;

  /// \return the loop data at index \p idx.
  LoopData &getLoop(uint32_t idx) {
    assert(idx < loopDatas_.size() && "Invalid loop index");
    return loopDatas_[idx];
  }

  /// \return the captured range at index \p idx.
  CapturedRange &getCapturedRange(uint32_t idx) {
    // Captured ranges are allocated after loops, so add the loop count.
    assert(idx < capturedRanges_.size() && "Invalid captured range index");
    return capturedRanges_[idx];
  }

  /// Construct a state that can hold \p markedCount submatches and \p loopCount
  /// loop datas.
  State(uint32_t markedCount, uint32_t loopCount)
      : capturedRanges_(markedCount, {kNotMatched, kNotMatched}),
        loopDatas_(loopCount, {0, 0}) {}

  State(const State &) = default;
  State &operator=(const State &) = default;
  State(State &&) = default;
  State &operator=(State &&) = default;
};

/// ES5.1 7.3
template <class CharT>
bool isLineTerminator(CharT c) {
  return c == u'\u000A' || c == u'\u000D' || c == u'\u2028' || c == u'\u2029';
}

template <class Traits>
bool matchesLeftAnchor(Context<Traits> &ctx, State<Traits> &s) {
  bool matchesAnchor = false;
  if (s.current_ == ctx.first_ &&
      !(ctx.flags_ & constants::matchPreviousCharAvailable)) {
    // Beginning of text.
    matchesAnchor = true;
  } else if (
      (ctx.syntaxFlags_ & constants::multiline) &&
      (s.current_ > ctx.first_ ||
       (ctx.flags_ & constants::matchPreviousCharAvailable)) &&
      isLineTerminator(s.current_[-1])) {
    // Multiline and after line terminator.
    matchesAnchor = true;
  }
  return matchesAnchor;
}

template <class Traits>
bool matchesRightAnchor(Context<Traits> &ctx, State<Traits> &s) {
  bool matchesAnchor = false;
  if (s.current_ == ctx.last_ && !(ctx.flags_ & constants::matchNotEndOfLine)) {
    matchesAnchor = true;
  } else if (
      (ctx.syntaxFlags_ & constants::multiline) && (s.current_ < ctx.last_) &&
      isLineTerminator(s.current_[0])) {
    matchesAnchor = true;
  }
  return matchesAnchor;
}

/// \return true if all chars, stored in contiguous memory after \p insn,
/// match the chars in state \p s in the same order. Note the count of chars
/// is given in \p insn.
template <class Traits>
bool matchesNChar8(const MatchNChar8Insn *insn, State<Traits> &s) {
  auto insnCharPtr = reinterpret_cast<const char *>(insn + 1);
  auto charCount = insn->charCount;
  for (int offset = 0; offset < charCount; offset++) {
    if (s.current_[offset] != insnCharPtr[offset]) {
      return false;
    }
  }
  return true;
}

template <class Traits>
bool Context<Traits>::matchesNCharICase8(
    const MatchNCharICase8Insn *insn,
    State<Traits> &s) {
  auto insnCharPtr = reinterpret_cast<const char *>(insn + 1);
  auto charCount = insn->charCount;
  for (int offset = 0; offset < charCount; offset++) {
    char c = s.current_[offset];
    char instC = insnCharPtr[offset];
    if (c != instC && (char32_t)traits_.canonicalize(c) != (char32_t)instC) {
      return false;
    }
  }
  return true;
}

/// \return true if the character \p ch matches a bracket instruction \p insn,
/// containing the bracket ranges \p ranges. Note the count of ranges is given
/// in \p insn.
template <class Traits>
bool bracketMatchesChar(
    const Context<Traits> &ctx,
    const BracketInsn *insn,
    const BracketRange32 *ranges,
    char16_t ch) {
  const auto &traits = ctx.traits_;
  // Note that if the bracket is negated /[^abc]/, we want to return true if we
  // do not match, false if we do. Implement this by xor with the negate flag.

  // Check character classes.
  // Note we don't have to canonicalize here, because canonicalization does not
  // affect which character class a character is in (i.e. a character doesn't
  // become a digit after uppercasing).
  if (insn->positiveCharClasses || insn->negativeCharClasses) {
    for (auto charClass : {CharacterClass::Digits,
                           CharacterClass::Spaces,
                           CharacterClass::Words}) {
      if ((insn->positiveCharClasses & charClass) &&
          traits.characterHasType(ch, charClass))
        return true ^ insn->negate;
      if ((insn->negativeCharClasses & charClass) &&
          !traits.characterHasType(ch, charClass))
        return true ^ insn->negate;
    }
  }

  bool contained =
      traits.rangesContain(llvm::makeArrayRef(ranges, insn->rangeCount), ch);
  return contained ^ insn->negate;
}

/// Do initialization of the given state before it enters the loop body
/// described by the LoopInsn \p loop, including setting up any backtracking
/// state.
/// \return true if backtracking was prepared, false if it overflowed.
template <class Traits>
bool Context<Traits>::prepareToEnterLoopBody(
    State<Traits> *s,
    const BeginLoopInsn *loop,
    BacktrackStack &bts) {
  LoopData &loopData = s->getLoop(loop->loopId);
  if (!pushBacktrack(
          bts, BacktrackInsn::makeSetLoopData(loop->loopId, loopData))) {
    return false;
  }
  loopData.iterations++;
  loopData.entryPosition = s->current_ - first_;

  // Backtrack and reset contained capture groups.
  for (uint32_t mexp = loop->mexpBegin; mexp != loop->mexpEnd; mexp++) {
    auto &captureRange = s->getCapturedRange(mexp);
    if (!pushBacktrack(
            bts, BacktrackInsn::makeSetCaptureGroup(mexp, captureRange))) {
      return false;
    }
    captureRange = {kNotMatched, kNotMatched};
  }
  return true;
}

template <class Traits>
bool Context<Traits>::performEnterNonGreedyLoop(
    State<Traits> *s,
    const BeginLoopInsn *loop,
    uint32_t bodyIp,
    LoopData loopData,
    BacktrackStack &backtrackStack) {
  assert(loop->opcode == Opcode::BeginLoop && "Not a BeginLoopInsn");
  s->getLoop(loop->loopId) = loopData;

  // Set the IP and input position, and initialize the state for entering the
  // loop.
  s->ip_ = bodyIp;
  s->current_ = first_ + loopData.entryPosition;
  prepareToEnterLoopBody(s, loop, backtrackStack);
  return true;
}

template <class Traits>
bool Context<Traits>::backtrack(BacktrackStack &bts, State<Traits> *s) {
  while (!bts.empty()) {
    BacktrackInsn &binsn = bts.back();
    switch (binsn.op) {
      case BacktrackOp::SetCaptureGroup:
        s->getCapturedRange(binsn.setCaptureGroup.mexp) =
            binsn.setCaptureGroup.range;
        bts.pop_back();
        break;

      case BacktrackOp::SetLoopData:
        s->getLoop(binsn.setLoopData.loopId) = binsn.setLoopData.loopData;
        bts.pop_back();
        break;

      case BacktrackOp::SetPosition:
        s->current_ = binsn.setPosition.value;
        s->ip_ = binsn.setPosition.ip;
        bts.pop_back();
        return true;

      case BacktrackOp::EnterNonGreedyLoop: {
        auto fields = binsn.enterNonGreedyLoop;
        bts.pop_back();
        performEnterNonGreedyLoop(
            s, fields.loopInsn, fields.bodyIp, fields.loopData, bts);
        return true;
      }

      case BacktrackOp::GreedyWidth1Loop:
      case BacktrackOp::NongreedyWidth1Loop: {
        // In both of these instructions, we have a range [min, max] containing
        // possible match locations, and the match failed at the max location
        // (if we are greedy) or the min location (nongreedy). Backtrack by
        // decrementing the max (incrementing the min) if we are greedy
        // (nongreedy), setting the IP to that location, and jumping to the loop
        // exit.
        assert(
            binsn.width1Loop.min <= binsn.width1Loop.max &&
            "Loop min should be <= max");
        if (binsn.width1Loop.min == binsn.width1Loop.max) {
          // We have backtracked as far as possible. Give up.
          bts.pop_back();
          break;
        }
        if (binsn.op == BacktrackOp::GreedyWidth1Loop) {
          binsn.width1Loop.max--;
          s->current_ = binsn.width1Loop.max;
        } else {
          binsn.width1Loop.min++;
          s->current_ = binsn.width1Loop.min;
        }
        s->ip_ = binsn.width1Loop.continuation;
        return true;
      }
    }
  }
  // Exhausted the backtracking stack.
  return false;
}

template <class Traits>
template <Width1Opcode w1opcode>
bool Context<Traits>::matchWidth1(const Insn *base, CodeUnit c) const {
  // Note this switch should resolve at compile time.
  assert(
      base->opcode == static_cast<Opcode>(w1opcode) &&
      "Instruction has wrong opcode");
  switch (w1opcode) {
    case Width1Opcode::MatchChar8: {
      const auto *insn = llvm::cast<MatchChar8Insn>(base);
      return c == insn->c;
    }

    case Width1Opcode::MatchChar16: {
      const auto *insn = llvm::cast<MatchChar16Insn>(base);
      return c == insn->c;
    }

    case Width1Opcode::MatchCharICase8: {
      const auto *insn = llvm::cast<MatchCharICase8Insn>(base);
      return c == (CodePoint)insn->c ||
          (CodePoint)traits_.canonicalize(c) == (CodePoint)insn->c;
    }

    case Width1Opcode::MatchCharICase16: {
      const auto *insn = llvm::cast<MatchCharICase16Insn>(base);
      return c == insn->c ||
          (char32_t)traits_.canonicalize(c) == (char32_t)insn->c;
    }

    case Width1Opcode::MatchAnyButNewline:
      return !isLineTerminator(c);

    case Width1Opcode::Bracket: {
      // BracketInsn is followed by a list of BracketRange32s.
      const BracketInsn *insn = llvm::cast<BracketInsn>(base);
      const BracketRange32 *ranges =
          reinterpret_cast<const BracketRange32 *>(insn + 1);
      return bracketMatchesChar<Traits>(*this, insn, ranges, c);
    }
  }
  llvm_unreachable("Invalid width 1 opcode");
}

template <class Traits>
template <Width1Opcode w1opcode>
uint32_t Context<Traits>::matchWidth1LoopBody(
    const Insn *insn,
    const CodeUnit *pos,
    uint32_t max) {
  uint32_t iters = 0;
  for (; iters < max; iters++) {
    if (!matchWidth1<w1opcode>(insn, pos[iters]))
      break;
  }
  return iters;
}

template <class Traits>
bool Context<Traits>::matchWidth1Loop(
    const Width1LoopInsn *insn,
    State<Traits> *s,
    BacktrackStack &bts) {
  const CodeUnit *pos = s->current_;
  uint32_t matched = 0, minMatch = insn->min, maxMatch = insn->max;

  // Limit our max to the smaller of the maximum in the loop and number of
  // number of characters remaining. This allows us to avoid having to test for
  // end of input in the loop body.
  uint32_t remaining = last_ - pos;
  maxMatch = std::min(remaining, maxMatch);

  // The loop body follows the loop instruction.
  const Insn *body = static_cast<const Insn *>(&insn[1]);

  // Match as far as we can up to maxMatch. Note we do this even if the loop is
  // non-greedy: we compute how far we might conceivably have to backtrack
  // (except in non-greedy loops we're "backtracking" by moving forwards).
  using W1 = Width1Opcode;
  switch (static_cast<Width1Opcode>(body->opcode)) {
    case W1::MatchChar8:
      matched = matchWidth1LoopBody<W1::MatchChar8>(body, pos, maxMatch);
      break;
    case W1::MatchChar16:
      matched = matchWidth1LoopBody<W1::MatchChar16>(body, pos, maxMatch);
      break;
    case W1::MatchCharICase8:
      matched = matchWidth1LoopBody<W1::MatchCharICase8>(body, pos, maxMatch);
      break;
    case W1::MatchCharICase16:
      matched = matchWidth1LoopBody<W1::MatchCharICase16>(body, pos, maxMatch);
      break;
    case W1::MatchAnyButNewline:
      matched =
          matchWidth1LoopBody<W1::MatchAnyButNewline>(body, pos, maxMatch);
      break;
    case W1::Bracket:
      matched = matchWidth1LoopBody<W1::Bracket>(body, pos, maxMatch);
      break;
  }

  // If we iterated less than the minimum, we failed to match.
  if (matched < minMatch) {
    return false;
  }
  assert(
      minMatch <= matched && matched <= maxMatch &&
      "matched should be between min and max match count");

  // Now we know the valid match range.
  // Compute the beginning and end pointers in this range.
  const CodeUnit *minPos = pos + minMatch;
  const CodeUnit *maxPos = pos + matched;

  // If min == max (e.g. /a{3}/) then no backtracking is possible. If min < max,
  // backtracking is possible and we need to add a backtracking instruction.
  if (minPos < maxPos) {
    BacktrackInsn backtrack{insn->greedy ? BacktrackOp::GreedyWidth1Loop
                                         : BacktrackOp::NongreedyWidth1Loop};
    backtrack.width1Loop.continuation = insn->notTakenTarget;
    backtrack.width1Loop.min = minPos;
    backtrack.width1Loop.max = maxPos;
    if (!pushBacktrack(bts, backtrack)) {
      return false;
    }
  }
  // Set the state's current position to either the minimum or maximum location,
  // and point it to the exit of the loop.
  s->current_ = insn->greedy ? maxPos : minPos;
  s->ip_ = insn->notTakenTarget;
  return true;
}

template <class Traits>
auto Context<Traits>::match(
    State<Traits> *s,
    const CodeUnit *startLoc,
    bool onlyAtStart) -> const CodeUnit * {
  using State = State<Traits>;
  BacktrackStack backtrackStack;

  // Pull out the instruction portion of the bytecode, following the header.
  const uint8_t *const bytecode = &bytecodeStream_[sizeof(RegexBytecodeHeader)];

  // Save the incoming IP in case we have to loop.
  const auto startIp = s->ip_;

  // Check how many locations we'll need to check.
  // Note that we do want to check the empty range [last_, last_)
  const size_t locsToCheckCount = onlyAtStart ? 1 : 1 + (last_ - startLoc);

  // Macro used when a state fails to match.
#define BACKTRACK()                   \
  do {                                \
    if (backtrack(backtrackStack, s)) \
      goto backtrackingSucceeded;     \
    goto backtrackingExhausted;       \
  } while (0)

  for (size_t locIndex = 0; locIndex < locsToCheckCount; locIndex++) {
    const CodeUnit *potentialMatchLocation = startLoc + locIndex;
    s->current_ = potentialMatchLocation;
    s->ip_ = startIp;
  backtrackingSucceeded:
    for (;;) {
      const Insn *base = reinterpret_cast<const Insn *>(&bytecode[s->ip_]);
      switch (base->opcode) {
        case Opcode::Goal:
          return potentialMatchLocation;

        case Opcode::LeftAnchor:
          if (!matchesLeftAnchor(*this, *s))
            BACKTRACK();
          s->ip_ += sizeof(LeftAnchorInsn);
          break;

        case Opcode::RightAnchor:
          if (!matchesRightAnchor(*this, *s))
            BACKTRACK();
          s->ip_ += sizeof(RightAnchorInsn);
          break;

        case Opcode::MatchAnyButNewline:
          if (s->current_ == last_ ||
              !matchWidth1<Width1Opcode::MatchAnyButNewline>(
                  base, *s->current_))
            BACKTRACK();
          s->current_++;
          s->ip_ += sizeof(MatchAnyButNewlineInsn);
          break;

        case Opcode::U16MatchAnyButNewline: {
          hermes_fatal("Unimplemented");
          break;
        }

        case Opcode::MatchChar8: {
          if (s->current_ == last_ ||
              !matchWidth1<Width1Opcode::MatchChar8>(base, *s->current_))
            BACKTRACK();
          s->current_++;
          s->ip_ += sizeof(MatchChar8Insn);
          break;
        }

        case Opcode::MatchChar16: {
          if (s->current_ == last_ ||
              !matchWidth1<Width1Opcode::MatchChar16>(base, *s->current_))
            BACKTRACK();
          s->current_++;
          s->ip_ += sizeof(MatchChar16Insn);
          break;
        }

        case Opcode::U16MatchChar32: {
          hermes_fatal("Unimplemented");
          break;
        }

        case Opcode::MatchCharICase8: {
          if (s->current_ == last_ ||
              !matchWidth1<Width1Opcode::MatchCharICase8>(base, *s->current_))
            BACKTRACK();
          s->current_++;
          s->ip_ += sizeof(MatchCharICase8Insn);
          break;
        }

        case Opcode::MatchCharICase16: {
          if (s->current_ == last_ ||
              !matchWidth1<Width1Opcode::MatchCharICase16>(base, *s->current_))
            BACKTRACK();
          s->current_++;
          s->ip_ += sizeof(MatchCharICase16Insn);
          break;
        }

        case Opcode::U16MatchCharICase32: {
          hermes_fatal("Unimplemented");
          break;
        }

        case Opcode::MatchNChar8: {
          const auto *insn = llvm::cast<MatchNChar8Insn>(base);

          if (last_ - s->current_ < insn->charCount || !matchesNChar8(insn, *s))
            BACKTRACK();
          s->current_ += insn->charCount;
          s->ip_ += insn->totalWidth();
          break;
        }

        case Opcode::MatchNCharICase8: {
          const auto *insn = llvm::cast<MatchNCharICase8Insn>(base);

          if (last_ - s->current_ < insn->charCount ||
              !matchesNCharICase8(insn, *s))
            BACKTRACK();
          s->current_ += insn->charCount;
          s->ip_ += insn->totalWidth();
          break;
        }

        case Opcode::Alternation: {
          // We have an alternation. Determine which of our first and second
          // branches are viable. If both are, we have to split our state.
          const AlternationInsn *alt = llvm::cast<AlternationInsn>(base);
          bool primaryViable =
              flagsSatisfyConstraints(flags_, alt->primaryConstraints);
          bool secondaryViable =
              flagsSatisfyConstraints(flags_, alt->secondaryConstraints);
          if (primaryViable && secondaryViable) {
            // We need to explore both branches. Explore the primary branch
            // first, backtrack to the secondary one.
            s->ip_ += sizeof(AlternationInsn);
            if (!pushBacktrack(
                    backtrackStack,
                    BacktrackInsn::makeSetPosition(
                        alt->secondaryBranch, s->current_))) {
              return nullptr;
            }
          } else if (primaryViable) {
            s->ip_ += sizeof(AlternationInsn);
          } else if (secondaryViable) {
            s->ip_ = alt->secondaryBranch;
          } else {
            BACKTRACK();
          }
          break;
        }

        case Opcode::Jump32:
          s->ip_ = llvm::cast<Jump32Insn>(base)->target;
          break;

        case Opcode::Bracket: {
          if (s->current_ == last_ ||
              !matchWidth1<Width1Opcode::Bracket>(base, *s->current_))
            BACKTRACK();
          s->current_++;
          s->ip_ += llvm::cast<BracketInsn>(base)->totalWidth();
          break;
        }

        case Opcode::WordBoundary: {
          const WordBoundaryInsn *insn = llvm::cast<WordBoundaryInsn>(base);
          bool prevIsWordchar = false;
          if (s->current_ != first_ ||
              (flags_ & constants::matchPreviousCharAvailable))
            prevIsWordchar = traits_.characterHasType(
                s->current_[-1], CharacterClass::Words);

          bool currentIsWordchar = false;
          if (s->current_ != last_)
            currentIsWordchar =
                traits_.characterHasType(s->current_[0], CharacterClass::Words);
          bool isWordBoundary = (prevIsWordchar != currentIsWordchar);
          if (isWordBoundary ^ insn->invert)
            s->ip_ += sizeof(WordBoundaryInsn);
          else
            BACKTRACK();
          break;
        }

        case Opcode::BeginMarkedSubexpression: {
          const auto *insn = llvm::cast<BeginMarkedSubexpressionInsn>(base);
          if (!pushBacktrack(
                  backtrackStack,
                  BacktrackInsn::makeSetCaptureGroup(
                      insn->mexp - 1, {kNotMatched, kNotMatched}))) {
            return nullptr;
          }
          s->getCapturedRange(insn->mexp - 1).start = s->current_ - first_;
          s->ip_ += sizeof(BeginMarkedSubexpressionInsn);
          break;
        }

        case Opcode::EndMarkedSubexpression: {
          const auto *insn = llvm::cast<EndMarkedSubexpressionInsn>(base);
          s->getCapturedRange(insn->mexp - 1).end = s->current_ - first_;
          s->ip_ += sizeof(EndMarkedSubexpressionInsn);
          break;
        }

        case Opcode::BackRef: {
          const auto insn = llvm::cast<BackRefInsn>(base);
          CapturedRange cr = s->getCapturedRange(insn->mexp - 1);
          // Note we have to check whether cr.end has matched here, not
          // cr.start, because we may be in the middle of the capture group we
          // are examining, e.g. /(abc\1)/.
          if (cr.end == kNotMatched) {
            // Backreferences to a capture group that did not match always
            // succeed (ES5 15.10.2.9)
            s->ip_ += sizeof(BackRefInsn);
            break;
          }

          assert(
              cr.start != kNotMatched &&
              "capture group exited but not entered");
          // Check to see if we have enough space left in the string.
          const uint32_t length = cr.end - cr.start;
          if (uint32_t(last_ - s->current_) < length)
            BACKTRACK();

          // Check to see if the captured input string matches the current
          // string.
          bool matches;
          if (syntaxFlags_ & constants::icase) {
            // Case-insensitive comparison.
            matches = std::equal(
                first_ + cr.start,
                first_ + cr.end,
                s->current_,
                [&](CodeUnit a, CodeUnit b) {
                  return traits_.canonicalize(a) == traits_.canonicalize(b);
                });
          } else {
            // Direct comparison.
            matches =
                std::equal(first_ + cr.start, first_ + cr.end, s->current_);
          }
          if (!matches)
            BACKTRACK();

          s->ip_ += sizeof(BackRefInsn);
          s->current_ += length;
          break;
        }

        case Opcode::Lookahead: {
          const LookaheadInsn *insn = llvm::cast<LookaheadInsn>(base);
          bool matched = false;
          if (flagsSatisfyConstraints(flags_, insn->constraints)) {
            // Copy the state. This is because if the match fails (or if we are
            // inverted) we need to restore its capture groups.
            State savedState{*s};

            // Invoke match() recursively with our expression.
            // Save and restore the position because lookaheads do not consume
            // anything.
            s->ip_ += sizeof(LookaheadInsn);
            matched = this->match(s, s->current_, true /* onlyAtStart */);
            s->current_ = savedState.current_;

            // Restore capture groups unless we are a positive lookahead that
            // successfully matched. If we are a successfully matching positive
            // lookahead, set up backtracking to reset the capture groups. Note
            // we never backtrack INTO a successfully matched lookahead:
            // once a lookahead finds a match it forgets all other ways it Could
            // have matched. (ES 5.1 15.10.2.8 Note 2).
            if (matched && !insn->invert) {
              // Backtrack capture groups in the lookahead expression.
              for (uint32_t i = insn->mexpBegin, e = insn->mexpEnd; i < e;
                   i++) {
                CapturedRange cr = savedState.getCapturedRange(i);
                if (!pushBacktrack(
                        backtrackStack,
                        BacktrackInsn::makeSetCaptureGroup(i, cr))) {
                  error_ = MatchRuntimeErrorType::MaxStackDepth;
                  return nullptr;
                }
              }
            } else {
              // Restore the saved state.
              *s = std::move(savedState);
            }
          }

          // 'matched' tells us whether the enclosed assertion expression
          // matched the input. This instruction matched the input if it is a
          // positive assertion (invert == false) and the expression matched,
          // or a negative assertion (invert == true) and the expression did
          // not match. Hence xor with invert.
          if (matched ^ insn->invert)
            s->ip_ = insn->continuation;
          else
            BACKTRACK();
          break;
        }

        case Opcode::BeginLoop: {
          // Here we are entering a loop from outside, not jumping back into
          // it.
          const BeginLoopInsn *loop = llvm::cast<BeginLoopInsn>(base);
          s->getLoop(loop->loopId).iterations = 0;
          // Check to see if the loop body is viable. If not, and the loop has
          // a nonzero minimum iteration, then we know we won't match and we
          // can reject the state. If it does have a minimum iteration, we can
          // just skip to the not-taken target. Note that this is a static
          // property of the loop so we don't need to check it on every
          // iteration, only the first one.
          if (!flagsSatisfyConstraints(flags_, loop->loopeeConstraints)) {
            if (loop->min > 0) {
              BACKTRACK();
            } else {
              s->ip_ = loop->notTakenTarget;
              break;
            }
          }
          goto runLoop;
        }

        case Opcode::EndLoop:
          // This is reached after the body of a loop finishes executing.
          // Move the IP to the loop and run it again immediately.
          s->ip_ = llvm::cast<EndLoopInsn>(base)->target;
          base = reinterpret_cast<const Insn *>(&bytecode[s->ip_]);
          // Note fall through.

        runLoop : {
          const BeginLoopInsn *loop = llvm::cast<BeginLoopInsn>(base);
          auto &loopData = s->getLoop(loop->loopId);
          uint32_t iteration = loopData.iterations;

          uint32_t loopTakenIp = s->ip_ + sizeof(BeginLoopInsn);
          uint32_t loopNotTakenIp = loop->notTakenTarget;

          bool doLoopBody = iteration < loop->max;
          bool doNotTaken = iteration >= loop->min;

          // Check to see if we have looped more than the minimum number of
          // iterations, and if so, whether the subexpression we looped over
          // matched an empty string. ES6 21.2.2.5.1 Note 4: "once the
          // minimum number of repetitions has been satisfied, any more
          // expansions of Atom that match the empty character sequence are
          // not considered for further repetitions."
          if (iteration > loop->min &&
              first_ + loopData.entryPosition == s->current_)
            BACKTRACK();

          if (!doLoopBody && !doNotTaken) {
            BACKTRACK();
          } else if (doLoopBody && !doNotTaken) {
            prepareToEnterLoopBody(s, loop, backtrackStack);
            s->ip_ = loopTakenIp;

          } else if (doNotTaken && !doLoopBody) {
            s->ip_ = loop->notTakenTarget;
          } else {
            assert(
                doNotTaken && doLoopBody &&
                "Must be exploring loop not taken and body");
            if (!loop->greedy) {
              // Backtrack by entering this non-greedy loop.
              loopData.entryPosition = s->current_ - first_;
              if (!pushBacktrack(
                      backtrackStack,
                      BacktrackInsn::makeEnterNonGreedyLoop(
                          loop, loopTakenIp, loopData))) {
                error_ = MatchRuntimeErrorType::MaxStackDepth;
                return nullptr;
              }
              s->ip_ = loop->notTakenTarget;
            } else {
              // Backtrack by exiting this greedy loop.
              if (!pushBacktrack(
                      backtrackStack,
                      BacktrackInsn::makeSetPosition(
                          loopNotTakenIp, s->current_))) {
                error_ = MatchRuntimeErrorType::MaxStackDepth;
                return nullptr;
              }
              prepareToEnterLoopBody(s, loop, backtrackStack);
              s->ip_ = loopTakenIp;
            }
          }
          break;
        }

        case Opcode::BeginSimpleLoop: {
          // Here we are entering a simple loop from outside,
          // not jumping back into it.
          const BeginSimpleLoopInsn *loop =
              llvm::cast<BeginSimpleLoopInsn>(base);

          if (!flagsSatisfyConstraints(flags_, loop->loopeeConstraints)) {
            s->ip_ = loop->notTakenTarget;
            break;
          }

          goto runSimpleLoop;
        }

        case Opcode::EndSimpleLoop:
          s->ip_ = llvm::cast<EndSimpleLoopInsn>(base)->target;
          base = reinterpret_cast<const Insn *>(&bytecode[s->ip_]);
          // Note: fall-through.

        runSimpleLoop : {
          const BeginSimpleLoopInsn *loop =
              llvm::cast<BeginSimpleLoopInsn>(base);
          // Since this is a simple loop, we'll always need to explore both
          // exiting the loop at this point and continuing to loop.
          // Note simple loops are always greedy.
          backtrackStack.push_back(BacktrackInsn::makeSetPosition(
              loop->notTakenTarget, s->current_));
          s->ip_ += sizeof(BeginSimpleLoopInsn);
          break;
        }

        case Opcode::Width1Loop: {
          const Width1LoopInsn *loop = llvm::cast<Width1LoopInsn>(base);
          if (!matchWidth1Loop(loop, s, backtrackStack))
            BACKTRACK();
          break;
        }
      }
    }
  // The search failed at this location.
  backtrackingExhausted:
    continue;
  }
  // The match failed.
  return nullptr;
}

/// Entry point for searching a string via regex compiled bytecode.
/// Given the bytecode \p bytecode, search the range starting at \p first up to
/// (not including) \p last with the flags \p matchFlags. If the search
/// succeeds, poopulate MatchResults with the capture groups. \return true if
/// some portion of the string matched the regex represented by the bytecode,
/// false otherwise.
template <typename CharT, class Traits>
MatchRuntimeResult searchWithBytecodeImpl(
    llvm::ArrayRef<uint8_t> bytecode,
    const CharT *first,
    const CharT *last,
    MatchResults<const CharT *> &m,
    constants::MatchFlagType matchFlags) {
  assert(
      bytecode.size() >= sizeof(RegexBytecodeHeader) && "Bytecode too small");
  auto header = reinterpret_cast<const RegexBytecodeHeader *>(bytecode.data());

  // Check for match impossibility before doing anything else.
  if (!flagsSatisfyConstraints(matchFlags, header->constraints))
    return MatchRuntimeResult::NoMatch;
  auto markedCount = header->markedCount;
  auto loopCount = header->loopCount;

  Context<Traits> ctx(
      bytecode,
      matchFlags,
      static_cast<constants::SyntaxFlags>(header->syntaxFlags),
      first,
      last,
      header->markedCount,
      header->loopCount);
  State<Traits> state{markedCount, loopCount};

  // We check only one location if either the regex pattern constrains us to, or
  // the flags request it (via the sticky flag 'y').
  bool onlyAtStart = (header->constraints & MatchConstraintAnchoredAtStart) ||
      (matchFlags & constants::matchOnlyAtStart);

  auto result = MatchRuntimeResult::NoMatch;
  if (const CharT *matchStartLoc = ctx.match(&state, ctx.first_, onlyAtStart)) {
    // Match succeeded.
    m.resize(1 + markedCount);
    m[0].first = matchStartLoc;
    m[0].second = state.current_;
    m[0].matched = true;
    for (uint32_t idx = 0; idx < markedCount; idx++) {
      CapturedRange cr = state.getCapturedRange(idx);
      bool matched = (cr.start != kNotMatched);
      m[idx + 1].first = matched ? ctx.first_ + cr.start : ctx.last_;
      m[idx + 1].second = matched ? ctx.first_ + cr.end : ctx.last_;
      m[idx + 1].matched = matched;
    }
    result = MatchRuntimeResult::Match;
  }

  // A stack overflow occurred when looking for a match.
  if (ctx.error_ == MatchRuntimeErrorType::MaxStackDepth) {
    return MatchRuntimeResult::StackOverflow;
  }
  return result;
}

MatchRuntimeResult searchWithBytecode(
    llvm::ArrayRef<uint8_t> bytecode,
    const char16_t *first,
    const char16_t *last,
    MatchResults<const char16_t *> &m,
    constants::MatchFlagType matchFlags) {
  return searchWithBytecodeImpl<char16_t, UTF16RegexTraits>(
      bytecode, first, last, m, matchFlags);
}

MatchRuntimeResult searchWithBytecode(
    llvm::ArrayRef<uint8_t> bytecode,
    const char *first,
    const char *last,
    MatchResults<const char *> &m,
    constants::MatchFlagType matchFlags) {
  return searchWithBytecodeImpl<char, ASCIIRegexTraits>(
      bytecode, first, last, m, matchFlags);
}

} // namespace regex
} // namespace hermes
