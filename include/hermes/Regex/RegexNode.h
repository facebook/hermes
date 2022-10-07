/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
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

#ifndef HERMES_REGEX_NODE_H
#define HERMES_REGEX_NODE_H

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Platform/Unicode/CodePointSet.h"

#include "hermes/Regex/RegexBytecode.h"
#include "hermes/Regex/RegexTypes.h"

#include "llvh/ADT/SmallVector.h"

#include <string>
#include <vector>

namespace hermes {
namespace regex {

class Node;

/// A NodeList is list of Nodes.
using NodeList = std::vector<Node *>;
/// A NodeHolder is list of owned Nodes. Note it is move-only.
using NodeHolder = std::vector<std::unique_ptr<Node>>;

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
  using CodePointList = llvh::SmallVector<CodePoint, 5>;

  /// Default constructor and destructor.
  Node() = default;
  virtual ~Node() = default;

  /// Compile a list of nodes \p nodes to a bytecode stream \p bcs.
  static void compile(const NodeList &nodes, RegexBytecodeStream &bcs) {
    std::vector<Node *> stack;
    stack.insert(stack.end(), nodes.rbegin(), nodes.rend());
    while (!stack.empty()) {
      if (auto n = stack.back()->emitStep(bcs)) {
        stack.insert(stack.end(), n->rbegin(), n->rend());
      } else {
        stack.pop_back();
      }
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
  inline static void
  optimizeNodeList(NodeList &nodes, SyntaxFlags flags, NodeHolder &nodeHolder);

  /// \return whether the node always matches exactly one character.
  virtual bool matchesExactlyOneCharacter() const {
    return false;
  }

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

  /// \return pointers to the NodeLists contained in this node.
  virtual llvh::SmallVector<NodeList *, 1> getChildren() {
    return {};
  }

  /// Reverse the order of children of this node. The default implementation
  /// does nothing, but nodes which store a child list should reverse the order
  /// of that list and then recurse.
  virtual void reverseChildren() {}

 private:
  /// Emit part of this node into the bytecode compiler \p bcs. This function
  /// will be called repeatedly until it returns a nullptr. This is an
  /// overrideable function - subclasses should override this to emit
  /// node-specific bytecode. The default emits nothing, so that base Node is
  /// just a no-op.
  /// \return a pointer to a node that must be emitted fully before calling
  /// emitStep again. If the returned pointer is null, this node has been fully
  /// emitted and no further calls to emitStep are necessary.
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) {
    return nullptr;
  }
};

/// GoalNode is the terminal Node that represents successful execution.
class GoalNode final : public Node {
 private:
  NodeList *emitStep(RegexBytecodeStream &bcs) override {
    bcs.emit<GoalInsn>();
    return nullptr;
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
  uint16_t mexpBegin_;

  /// One past the last marked subexpression contained in our looped expression.
  /// Marked subexpressions are [begin, end).
  uint16_t mexpEnd_;

  /// Whether this loop is greedy.
  bool greedy_;

  /// The expression that gets looped.
  NodeList loopee_;

  /// The constraints on what the loopee can match.
  MatchConstraintSet loopeeConstraints_;

  /// The function to call to emit the end instructions for a loop node. See
  /// emitStep for usage.
  std::function<void()> endLoop_;

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
      uint16_t mexpBegin,
      uint16_t mexpEnd,
      NodeList loopee)
      : min_(min),
        max_(max),
        loopId_(loopId),
        mexpBegin_(mexpBegin),
        mexpEnd_(mexpEnd),
        greedy_(greedy),
        loopee_(std::move(loopee)),
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

  virtual llvh::SmallVector<NodeList *, 1> getChildren() override {
    return {&loopee_};
  }

 protected:
  void reverseChildren() override {
    reverseNodeList(loopee_);
  }

 private:
  /// Override of emitStep() to compile our looped expression and add a jump
  /// back to the loop.
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    if (!endLoop_) {
      // This is the first call to emitStep() for this node, set up the loop
      // node
      auto loopEntryPosition = bcs.currentOffset();
      if (isWidth1Loop()) {
        assert(
            mexpBegin_ == mexpEnd_ &&
            "Width 1 loops should not contain capture groups");
        auto loopInsn = bcs.emit<Width1LoopInsn>();
        loopInsn->loopId = loopId_;
        loopInsn->min = min_;
        loopInsn->max = max_;
        loopInsn->greedy = greedy_;
        endLoop_ = [loopInsn, &bcs]() mutable {
          loopInsn->notTakenTarget = bcs.currentOffset();
        };
      } else if (isSimpleLoop()) {
        auto loopInsn = bcs.emit<BeginSimpleLoopInsn>();
        loopInsn->loopeeConstraints = loopeeConstraints_;
        endLoop_ = [loopInsn, loopEntryPosition, &bcs]() mutable {
          bcs.emit<EndSimpleLoopInsn>()->target = loopEntryPosition;
          loopInsn->notTakenTarget = bcs.currentOffset();
        };
      } else {
        auto loopInsn = bcs.emit<BeginLoopInsn>();
        loopInsn->loopId = loopId_;
        loopInsn->min = min_;
        loopInsn->max = max_;
        loopInsn->mexpBegin = mexpBegin_;
        loopInsn->mexpEnd = mexpEnd_;
        loopInsn->greedy = greedy_;
        loopInsn->loopeeConstraints = loopeeConstraints_;
        endLoop_ = [loopInsn, loopEntryPosition, &bcs]() mutable {
          bcs.emit<EndLoopInsn>()->target = loopEntryPosition;
          loopInsn->notTakenTarget = bcs.currentOffset();
        };
      }
      return &loopee_;
    }
    // This is the second call to emitStep, emit the end instructions and set
    // notTakenTarget.
    endLoop_();
    endLoop_ = nullptr;
    return nullptr;
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
  // In an expression like /a|b|c/ this vector will contain NodeLists for a, b,
  // and c.
  std::vector<NodeList> alternatives_;
  // Contains the constraints applicable to an element.
  std::vector<MatchConstraintSet> elementConstraints_;
  // Each element i in restConstraints_ contains the constraints applicable to
  // the sublist starting at i. For instance, the first element contains the
  // intersection of the constraints for all nodes, whereas the last element
  // contains the constraint for only that element.
  std::vector<MatchConstraintSet> restConstraints_;
  // Used to keep track of the jump instructions produced while emitting, these
  // jumps need to be set at the end of emitting so the executor can jump
  // directly to the end when a match is found.
  std::vector<RegexBytecodeStream::InstructionWrapper<Jump32Insn>> jumps_;
  // This corresponds to a function that should be called at the start of the
  // next call to emitStep_. It is mostly used as a convenience to keep the
  // logic simpler and to wrap up the state that needs to be preserved between
  // two calls. It returns a boolean corresponding to whether the current call
  // to emitStep is the last.
  std::function<bool()> callNext_;

 public:
  /// Constructor for an Alternation.
  /// Accepts a list of NodeLists in \p alternatives.
  AlternationNode(std::vector<NodeList> alternatives)
      : alternatives_(std::move(alternatives)),
        elementConstraints_(alternatives_.size()),
        restConstraints_(alternatives_.size()) {
    assert(alternatives_.size() > 1 && "Must give at least 2 alternatives");

    jumps_.reserve(alternatives_.size());

    // restConstraints_ needs to be set in reverse order, since each element
    // depends on the element after it.
    elementConstraints_.back() = matchConstraintsForList(alternatives_.back());
    restConstraints_.back() = elementConstraints_.back();

    for (auto i = alternatives_.size() - 1; i-- > 0;) {
      elementConstraints_[i] = matchConstraintsForList(alternatives_[i]);
      restConstraints_[i] = restConstraints_[i + 1] & elementConstraints_[i];
    }
  }

  /// Alternations are constrained by the intersection of all their
  /// alternatives' constraints. This intersection is equal to the first element
  /// in restConstraints.
  virtual MatchConstraintSet matchConstraints() const override {
    return restConstraints_.front() | Super::matchConstraints();
  }

  virtual llvh::SmallVector<NodeList *, 1> getChildren() override {
    llvh::SmallVector<NodeList *, 1> ret;
    ret.reserve(alternatives_.size());
    for (auto &alternative : alternatives_) {
      ret.push_back(&alternative);
    }
    return ret;
  }

  void reverseChildren() override {
    for (auto &alternative : alternatives_) {
      reverseNodeList(alternative);
    }
  }

 private:
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    // Instruction stream looks like:
    //   [Alternation][PrimaryBranch][Jump][SecondaryBranch][...]
    //     |____________________________|____^               ^
    //                                  |____________________|
    // Where the Alternation has a JumpOffset to its secondary branch.

    // Check if the callback is set, if so, call it and check whether this is
    // the last call for emitStep
    if (callNext_ && callNext_()) {
      // This is the last call to emitStep, clean up and return nullptr
      jumps_.clear();
      callNext_ = nullptr;
      return nullptr;
    }
    // Do not emit an alternation instruction for the very last alternative
    // because it is simply the secondary option of the penultimate
    // alternative. Note that jumps_.size() here is used to tell us how many
    // times this function has already been called
    if (jumps_.size() < alternatives_.size() - 1) {
      auto altInsn = bcs.emit<AlternationInsn>();
      altInsn->primaryConstraints = elementConstraints_[jumps_.size()];
      altInsn->secondaryConstraints = restConstraints_[jumps_.size() + 1];
      callNext_ = [this, altInsn, &bcs]() mutable {
        jumps_.push_back(bcs.emit<Jump32Insn>());
        altInsn->secondaryBranch = bcs.currentOffset();
        return false;
      };
      return &alternatives_[jumps_.size()];
    }
    // If the program reaches here, it means that the current call to emitStep
    // is for the last alternative. On the next call, it should just clean
    // things up and return.
    callNext_ = [this, &bcs]() mutable {
      // Set the jump instruction for every
      // alternation to jump to the end of the block.
      for (auto &jump : jumps_) {
        jump->target = bcs.currentOffset();
      }
      return true;
    };
    return &alternatives_.back();
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
  uint16_t mexp_;

  // Whether to emit the beginning or the end in emitStep
  bool emitEnd_ = false;

 public:
  explicit MarkedSubexpressionNode(NodeList contents, uint16_t mexp)
      : contents_(std::move(contents)),
        contentsConstraints_(matchConstraintsForList(contents_)),
        mexp_(mexp) {}

  void reverseChildren() override {
    reverseNodeList(contents_);
  }

  virtual llvh::SmallVector<NodeList *, 1> getChildren() override {
    return {&contents_};
  }

  virtual MatchConstraintSet matchConstraints() const override {
    return contentsConstraints_ | Super::matchConstraints();
  }

 private:
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    if (!emitEnd_) {
      bcs.emit<BeginMarkedSubexpressionInsn>()->mexp = mexp_;
      emitEnd_ = true;
      return &contents_;
    }
    emitEnd_ = false;
    bcs.emit<EndMarkedSubexpressionInsn>()->mexp = mexp_;
    return nullptr;
  }
};

/// BackRefNode represents a backreference node.
class BackRefNode final : public Node {
  // The backreference like \3.
  uint16_t mexp_;

 public:
  explicit BackRefNode(unsigned mexp) : mexp_(mexp) {}

  void setBackRef(unsigned mexp) {
    mexp_ = mexp;
  }

 private:
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    bcs.emit<BackRefInsn>()->mexp = mexp_;
    return nullptr;
  }
};

/// WordBoundaryNode represents a \b or \B assertion in a regex.
class WordBoundaryNode final : public Node {
  using Super = Node;

  /// Whether the boundary is inverted (\B instead of \b).
  bool invert_;

 public:
  WordBoundaryNode(bool invert) : invert_(invert) {}

 private:
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    bcs.emit<WordBoundaryInsn>()->invert = invert_;
    return nullptr;
  }
};

/// LeftAnchorNode is a ^: anchors at the beginning of a line.
class LeftAnchorNode final : public Node {
  using Super = Node;

  bool multiline_;

 public:
  LeftAnchorNode(SyntaxFlags flags) : multiline_(flags.multiline) {}

  virtual MatchConstraintSet matchConstraints() const override {
    MatchConstraintSet result = 0;
    // We are anchored at the start, unless we are multiline, in which case we
    // can match after a newline.
    if (!multiline_) {
      result |= MatchConstraintAnchoredAtStart;
    }
    return result | Super::matchConstraints();
  }

 private:
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    bcs.emit<LeftAnchorInsn>();
    return nullptr;
  }
};

/// RightAnchorNode is regex $: anchors at end of the line.
class RightAnchorNode : public Node {
  using Super = Node;

 public:
  RightAnchorNode() {}

 private:
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    bcs.emit<RightAnchorInsn>();
    return nullptr;
  }
};

/// MatchAny is a .: matches any character (including newlines iff the dotAll
/// flag is set).
class MatchAnyNode final : public Node {
  using Super = Node;
  bool unicode_;
  bool dotAll_;

 public:
  /// Construct a MatchAny.
  /// If \p unicode is set, emit bytecode that treats surrogate pairs as a
  /// single character.
  /// If \p dotAll is set, match newlines. Otherwise, don't match newlines.
  explicit MatchAnyNode(SyntaxFlags flags)
      : unicode_(flags.unicode), dotAll_(flags.dotAll) {}

  virtual MatchConstraintSet matchConstraints() const override {
    return MatchConstraintNonEmpty | Super::matchConstraints();
  }

  virtual bool matchesExactlyOneCharacter() const override {
    // In Unicode we may match a surrogate pair.
    return !unicode_;
  }

 private:
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    if (unicode_) {
      if (dotAll_) {
        bcs.emit<U16MatchAnyInsn>();
      } else {
        bcs.emit<U16MatchAnyButNewlineInsn>();
      }
    } else {
      if (dotAll_) {
        bcs.emit<MatchAnyInsn>();
      } else {
        bcs.emit<MatchAnyButNewlineInsn>();
      }
    }
    return nullptr;
  }
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

  // The code points we wish to match against.
  CodePointList chars_;

  /// Whether we are case insensitive (true) or case sensitive (false).
  const bool icase_;

  /// Whether the unicode flag is set.
  const bool unicode_;

 public:
  MatchCharNode(CodePointList chars, SyntaxFlags flags)
      : chars_(std::move(chars)),
        icase_(flags.ignoreCase),
        unicode_(flags.unicode) {}

  virtual MatchConstraintSet matchConstraints() const override {
    MatchConstraintSet result = MatchConstraintNonEmpty;
    // If our character is not ASCII, then we cannot match pure-ASCII strings.
    if (!std::all_of(chars_.begin(), chars_.end(), isASCII)) {
      result |= MatchConstraintNonASCII;
    }
    return result | Super::matchConstraints();
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
  void emitASCIIList(llvh::ArrayRef<CodePoint> chars, RegexBytecodeStream &bcs)
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
      llvh::ArrayRef<CodePoint> chars,
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
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    llvh::ArrayRef<CodePoint> remaining{chars_};
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
    return nullptr;
  }
};

// BracketNode represents a character class: /[a-zA-Z]/...
template <class Traits>
class BracketNode : public Node {
  using Super = Node;
  using Super::CodePoint;

  const Traits &traits_;
  CodePointSet codePointSet_;
  std::vector<CharacterClass> classes_;
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
  BracketNode(const Traits &traits, bool negate, SyntaxFlags flags)
      : traits_(traits),
        negate_(negate),
        icase_(flags.ignoreCase),
        unicode_(flags.unicode) {}

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

  virtual bool matchesExactlyOneCharacter() const override {
    // A unicode bracket may match a surrogate pair.
    return !unicode_;
  }

 private:
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    if (unicode_) {
      populateInstruction(bcs, bcs.emit<U16BracketInsn>());
    } else {
      populateInstruction(bcs, bcs.emit<BracketInsn>());
    }
    return nullptr;
  }
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

  /// Callback to emit the end of a lookaround instruction
  std::function<void()> endLookaround_;

 public:
  LookaroundNode(
      NodeList exp,
      uint16_t mexpBegin,
      uint16_t mexpEnd,
      bool invert,
      bool forwards)
      : exp_(std::move(exp)),
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

  virtual llvh::SmallVector<NodeList *, 1> getChildren() override {
    return {&exp_};
  }

 private:
  // Override emitStep() to compile our lookahead expression.
  virtual NodeList *emitStep(RegexBytecodeStream &bcs) override {
    if (endLookaround_) {
      endLookaround_();
      endLookaround_ = nullptr;
      return nullptr;
    }
    auto lookaround = bcs.emit<LookaroundInsn>();
    lookaround->invert = invert_;
    lookaround->forwards = forwards_;
    lookaround->constraints = expConstraints_;
    lookaround->mexpBegin = mexpBegin_;
    lookaround->mexpEnd = mexpEnd_;
    endLookaround_ = [lookaround, &bcs]() mutable {
      lookaround->continuation = bcs.currentOffset();
    };
    return &exp_;
  }
};

void Node::reverseNodeList(NodeList &nodes) {
  // If we have a goal node it must come at the end.
#ifndef NDEBUG
  for (const auto &node : nodes) {
    assert(
        (!node->isGoal() || (node == nodes.back())) &&
        "Goal node should only be at end");
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

void Node::optimizeNodeList(
    NodeList &rootNodes,
    SyntaxFlags flags,
    NodeHolder &nodeHolder) {
  std::vector<NodeList *> stack;
  stack.push_back(&rootNodes);

  while (!stack.empty()) {
    auto &nodes = *stack.back();
    stack.pop_back();

    // Merge adjacent runs of char nodes.
    // For example, [CharNode('a') CharNode('b') CharNode('c')] becomes
    // [CharNode('abc')].
    for (size_t idx = 0, max = nodes.size(); idx < max; idx++) {
      auto childNodes = nodes[idx]->getChildren();
      stack.insert(stack.end(), childNodes.begin(), childNodes.end());
      // Get the range of nodes that can be successfully coalesced.
      CodePointList chars;
      size_t rangeStart = idx;
      while (idx < max && nodes[idx]->tryCoalesceCharacters(&chars)) {
        idx++;
      }
      if (idx - rangeStart >= 2) {
        // We successfully coalesced some nodes.
        // Replace the range with a new node.
        nodeHolder.emplace_back(new MatchCharNode(std::move(chars), flags));
        nodes[rangeStart] = nodeHolder.back().get();
        // Fill the remainder of the range with null (we'll clean them up after
        // the loop) and skip to the end of the range.
        // Note that rangeEnd may be one past the last valid element.
        std::fill(
            nodes.begin() + (rangeStart + 1), nodes.begin() + idx, nullptr);
      }
    }

    // Remove any nulls that we introduced.
    nodes.erase(std::remove(nodes.begin(), nodes.end(), nullptr), nodes.end());
  }
}

} // namespace regex
} // namespace hermes
#endif // HERMES_REGEX_NODE_H
