/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_TWINECHAR16_H
#define HERMES_SUPPORT_TWINECHAR16_H

#include <cassert>

#include "hermes/Support/Conversions.h"
#include "hermes/VM/StringRefUtils.h"

namespace llvh {
class raw_ostream;
class raw_svector_ostream;
} // namespace llvh

namespace hermes {
namespace vm {

class StringPrimitive;

/// Lightweight data structure for representing concatenation of UTF16 strings
/// as a binary tree. This is meant to be used as a temporary data structure for
/// string construction, and therefore doesn't have an assignment operator.
///
/// NOTE: TwineChar16 should not be stored, as it is only intended for use as a
/// temporary. It should only be passed as an argument to other functions,
/// or used as a temporary value. The implementation stores pointers to stack
/// objects which can be freed, so it must not be stored in a variable.
/// TwineChar16 should only be accepted as `const TwineChar16 &` when passing
/// it as an argument to a function, which will represent a concatenated string.
///
/// The general use pattern is to construct TwineChar16 with either a
/// single-argument constructor or a two-argument constructor that performs
/// concatenation slightly more efficiently, pass it as a const reference,
/// and the callee may export the resultant string using toChar16Str or
/// toVector, which write to memory owned by the user of the TwineChar16.
/// Concatenation can be performed using the concat() method.
/// There is also an operator+ overload which calls concat().
///
/// Each TwineChar16 has two child nodes, which can either be leaves or other
/// twines. If they are leaves, then they contain some form of string directly
/// (char *, char16_t *, UTF16Ref, or StringPrimitive *). We also store the size
/// of the strings stored in the the left and right nodes, allowing for constant
/// time size() calls. The resultant string can be read by visiting the leaves
/// in preorder.
///
/// Based on llvm/ADT/Twine.h, which doesn't keep track of size and doesn't
/// support 16-bit characters.
class TwineChar16 {
 private:
  /// Indicates what kind of node a Child is.
  enum NodeKind {
    /// Null twine concatenates with any other twine to produce null.
    NullKind,
    /// Empty twine has no characters.
    EmptyKind,
    /// Another twine.
    TwineKind,
    /// const char *
    CharStrKind,
    /// const char16_t *
    Char16StrKind,
    /// Pointer to a StringPrimitive.
    StringPrimitiveKind,
    /// Integer
    Int32Kind,
    /// Unsigned integer
    Unsigned32Kind,
    /// Floating point
    DoubleKind,
  };

  /// Node in the twine tree.
  union Node {
    const TwineChar16 *twine;

    /// Plain char array.
    const char *charStr;

    /// Raw zero-terminated char16_t array.
    const char16_t *char16Str;

    const StringPrimitive *stringPrimitive;

    int32_t int32;
    uint32_t uint32;
    double flt;
  };

  /// Left child and the corresponding kind.
  /// If NullKind, the twine is null, and if empty, the twine is empty.
  Node leftChild_;
  NodeKind leftKind_;

  /// Right child and the corresponding kind.
  /// If non-empty, then the left child must also be non-empty.
  Node rightChild_;
  NodeKind rightKind_;

  /// Store the sizes of the children for fast size().
  size_t leftSize_;
  size_t rightSize_;

 public:
  /// Make an empty twine.
  TwineChar16()
      : leftKind_(EmptyKind),
        rightKind_(EmptyKind),
        leftSize_(0),
        rightSize_(0) {
    assert(isValid());
  }

  TwineChar16(const TwineChar16 &other) = default;

  TwineChar16(const char *str) : TwineChar16(llvh::StringRef(str)) {}

  /// Make a twine out of the null-terminated \p str.
  TwineChar16(const llvh::StringRef str)
      : rightKind_(EmptyKind), leftSize_(str.size()), rightSize_(0) {
    if (str.size() == 0) {
      leftKind_ = EmptyKind;
    } else {
      leftChild_.charStr = str.begin();
      leftKind_ = CharStrKind;
    }
    assert(isValid());
  }

  /// Make a twine out of the null-terminated \p str.
  TwineChar16(const char16_t *str)
      : rightKind_(EmptyKind),
        leftSize_(utf16_traits::length(str)),
        rightSize_(0) {
    if (leftSize_ == 0) {
      leftKind_ = EmptyKind;
    } else {
      leftChild_.char16Str = str;
      leftKind_ = Char16StrKind;
    }
    assert(isValid());
  }

  /// Make a twine out of the UTF16Ref.
  TwineChar16(const UTF16Ref str)
      : leftKind_(Char16StrKind),
        rightKind_(EmptyKind),
        leftSize_(str.size()),
        rightSize_(0) {
    leftChild_.char16Str = str.begin();
    assert(isValid());
  }

  /// Make a twine out of the StringPrimitive.
  TwineChar16(const StringPrimitive *str);

  /// Make a twine out of the integer.
  TwineChar16(const int32_t i)
      : leftKind_(Int32Kind), rightKind_(EmptyKind), rightSize_(0) {
    // snprintf returns the number of characters that i takes to write.
    leftSize_ = ::snprintf(0, 0, "%" PRId32, i);
    leftChild_.int32 = i;
    assert(isValid());
  }

  TwineChar16(const uint32_t i)
      : leftKind_(Unsigned32Kind), rightKind_(EmptyKind), rightSize_(0) {
    // snprintf returns the number of characters that i takes to write.
    leftSize_ = ::snprintf(0, 0, "%" PRIu32, i);
    leftChild_.uint32 = i;
    assert(isValid());
  }

  TwineChar16(const double f)
      : leftKind_(DoubleKind), rightKind_(EmptyKind), rightSize_(0) {
    char buf[NUMBER_TO_STRING_BUF_SIZE];
    // This is the only way to calculate the size.
    leftSize_ = numberToString(f, buf, sizeof(buf));
    leftChild_.flt = f;
    assert(isValid());
  }

  /// Make a twine out of the concatenation of the null-terminated strings.
  TwineChar16(const llvh::StringRef left, const char16_t *right)
      : leftKind_(CharStrKind),
        rightKind_(Char16StrKind),
        leftSize_(left.size()),
        rightSize_(utf16_traits::length(right)) {
    leftChild_.charStr = left.begin();
    rightChild_.char16Str = right;
    assert(isValid());
  }

  /// Make a twine out of the concatenation of the null-terminated strings.
  TwineChar16(const char16_t *left, const llvh::StringRef right)
      : leftKind_(Char16StrKind),
        rightKind_(CharStrKind),
        leftSize_(utf16_traits::length(left)),
        rightSize_(right.size()) {
    leftChild_.char16Str = left;
    rightChild_.charStr = right.begin();
    assert(isValid());
  }

  /// Make a twine out of the concatenation of the null-terminated strings.
  TwineChar16(const char16_t *left, const char16_t *right)
      : leftKind_(Char16StrKind),
        rightKind_(Char16StrKind),
        leftSize_(utf16_traits::length(left)),
        rightSize_(utf16_traits::length(right)) {
    leftChild_.char16Str = left;
    rightChild_.char16Str = right;
    assert(isValid());
  }

  /// Make a twine out of the concatenation of the UTF16Refs.
  TwineChar16(const UTF16Ref left, const UTF16Ref right)
      : leftKind_(Char16StrKind),
        rightKind_(Char16StrKind),
        leftSize_(left.size()),
        rightSize_(right.size()) {
    leftChild_.char16Str = left.begin();
    rightChild_.char16Str = right.begin();
    assert(isValid());
  }

  /// Make a twine out of the concatenation of the UTF16Ref and raw string.
  TwineChar16(const UTF16Ref left, const char16_t *right)
      : leftKind_(Char16StrKind),
        rightKind_(Char16StrKind),
        leftSize_(left.size()),
        rightSize_(utf16_traits::length(right)) {
    leftChild_.char16Str = left.begin();
    rightChild_.char16Str = right;
    assert(isValid());
  }

  /// Make a twine out of the concatenation of the raw string and UTF16Ref.
  TwineChar16(const char16_t *left, const UTF16Ref right)
      : leftKind_(Char16StrKind),
        rightKind_(Char16StrKind),
        leftSize_(utf16_traits::length(left)),
        rightSize_(right.size()) {
    leftChild_.char16Str = left;
    rightChild_.char16Str = right.begin();
    assert(isValid());
  }

  /// Make a twine out of the concatenation of the raw string and number.
  TwineChar16(const llvh::StringRef left, const int32_t right)
      : leftKind_(CharStrKind), rightKind_(Int32Kind), leftSize_(left.size()) {
    // snprintf returns the number of characters that right takes to write.
    rightSize_ = ::snprintf(0, 0, "%" PRId32, right);
    leftChild_.charStr = left.begin();
    rightChild_.int32 = right;
    assert(isValid());
  }

  /// Return a new null twine.
  static inline TwineChar16 createNull() {
    return TwineChar16(NullKind);
  }

  /// Concatenate twine \p other onto this one, returning a new twine.
  /// Does not modify the current twine.
  TwineChar16 concat(const TwineChar16 &other) const;

  /// \return the size of the string stored in the twine.
  inline size_t size() const {
    assert(isValid());
    return leftSize_ + rightSize_;
  }

  /// \return true if the string stored in the twine is empty.
  inline bool empty() const {
    assert(isValid());
    return size() == 0;
  }

  /// Output the string representation of the twine to an array.
  /// min(maxlen, size()) characters are copied, with no null terminator.
  /// \param out the start of the output location.
  /// \param maxlen the maximum number of characters to copy.
  /// \return the number of characters copied.
  size_t toChar16Str(char16_t *out, size_t maxlen) const;

  /// Append the string representation of this twine to \p out.
  /// Adds size() elements to \p out, does not include a null terminator.
  void toVector(llvh::SmallVectorImpl<char16_t> &out) const;

  /// Print the string representation of this twine to \p os.
  void print(llvh::raw_ostream &os) const;

 private:
  /// Make a nullary twine with the specified left kind.
  TwineChar16(NodeKind kind)
      : leftKind_(kind), rightKind_(EmptyKind), leftSize_(0), rightSize_(0) {
    assert(isNullary());
    assert(isValid());
  }

  /// Make a binary twine.
  TwineChar16(const TwineChar16 &left, const TwineChar16 &right)
      : leftKind_(TwineKind),
        rightKind_(TwineKind),
        leftSize_(left.size()),
        rightSize_(right.size()) {
    leftChild_.twine = &left;
    rightChild_.twine = &right;
    assert(isValid());
  }

  /// Custom twine with explicit values.
  TwineChar16(
      Node leftChild,
      NodeKind leftKind,
      size_t leftLen,
      Node rightChild,
      NodeKind rightKind,
      size_t rightLen)
      : leftChild_(leftChild),
        leftKind_(leftKind),
        rightChild_(rightChild),
        rightKind_(rightKind),
        leftSize_(leftLen),
        rightSize_(rightLen) {
    assert(isValid());
  }

  /// Twines are meant as temporary string constructions.
  TwineChar16 &operator=(const TwineChar16 &other) = delete;

  /// \return true if the left child is null.
  inline bool isNull() const {
    return leftKind_ == NullKind;
  }

  /// \return true if the left child is empty.
  inline bool isEmpty() const {
    return leftKind_ == EmptyKind;
  }

  /// \return true if twine is either null or empty.
  inline bool isNullary() const {
    return isNull() || isEmpty();
  }

  /// \return true if only the left side is populated.
  inline bool isUnary() const {
    return !isNullary() && rightKind_ == EmptyKind;
  }

  /// \return true if both children contain content.
  inline bool isBinary() const {
    return leftKind_ != NullKind && rightKind_ != EmptyKind;
  }

  /// Checks internal structural invariants of the twine.
  inline bool isValid() const {
    // Right side never gets assigned null, only the left does.
    if (rightKind_ == NullKind) {
      fprintf(stderr, "null right\n");
      return false;
    }

    // Nullary strings must have an empty right side.
    if (isNullary() && rightKind_ != EmptyKind) {
      fprintf(stderr, "nullary with nonempty right\n");
      return false;
    }

    // If the left side is empty, then it have anything on the right.
    if (leftKind_ == EmptyKind && rightKind_ != EmptyKind) {
      fprintf(stderr, "left empty, right not\n");
      return false;
    }

    // If twines are stored as children, they should be binary.
    if ((leftKind_ == TwineKind && !leftChild_.twine->isBinary()) ||
        (rightKind_ == TwineKind && !rightChild_.twine->isBinary())) {
      fprintf(stderr, "non-binary twine child\n");
      return false;
    }

    return true;
  }
};

inline TwineChar16 TwineChar16::concat(const TwineChar16 &other) const {
  assert(isValid());
  assert(other.isValid());
  if (isNull() || other.isNull()) {
    return TwineChar16::createNull();
  }

  // If one of the strings is empty, just return the other one.
  if (isEmpty()) {
    return other;
  }
  if (other.isEmpty()) {
    return *this;
  }

  // Make a new twine, with two twine children.
  Node newLeft;
  newLeft.twine = this;
  NodeKind newLeftKind = TwineKind;
  size_t leftLen = size();

  Node newRight;
  newRight.twine = &other;
  NodeKind newRightKind = TwineKind;
  size_t rightLen = other.size();

  // If either of the operands are unary, promote their left children.
  if (isUnary()) {
    newLeft = leftChild_;
    newLeftKind = leftKind_;
  }
  if (other.isUnary()) {
    newRight = other.leftChild_;
    newRightKind = other.leftKind_;
  }

  return TwineChar16(
      newLeft, newLeftKind, leftLen, newRight, newRightKind, rightLen);
}

inline TwineChar16 operator+(
    const TwineChar16 &left,
    const TwineChar16 &right) {
  return left.concat(right);
}

inline TwineChar16 operator+(
    const llvh::StringRef left,
    const char16_t *right) {
  return TwineChar16(left, right);
}

inline TwineChar16 operator+(const llvh::StringRef left, const int32_t right) {
  return TwineChar16(left, right);
}

inline llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    const TwineChar16 &other) {
  other.print(os);
  return os;
}

} // namespace vm
} // namespace hermes
#endif
