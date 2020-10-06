/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Platform/Unicode/CodePointSet.h"

#include "hermes/Regex/Regex.h"
#include "hermes/Regex/RegexTraits.h"
#include "llvh/Support/SaveAndRestore.h"
namespace hermes {
namespace regex {

using llvh::Optional;

/// Parser is a class responsible for implementing the productions of the
/// regex grammar, using a handwritten recursive descent parser.
template <class RegexType, class ForwardIterator>
class Parser {
  // The character type that we are parsing.
  using CharT = typename std::iterator_traits<ForwardIterator>::value_type;

  // A Unicode code point.
  using CodePoint = uint32_t;

  // The type of a node in our regex.
  using Node = typename RegexType::Node;

  // An element of a class ranges.
  // This may be either a code point, or a CharacterClass.
  struct ClassAtom {
    CodePoint codePoint = -1;
    llvh::Optional<CharacterClass> charClass{};

    explicit ClassAtom(CodePoint cp) : codePoint(cp) {}
    ClassAtom(CharacterClass::Type cc, bool invert)
        : charClass(CharacterClass(cc, invert)) {}
  };

  // The regexp that we are building. This receives the results of our
  // productions. This may be a real regex or a dummy regex.
  RegexType *re_;

  // The current and end iterators in the regex input string.
  ForwardIterator current_;
  const ForwardIterator end_;

  // The error that was set, if any.
  constants::ErrorType error_ = constants::ErrorType::None;

  // Flags for the regex.
  const SyntaxFlags flags_;

  // See comment --DecimalEscape--.
  const uint32_t backRefLimit_;
  uint32_t maxBackRef_ = 0;

  // Maximum depth of our parser. This is effectively the depth of C stack
  // frames that the parser may use, and is the mechanism that prevents stack
  // overflow for overly complex expressions.
  static constexpr uint32_t kMaxParseDepth =
#ifdef HERMES_LIMIT_STACK_DEPTH
      256
#elif defined(_MSC_VER) && defined(HERMES_SLOW_DEBUG)
      256
#elif defined(_MSC_VER)
      512
#else
      1024
#endif
      ;

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
    assert(check(c) && "Could not consume char");
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
    if (!check(c))
      return false;
    consume(c);
    return true;
  }

  /// Check if the next character to be consumed is \p c
  /// \return true if the character is \p c, false if not.
  bool check(CharT c) const {
    return current_ != end_ && *current_ == c;
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
    return llvh::None;
  }

  /// Our parser uses an explicit stack to avoid stack overflow for deeply
  /// nested regexps. This represents a level in the stack.
  struct ParseStackElement {
    enum Type {
      /// We are parsing an alternation: a|b|c
      Alternation,

      /// We are parsing a capturing group: ().
      CapturingGroup,

      /// We are parsing a non-capturing group: (?:).
      NonCapturingGroup,

      /// We are parsing a lookaround:
      // Positive lookahead (?=)
      // Negative lookahead (?!)
      // Positive lookbehind (?<=)
      // Negative lookbehind (?<!)
      LookAround,

    } type;

    /// The splice point.
    /// For Alternation elements, nodes after this represent an alternative.
    /// For Group elements, nodes after this are part of the group.
    Node *splicePoint{nullptr};

    /// For group elements, the marked subexpression index for this element.
    /// For lookaround elements, the marked subexpression index when this
    /// lookaround element was pushed onto the stack.
    /// Ignored for alternation elements.
    uint32_t mexp{0};

    /// A quantifier prepared for the currently parsed group.
    /// Ignored for alternations (which cannot be quantified).
    Quantifier quant{};

    /// If this is an Alternation, the list of alternatives
    /// Ignored for group elements.
    std::vector<NodeList> alternatives;

    // True if this lookaround represents a negative lookaround. Ignored for
    // non-lookarounds
    bool negateLookaround;

    // True if this lookaround is a lookahead. Ignored for non-lookarounds.
    bool forwardLookaround;

    explicit ParseStackElement(Type type) : type(type) {}
  };

  using ParseStack = llvh::SmallVector<ParseStackElement, 4>;

  /// Push an empty alternation onto the parse stack \p stack.
  void openAlternation(ParseStack &stack) {
    // Should never have two adjacent alternations on the stack
    assert(
        stack.empty() || stack.back().type != ParseStackElement::Alternation);
    ParseStackElement elem(ParseStackElement::Alternation);
    elem.splicePoint = re_->currentNode();
    stack.push_back(std::move(elem));
  }

  /// Close any alternation on top of the stack.
  void closeAlternation(ParseStack &stack) {
    if (!stack.empty() && stack.back().type == ParseStackElement::Alternation) {
      auto alternatives = std::move(stack.back().alternatives);
      auto last = re_->spliceOut(stack.back().splicePoint);
      stack.pop_back();
      alternatives.push_back(std::move(last));
      re_->pushAlternation(std::move(alternatives));
    }
  }

  /// Open a capturing group, pushing it onto \p stack.
  void openCapturingGroup(ParseStack &stack) {
    ParseStackElement elem(ParseStackElement::CapturingGroup);
    // Quantifier must be prepared before incrementing the marked counter
    // because it uses the marked counter to perform the simple loop
    // optimisation
    elem.quant = prepareQuantifier();
    elem.mexp = re_->incrementMarkedCount();
    elem.splicePoint = re_->currentNode();
    stack.push_back(std::move(elem));
  }

  /// Open a non-capturing group, pushing it onto \p stack.
  void openNonCapturingGroup(ParseStack &stack) {
    ParseStackElement elem(ParseStackElement::NonCapturingGroup);
    elem.quant = prepareQuantifier();
    elem.splicePoint = re_->currentNode();
    stack.push_back(std::move(elem));
  }

  /// Open a lookaround, pushing it onto \p stack.
  void openLookaround(ParseStack &stack, bool negate, bool forwards) {
    ParseStackElement elem(ParseStackElement::LookAround);
    elem.mexp = re_->markedCount();
    elem.forwardLookaround = forwards;
    elem.negateLookaround = negate;
    elem.quant = prepareQuantifier();
    elem.splicePoint = re_->currentNode();
    stack.push_back(std::move(elem));
  }

  /// Close a group or lookaround on top of \p stack, emitting the proper nodes
  /// into the regex.
  void closeGroup(ParseStack &stack) {
    assert(!stack.empty() && "Stack must not be empty");
    ParseStackElement elem = std::move(stack.back());
    stack.pop_back();
    bool quantifierAllowed = true;
    switch (elem.type) {
      case ParseStackElement::Alternation:
        llvm_unreachable("Alternations must be popped via closeAlternation()");
        break;

      case ParseStackElement::CapturingGroup:
        re_->pushMarkedSubexpression(
            re_->spliceOut(elem.splicePoint), elem.mexp);
        break;

      case ParseStackElement::NonCapturingGroup:
        break;

      case ParseStackElement::LookAround: {
        quantifierAllowed = !(flags_.unicode);
        bool negate = elem.negateLookaround;
        bool forwards = elem.forwardLookaround;
        auto mexpStart = elem.mexp;
        auto mexpEnd = re_->markedCount();
        auto expr = re_->spliceOut(elem.splicePoint);
        re_->pushLookaround(
            std::move(expr), mexpStart, mexpEnd, negate, forwards);
        break;
      }
    }

    // If the group is followed by a quantifier, then quantify it.
    if (tryConsumeQuantifier(&elem.quant)) {
      if (!quantifierAllowed) {
        setError(constants::ErrorType::InvalidRepeat);
        return;
      }
      applyQuantifier(elem.quant);
    }
  }

  /// ES6 21.2.2.3 Disjunction.
  void consumeDisjunction() {
    Node *const firstNode = re_->currentNode();
    ParseStack stack;

    while (current_ != end_) {
      switch (*current_) {
        case '|': {
          consume('|');
          auto *splicePoint =
              stack.empty() ? firstNode : stack.back().splicePoint;
          auto nodes = re_->spliceOut(splicePoint);
          if (stack.empty() ||
              stack.back().type != ParseStackElement::Alternation) {
            // Open a new alternation
            openAlternation(stack);
          }
          stack.back().alternatives.push_back(std::move(nodes));
          break;
        }

        case '(': {
          // Open a new group of the right type.
          if (tryConsume("(?=")) {
            // Positive lookeahead, negate = false, forwards = true
            openLookaround(stack, false, true);
          } else if (tryConsume("(?!")) {
            // Negative lookeahead, negate = true, forwards = true
            openLookaround(stack, true, true);
          } else if (tryConsume("(?<=")) {
            // Positive lookbehind, negate = false, forwards = false
            openLookaround(stack, false, false);
          } else if (tryConsume("(?<!")) {
            // Negative lookbehind, negate = true, forwards = false
            openLookaround(stack, true, false);
          } else if (tryConsume("(?:")) {
            openNonCapturingGroup(stack);
          } else {
            consume('(');
            openCapturingGroup(stack);
          }
          break;
        }

        case ')': {
          // Close any in-flight alternation and pop a group stack
          // element.
          consume(')');
          closeAlternation(stack);
          if (stack.empty()) {
            setError(constants::ErrorType::UnbalancedParenthesis);
            return;
          }
          closeGroup(stack);
          break;
        }
      }

      if (stack.size() > kMaxParseDepth) {
        setError(constants::ErrorType::PatternExceedsParseLimits);
        return;
      }
      consumeTerm();
    }

    assert(current_ == end_ && "Should have consumed all input");
    closeAlternation(stack);
    if (!stack.empty()) {
      setError(constants::ErrorType::UnbalancedParenthesis);
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
          // This may be an ES6 21.2.2.6 Assertion (\b or \B) or an AtomeEscape.
          if (current_ == end_) {
            setError(constants::ErrorType::EscapeIncomplete);
            return;
          } else if (*current_ == 'b' || *current_ == 'B') {
            re_->pushWordBoundary(*current_ == 'B' /* invert */);
            consume(*current_);
            quantifierAllowed = false;
          } else {
            consumeAtomEscape();
          }
          break;
        }

        case '.': {
          consume('.');
          re_->pushMatchAny();
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
          // Under Unicode, this is always an error.
          // Without Unicode, it is an error if it is a valid quantifier.
          Quantifier tmp;
          if (tryConsumeQuantifier(&tmp)) {
            setError(constants::ErrorType::InvalidRepeat);
            return;
          } else if (flags_.unicode) {
            setError(constants::ErrorType::InvalidQuantifierBracket);
            return;
          }
          re_->pushChar(consume('{'));
          break;
        }

        case '|':
        case '(':
        case ')': {
          // This is a new alternation, or the beginning or end of a group or
          // assertion. Transfer control back to consumeDisjunction() so it can
          // act on it.
          return;
        }

        case '}':
        case ']': {
          // These syntax characters are allowed as atoms in
          // ExtendedPatternCharacter production of ES9 Annex B 1.4.
          // However they are disallowed under Unicode, where Annex B does not
          // apply.
          if (flags_.unicode) {
            setError(
                c == '}' ? constants::ErrorType::InvalidQuantifierBracket
                         : constants::ErrorType::UnbalancedBracket);
            return;
          }
        }
          // Fall-through

        default: {
          // Ordinary character or surrogate pair.
          if (auto cp = tryConsumeSurrogatePair()) {
            re_->pushChar(*cp);
          } else {
            re_->pushChar(consume(c));
          }
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

  /// If Unicode is set, try to consume a surrogate pair.
  Optional<CodePoint> tryConsumeSurrogatePair() {
    if (!(flags_.unicode))
      return llvh::None;
    auto saved = current_;
    auto hi = consumeCharIf(isHighSurrogate);
    auto lo = consumeCharIf(isLowSurrogate);
    if (hi && lo) {
      return decodeSurrogatePair(*hi, *lo);
    }
    current_ = saved;
    return llvh::None;
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
    bool unicode = flags_.unicode;
    bool negate = tryConsume('^');
    auto bracket = re_->startBracketList(negate);

    // Helper to add a ClassAtom to our bracket.
    auto addClassAtom = [&bracket](const ClassAtom &atom) {
      if (atom.charClass) {
        bracket->addClass(*atom.charClass);
      } else {
        bracket->addChar(atom.codePoint);
      }
    };

    for (;;) {
      if (current_ == end_) {
        setError(constants::ErrorType::UnbalancedBracket);
        return;
      }

      if (tryConsume(']')) {
        // End of bracket.
        return;
      }

      // Parse a code point or character class.
      Optional<ClassAtom> first = tryConsumeBracketClassAtom();
      if (!first)
        continue;

      // See if we have a dash.
      if (!tryConsume('-')) {
        addClassAtom(*first);
        continue;
      }

      // We have a dash; we may have a range.
      Optional<ClassAtom> second = tryConsumeBracketClassAtom();
      if (!second) {
        // No second atom. For example: [a-].
        addClassAtom(*first);
        addClassAtom(ClassAtom('-'));
        continue;
      }

      // We have a range like [a-z].
      // Ranges can't contain character classes: [\d-z] is invalid.
      if (first->charClass || second->charClass) {
        if (unicode) {
          // The unicode path is an error.
          setError(constants::ErrorType::CharacterRange);
          return;
        } else {
          // The non-unicode path just pretends the range doesn't exist.
          // /[\d-A]/ is the same as /[\dA-]/.
          // Note we still have to process all three characters. For
          // example:
          // [\d-a-z] contains the atoms \d, -, a, -, z.
          // It does NOT contain the range a-z.
          addClassAtom(*first);
          addClassAtom(ClassAtom('-'));
          addClassAtom(*second);
          continue;
        }
      }

      // Here we know it's a real range: [a-z] and not [\d-f].
      // However it could be out of order: [z-a]
      // ES6 21.2.2.15.1 "If i > j, throw a SyntaxError exception"
      if (first->codePoint > second->codePoint) {
        setError(constants::ErrorType::CharacterRange);
        return;
      }
      // This range has been validated.
      bracket->addRange(first->codePoint, second->codePoint);
    }
  }

  Optional<ClassAtom> tryConsumeBracketClassAtom() {
    if (current_ == end_) {
      return llvh::None;
    }
    CharT c = *current_;
    switch (c) {
      case ']': {
        // End of bracket. Note we don't consume it here.
        return llvh::None;
      }

      case '\\': {
        consume('\\');
        if (current_ == end_) {
          setError(constants::ErrorType::EscapeIncomplete);
          return llvh::None;
        }
        CharT ec = *current_;
        switch (ec) {
            /// ES6 21.2.2.12 CharacterClassEscape.
            /// Note this is used inside brackets only, like /[\d]/.
          case 'd':
          case 'D': {
            consume(ec);
            return ClassAtom(CharacterClass::Digits, ec == 'D' /* invert */);
          }
          case 's':
          case 'S': {
            consume(ec);
            return ClassAtom(CharacterClass::Spaces, ec == 'S' /* invert */);
          }
          case 'w':
          case 'W': {
            consume(ec);
            return ClassAtom(CharacterClass::Words, ec == 'W' /* invert */);
          }

          case 'b': {
            // "Return the CharSet containing the single character <BS>
            // U+0008 (BACKSPACE)"
            consume('b');
            return ClassAtom(0x08);
          }

          case '-':
            // ES6 21.2.1 ClassEscape: \- escapes -, in Unicode expressions
            // only.
            if ((flags_.unicode) && tryConsume('-')) {
              return ClassAtom('-');
            }
            LLVM_FALLTHROUGH;

          default: {
            return ClassAtom(consumeCharacterEscape());
          }
        }
      }

      default: {
        // Ordinary character or surrogate pair.
        if (auto cp = tryConsumeSurrogatePair()) {
          return ClassAtom(*cp);
        } else {
          return ClassAtom(consume(c));
        }
      }
    }
  }

  // ES6 B.1.2 LegacyOctalEscapeSequence
  // Note this is required by Annex B for regexp even in strict mode.
  CodePoint consumeLegacyOctalEscapeSequence() {
    // LegacyOctalEscapeSequence:
    //   OctalDigit [lookahead not OctalDigit]
    //   ZeroToThree OctalDigit [lookahead not OctalDigit]
    //   FourToSeven OctalDigit
    //   ZeroToThree OctalDigit OctalDigit
    // We implement this more directly.
    // Note this is forbidden in Unicode.
    if (flags_.unicode) {
      setError(constants::ErrorType::EscapeInvalid);
      return 0;
    }
    auto isOctalDigit = [](CharT c) { return '0' <= c && c <= '7'; };
    assert(
        current_ != end_ && isOctalDigit(*current_) &&
        "Should have leading octal digit");
    auto d1 = *current_++;
    auto d2 = consumeCharIf(isOctalDigit);
    auto d3 = (d1 <= '3' ? consumeCharIf(isOctalDigit) : llvh::None);

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
  CodePoint consumeDecimalIntegerLiteral() {
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
  Optional<CodePoint> tryConsumeDecimalIntegerLiteral() {
    if (current_ != end_ && '0' <= *current_ && *current_ <= '9')
      return consumeDecimalIntegerLiteral();
    return llvh::None;
  }

  /// ES6 11.8.3 HexDigit .
  /// \return a uint derived from exactly \p count hex digits, or None.
  Optional<CodePoint> tryConsumeHexDigits(uint32_t count) {
    auto hexDigitValue = [](CharT c) -> Optional<uint32_t> {
      if ('0' <= c && c <= '9')
        return c - '0';
      if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
      if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
      return llvh::None;
    };

    auto saved = current_;
    uint32_t result = 0;
    for (uint32_t i = 0; i < count; i++) {
      if (auto c = consumeCharIf(hexDigitValue)) {
        result = result * 16 + *hexDigitValue(*c);
      } else {
        current_ = saved;
        return llvh::None;
      }
    }
    return result;
  }

  /// ES6 21.2.2.10 CharacterEscape.
  /// Given that we have consumed a backslash and there is a following
  /// character, consume the next character as a CharacterEscape. \return the
  /// escaped character. Note this cannot fail as IdentityEscape is a fallback
  /// (e.g. /\q/ is the same as /q/).
  CodePoint consumeCharacterEscape() {
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
          return identityEscape('c');
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

      case 'u': {
        if (auto ret = tryConsumeUnicodeEscapeSequence()) {
          return *ret;
        } else {
          // IdentityEscape
          return identityEscape(consume(c));
        }
      }

      case 'x': {
        consume(c);
        if (auto ret = tryConsumeHexDigits(2)) {
          return *ret;
        } else {
          // Not followed by sufficient hex digits.
          // Note this is not an error; for example /\x1Z/ matches "x1Z" via
          // IdentityEscape.
          return identityEscape(c);
        }
      }

      default: {
        // IdentityEscape
        return identityEscape(consume(c));
      }
    }
  }

  /// ES6 21.2.1 IdentityEscape
  CodePoint identityEscape(CharT c) {
    // In Unicode regexps, only syntax characters and '/' may be escaped.
    if (flags_.unicode) {
      if (c == 0 || c > 127 || !strchr("^$\\.*+?()[]{}|/", c)) {
        setError(constants::ErrorType::EscapeInvalid);
      }
    }
    // TODO: disallow "UnicodeIDContinue".
    return c;
  }

  /// ES6 21.2.2.10 RegExpUnicodeEscapeSequence
  Optional<CodePoint> tryConsumeUnicodeEscapeSequence() {
    auto saved = current_;
    if (!consume('u')) {
      return llvh::None;
    }

    // Non-unicode path only supports \uABCD style escapes.
    if (!(flags_.unicode)) {
      if (auto ret = tryConsumeHexDigits(4)) {
        return *ret;
      }
      current_ = saved;
      return llvh::None;
    }

    // Unicode path.
    // Check for \u{ABCD123} style escapes.
    // Note that all \u in Unicode regexps must result in a valid escape; there
    // is no fallback to IdentityEscape. This means we must not return None.
    if (tryConsume('{')) {
      uint32_t result = 0;
      size_t digitCount = 0;
      while (auto digit = tryConsumeHexDigits(1)) {
        digitCount++;
        result = result * 16 + *digit;
        // 21.2.1.1: It is a Syntax Error if the MV of HexDigits > 1114111
        if (result > 1114111) {
          setError(constants::ErrorType::EscapeOverflow);
          return 0;
        }
      }
      if (!tryConsume('}')) {
        setError(constants::ErrorType::EscapeInvalid);
        return 0;
      }
      if (digitCount == 0) {
        // input was like \u{}
        setError(constants::ErrorType::EscapeInvalid);
        return 0;
      }
      return result;
    }

    // Check for \uABCD style escapes.
    if (auto hi = tryConsumeHexDigits(4)) {
      if (isHighSurrogate(*hi)) {
        // This is a leading surrogate.
        // Look for a trailing surrogate.
        auto saved2 = current_;
        if (tryConsume("\\u")) {
          if (auto lo = tryConsumeHexDigits(4)) {
            if (isLowSurrogate(*lo)) {
              return decodeSurrogatePair(*hi, *lo);
            }
          }
        }
        // No trailing surrogate.
        current_ = saved2;
      }
      return *hi;
    }

    // All \u failed escapes in Unicode regexps are an error.
    setError(constants::ErrorType::EscapeInvalid);
    return 0;
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
        // In Unicode mode, this is always a backreference.
        // In non-unicode mode, this is a backreference if its value does not
        // exceed the number of capture groups. Otherwise it is an octal escape
        // if its value is octal. Otherwise it is IdentityEscape.
        auto saved = current_;
        uint32_t decimal = consumeDecimalIntegerLiteral();
        bool unicode = flags_.unicode;
        if (unicode || decimal <= backRefLimit_) {
          // Backreference.
          maxBackRef_ = std::max(maxBackRef_, decimal);
          re_->pushBackRef(decimal);
        } else if (c < '8' && !unicode) {
          // Octal.
          current_ = saved;
          re_->pushChar(consumeLegacyOctalEscapeSequence());
        } else {
          // IdentityEscape.
          current_ = saved;
          re_->pushChar(identityEscape(consume(c)));
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
      SyntaxFlags flags,
      uint32_t backRefLimit)
      : re_(re),
        current_(start),
        end_(end),
        flags_(flags),
        backRefLimit_(backRefLimit) {}

  constants::ErrorType performParse() {
    consumeDisjunction();
    assert(
        current_ == end_ && "We should always consume all input even on error");
    return error_;
  }

  /// \return the maximum number of backreferences encountered
  uint32_t maxBackRef() const {
    return maxBackRef_;
  }
}; // namespace regex

template <typename Receiver>
constants::ErrorType parseRegex(
    const char16_t *start,
    const char16_t *end,
    Receiver *receiver,
    SyntaxFlags flags,
    uint32_t backRefLimit,
    uint32_t *outMaxBackRef) {
  Parser<Receiver, const char16_t *> parser(
      receiver, start, end, flags, backRefLimit);
  auto result = parser.performParse();
  *outMaxBackRef = parser.maxBackRef();
  return result;
}

// Explicitly instantiate the parser for UTF16RegexTraits only.
template constants::ErrorType parseRegex(
    const char16_t *start,
    const char16_t *end,
    Regex<UTF16RegexTraits> *receiver,
    SyntaxFlags flags,
    uint32_t backRefLimit,
    uint32_t *outMaxBackRef);

} // namespace regex
} // namespace hermes
