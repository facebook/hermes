/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
// -*- C++ -*-
//===--------------------------- regex ------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef HERMES_REGEX_COMPILER_H
#define HERMES_REGEX_COMPILER_H

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Platform/Unicode/CodePointSet.h"
#include "hermes/Support/Compiler.h"

#include "hermes/Regex/RegexBytecode.h"

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Platform/Unicode/CodePointSet.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

#include <initializer_list>
#include <limits>
#include <string>
#include <vector>

namespace hermes {
namespace regex {

using namespace std;

namespace constants {

// SyntaxFlags
// Note these are encoded into bytecode files, so changing their values is a
// breaking change.
enum SyntaxFlags : uint8_t {
  icase = 1 << 0,
  multiline = 1 << 2,
  unicode = 1 << 3,
};

inline constexpr SyntaxFlags operator~(SyntaxFlags x) {
  return SyntaxFlags(~int(x) & 0x1FF);
}

inline constexpr SyntaxFlags operator&(SyntaxFlags x, SyntaxFlags y) {
  return SyntaxFlags(int(x) & int(y));
}

inline constexpr SyntaxFlags operator|(SyntaxFlags x, SyntaxFlags y) {
  return SyntaxFlags(int(x) | int(y));
}

inline constexpr SyntaxFlags operator^(SyntaxFlags x, SyntaxFlags y) {
  return SyntaxFlags(int(x) ^ int(y));
}

inline SyntaxFlags &operator&=(SyntaxFlags &x, SyntaxFlags y) {
  x = x & y;
  return x;
}

inline SyntaxFlags &operator|=(SyntaxFlags &x, SyntaxFlags y) {
  x = x | y;
  return x;
}

inline SyntaxFlags &operator^=(SyntaxFlags &x, SyntaxFlags y) {
  x = x ^ y;
  return x;
}

// MatchFlagType

enum MatchFlagType {
  /// Default match options.
  matchDefault = 0,

  /// ^ anchors should not treat the input start as a line start.
  matchNotBeginningOfLine = 1 << 0,

  /// $ anchors should not treat the input end as a line end.
  matchNotEndOfLine = 1 << 1,

  /// Hint that the input is composed entirely of ASCII characters.
  matchInputAllAscii = 1 << 2,

  /// Do not search for a match past the search start location.
  matchOnlyAtStart = 1 << 3,
};

inline constexpr MatchFlagType operator~(MatchFlagType x) {
  return MatchFlagType(~int(x) & 0x0FFF);
}

inline constexpr MatchFlagType operator&(MatchFlagType x, MatchFlagType y) {
  return MatchFlagType(int(x) & int(y));
}

inline constexpr MatchFlagType operator|(MatchFlagType x, MatchFlagType y) {
  return MatchFlagType(int(x) | int(y));
}

inline constexpr MatchFlagType operator^(MatchFlagType x, MatchFlagType y) {
  return MatchFlagType(int(x) ^ int(y));
}

inline MatchFlagType &operator&=(MatchFlagType &x, MatchFlagType y) {
  x = x & y;
  return x;
}

inline MatchFlagType &operator|=(MatchFlagType &x, MatchFlagType y) {
  x = x | y;
  return x;
}

inline MatchFlagType &operator^=(MatchFlagType &x, MatchFlagType y) {
  x = x ^ y;
  return x;
}

enum class ErrorType {
  /// No error occurred.
  None,

  /// An escaped value would overflow: /\xFFFFFFFFFFF/
  EscapeOverflow,

  /// incomplete escape: new RegExp("\\")
  EscapeIncomplete,

  /// Invalid escape: new RegExp("\\123", "u")
  EscapeInvalid,

  /// Mismatched [ and ].
  UnbalancedBracket,

  /// Mismatched ( and ).
  UnbalancedParenthesis,

  /// Braces have valid syntax, but the range is invalid, such as {5,3}.
  BraceRange,

  /// Invalid character range, such as [b-a].
  CharacterRange,

  /// A lone { or } was found.
  InvalidQuantifierBracket,

  /// One of *?+{ was not preceded by a valid regular expression.
  InvalidRepeat,

  /// The pattern exceeded internal limits, such as capture group or loop count.
  PatternExceedsParseLimits,
};

/// \return an error message for the given \p error.
inline const char *messageForError(ErrorType error) {
  switch (error) {
    case ErrorType::EscapeOverflow:
      return "Escaped value too large";
    case ErrorType::EscapeIncomplete:
      return "Incomplete escape";
    case ErrorType::EscapeInvalid:
      return "Invalid escape";
    case ErrorType::UnbalancedBracket:
      return "Character class not closed";
    case ErrorType::UnbalancedParenthesis:
      return "Parenthesized expression not closed";
    case ErrorType::BraceRange:
      return "Quantifier range out of order";
    case ErrorType::CharacterRange:
      return "Character class range out of order";
    case ErrorType::InvalidQuantifierBracket:
      return "Invalid quantifier bracket";
    case ErrorType::InvalidRepeat:
      return "Quantifier has nothing to repeat";
    case ErrorType::PatternExceedsParseLimits:
      return "Pattern exceeds parse limits";
    case ErrorType::None:
      return "No error";
  }
  llvm_unreachable("Unknown error");
  return nullptr;
}

/// Maximum number of supported capture groups.
/// This is therefore also the maximum valid backreference.
constexpr uint16_t kMaxCaptureGroupCount = 65535;

/// Maximum number of supported loops.
constexpr uint16_t kMaxLoopCount = 65535;

} // namespace constants

/// After compiling a regex, there are certain properties we can test for that
/// enable us to quickly rule out matches. We refer to these as
/// MatchConstraints: they constrain the strings that may match the regex.
enum MatchConstraintFlags : uint8_t {
  /// If set, ASCII strings can never match because we require at least one
  /// non-ASCII character.
  MatchConstraintNonASCII = 1 << 0,

  /// If set, the regex can only match at the beginning of the input string, due
  /// to ^ anchors.
  MatchConstraintAnchoredAtStart = 1 << 1,

  /// If set, the regex cannot possibly match an empty string, e.g. /a/
  MatchConstraintNonEmpty = 1 << 2,
};

/// \return whether a code point \p cp is ASCII.
inline bool isASCII(uint32_t c) {
  return c <= 127;
}

// Type wrapping up a character class, like \d or \S.
struct CharacterClass {
  enum Type : uint8_t {
    Digits = 1 << 0, // \d \D
    Spaces = 1 << 1, // \s \S
    Words = 1 << 2, // \w \W
  } type_;

  // Whether the class is inverted (\D instead of \d).
  bool inverted_;

  CharacterClass(Type type, bool invert) : type_(type), inverted_(invert) {}
};

class Node;

/// A NodeList is list of owned Nodes. Note it is move-only.
using NodeList = std::vector<std::unique_ptr<Node>>;

/// Base class representing some part of a compiled regular expression.
/// A Node is part of an expression that knows how to match against a State.
/// There are nodes for Alternations, Literals, etc.
/// Node itself is useful as an empty matcher: it always succeeds while
/// consuming nothing.
class Node {
 public:
  /// Nodes may not be copied or moved.
  /// They are generally managed via unique_ptr.
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;
  Node(Node &&) = delete;
  Node &operator=(Node &&) = delete;

  using CodePoint = uint32_t;
  using CodePointList = llvm::SmallVector<CodePoint, 5>;

  /// Default constructor and destructor.
  Node() = default;
  virtual ~Node() = default;

  /// Compile a list of nodes \p nodes to a bytecode stream \p bcs.
  static void compile(const NodeList &nodes, RegexBytecodeStream &bcs) {
    for (const auto &node : nodes) {
      node->emit(bcs);
    }
  }

  /// \return the match contraints for the list of nodes \p nodes.
  static MatchConstraintSet matchConstraintsForList(const NodeList &nodes) {
    MatchConstraintSet result = 0;
    for (const auto &node : nodes) {
      result |= node->matchConstraints();
    }
    return result;
  }

  /// Reverse the order of the node list \p nodes, and recursively ask each node
  /// to reverse the order of its children.
  inline static void reverseNodeList(NodeList &nodes);

  /// Perform optimizations on the given node list \p nodes, subject to the
  /// given \p flags.
  inline static void optimizeNodeList(
      NodeList &nodes,
      constants::SyntaxFlags flags);

  /// \return whether the node always matches exactly one character.
  virtual bool matchesExactlyOneCharacter() const {
    return false;
  }

  /// Perform optimization on Node's contents, subject to the given \p flags.
  virtual void optimizeNodeContents(constants::SyntaxFlags flags) {}

  /// If this Node can be coalesced into a single MatchCharNode,
  /// then add the node's characters to \p output and \return true.
  /// Otherwise \return false.
  virtual bool tryCoalesceCharacters(CodePointList *output) const {
    return false;
  }

 protected:
  /// \return the match constraints for this node.
  /// This should be overridden by subclasses to report the constraints for that
  /// node.
  virtual MatchConstraintSet matchConstraints() const {
    return 0;
  }

  /// \return whether this is a goal node.
  virtual bool isGoal() const {
    return false;
  }

  /// Reverse the order of children of this node. The default implementation
  /// does nothing, but nodes which store a child list should reverse the order
  /// of that list and then recurse.
  virtual void reverseChildren() {}

  /// Emit this node into the bytecode compiler \p bcs. This is an overrideable
  /// function - subclasses should override this to emit node-specific bytecode.
  /// The default emits nothing, so that base Node is just a no-op.
  virtual void emit(RegexBytecodeStream &bcs) const {}
};

/// GoalNode is the terminal Node that represents successful execution.
class GoalNode final : public Node {
 public:
  void emit(RegexBytecodeStream &bcs) const override {
    bcs.emit<GoalInsn>();
  }

 protected:
  bool isGoal() const override {
    return true;
  }
};

class LoopNode final : public Node {
  using Super = Node;

  /// Minimum required number of executions of the loop.
  uint32_t min_;

  /// Maximum allowed number of executions of the loop.
  /// Invariant: max_ >= min_
  uint32_t max_;

  /// Index of the loop in the regular expression.
  uint32_t loopId_;

  /// First marked subexpression contained in our looped expression.
  uint32_t mexpBegin_;

  /// One past the last marked subexpression contained in our looped expression.
  /// Marked subexpressions are [begin, end).
  uint32_t mexpEnd_;

  /// Whether this loop is greedy.
  bool greedy_;

  /// The expression that gets looped.
  NodeList loopee_;

  /// The constraints on what the loopee can match.
  MatchConstraintSet loopeeConstraints_;

 public:
  /// Construct a LoopNode with the given \p loopId.
  /// A successful loop executes at least \p min times but not more than \p max
  /// times. \p greedy controls whether the loop attempts to execute as many
  /// times as possible, or as few as possible.
  /// The marked subexpressions contained within the loop are given by \p
  /// mexpBegin and \p mexpEnd, forming the half-open range [mexpBegin,
  /// mexpEnd). The expression to be looped is given as \p loopee.
  explicit LoopNode(
      uint32_t loopId,
      uint32_t min,
      uint32_t max,
      bool greedy,
      uint32_t mexpBegin,
      uint32_t mexpEnd,
      NodeList loopee)
      : min_(min),
        max_(max),
        loopId_(loopId),
        mexpBegin_(mexpBegin),
        mexpEnd_(mexpEnd),
        greedy_(greedy),
        loopee_(move(loopee)),
        loopeeConstraints_(matchConstraintsForList(loopee_)) {}

  /// We inherit the loopee's match constraints unless the loop is optional (min
  /// is 0).
  virtual MatchConstraintSet matchConstraints() const override {
    MatchConstraintSet result = 0;
    if (min_ > 0) {
      result |= loopeeConstraints_;
    }
    return result | Super::matchConstraints();
  }

  virtual void optimizeNodeContents(constants::SyntaxFlags flags) override {
    optimizeNodeList(loopee_, flags);
  }

 protected:
  void reverseChildren() override {
    reverseNodeList(loopee_);
  }

 private:
  /// Override of emit() to compile our looped expression and add a jump
  /// back to the loop.
  virtual void emit(RegexBytecodeStream &bcs) const override {
    // The loop body comes immediately after, and we have a jump for the
    // not-taken branch.
    uint32_t loopEntryPosition = bcs.currentOffset();

    if (isWidth1Loop()) {
      assert(
          mexpBegin_ == mexpEnd_ &&
          "Width 1 loops should not contain capture groups");
      auto loopInsn = bcs.emit<Width1LoopInsn>();
      loopInsn->loopId = loopId_;
      loopInsn->min = min_;
      loopInsn->max = max_;
      loopInsn->greedy = greedy_;

      // Append the body. Width1 loops do not have or need a loop end.
      compile(loopee_, bcs);

      // Tell the loop how to exit.
      loopInsn->notTakenTarget = bcs.currentOffset();
    } else if (isSimpleLoop()) {
      auto simpleLoopInsn = bcs.emit<BeginSimpleLoopInsn>();
      simpleLoopInsn->loopeeConstraints = loopeeConstraints_;

      compile(loopee_, bcs);
      bcs.emit<EndSimpleLoopInsn>()->target = loopEntryPosition;

      simpleLoopInsn->notTakenTarget = bcs.currentOffset();
    } else {
      auto loopInsn = bcs.emit<BeginLoopInsn>();
      loopInsn->loopId = loopId_;
      loopInsn->min = min_;
      loopInsn->max = max_;
      loopInsn->mexpBegin = mexpBegin_;
      loopInsn->mexpEnd = mexpEnd_;
      loopInsn->greedy = greedy_;
      loopInsn->loopeeConstraints = loopeeConstraints_;

      // Append the body and then the loop end.
      compile(loopee_, bcs);
      bcs.emit<EndLoopInsn>()->target = loopEntryPosition;

      // Tell the loop how to exit.
      loopInsn->notTakenTarget = bcs.currentOffset();
    }
  }

  // Checks if the loopee always matches exactly one character so that we can
  /// make this a width 1 loop. See BeginWidth1LoopInsn in RegexBytecode.h.
  bool isWidth1Loop() const {
    if (loopee_.size() != 1)
      return false;
    return loopee_[0]->matchesExactlyOneCharacter();
  }

  // Checks if we can make this a "simple" loop. See BeginSimpleLoopInsn
  // in RegexBytecode.h.
  bool isSimpleLoop() const {
    return min_ == 0 && max_ == std::numeric_limits<uint32_t>::max() &&
        mexpBegin_ == mexpEnd_ && greedy_ &&
        (loopeeConstraints_ & MatchConstraintNonEmpty);
  }
};

/// AlternationNode represents a | Node in a regex.
class AlternationNode final : public Node {
  using Super = Node;

  // The alternatives of this alternation.
  // In an expression like /a|b/, firstBranch_ is a and secondBranch_ is b.
  NodeList first_;
  NodeList second_;

  // We record the match constraints for our branches, so we can prune
  // impossible alternatives.
  MatchConstraintSet firstConstraints_;
  MatchConstraintSet secondConstraints_;

 public:
  /// Constructor for an Alternation.
  /// Accepts the two branches \p firstBranch and \p secondBranch
  AlternationNode(NodeList first, NodeList second)
      : first_(move(first)),
        second_(move(second)),
        firstConstraints_(matchConstraintsForList(first_)),
        secondConstraints_(matchConstraintsForList(second_)) {}

  /// Alternations are constrained by the intersection of their branches'
  /// constraints.
  virtual MatchConstraintSet matchConstraints() const override {
    MatchConstraintSet result = firstConstraints_ & secondConstraints_;
    return result | Super::matchConstraints();
  }

  virtual void optimizeNodeContents(constants::SyntaxFlags flags) override {
    optimizeNodeList(first_, flags);
    optimizeNodeList(second_, flags);
  }

  void emit(RegexBytecodeStream &bcs) const override {
    // Instruction stream looks like:
    //   [Alternation][PrimaryBranch][Jump][SecondaryBranch][...]
    //     |____________________________|____^               ^
    //                                  |____________________|
    // Where the Alternation has a JumpOffset to its secondary branch.
    auto altInsn = bcs.emit<AlternationInsn>();
    altInsn->primaryConstraints = firstConstraints_;
    altInsn->secondaryConstraints = secondConstraints_;
    compile(first_, bcs);
    auto firstBranchCont = bcs.emit<Jump32Insn>();
    altInsn->secondaryBranch = bcs.currentOffset();
    compile(second_, bcs);
    firstBranchCont->target = bcs.currentOffset();
  }

  void reverseChildren() override {
    reverseNodeList(first_);
    reverseNodeList(second_);
  }
};

/// MarkedSubexpressionNode is a capture group.
class MarkedSubexpressionNode final : public Node {
  using Super = Node;

  // The contents of our expression.
  NodeList contents_;

  // Match constraints for our contents.
  MatchConstraintSet contentsConstraints_;

  // The index of the marked subexpression.
  uint32_t mexp_;

 public:
  explicit MarkedSubexpressionNode(NodeList contents, uint32_t mexp)
      : contents_(std::move(contents)),
        contentsConstraints_(matchConstraintsForList(contents_)),
        mexp_(mexp) {}

  virtual void emit(RegexBytecodeStream &bcs) const override {
    uint16_t mexp16 = static_cast<uint16_t>(mexp_);
    assert(mexp16 == mexp_ && "Subexpression too large");
    bcs.emit<BeginMarkedSubexpressionInsn>()->mexp = mexp16;
    compile(contents_, bcs);
    bcs.emit<EndMarkedSubexpressionInsn>()->mexp = mexp16;
  }

  void reverseChildren() override {
    reverseNodeList(contents_);
  }

  virtual void optimizeNodeContents(constants::SyntaxFlags flags) override {
    optimizeNodeList(contents_, flags);
  }

  virtual MatchConstraintSet matchConstraints() const override {
    return contentsConstraints_ | Super::matchConstraints();
  }
};

/// BackRefNode represents a backreference node.
class BackRefNode final : public Node {
  // The backreference like \3.
  uint32_t mexp_;

 public:
  explicit BackRefNode(unsigned mexp) : mexp_(mexp) {}

  virtual void emit(RegexBytecodeStream &bcs) const override {
    assert(mexp_ > 0 && "Subexpression cannot be zero");
    assert(static_cast<uint16_t>(mexp_) == mexp_ && "Subexpression too large");
    bcs.emit<BackRefInsn>()->mexp = static_cast<uint16_t>(mexp_);
  }
};

/// WordBoundaryNode represents a \b or \B assertion in a regex.
class WordBoundaryNode final : public Node {
  using Super = Node;

  /// Whether the boundary is inverted (\B instead of \b).
  bool invert_;

 public:
  WordBoundaryNode(bool invert) : invert_(invert) {}

  virtual void emit(RegexBytecodeStream &bcs) const override {
    bcs.emit<WordBoundaryInsn>()->invert = invert_;
  }
};

/// LeftAnchorNode is a ^: anchors at the beginning of a line.
class LeftAnchorNode final : public Node {
  using Super = Node;

  bool multiline_;

 public:
  LeftAnchorNode(bool multiline) : multiline_(multiline) {}

  virtual MatchConstraintSet matchConstraints() const override {
    MatchConstraintSet result = 0;
    // We are anchored at the start, unless we are multiline, in which case we
    // can match after a newline.
    if (!multiline_) {
      result |= MatchConstraintAnchoredAtStart;
    }
    return result | Super::matchConstraints();
  }

  void emit(RegexBytecodeStream &bcs) const override {
    bcs.emit<LeftAnchorInsn>();
  }
};

/// RightAnchorNode is regex $: anchors at end of the line.
class RightAnchorNode : public Node {
  using Super = Node;

 public:
  RightAnchorNode() {}

  void emit(RegexBytecodeStream &bcs) const override {
    bcs.emit<RightAnchorInsn>();
  }
};

/// MatchAnyButNewlineNode is a .: matches any character except a newline.
class MatchAnyButNewlineNode final : public Node {
  using Super = Node;

 public:
  /// Construct a MatchAnyButNewLine. If \p unicode is set, emit bytecode that
  /// treats surrogate pairs as a single character.
  explicit MatchAnyButNewlineNode(bool unicode) : unicode_(unicode) {}

  virtual MatchConstraintSet matchConstraints() const override {
    return MatchConstraintNonEmpty | Super::matchConstraints();
  }

  void emit(RegexBytecodeStream &bcs) const override {
    if (unicode_) {
      bcs.emit<U16MatchAnyButNewlineInsn>();
    } else {
      bcs.emit<MatchAnyButNewlineInsn>();
    }
  }

  virtual bool matchesExactlyOneCharacter() const override {
    // In Unicode we may match a surrogate pair.
    return !unicode_;
  }

 private:
  bool unicode_;
};

/// MatchChar matches one or more characters, specified as a parameter to the
/// constructor.
class MatchCharNode final : public Node {
  using Super = Node;
  using Super::CodePointList;

  /// The minimum number of characters we will output in a MatchCharN
  /// instruction.
  static constexpr size_t kMinMatchCharNCount = 3;

  /// The maximum number of characters supported in a MatchCharN instruction.
  static constexpr size_t kMaxMatchCharNCount = UINT8_MAX;

 public:
  MatchCharNode(CodePointList chars, constants::SyntaxFlags flags)
      : chars_(std::move(chars)),
        icase_(flags & constants::icase),
        unicode_(flags & constants::unicode) {}

  virtual MatchConstraintSet matchConstraints() const override {
    MatchConstraintSet result = MatchConstraintNonEmpty;
    // If our character is not ASCII, then we cannot match pure-ASCII strings.
    if (!std::all_of(chars_.begin(), chars_.end(), isASCII)) {
      result |= MatchConstraintNonASCII;
    }
    return result | Super::matchConstraints();
  }

  void emit(RegexBytecodeStream &bcs) const override {
    llvm::ArrayRef<CodePoint> remaining{chars_};
    while (!remaining.empty()) {
      // Output any run (possibly empty) of ASCII chars.
      auto asciis = remaining.take_while(isASCII);
      emitASCIIList(asciis, bcs);
      remaining = remaining.drop_front(asciis.size());

      // Output any run (possibly empty) of non-ASCII chars.
      auto nonAsciis = remaining.take_until(isASCII);
      emitNonASCIIList(nonAsciis, bcs);
      remaining = remaining.drop_front(nonAsciis.size());
    }
  }

  void reverseChildren() override {
    std::reverse(chars_.begin(), chars_.end());
  }

  bool tryCoalesceCharacters(CodePointList *output) const override {
    output->append(chars_.begin(), chars_.end());
    return true;
  }

  /// \return whether matching the code point \p cp may require
  /// decoding a surrogate pair from the input string.
  bool mayRequireDecodingSurrogatePair(uint32_t cp) const {
    if (!isMemberOfBMP(cp)) {
      return true;
    }
    if (unicode_ && (isHighSurrogate(cp) || isLowSurrogate(cp))) {
      // We have been asked to emit a literal surrogate into a Unicode string.
      // This can only match against an unpaired surrogate in the input string.
      // Thus ensure we emit a full 32 bit match, so that we decode surrogate
      // pairs from the input.
      return true;
    }
    return false;
  }

 protected:
  virtual bool matchesExactlyOneCharacter() const override {
    // If our character is astral it will need to match a surrogate pair, which
    // requires two characters.
    return chars_.size() == 1 &&
        !mayRequireDecodingSurrogatePair(chars_.front());
  }

  /// Emit a list of ASCII characters into bytecode stream \p bcs.
  void emitASCIIList(llvm::ArrayRef<CodePoint> chars, RegexBytecodeStream &bcs)
      const {
    assert(
        std::all_of(chars.begin(), chars.end(), isASCII) &&
        "All characters should be ASCII");

    // Output groups of at least kMinMatchCharNCount, but no more than
    // kMaxMatchCharNCount.
    auto remaining = chars;
    while (remaining.size() >= kMinMatchCharNCount) {
      size_t groupLen = std::min((size_t)kMaxMatchCharNCount, remaining.size());
      auto group = remaining.take_front(groupLen);
      remaining = remaining.drop_front(groupLen);

      if (icase_) {
        bcs.emit<MatchNCharICase8Insn>()->charCount = groupLen;
      } else {
        bcs.emit<MatchNChar8Insn>()->charCount = groupLen;
      }
      for (char c : group) {
        bcs.emitChar8(c);
      }
    }

    // Output any remaining as individual characters.
    for (CodePoint c : remaining) {
      if (icase_) {
        bcs.emit<MatchCharICase8Insn>()->c = c;
      } else {
        bcs.emit<MatchChar8Insn>()->c = c;
      }
    }
  }

  /// Emit a list of non-ASCII characters into bytecode stream \p bcs.
  void emitNonASCIIList(
      llvm::ArrayRef<CodePoint> chars,
      RegexBytecodeStream &bcs) const {
    for (uint32_t c : chars) {
      if (mayRequireDecodingSurrogatePair(c)) {
        if (icase_) {
          bcs.emit<U16MatchCharICase32Insn>()->c = c;
        } else {
          bcs.emit<U16MatchChar32Insn>()->c = c;
        }
      } else {
        if (icase_) {
          bcs.emit<MatchCharICase16Insn>()->c = c;
        } else {
          bcs.emit<MatchChar16Insn>()->c = c;
        }
      }
    }
  }

 private:
  // The code points we wish to match against.
  CodePointList chars_;

  /// Whether we are case insensitive (true) or case sensitive (false).
  const bool icase_;

  /// Whether the unicode flag is set.
  const bool unicode_;
};

// BracketNode represents a character class: /[a-zA-Z]/...
template <class Traits>
class BracketNode : public Node {
  using Super = Node;
  using Super::CodePoint;

  const Traits &traits_;
  CodePointSet codePointSet_;
  vector<CharacterClass> classes_;
  bool negate_;
  bool icase_;
  bool unicode_;

  /// \return whether this bracket can match an ASCII character.
  bool canMatchASCII() const {
    // Note we don't have to be concerned with case-insensitive ranges here,
    // except in the Unicode path.
    // The ES canonicalize() function disallows non-ASCII characters
    // from being canonicalized to ASCII. That is, a non-ASCII character in a
    // regexp may never match an ASCII input character even in a
    // case-insensitive regex, unless the unicode flag is set.
    // If we are case-sensitive and unicode, just pessimistically return true.
    if (icase_ && unicode_)
      return true;

    // If we are negated, look only for the range [0, 127] in ranges. We don't
    // bother to check for the more elaborate cases.
    if (negate_) {
      for (const CodePointRange &range : codePointSet_.ranges()) {
        if (range.first == 0 && range.length > 127) {
          // We are an inverted class containing the range [0, 127] or a
          // super-range; we cannot match ASCII.
          return false;
        }
      }
      return true;
    } else {
      // Not negated.
      // All character classes \w, etc. can match ASCII. Also check for
      // singletons.
      if (!classes_.empty())
        return true;

      // Check ranges. It is sufficient to check the start of the range; the end
      // is necessarily larger than the start, so the end cannot be ASCII unless
      // the start is.
      for (const CodePointRange &range : codePointSet_.ranges()) {
        if (isASCII(range.first))
          return true;
      }

      // None of our components can match ASCII.
      return false;
    }
  }

  /// Helper implementation of emit(). Given that we have emitted an instruction
  /// \p insn (which is either BracketInsn or U16BracketInsn), emit our bracket
  /// ranges and populate the instruction's fields.
  template <typename Insn>
  void populateInstruction(RegexBytecodeStream &bcs, Insn insn) const {
    insn->negate = negate_;
    for (CharacterClass cc : classes_) {
      if (!cc.inverted_) {
        insn->positiveCharClasses |= cc.type_;
      } else {
        insn->negativeCharClasses |= cc.type_;
      }
    }

    // Canonicalize our code point set if needed.
    CodePointSet cps = icase_
        ? makeCanonicallyEquivalent(codePointSet_, unicode_)
        : codePointSet_;
    for (const CodePointRange &range : cps.ranges()) {
      assert(range.length > 0 && "Ranges should never be empty");
      bcs.emitBracketRange(
          BracketRange32{range.first, range.first + range.length - 1});
    }
    insn->rangeCount = cps.ranges().size();
  }

 public:
  BracketNode(const Traits &traits, bool negate, constants::SyntaxFlags flags)
      : traits_(traits),
        negate_(negate),
        icase_(flags & constants::SyntaxFlags::icase),
        unicode_(flags & constants::SyntaxFlags::unicode) {}

  void addChar(CodePoint c) {
    codePointSet_.add(c);
  }

  void addRange(CodePoint a, CodePoint b) {
    assert(a <= b && "Invalid range");
    uint32_t length = b - a + 1;
    codePointSet_.add(CodePointRange{a, length});
  }

  void addClass(CharacterClass cls) {
    classes_.push_back(cls);
  }

  virtual MatchConstraintSet matchConstraints() const override {
    MatchConstraintSet result = 0;
    if (!canMatchASCII())
      result |= MatchConstraintNonASCII;

    if (!(codePointSet_.ranges().empty() && classes_.empty()))
      result |= MatchConstraintNonEmpty;

    return result | Super::matchConstraints();
  }

  virtual void emit(RegexBytecodeStream &bcs) const override {
    if (unicode_) {
      populateInstruction(bcs, bcs.emit<U16BracketInsn>());
    } else {
      populateInstruction(bcs, bcs.emit<BracketInsn>());
    }
  }

  virtual bool matchesExactlyOneCharacter() const override {
    // A unicode bracket may match a surrogate pair.
    return !unicode_;
  }
};

// Forward declaration.
template <class RegexType, class ForwardIterator>
class Parser;

template <class Traits>
class Regex {
  // Enable the Parser to add nodes to us.
  template <class A, class B>
  friend class Parser;

  using CharT = typename Traits::CodeUnit;
  using CodePoint = typename Traits::CodePoint;
  using Node = regex::Node;
  using BracketNode = regex::BracketNode<Traits>;

 private:
  Traits traits_;
  constants::SyntaxFlags flags_ = {};

  // Number of capture groups encountered so far.
  uint32_t markedCount_ = 0;

  // Number of loops encountered so far.
  uint32_t loopCount_ = 0;

  // The list of nodes so far.
  NodeList nodes_;

  // The error, which may be set after parsing.
  constants::ErrorType error_ = constants::ErrorType::None;

  // Constraints on the type of strings that can match this regex.
  MatchConstraintSet matchConstraints_ = 0;

  /// Implementation of make_unique(). Construct a unique_ptr to a new Node from
  /// the given args \p args.
  /// \return the unique_ptr
  template <typename NodeType, typename... Args>
  static unique_ptr<NodeType> make_unique(Args &&... args) {
    return unique_ptr<NodeType>(new NodeType(forward<Args>(args)...));
  }

  /// Construct and and append a node of type NodeType at the end of the nodes_
  /// list. The node should be constructible from \p args.
  /// \return an observer pointer to the new node.
  template <typename NodeType, typename... Args>
  NodeType *appendNode(Args &&... args) {
    unique_ptr<NodeType> node = make_unique<NodeType>(forward<Args>(args)...);
    NodeType *nodePtr = node.get();
    nodes_.push_back(move(node));
    return nodePtr;
  }

  /// \return the "current" node, which is the last (rightmost) node created.
  Node *currentNode() {
    return nodes_.back().get();
  }

  /// \return the number of marked subexpressions.
  uint32_t markedCount() const {
    return markedCount_;
  }

  /// \increment the number of marked subexpressions and return the value.
  uint32_t incrementMarkedCount() {
    return ++markedCount_;
  }

  /// Given that the node \p splicePoint is in our node list, remove all nodes
  /// after it. \return a list of the removed nodes.
  NodeList spliceOut(Node *splicePoint) {
    assert(splicePoint && "null node in spliceOut");
    // Find the index of the splice point. We expect it to be towards the end.
    size_t spliceIndex = nodes_.size();
    while (spliceIndex--) {
      if (nodes_[spliceIndex].get() == splicePoint)
        break;
    }
    assert(spliceIndex < nodes_.size() && "Node not in node list");
    // Move all nodes after the splice index into a new vector.
    // Note this may be empty.
    auto firstToMove = nodes_.begin() + spliceIndex + 1;
    NodeList result;
    std::move(firstToMove, nodes_.end(), std::back_inserter(result));
    nodes_.erase(firstToMove, nodes_.end());
    return result;
  }

 public:
  /// Compile the regex into bytecode. Return the resulting bytecode.
  std::vector<uint8_t> compile() const {
    assert(valid() && "Cannot compile invalid regex.");
    // TODO: add validation for the loop and reduce the size of markedCount_ and
    // loopCount_ to uint16_t.
    assert(
        markedCount_ <= constants::kMaxCaptureGroupCount &&
        "Too many capture groups");
    assert(loopCount_ <= constants::kMaxLoopCount && "Too many loops");
    RegexBytecodeHeader header = {static_cast<uint16_t>(markedCount_),
                                  static_cast<uint16_t>(loopCount_),
                                  flags_,
                                  matchConstraints_};
    RegexBytecodeStream bcs(header);
    Node::compile(nodes_, bcs);
    return bcs.acquireBytecode();
  }

  // Constructors
  Regex() = default;
  explicit Regex(const CharT *p, constants::SyntaxFlags f = {})
      : Regex(p, p + char_traits<CharT>::length(p), f) {}

  Regex(const CharT *first, const CharT *last, constants::SyntaxFlags f = {})
      : flags_(f) {
    error_ = parse(first, last);
  }

  // Disallow copy-assignment and copy-construction.
  Regex &operator=(const Regex &) = delete;
  Regex(const Regex &) = delete;

  /// Move-assignment and move-construction.
  Regex &operator=(Regex &&) = default;
  Regex(Regex &&) = default;

  // Accessors.
  unsigned markCount() const {
    return markedCount_;
  }
  constants::SyntaxFlags flags() const {
    return flags_;
  }

  /// \return any errors produced during parsing, or ErrorType::None if none.
  constants::ErrorType getError() const {
    return error_;
  }

  /// \return whether the regex was parsed successfully.
  bool valid() const {
    return error_ == constants::ErrorType::None;
  }

  /// \return the set of match constraints for the regex.
  MatchConstraintSet matchConstraints() const {
    return matchConstraints_;
  }

 private:
  template <class ForwardIterator>
  constants::ErrorType parse(ForwardIterator first, ForwardIterator last);

  /// Attempt to parse the regex from the range [\p first, \p last), using
  /// \p backRefLimit as the maximum decimal escape to interpret as a
  /// backreference.  The maximum backreference that was in fact encountered
  /// is returned by reference in \p out_max_back_ref, if that is larger than
  /// its current value. \return an error code.
  template <class ForwardIterator>
  constants::ErrorType parseWithBackRefLimit(
      ForwardIterator first,
      ForwardIterator last,
      uint32_t backRefLimit,
      uint32_t *outMaxBackRef);
  void pushLeftAnchor();
  void pushRightAnchor();
  void pushMatchAnyButNewline();
  void pushLoop(
      uint32_t min,
      uint32_t max,
      NodeList loopedList,
      uint32_t mexp_begin,
      bool greedy);
  BracketNode *startBracketList(bool negate);
  void pushChar(CodePoint c);
  void pushCharClass(CharacterClass c);
  void pushBackRef(uint32_t i);
  void pushAlternation(NodeList left, NodeList right);
  void pushMarkedSubexpression(NodeList, uint32_t mexp);
  void pushWordBoundary(bool);
  void pushLookaround(NodeList, uint16_t, uint16_t, bool, bool);
};

/// Node for lookaround assertions like (?=...) and (?!...)
class LookaroundNode : public Node {
  using Super = Node;

  /// The contained expression representing our lookaround assertion.
  NodeList exp_;

  /// Match constraints for our contained expression.
  MatchConstraintSet expConstraints_;

  /// Whether the lookaround assertion is negative (?!) or positive (?=).
  const bool invert_;

  /// Whether the lookaround is forwards (true) or backwards (false).
  const bool forwards_;

  /// The marked subexpressions contained within this lookaround.
  uint16_t mexpBegin_;
  uint16_t mexpEnd_;

 public:
  LookaroundNode(
      NodeList exp,
      uint32_t mexpBegin,
      uint32_t mexpEnd,
      bool invert,
      bool forwards)
      : exp_(move(exp)),
        expConstraints_(matchConstraintsForList(exp_)),
        invert_(invert),
        forwards_(forwards),
        mexpBegin_(mexpBegin),
        mexpEnd_(mexpEnd) {
    // Clear AnchoredAtStart for lookbehind assertions.
    // For example:
    //    /(?<=^abc)def/.exec("abcdef")
    // this matches the substring "def", even though that substring is not
    // anchored at the start.
    if (!forwards_) {
      expConstraints_ &= ~MatchConstraintAnchoredAtStart;
    }
  }

  virtual MatchConstraintSet matchConstraints() const override {
    // Positive lookarounds apply their match constraints.
    // e.g. if our assertion is anchored at the start, so are we.
    MatchConstraintSet result = 0;
    if (!invert_) {
      result |= expConstraints_;
    }
    // Lookarounds match an empty string even if their contents do not.
    result &= ~MatchConstraintNonEmpty;
    return result | Super::matchConstraints();
  }

  virtual void optimizeNodeContents(constants::SyntaxFlags flags) override {
    optimizeNodeList(exp_, flags);
  }

  // Override emit() to compile our lookahead expression.
  virtual void emit(RegexBytecodeStream &bcs) const override {
    auto lookaround = bcs.emit<LookaroundInsn>();
    lookaround->invert = invert_;
    lookaround->forwards = forwards_;
    lookaround->constraints = expConstraints_;
    lookaround->mexpBegin = mexpBegin_;
    lookaround->mexpEnd = mexpEnd_;
    compile(exp_, bcs);
    lookaround->continuation = bcs.currentOffset();
  }
};

template <typename Receiver>
constants::ErrorType parseRegex(
    const char16_t *start,
    const char16_t *end,
    Receiver *receiver,
    constants::SyntaxFlags flags,
    uint32_t backRefLimit,
    uint32_t *outMaxBackRef);

template <class Traits>
template <class ForwardIterator>
constants::ErrorType Regex<Traits>::parse(
    ForwardIterator first,
    ForwardIterator last) {
  uint32_t maxBackRef = 0;
  auto result = parseWithBackRefLimit(
      first, last, constants::kMaxCaptureGroupCount, &maxBackRef);

  // Validate loop and capture group count.
  if (markedCount_ > constants::kMaxCaptureGroupCount ||
      loopCount_ > constants::kMaxLoopCount) {
    return constants::ErrorType::PatternExceedsParseLimits;
  }

  // See comment --DecimalEscape--
  // We parsed without a backreference limit because we had to parse to discover
  // the limit. Now we know that we wrongly interpreted a decimal escape as a
  // backreference. See ES6 Annex B.1.4 DecimalEscape "but only if the integer
  // value DecimalEscape is <= NCapturingParens". Now that we know the true
  // capture group count, either produce an error (if Unicode) or re-parse with
  // that as the limit so overlarge decimal escapes will be ignored.
  if (result == constants::ErrorType::None && maxBackRef > markedCount_) {
    if (flags_ & constants::SyntaxFlags::unicode) {
      return constants::ErrorType::EscapeInvalid;
    }

    uint32_t backRefLimit = markedCount_;
    uint32_t reparsedMaxBackRef = 0;
    loopCount_ = 0;
    markedCount_ = 0;
    matchConstraints_ = 0;
    result =
        parseWithBackRefLimit(first, last, backRefLimit, &reparsedMaxBackRef);
    assert(
        result == constants::ErrorType::None &&
        "regex reparsing should never fail if the first parse succeeded");
    assert(
        reparsedMaxBackRef <= backRefLimit &&
        "invalid backreference generated");
    (void)reparsedMaxBackRef;
  }
  return result;
}

template <class Traits>
template <class ForwardIterator>
constants::ErrorType Regex<Traits>::parseWithBackRefLimit(
    ForwardIterator first,
    ForwardIterator last,
    uint32_t backRefLimit,
    uint32_t *outMaxBackRef) {
  // Initialize our node list with a single no-op node (it must never be empty.)
  nodes_.clear();
  nodes_.push_back(make_unique<Node>());
  auto result =
      parseRegex(first, last, this, flags_, backRefLimit, outMaxBackRef);

  // If we succeeded, add a goal node as the last node and perform optimizations
  // on the list.
  if (result == constants::ErrorType::None) {
    nodes_.push_back(make_unique<GoalNode>());
    Node::optimizeNodeList(nodes_, flags_);
  }

  // Compute any match constraints.
  matchConstraints_ = Node::matchConstraintsForList(nodes_);

  return result;
}

template <class Traits>
void Regex<Traits>::pushLoop(
    uint32_t min,
    uint32_t max,
    NodeList loopedExpr,
    uint32_t mexp_begin,
    bool greedy) {
  appendNode<LoopNode>(
      loopCount_++,
      min,
      max,
      greedy,
      mexp_begin,
      markedCount_,
      move(loopedExpr));
}

template <class Traits>
void Regex<Traits>::pushChar(CodePoint c) {
  bool icase = flags() & constants::icase;
  if (icase)
    c = traits_.canonicalize(c, flags() & constants::unicode);
  appendNode<MatchCharNode>(Node::CodePointList{c}, flags());
}

template <class Traits>
void Regex<Traits>::pushCharClass(CharacterClass c) {
  auto bracket = startBracketList(false);
  bracket->addClass(c);
}

template <class Traits>
void Regex<Traits>::pushMarkedSubexpression(NodeList nodes, uint32_t mexp) {
  appendNode<MarkedSubexpressionNode>(std::move(nodes), mexp);
}

template <class Traits>
void Regex<Traits>::pushLeftAnchor() {
  appendNode<LeftAnchorNode>(flags_ & constants::multiline);
}

template <class Traits>
void Regex<Traits>::pushRightAnchor() {
  appendNode<RightAnchorNode>();
}

template <class Traits>
void Regex<Traits>::pushMatchAnyButNewline() {
  appendNode<MatchAnyButNewlineNode>(flags_ & constants::unicode);
}

template <class Traits>
void Regex<Traits>::pushWordBoundary(bool invert) {
  appendNode<WordBoundaryNode>(invert);
}

template <class Traits>
void Regex<Traits>::pushBackRef(uint32_t i) {
  appendNode<BackRefNode>(i);
}

template <class Traits>
void Regex<Traits>::pushAlternation(NodeList left, NodeList right) {
  appendNode<AlternationNode>(move(left), move(right));
}

template <class Traits>
BracketNode<Traits> *Regex<Traits>::startBracketList(bool negate) {
  return appendNode<BracketNode>(traits_, negate, flags_);
}

template <class Traits>
void Regex<Traits>::pushLookaround(
    NodeList exp,
    uint16_t mexpBegin,
    uint16_t mexpEnd,
    bool invert,
    bool forwards) {
  if (!forwards) {
    Node::reverseNodeList(exp);
  }
  exp.push_back(make_unique<GoalNode>());
  appendNode<LookaroundNode>(move(exp), mexpBegin, mexpEnd, invert, forwards);
}

void Node::reverseNodeList(NodeList &nodes) {
  // If we have a goal node it must come at the end.
#ifndef NDEBUG
  for (const auto &node : nodes) {
    assert(
        !node->isGoal() ||
        (node == nodes.back()) && "Goal node should only be at end");
  }
#endif

  // Reverse this list, excluding any terminating goal.
  if (!nodes.empty()) {
    bool hasGoal = nodes.back()->isGoal();
    std::reverse(nodes.begin(), nodes.end() - (hasGoal ? 1 : 0));
  }

  // Recursively reverse child nodes.
  for (auto &node : nodes) {
    node->reverseChildren();
  }
}

void Node::optimizeNodeList(NodeList &nodes, constants::SyntaxFlags flags) {
  // Recursively optimize child nodes.
  for (auto &node : nodes) {
    node->optimizeNodeContents(flags);
  }

  // Merge adjacent runs of char nodes.
  // For example, [CharNode('a') CharNode('b') CharNode('c')] becomes
  // [CharNode('abc')].
  for (size_t idx = 0, max = nodes.size(); idx < max; idx++) {
    // Get the range of nodes that can be successfully coalesced.
    Node::CodePointList chars;
    size_t rangeStart = idx;
    size_t rangeEnd = idx;
    for (; rangeEnd < max; rangeEnd++) {
      if (!nodes[rangeEnd]->tryCoalesceCharacters(&chars)) {
        break;
      }
    }
    if (rangeEnd - rangeStart >= 3) {
      // We successfully coalesced some nodes.
      // Replace the range with a new node.
      nodes[rangeStart] =
          unique_ptr<MatchCharNode>(new MatchCharNode(std::move(chars), flags));
      // Fill the remainder of the range with null (we'll clean them up after
      // the loop) and skip to the end of the range.
      // Note that rangeEnd may be one past the last valid element.
      std::fill(
          nodes.begin() + (rangeStart + 1), nodes.begin() + rangeEnd, nullptr);
      idx = rangeEnd - 1;
    }
  }

  // Remove any nulls that we introduced.
  nodes.erase(std::remove(nodes.begin(), nodes.end(), nullptr), nodes.end());
}

} // namespace regex
} // namespace hermes

#endif // HERMES_REGEX_COMPILER_H
