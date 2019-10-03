/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Regex/Compiler.h"
#include "hermes/Regex/RegexTraits.h"
namespace hermes {
namespace regex {

using llvm::Optional;

/// Parser is a class responsible for implementing the productions of the
/// regex grammar, using a handwritten recursive descent parser.
template <class RegexType, class ForwardIterator>
class Parser {
  // The character type that we are parsing.
  using CharT = typename std::iterator_traits<ForwardIterator>::value_type;

  /// Simple type representing a list of characters.
  /// Size of 2 because we use this to collect the start and end of char class
  /// ranges, like /[a-z]/.
  using CharList = llvm::SmallVector<char16_t, 2>;

  // The type of a node in our regex.
  using Node = typename RegexType::Node;

  // The type of a bracket node in particular.
  using BracketNode = typename RegexType::BracketNode;

  // The regexp that we are building. This receives the results of our
  // productions. This may be a real regex or a dummy regex.
  RegexType *re_;

  // The current and end iterators in the regex input string.
  ForwardIterator current_;
  const ForwardIterator end_;

  // The error that was set, if any.
  constants::ErrorType error_ = constants::ErrorType::None;

  // See comment --DecimalEscape--.
  const uint32_t backRefLimit_;
  uint32_t maxBackRef_ = 0;

  /// Set the error \p err, if not already set to a different error.
  /// Also move our input to end, to abort parsing.
  /// \return false, for convenience.
  void setError(constants::ErrorType err) {
    if (error_ == constants::ErrorType::None) {
      error_ = err;
      current_ = end_;
    }
  }

  /// Type wrapping up a quantifier.
  /// This includes all bracket expressions /a{1, 3}/, as well as *, +, ?
  struct Quantifier {
    /// Minimum number of repetitions.
    uint32_t min_ = 0;

    /// Maximum number of repetitions, or uint32_t::max for unlimited.
    uint32_t max_ = std::numeric_limits<uint32_t>::max();

    /// Whether this quantifier is greedy.
    bool greedy_ = true;

    /// The first marked subexpression of the term that this quantifies.
    /// For example, in the regex /(a)(b)((c)|d){3, 5}/ this would be 2, because
    /// we are quantifying the marked subexpression at index 2.
    uint32_t startMarkedSubexprs_;

    /// The start node of the expression which we are quantifying. This is owned
    /// by the regex. The end node is always the last node of the regex.
    Node *quantifiedNodeStart_ = nullptr;
  };

  /// \return a quantifier prepopulated with our current state.
  Quantifier prepareQuantifier() {
    Quantifier q;
    q.startMarkedSubexprs_ = re_->markedCount();
    q.quantifiedNodeStart_ = re_->currentNode();
    return q;
  }

  /// Given a quantifier \p quant, replace the expression it captured with a
  /// quantified expression.
  void applyQuantifier(const Quantifier &quant) {
    if (quant.min_ > quant.max_) {
      setError(constants::ErrorType::BraceRange);
      return;
    }
    auto quantifiedExpression = re_->spliceOut(quant.quantifiedNodeStart_);
    re_->pushLoop(
        quant.min_,
        quant.max_,
        move(quantifiedExpression),
        quant.startMarkedSubexprs_,
        quant.greedy_);
  }

  /// Consume a single character which must be the next character in the string.
  /// \return the character.
  CharT consume(CharT c) {
    assert(current_ != end_ && *current_ == c && "Could not consume char");
    current_++;
    return c;
  }

  /// Attempt to consume a string literal.
  /// \return true if the entire string could be consumed, false if not.
  bool tryConsume(const char *seq) {
    auto cursor = current_;
    for (size_t i = 0; seq[i]; i++) {
      if (cursor == end_ || *cursor != seq[i])
        return false;
      ++cursor;
    }
    current_ = cursor;
    return true;
  }

  /// Attempt to consume a single character.
  /// \return true if the entire string could be consumed, false if not.
  bool tryConsume(CharT c) {
    if (current_ == end_ || *current_ != c)
      return false;
    consume(c);
    return true;
  }

  /// If the cursor is on a character (i.e. not at end_), and that character
  /// satisfies a predicate \p pred, then advance the cursor.
  /// \return the character if it satisfies the predicate, otherwise None.
  template <typename Pred>
  Optional<CharT> consumeCharIf(const Pred &pred) {
    if (current_ != end_ && pred(*current_)) {
      CharT c = *current_;
      consume(c);
      return c;
    }
    return llvm::None;
  }

  /// ES6 21.2.2.3 Disjunction.
  void consumeDisjunction() {
    auto *cursor = re_->currentNode();
    consumeTerm();
    while (tryConsume('|')) {
      auto firstBranch = re_->spliceOut(cursor);
      consumeTerm();
      auto secondBranch = re_->spliceOut(cursor);
      re_->pushAlternation(move(firstBranch), move(secondBranch));
    }
  }

  /// ES6 21.2.2.5 Term.
  void consumeTerm() {
    while (current_ != end_) {
      Quantifier quant = prepareQuantifier();
      bool quantifierAllowed = true;
      const CharT c = *current_;
      switch (c) {
        case '^':
          re_->pushLeftAnchor();
          consume('^');
          quantifierAllowed = false;
          break;

        case '$':
          re_->pushRightAnchor();
          consume('$');
          quantifierAllowed = false;
          break;

        case '\\': {
          consume('\\');
          consumeAtomEscape();
          break;
        }

        case '.': {
          consume('.');
          re_->pushMatchAnyButNewline();
          break;
        }

        case '(': {
          if (tryConsume("(?=")) {
            // Positive lookahead.
            consumeLookaheadAssertion(false /* negate */);
          } else if (tryConsume("(?!")) {
            // Negative lookahead.
            consumeLookaheadAssertion(true /* negate */);
          } else if (tryConsume("(?:")) {
            // Non-capturing group.
            consumeDisjunction();
          } else {
            // Capturing group.
            consume('(');
            re_->pushBeginMarkedSubexpression();
            auto mexp = re_->markedCount();
            consumeDisjunction();
            re_->pushEndMarkedSubexpression(mexp);
          }
          if (!tryConsume(')')) {
            setError(constants::ErrorType::UnbalancedParenthesis);
            return;
          }
          break;
        }

        case '[': {
          consumeCharacterClass();
          break;
        }

        case '*':
        case '+':
        case '?': {
          setError(constants::ErrorType::InvalidRepeat);
          return;
        }

        case '{': {
          // If this is a valid quantifier, it is an error.
          Quantifier tmp;
          if (tryConsumeQuantifier(&tmp)) {
            setError(constants::ErrorType::InvalidRepeat);
            return;
          }
          re_->pushChar(consume('{'));
          break;
        }

        case '|':
        case ')': {
          // End of the disjunction or group.
          return;
        }

        default: {
          // Ordinary char.
          re_->pushChar(consume(c));
          break;
        }
      }

      // We just parsed one term. Try parsing a quantifier.
      if (tryConsumeQuantifier(&quant)) {
        if (!quantifierAllowed) {
          setError(constants::ErrorType::InvalidRepeat);
          return;
        }
        applyQuantifier(quant);
      }
    }
  }

  /// ES6 21.2.2.7 Quantifier.
  bool tryConsumeQuantifier(Quantifier *quantifier) {
    if (!tryConsumeQuantifierPrefix(quantifier)) {
      return false;
    }
    quantifier->greedy_ = !tryConsume('?');
    return true;
  }

  /// ES6 21.2.2.7 QuantifierPrefix.
  bool tryConsumeQuantifierPrefix(Quantifier *quantifier) {
    if (current_ == end_) {
      return false;
    }
    constexpr uint32_t uintmax = std::numeric_limits<uint32_t>::max();
    switch (*current_) {
      case '*':
        consume('*');
        quantifier->min_ = 0;
        quantifier->max_ = uintmax;
        return true;

      case '+':
        consume('+');
        quantifier->min_ = 1;
        quantifier->max_ = uintmax;
        return true;

      case '?':
        consume('?');
        quantifier->min_ = 0;
        quantifier->max_ = 1;
        return true;

      case '{': {
        auto saved = current_;
        consume('{');
        if (auto v1 = tryConsumeDecimalIntegerLiteral()) {
          quantifier->min_ = *v1;
          if (!tryConsume(',')) {
            // Like {3}
            quantifier->max_ = *v1;
          } else if (auto v2 = tryConsumeDecimalIntegerLiteral()) {
            // Like {3,5}
            quantifier->max_ = *v2;
          } else {
            // Like {3,}
            quantifier->max_ = uintmax;
          }
          if (tryConsume('}')) {
            return true;
          }
        }
        current_ = saved;
        return false;
      }

      default:
        return false;
    }
  }

  /// ES6 21.2.2.13 CharacterClass.
  void consumeCharacterClass() {
    consume('[');
    bool negate = tryConsume('^');
    auto bracket = re_->startBracketList(negate);
    for (;;) {
      if (current_ == end_) {
        setError(constants::ErrorType::UnbalancedBracket);
        return;
      }

      if (tryConsume(']')) {
        // End of bracket.
        return;
      }

      // Specially handle character class escapes like \d or \D.
      // These cannot participate in ranges.
      if (tryConsumeCharacterClassEscape(bracket)) {
        continue;
      }

      // We may have a single atom like [abc], or a range like [a-c]
      auto first = tryConsumeBracketNonClassAtom();
      assert(first && "Should always have a left atom");
      if (!tryConsume('-')) {
        // The atom was not followed by a range.
        bracket->addChar(*first);
      } else if (auto second = tryConsumeBracketNonClassAtom()) {
        // Range like [a-c].
        // ES6 21.2.2.15.1 "If i > j, throw a SyntaxError exception"
        if (*first > *second) {
          setError(constants::ErrorType::CharacterRange);
          return;
        }
        bracket->addRange(*first, *second);
      } else {
        // We found a dash but not a range. Examples:
        // [a-] (dash is last)
        // [a-\w] (character class)
        // Here the dash is just an ordinary character.
        bracket->addChar(*first);
        bracket->addChar('-');
      }
    }
  }

  /// ES6 21.2.2.12 CharacterClassEscape.
  /// Note this is used inside brackets only, like /[\d]/.
  bool tryConsumeCharacterClassEscape(BracketNode *bracket) {
    if (current_ != end_ && *current_ == '\\') {
      auto next = current_ + 1;
      if (next != end_) {
        CharT c = *next;
        switch (c) {
          case 'd':
          case 'D':
            current_ = next + 1;
            bracket->addClass({CharacterClass::Digits, c == 'D' /* invert */});
            return true;

          case 's':
          case 'S':
            current_ = next + 1;
            bracket->addClass({CharacterClass::Spaces, c == 'S' /* invert */});
            return true;

          case 'w':
          case 'W':
            current_ = next + 1;
            bracket->addClass({CharacterClass::Words, c == 'W' /* invert */});
            return true;
        }
      }
    }
    return false;
  }

  Optional<char16_t> tryConsumeBracketNonClassAtom() {
    if (current_ == end_) {
      return llvm::None;
    }
    CharT c = *current_;
    switch (c) {
      case ']': {
        // End of bracket. Note we don't consume it here.
        return llvm::None;
      }

      case '\\': {
        auto saved = current_;
        consume('\\');
        if (current_ == end_) {
          setError(constants::ErrorType::EscapeIncomplete);
          return llvm::None;
        }
        CharT ec = *current_;
        switch (ec) {
          case 'd':
          case 'D':
          case 's':
          case 'S':
          case 'w':
          case 'W': {
            // Character classes get handled in
            // tryConsumeCharacterClassEscape. We need to handle these here so
            // that they do not become parts of ranges, e.g. /[a-\d]/ should
            // be the same as /[\da-]/.
            current_ = saved;
            return llvm::None;
          }

          case 'b': {
            // "Return the CharSet containing the single character <BS>
            // U+0008 (BACKSPACE)"
            consume('b');
            return 0x08;
          }

          default: {
            return consumeCharacterEscape();
          }
        }
      }

      default: {
        // Ordinary character.
        consume(c);
        return c;
      }
    }
  }

  // ES6 B.1.2 LegacyOctalEscapeSequence
  // Note this is required by Annex B for regexp even in strict mode.
  char16_t consumeLegacyOctalEscapeSequence() {
    // LegacyOctalEscapeSequence:
    //   OctalDigit [lookahead not OctalDigit]
    //   ZeroToThree OctalDigit [lookahead not OctalDigit]
    //   FourToSeven OctalDigit
    //   ZeroToThree OctalDigit OctalDigit
    // We implement this more directly.
    auto isOctalDigit = [](CharT c) { return '0' <= c && c <= '7'; };
    assert(
        current_ != end_ && isOctalDigit(*current_) &&
        "Should have leading octal digit");
    auto d1 = *current_++;
    auto d2 = consumeCharIf(isOctalDigit);
    auto d3 = (d1 <= '3' ? consumeCharIf(isOctalDigit) : llvm::None);

    char16_t result = d1 - '0';
    if (d2)
      result = result * 8 + (*d2 - '0');
    if (d3)
      result = result * 8 + (*d3 - '0');
    return result;
  }

  /// ES6 11.8.3 DecimalIntegerLiteral .
  /// If the value would overflow, uint32_t::max() is returned.
  /// All decimal digits are consumed regardless.
  uint32_t consumeDecimalIntegerLiteral() {
    auto decimalDigit = [](CharT c) { return '0' <= c && c <= '9'; };
    assert(
        current_ != end_ && decimalDigit(*current_) &&
        "Not a decimal integer literal");
    // Note that 'max' is a 64 bit value, but contains the max of u32.
    const uint64_t u32max = std::numeric_limits<uint32_t>::max();
    uint64_t bigResult = 0;
    while (auto digit = consumeCharIf(decimalDigit)) {
      bigResult = bigResult * 10 + (*digit - '0');
      if (bigResult > u32max) {
        bigResult = u32max;
      }
    }
    return static_cast<uint32_t>(bigResult);
  }

  /// ES6 11.8.3 DecimalIntegerLiteral .
  Optional<uint32_t> tryConsumeDecimalIntegerLiteral() {
    if (current_ != end_ && '0' <= *current_ && *current_ <= '9')
      return consumeDecimalIntegerLiteral();
    return llvm::None;
  }

  /// ES6 11.8.3 HexDigit .
  /// \return a uint derived from exactly \p count hex digits, or None.
  Optional<uint32_t> tryConsumeHexDigits(uint32_t count) {
    auto hexDigitValue = [](CharT c) -> Optional<uint32_t> {
      if ('0' <= c && c <= '9')
        return c - '0';
      if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
      if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
      return llvm::None;
    };

    auto saved = current_;
    uint32_t result = 0;
    for (uint32_t i = 0; i < count; i++) {
      if (auto c = consumeCharIf(hexDigitValue)) {
        result = result * 16 + *hexDigitValue(*c);
      } else {
        current_ = saved;
        return llvm::None;
      }
    }
    return result;
  }

  /// ES6 21.2.2.10 CharacterEscape.
  /// Given that we have consumed a backslash and there is a following
  /// character, consume the next character as a CharacterEscape. \return the
  /// escaped character. Note this cannot fail as IdentityEscape is a fallback
  /// (e.g. /\q/ is the same as /q/).
  char16_t consumeCharacterEscape() {
    if (current_ == end_) {
      setError(constants::ErrorType::EscapeIncomplete);
      return 0;
    }
    const CharT c = *current_;
    switch (c) {
      case 'f':
        consume('f');
        return 0xC;
      case 'n':
        consume('n');
        return 0xA;
      case 'r':
        consume('r');
        return 0xD;
      case 't':
        consume('t');
        return 0x9;
      case 'v':
        consume('v');
        return 0xB;

      case 'c': {
        consume('c');
        auto isControlLetter = [](CharT c) {
          return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
        };
        if (auto cc = consumeCharIf(isControlLetter)) {
          return *cc % 32;
        } else {
          // IdentityEscape.
          return 'c';
        }
      }

      case '0': {
        // CharacterEscape :: "0 [lookahead != DecimalDigit]"
        auto saved = current_;
        consume('0');
        if (current_ == end_ || !('0' <= *current_ && *current_ <= '9')) {
          return '\0';
        } else {
          current_ = saved;
          return consumeLegacyOctalEscapeSequence();
        }
      }

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7': {
        return consumeLegacyOctalEscapeSequence();
      }

      case 'x':
      case 'u': {
        // x is two-char hex escape, u is four-char Unicode escape.
        uint32_t digitCount = (c == 'x' ? 2 : 4);
        consume(c);
        if (auto ret = tryConsumeHexDigits(digitCount)) {
          return *ret;
        } else {
          // Not followed by sufficient hex digits.
          // Note this is not an error; for example /\x1Z/ matches "x1Z" via
          // IdentityEscape.
          return c;
        }
      }

      default: {
        // IdentityEscape
        return consume(c);
      }
    }
  }

  /// ES6 21.2.2.6 Assertion.
  /// This implements positive and negative lookaheads.
  /// We will have consumed the leading paren; the cursor is at the
  /// disjunction.
  void consumeLookaheadAssertion(bool negate) {
    // Parse a disjunction, then splice it out from our list.
    auto mexpBegin = re_->markedCount();
    auto exprStart = re_->currentNode();
    consumeDisjunction();
    auto mexpEnd = re_->markedCount();
    auto expr = re_->spliceOut(exprStart);
    re_->pushLookahead(move(expr), mexpBegin, mexpEnd, negate);
  }

  /// 21.2.2.9 AtomEscape.
  /// Here the backslash has been consumed.
  void consumeAtomEscape() {
    if (current_ == end_) {
      setError(constants::ErrorType::EscapeIncomplete);
      return;
    }

    CharT c = *current_;
    switch (c) {
      case 'b':
      case 'B':
        consume(c);
        re_->pushWordBoundary(c == 'B' /* invert */);
        break;

      case 'd':
      case 'D':
        consume(c);
        re_->pushCharClass({CharacterClass::Digits, c == 'D' /* invert */});
        break;

      case 's':
      case 'S':
        consume(c);
        re_->pushCharClass({CharacterClass::Spaces, c == 'S' /* invert */});
        break;

      case 'w':
      case 'W':
        consume(c);
        re_->pushCharClass({CharacterClass::Words, c == 'W' /* invert */});
        break;

        // Note backreferences may NOT begin with 0.
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        // This may be a backreference, in which case we parse it via decimal.
        // Otherwise, this is an octal escape unless our value is not octal
        // (>= 8). Otherwise this is identity escape.
        auto saved = current_;
        uint32_t decimal = consumeDecimalIntegerLiteral();
        if (decimal <= backRefLimit_) {
          // Backreference.
          maxBackRef_ = std::max(maxBackRef_, decimal);
          re_->pushBackRef(decimal);
        } else if (c < '8') {
          // Octal.
          current_ = saved;
          re_->pushChar(consumeLegacyOctalEscapeSequence());
        } else {
          // IdentityEscape.
          current_ = saved;
          re_->pushChar(consume(c));
        }
        break;
      }

      default: {
        re_->pushChar(consumeCharacterEscape());
        break;
      }
    }
  }

 public:
  /// Constructor from a regular expression \p re and regexp input string
  /// range \p start to \p end. Escapes like \123 are considered to be
  /// backrefs if they do not exceed backRefLimit.
  Parser(
      RegexType *re,
      ForwardIterator start,
      ForwardIterator end,
      uint32_t backRefLimit)
      : re_(re), current_(start), end_(end), backRefLimit_(backRefLimit) {}

  constants::ErrorType performParse() {
    consumeDisjunction();
    if (current_ != end_) {
      // We are top level and did not consume all the input.
      // The only way this can happen is if we have an unbalanced ).
      // TODO: express that directly in the grammar, so that the grammar
      // always consumes all input or errors.
      setError(constants::ErrorType::UnbalancedParenthesis);
    }
    return error_;
  }

  /// \return the maximum number of backreferences encountered
  uint32_t maxBackRef() const {
    return maxBackRef_;
  }
};

template <typename Receiver>
constants::ErrorType parseRegex(
    const char16_t *start,
    const char16_t *end,
    Receiver *receiver,
    uint32_t backRefLimit,
    uint32_t *outMaxBackRef) {
  Parser<Receiver, const char16_t *> parser(receiver, start, end, backRefLimit);
  auto result = parser.performParse();
  *outMaxBackRef = parser.maxBackRef();
  return result;
}

// Explicitly instantiate this for U16RegexTraits only.
template constants::ErrorType parseRegex(
    const char16_t *start,
    const char16_t *end,
    Regex<U16RegexTraits> *receiver,
    uint32_t backRefLimit,
    uint32_t *outMaxBackRef);

} // namespace regex
} // namespace hermes
