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
/// regex grammar, using a handwritten recursive descent parser. The grammar is
/// divided into Production and Terminal functions, identified via the prod_
/// and term_ prefixes. Both Productions and Terminals may succeed or fail,
/// represented as a simple boolean. They may also error. Errors are fatal and
/// effectively abort parsing: the parser will refuse to run any productions
/// after an error is set.
/// Productions may accept arguments. The arguments may be passed to the
/// operators which will pass them in turn to the Productions.
/// The operators are:
///   run: runs a single production. This takes care of saving and restoring the
///   parser state if the production fails.
///   sequence: runs a list of productions/terminals in order, returning true if
///   all succeed.
///   orderedChoice: runs a list of productions/terminals in order, returning
///   true if any succeeds.
/// An ideal Parser would be able to backtrack arbitrarily, but we build up a
/// regex as we go and have no capability to backtrack past adding nodes (yet).
/// Therefore this parser may be classified as "semi-predictive": it allows
/// backtracking across pure productions but not effectful ones. It would be
/// very desirable to make more/all productions pure so that we can backtrack
/// arbitrarily.
template <class RegexType, class ForwardIterator>
class Parser {
  friend RegexType;

  // The character type that we are parsing.
  using CharT = typename std::iterator_traits<ForwardIterator>::value_type;

  // The type of a node in our regex.
  using Node = typename RegexType::Node;

#if !defined(NDEBUG)
#define REGEX_TRACE_ENABLED 0
#endif

#if REGEX_TRACE_ENABLED
#define REGEX_TRACE(...) auto tracer = p->trace(__FUNCTION__, ##__VA_ARGS__)
#else
#define REGEX_TRACE(...)
#endif

  // The regexp that we are building. This receives the results of our
  // productions. This may be a real regex or a dummy regex.
  RegexType *re_;

  // The start, current, and end iterators in the regex input string.
  const ForwardIterator start_;
  ForwardIterator current_;
  const ForwardIterator end_;

  // The error that was set, if any.
  constants::ErrorType error_ = constants::ErrorType::None;

  // See comment --DecimalEscape--.
  const uint32_t backRefLimit_;
  uint32_t maxBackRef_ = 0;

#if REGEX_TRACE_ENABLED
  /// Tracing helper, allowing for pretty-printing of the regex parse process.
  /// The Parser maintains a parse depth, which is incremented on entry to each
  /// production/terminal and decremented on exit. This is used to indent the
  /// output of the trace() function.
  /// TODO: take advantage of this to prevent stack overflow.
  int parseDepth_ = 0;
  struct RegexpTracer {
    int &parseDepth_;
    RegexpTracer(int &td) : parseDepth_(td) {
      parseDepth_++;
    }
    ~RegexpTracer() {
      parseDepth_--;
    }
  };
  template <typename... Args>
  RegexpTracer trace(const char *func, Args &&... args) {
    auto &s = llvm::errs();
    for (int i = 0; i < parseDepth_; i++) {
      s << "  ";
    }
    if (func)
      s << func;
    // Usual trick to output parameter pack.
    using expander = int[];
    (void)expander{0, (void(s << " " << std::forward<Args>(args)), 0)...};
    s << '\n';
    return RegexpTracer{parseDepth_};
  }
#endif

  /// Set the error \p err, if not already set to a different error.
  /// \return false, for convenience.
  bool setError(constants::ErrorType err) {
    if (error_ == constants::ErrorType::None) {
#if REGEX_TRACE_ENABLED
      this->trace(
          nullptr,
          "set error",
          static_cast<int>(err),
          "at offset",
          std::distance(start_, current_));
#endif
      error_ = err;
    }
    return false;
  }

  /// \return whether an error has been set.
  bool errored() const {
    return error_ != constants::ErrorType::None;
  }

  /// \return an observer pointer to the current node of the regex.
  Node *currentNode() {
    return re_->currentNode();
  }

  /// Simple type representing a list of characters.
  /// Size of 2 because we use this to collect the start and end of char class
  /// ranges, like /[a-z]/.
  using CharList = llvm::SmallVector<char16_t, 2>;

  /// Simple type representing a list of unsigned values.
  /// Size of 2 because we use this to collect the start and end of quantifier
  /// ranges, like /abc{3,5}/
  using UIntList = llvm::SmallVector<uint32_t, 2>;

  /// Type wrapping up the result of parsing an atom inside a character class,
  /// like /[\b]/
  struct ClassAtom {
    /// Individual characters in the character set.
    CharList singletons_{};

    /// If the atom includes a range of characters, that range.
    Optional<std::pair<char16_t, char16_t>> range_{};

    /// If the atom includes a character class, then that class.
    Optional<CharacterClass> charClass_{};

    /// Add ourselves to a bracket expression \p bracket.
    /// TODO: eliminate the separate storage in bracket, just use this.
    template <typename Traits>
    void addToExpression(BracketNode<Traits> *bracket) const {
      for (CharT c : singletons_)
        bracket->addChar(c);

      if (range_)
        bracket->addRange(range_->first, range_->second);

      if (charClass_)
        bracket->addClass(*charClass_);
    }

    /// Default constructor.
    ClassAtom() = default;

    /// Convenience constructor for creating an atom from a singleton.
    explicit ClassAtom(char16_t c) : singletons_({c}) {}
  };

  using ClassAtomList = std::vector<ClassAtom>;

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

  /// Return a quantifier prepopulated with our current state.
  Quantifier prepareQuantifier() {
    Quantifier q;
    q.startMarkedSubexprs_ = re_->markedCount();
    q.quantifiedNodeStart_ = currentNode();
    return q;
  }

  /// Push a quantifier \p quant to our regular expression, which quantifies the
  /// last term (atom or assertion).
  /// \return true on success, false (and sets an error) if the quantifier is
  /// invalid.
  bool pushQuantifier(const Quantifier &quant) {
    if (quant.min_ > quant.max_)
      return setError(constants::ErrorType::BraceRange);
    auto quantifiedExpression = re_->spliceOut(quant.quantifiedNodeStart_);
    re_->pushLoop(
        quant.min_,
        quant.max_,
        move(quantifiedExpression),
        quant.startMarkedSubexprs_,
        quant.greedy_);
    return true;
  }

  /// If the cursor is on a character (i.e. not at end_), and that character
  /// satisfies a predicate \p pred, then advance the cursor.
  /// \return the character if it satisfies the predicate, otherwise None.
  template <typename Pred>
  Optional<CharT> consumeCharIf(const Pred &pred) {
    if (current_ != end_ && pred(*current_)) {
      CharT c = *current_++;
#if REGEX_TRACE_ENABLED
      this->trace(nullptr, "consumed ", char(c));
#endif
      return c;
    }
    return llvm::None;
  }

  /// Implementation of abstract operation ES6 21.2.2.15.1 CharacterRange.
  /// Given \p rangeCharPair which contains two characters, appends a ClassAtom
  /// to the given \p output.
  /// \return true on success, false on error.
  bool operationCharacterRange(
      const CharList &rangeCharPair,
      ClassAtomList *output) {
    assert(
        rangeCharPair.size() == 2 &&
        "Unexpected atom size for char class range");
    // ES6 21.2.2.15.1 says "If A does not contain exactly one character or B
    // does not contain exactly one character, throw a SyntaxError exception".
    // However this cannot happen in practice, because of Annex B's
    // "ClassEscape but only if ClassEscape evaluates to a CharSet with
    // exactly one character" and no production other than ClassEscape can
    // produce a ClassAtom that matches multiple characters.
    char16_t atomCharI = rangeCharPair[0];
    char16_t atomCharJ = rangeCharPair[1];
    // ES6 21.2.2.15.1 "If i > j, throw a SyntaxError exception"
    if (atomCharI > atomCharJ)
      return setError(constants::ErrorType::CharacterRange);
    ClassAtom atom;
    atom.range_ = std::make_pair(atomCharI, atomCharJ);
    output->push_back(std::move(atom));
    return true;
  }

  /// Helper for parsing a decimal value, following the rules of
  /// DecimalIntegerLiteral. The result is returned in \p result.
  /// If the value would overflow, all uint32_t::max() is returned; still all
  /// digits are consumed. There is no way for the caller to determine if
  /// overflow occurred (but current callers do not need to know).
  /// \return true if a number was parsed, false if not.
  bool consumeDecimalIntegerLiteral(uint32_t *result) {
    auto decimalDigit = [](CharT c) { return '0' <= c && c <= '9'; };
    uint32_t max = std::numeric_limits<uint32_t>::max();
    uint32_t consumedDigitCount = 0;
    uint64_t bigResult = 0;
    while (auto digit = consumeCharIf(decimalDigit)) {
      consumedDigitCount++;
      bigResult = bigResult * 10 + (*digit - '0');
      if (bigResult > max)
        bigResult = max;
    }
    *result = static_cast<uint32_t>(bigResult);
    return consumedDigitCount > 0;
  }

  /// Helper for constructing terminal symbols.
  /// This succeeds if the current character is the template parameter C.
  /// This accepts any argument, to make it easier to place it in sequences and
  /// ordered choices.
  template <CharT C, typename... Args>
  static bool term_Literal(Parser *p, Args... args) {
#if REGEX_TRACE_ENABLED
    // Output a note about the literal we requested and what's actually there.
    std::string current = "(current: ";
    if (p->current_ == p->end_)
      current += "none";
    else
      current += char(*p->current_);
    current += ')';
    REGEX_TRACE(char(C), current);
#endif
    auto equalToC = [](CharT c) { return c == C; };
    return bool(p->consumeCharIf(equalToC));
  }

  /// Like term_Literal, but for a string.
  static bool term_String(Parser *p, const char *s) {
    REGEX_TRACE(s);
    auto cursor = p->current_;
    size_t len = strlen(s);
    for (size_t i = 0; i < len; i++) {
      if (cursor == p->end_ || *cursor != s[i]) {
        return false;
      }
      cursor++;
    }
    p->current_ = cursor;
    return true;
  }

  /// Consume a sequence of decimal digits.
  /// This does not stop after consuming a leading zero.
  /// This is only called from QuantifierPrefix. If the value overflows, it
  /// saturates to uint32_t::max, producing an effectively infinite value
  /// for the quantifier.
  static bool term_DecimalDigits(Parser *p, UIntList *output) {
    REGEX_TRACE();
    uint32_t val = 0;
    if (p->consumeDecimalIntegerLiteral(&val)) {
      output->push_back(val);
      return true;
    }
    return false;
  }

  template <typename... Args>
  using ProductionFunc = bool (*)(Parser *, Args... args);

  /// Run a single production.
  /// \return false if we had an error or the production failed, true if the
  /// production succeeded in parsing.
  /// Note: this function used to be variadic like sequence() and
  /// orderedChoice(), but clang 8.0 refused to compile it, likely due to a
  /// clang bug. For now we only ever pass one argument.
  template <typename Arg>
  bool run(ProductionFunc<Arg> func, Arg arg) {
    auto saved = current_;
    if (!errored() && func(this, arg))
      return true;
    current_ = saved;
    return false;
  }

  // Zero-arg variant of run()
  bool run(ProductionFunc<> func) {
    auto saved = current_;
    if (!errored() && func(this))
      return true;
    current_ = saved;
    return false;
  }

  /// Type wrapping up a list of Productions.
  template <typename... Args>
  using ProductionFuncList = std::initializer_list<ProductionFunc<Args...>>;

  /// Sequence operator from PEG formalism.
  /// Attempts to run each production in the list \p funcs, in order.
  /// \return true if all productions succeed in order, false if any fails or if
  /// there was an error.
  template <typename... Args>
  bool sequence(ProductionFuncList<Args...> funcs, Args... args) {
    auto saved = current_;
    for (auto func : funcs) {
      if (!errored() && func(this, args...))
        continue;
      current_ = saved;
      return false;
    }
    return true;
  }

  /// Ordered choice operator from PEG formalism.
  /// Attempts to parse each production in the list \p funcs, in order.
  /// \return true if any production succeeds, false if all fail or if there was
  /// an error.
  template <typename... Args>
  bool orderedChoice(ProductionFuncList<Args...> funcs, Args... args) {
    auto saved = current_;
    for (auto func : funcs) {
      if (!errored() && func(this, args...))
        return true;
      current_ = saved;
    }
    return false;
  }

  // Following are the productions from the RegExp grammar. Note that Annex B
  // modifies a lot of this; the full grammar is not specified in one place in
  // the ES6 spec but is split between 21.2.1 and Annex B.1.4.

  // ES6 21.2.1 Pattern
  static bool prod_Pattern(Parser *p) {
    // Early out if empty, so we don't see obnoxious output from the empty
    // prototype regexp.
    if (p->current_ == p->end_)
      return true;
    REGEX_TRACE();
    return p->run(prod_Disjunction);
  }

  // ES6 21.2.1 Disjunction
  static bool prod_Disjunction(Parser *p) {
    REGEX_TRACE();
    // Disjunction :: Alternative | Alternative
    //             :: Alternative
    // Do not backtrack across prod_Alternative.
    bool parsed = false;

    // Capture the current right-before-goal node, then parse an Alternative.
    // Anything after that node is therefore part of the first branch of our
    // Alternation.
    auto cursor = p->currentNode();
    if (p->run(prod_Alternative)) {
      parsed = true;
      if (term_Literal<'|'>(p)) {
        // Capture everything from our cursor up to the end.
        // Since we took everything after the cursor, the regex's new end_ must
        // be the cursor.
        auto firstBranch = p->re_->spliceOut(cursor);
        assert(cursor == p->currentNode() && "Unexpected cursor value");
        parsed = p->run(prod_Disjunction);
        if (parsed) {
          auto secondBranch = p->re_->spliceOut(cursor);
          p->re_->pushAlternation(move(firstBranch), move(secondBranch));
        }
      }
    }
    return parsed;
  }

  // ES6 B.1.4 Term
  static bool prod_Term(Parser *p) {
    REGEX_TRACE();
    return p->run(prod_ExtendedTerm);
  }

  // ES6 21.2.1 Alternative
  // Note alternatives may be empty, so this always succeeds.
  static bool prod_Alternative(Parser *p) {
    REGEX_TRACE();
    // Note the spec contains right-recursion here:
    // Alternative :: Alternative Term
    // Rewrite this to be
    //   Alternative :: Term Alternative
    // and implement this with a simple loop to avoid blowing the stack.
    while (p->run(prod_Term))
      ;
    return true;
  }

  // ES6 B.1.4 ExtendedTerm
  static bool prod_ExtendedTerm(Parser *p) {
    REGEX_TRACE();
    // This one requires some care to avoid backtracking across effectful
    // productions.
    // ExtendedTerm ::
    //      Assertion
    //      AtomNoBrace Quantifier
    //      Atom
    //      QuantifiableAssertion Quantifier
    //  We handle assertions first.
    Quantifier quant = p->prepareQuantifier();
    if (p->run(prod_QuantifiableAssertion)) {
      // Note that all QuantifiableAssertions are also Assertions, so if the
      // Quantifier fails we just fall into Assertion.
      if (p->run(prod_Quantifier, &quant))
        return p->pushQuantifier(quant);
      return true;
    } else if (p->run(prod_Assertion)) {
      return true;
    } else if (p->run(prod_Atom, true)) {
      // Note that all AtomNoBrace are also Atoms, so if the
      // Quantifier fails we just fall into Atom.
      if (p->run(prod_Quantifier, &quant))
        p->pushQuantifier(quant);
      return true;
    } else {
      return p->run(prod_Atom, false);
    }
  }

  // ES6 B.1.4 Atom
  // ES6 B.1.4 AtomNoBrace
  static bool prod_Atom(Parser *p, bool noBrace) {
    REGEX_TRACE(noBrace ? "NoBrace" : "");

    // See comment QUANTIFIERS-WEIRDNESS
    if (noBrace && !p->run(prod_LookaheadNoQuantifier))
      return false;

    if (p->run(prod_PatternCharacter, noBrace))
      return true;

    if (term_Literal<'.'>(p)) {
      p->re_->pushMatchAnyButNewline();
      return true;
    }

    if (p->sequence({term_Literal<'\\'>, prod_AtomEscape}))
      return true;

    if (p->run(prod_CharacterClass))
      return true;

    if (term_String(p, "(?:")) {
      // Non-capturing group
      if (!p->sequence({prod_Disjunction, term_Literal<')'>}))
        return p->setError(constants::ErrorType::UnbalancedParenthesis);
      return true;
    }

    if (term_Literal<'('>(p)) {
      // Capturing group
      p->re_->pushBeginMarkedSubexpression();
      auto tempCount = p->re_->markedCount();
      if (!p->sequence({prod_Disjunction, term_Literal<')'>}))
        return p->setError(constants::ErrorType::UnbalancedParenthesis);
      p->re_->pushEndMarkedSubexpression(tempCount);
      return true;
    }

    return false;
  }

  // --QUANTIFIERS-WEIRDNESS--
  // The rules regarding quantifiers are strange.
  // { is allowed to be an atom as long as it's not also a quantifier. ex:
  //   /{/ is a valid regex that matches "{"
  //   /{5,,/ is a valid regex that matches "{5,,"
  //   /abc{5,,}/ is a valid regex that matches "abc{5,,}"
  // But...
  //   /{5}/ is NOT a valid regex because {5} is a valid quantifier.
  //   /{5,3}/ is NOT a valid regex even though {5,3} is an invalid
  //   quantifier.
  // We handle this by attempting to parse a quantifier. If we succeed
  // report it as an error: unexpected quantifier.
  // It appears as though the in-progress version of the ES spec formalizes this
  // with the "InvalidBracedQuantifier" production.
  // The rules are completely different for regexps with the Unicode
  // modifier, natch.
  // \return true if there is not a quantifier at the current position,
  // otherwise error.
  static bool prod_LookaheadNoQuantifier(Parser *p) {
    REGEX_TRACE();
    Quantifier quant;
    if (p->run(prod_Quantifier, &quant))
      return p->setError(constants::ErrorType::InvalidRepeat);
    return true;
  }

  // ES6 B.1.4 PatternCharacter
  // ES6 B.1.4 PatternCharacterNoBrace
  static bool prod_PatternCharacter(Parser *p, bool noBrace) {
    REGEX_TRACE(noBrace ? "NoBrace" : "");
    // PatternCharacter:
    //   "SourceCharacter but not one of ^ $ \ . * + ? ( ) [ ] | "
    // PatternCharacterNoBrace:
    //   "SourceCharacter but not one of ^ $ \ . * + ? ( ) [ ] { } | "
    // However v8 allows ] as an atom. This may be so that it can successfully
    // "parse" named character classes like /[[:alnum:]]/ that have a trailing
    // ], even though such classes are not supported. We allow this too by
    // allowing ] in PatternCharacter. We also must allow } in
    // PatternCharacterNoBrace since } may be quantified (mjsunit regexp-pcre.js
    // #639).
    auto notSpecial = [=](CharT c) {
      if (noBrace && c == '{')
        return false;
      const char *specials = "^$\\.*+?()[|";
      const char *end = specials + strlen(specials);
      return std::find(specials, end, c) == end;
    };
    if (auto c = p->consumeCharIf(notSpecial)) {
      p->re_->pushChar(*c);
      return true;
    }
    return false;
  }

  // ES6 B.1.4 QuantifiableAssertion
  // ( ? = Disjunction )
  // ( ? ! Disjunction )
  static bool prod_QuantifiableAssertion(Parser *p) {
    REGEX_TRACE();
    bool negate;
    if (term_String(p, "(?=")) {
      negate = false;
    } else if (term_String(p, "(?!")) {
      negate = true;
    } else {
      // not an assertion
      return false;
    }

    // Parse a disjunction, then splice it out from our list.
    auto mexpBegin = p->re_->markedCount();
    auto exprStart = p->currentNode();
    if (!p->run(prod_Disjunction))
      return false;
    auto mexpEnd = p->re_->markedCount();
    auto expr = p->re_->spliceOut(exprStart);
    p->re_->pushLookahead(move(expr), mexpBegin, mexpEnd, negate);

    if (!term_Literal<')'>(p))
      return p->setError(constants::ErrorType::UnbalancedParenthesis);

    return true;
  }

  // ES6 B.1.4 Assertion
  static bool prod_Assertion(Parser *p) {
    REGEX_TRACE();
    if (term_Literal<'^'>(p)) {
      p->re_->pushLeftAnchor();
    } else if (term_Literal<'$'>(p)) {
      p->re_->pushRightAnchor();
    } else if (term_String(p, "\\b")) {
      p->re_->pushWordBoundary(false);
    } else if (term_String(p, "\\B")) {
      p->re_->pushWordBoundary(true);
    } else {
      return p->run(prod_QuantifiableAssertion);
    }
    return true;
  }

  // ES6 21.2.1 Quantifier
  static bool prod_Quantifier(Parser *p, Quantifier *output) {
    REGEX_TRACE();
    // Capture the current node right before parsing the quantifier, so we know
    // the end of the quantified subexpression.
    if (p->run(prod_QuantifierPrefix, output)) {
      if (term_Literal<'?'>(p)) {
        output->greedy_ = false;
      }
      return true;
    }
    return false;
  }

  // ES6 21.2.1 QuantifierPrefix
  static bool prod_QuantifierPrefix(Parser *p, Quantifier *output) {
    REGEX_TRACE();
    if (term_Literal<'*'>(p)) {
      output->min_ = 0;
      output->max_ = std::numeric_limits<uint32_t>::max();
      return true;
    } else if (term_Literal<'+'>(p)) {
      output->min_ = 1;
      output->max_ = std::numeric_limits<uint32_t>::max();
      return true;
    } else if (term_Literal<'?'>(p)) {
      output->min_ = 0;
      output->max_ = 1;
      return true;
    }
    UIntList digits;
    if (p->sequence<UIntList *>(
            {term_Literal<'{'>, //
             term_DecimalDigits, //
             term_Literal<'}'>},
            &digits)) {
      assert(digits.size() == 1 && "Unexpected digits count");
      output->min_ = digits[0];
      output->max_ = digits[0];
      return true;
    }
    digits.clear();

    if (p->sequence<UIntList *>(
            {term_Literal<'{'>, //
             term_DecimalDigits, //
             term_Literal<','>, //
             term_Literal<'}'>},
            &digits)) {
      assert(digits.size() == 1 && "Unexpected digits count");
      output->min_ = digits[0];
      output->max_ = std::numeric_limits<uint32_t>::max();
      return true;
    }
    digits.clear();

    if (p->sequence<UIntList *>(
            {term_Literal<'{'>, //
             term_DecimalDigits, //
             term_Literal<','>, //
             term_DecimalDigits, //
             term_Literal<'}'>},
            &digits)) {
      assert(digits.size() == 2 && "Unexpected digits count");
      output->min_ = digits[0];
      output->max_ = digits[1];
      return true;
    }
    return false;
  }

  // ES6 B.1.4 AtomEscape :: DecimalEscape "but only if the integer value of
  // DecimalEscape is <= NCapturingParens"
  static bool prod_Backreference(Parser *p) {
    uint32_t backref = 0;
    // Backreferences may not start with a leading 0.
    if (term_Literal<'0'>(p)) {
      return false;
    }
    if (p->consumeDecimalIntegerLiteral(&backref)) {
      assert(backref > 0 && "backref should not have been zero");
      if (backref <= p->backRefLimit_) {
        p->maxBackRef_ = std::max(p->maxBackRef_, backref);
        p->re_->pushBackRef(backref);
        return true;
      }
    }
    return false;
  }

  // ES6 B.1.4 AtomEscape
  // --DecimalEscape--
  // The ES6 spec is rather confused about DecimalEscapes.
  // Consider a regexp like /\81/. Per the spec this should either produce a
  // backreference for capture group 80 or a decimal escape for char code 80,
  // but it actually produces either a backreference, OR char code 8 followed by
  // a "0". But now consider the decimal escape \70: this produces either a
  // backreference for capture group 70, or char code 56 (octal escape).
  // So the result of parsing a decimal escape can either be a backreference, or
  // one OR MORE characters. yarr from WebKit handles this by assuming every
  // decimal escape is a backreference, but if it finds "invalid" backreferences
  // it does a second parse now that it knows the backreference limit. We do the
  // same thing. Another approach would be to try to count the capture groups
  // ahead of time.
  static bool prod_AtomEscape(Parser *p) {
    REGEX_TRACE();
    // AtomEscape ::
    //   DecimalEscape "but only if the integer value of DecimalEscape is <=
    //   NCapturingParens"
    //   CharacterClassEscape
    //   CharacterEscape
    if (p->run(prod_Backreference)) {
      return true;
    }

    Optional<CharacterClass> charClass{};
    if (p->run(term_CharacterClassEscape, &charClass)) {
      assert(
          charClass &&
          "CharacterClassEscape succeeded without producing a character class");
      p->re_->pushCharClass(*charClass);
      return true;
    }

    CharList chars;
    if (p->run(prod_CharacterEscape, &chars)) {
      for (CharT c : chars)
        p->re_->pushChar(c);
      return true;
    }

    return false;
  }

  // ES6 21.2.1 ControlLetter
  static bool term_ControlLetter(Parser *p, CharList *output) {
    REGEX_TRACE();
    auto isControlLetter = [](CharT c) {
      return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
    };
    if (auto c = p->consumeCharIf(isControlLetter)) {
      output->push_back(*c % 32);
      return true;
    }
    return false;
  }

  // ES6 11.8.3 HexDigits
  // Note regexp never has unbounded HexDigits, so we specify the count as the
  // Count template parameter.
  template <int Count>
  static bool term_hexDigits(Parser *p, CharList *output) {
    REGEX_TRACE();
    static_assert(Count == 2 || Count == 4, "Bad count");
    auto hexDigitValue = [](CharT c) -> Optional<uint32_t> {
      if ('0' <= c && c <= '9')
        return c - '0';
      if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
      if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
      return llvm::None;
    };

    uint32_t val = 0;
    for (int i = 0; i < Count; i++) {
      if (auto c = p->consumeCharIf(hexDigitValue)) {
        val = val * 16 + *hexDigitValue(*c);
      } else {
        // Insufficient hex digits, e.g. /\u123/ or /\x1Z/.
        // We would like to return an error here, but the spec and tests demand
        // that we fall back to IdentityEscape, i.e. /\x1Z/ matches "x1Z".
        return false;
      }
    }
    // ES6 21.2.1.1: "It is a Syntax Error if the MV of HexDigits > 1114111"
    if (val > 1114111) {
      return p->setError(constants::ErrorType::EscapeOverflow);
    }
    output->push_back(val);
    return true;
  }

  // ES6 21.2.1 ControlEscape
  static bool prod_ControlEscape(Parser *p, CharList *output) {
    REGEX_TRACE();
    auto escapedValue = [](CharT c) -> Optional<CharT> {
      switch (c) {
        case 'f':
          return 0xC;
        case 'n':
          return 0xA;
        case 'r':
          return 0xD;
        case 't':
          return 0x9;
        case 'v':
          return 0xB;
        default:
          return llvm::None;
      }
    };
    if (auto c = p->consumeCharIf(escapedValue)) {
      output->push_back(*escapedValue(*c));
      return true;
    }
    return false;
  }

  // E6 A.1 HexEscapeSequence
  static bool prod_HexEscapeSequence(Parser *p, CharList *output) {
    REGEX_TRACE();
    return p->sequence<CharList *>(
        {term_Literal<'x'>, term_hexDigits<2>}, output);
  }

  // ES6 21.2.1 RegExpUnicodeEscapeSequence
  static bool prod_RegExpUnicodeEscapeSequence(Parser *p, CharList *output) {
    REGEX_TRACE();
    return p->sequence<CharList *>(
        {term_Literal<'u'>, term_hexDigits<4>}, output);
  }

  // ES 15.10.1 c ControlLetter
  static bool prod_C_ControlLetter(Parser *p, CharList *output) {
    REGEX_TRACE();
    return p->sequence<CharList *>(
        {term_Literal<'c'>, term_ControlLetter}, output);
  }

  // ES6 B.1.4 IdentityEscape
  static bool term_IdentityEscape(Parser *p, CharList *output) {
    REGEX_TRACE();
    // ES6 Annex B has this production as "SourceCharacter but not c" where
    // SourceCharacter is "any Unicode code point".
    // ES6 21.2.1 has this as "SourceCharacter but not UnicodeIDContinue"
    // In practice other engines appear to ignore both of these and just allow
    // any character here.
    auto anyChar = [](CharT c) { return true; };
    if (auto c = p->consumeCharIf(anyChar)) {
      output->push_back(*c);
      return true;
    }
    return false;
  }

  // ES7 B.1.2 LegacyOctalEscapeSequence
  // Note this is required by Annex B for regexp even in strict mode.
  static bool term_LegacyOctalEscapeSequence(Parser *p, CharList *output) {
    REGEX_TRACE();
    // LegacyOctalEscapeSequence:
    //   OctalDigit [lookahead not OctalDigit]
    //   ZeroToThree OctalDigit [lookahead not OctalDigit]
    //   FourToSeven OctalDigit
    //   ZeroToThree OctalDigit OctalDigit
    // We implement this more directly.
    auto isOctalDigit = [](CharT c) { return '0' <= c && c <= '7'; };
    auto c0 = p->consumeCharIf(isOctalDigit);
    if (!c0)
      return false;
    uint32_t val = (*c0 - '0');
    if (auto c1 = p->consumeCharIf(isOctalDigit)) {
      val = val * 8 + (*c1 - '0');
      if ('0' <= *c0 && *c0 <= '3') {
        if (auto c2 = p->consumeCharIf(isOctalDigit)) {
          val = val * 8 + (*c2 - '0');
        }
      }
    }
    output->push_back(val);
    return true;
  }

  // Implementation of the CharacterEscape :: "0 [lookahead != DecimalDigit]"
  // production from upcoming spec.
  static bool term_0_LookaheadNondigit(Parser *p, CharList *output) {
    REGEX_TRACE();
    if (term_Literal<'0'>(p)) {
      // "Lookahead not decimal digit"
      auto isDecimalDigit = [](CharT c) { return '0' <= c && c <= '9'; };
      if (!p->consumeCharIf(isDecimalDigit)) {
        output->push_back('\0');
        return true;
      }
    }
    return false;
  }

  // ES6 B.1.4 CharacterEscape, with modifications from upcoming spec.
  // CharacterEscape is produced from both ClassEscape (where it is added to a
  // character class) and AtomEscape (where it creates a character node in the
  // regex).
  static bool prod_CharacterEscape(Parser *p, CharList *output) {
    REGEX_TRACE();
    auto status = p->orderedChoice(
        {
            prod_ControlEscape, //
            prod_C_ControlLetter, //
            term_0_LookaheadNondigit, //
            prod_HexEscapeSequence, //
            prod_RegExpUnicodeEscapeSequence, //
            term_LegacyOctalEscapeSequence, //
            term_IdentityEscape, //
        },
        output);
    // If we failed to parse, then this is an escape error.
    // This can only occur with a trailing \: new RegExp("\\")
    if (!status)
      return p->setError(constants::ErrorType::EscapeIncomplete);
    return true;
  }

  // ES6 A.8 ClassRanges
  // We implement this not closely following the ES 5.1 grammar, as the grammar
  // for ranges is very confusing. For example, NonemptyClassRangesNoDash may in
  // fact produce a dash. Another problem is that the grammar is recursive:
  // ClassRanges produces NonemptyClassRanges produces NonemptyClassRangesNoDash
  // produces ClassRanges, which risks a stack overflow if implemented naively.
  // Rather than follow the grammar we implement the whole thing directly, which
  // is much simpler.
  static bool prod_ClassRanges(Parser *p, ClassAtomList *output) {
    REGEX_TRACE();
    for (;;) {
      // Try parsing a range like a-b
      CharList rangeCharPair;
      if (p->sequence<CharList *>(
              {prod_ClassAtomInRange, //
               term_Literal<'-'>, //
               prod_ClassAtomInRange},
              &rangeCharPair)) {
        p->operationCharacterRange(rangeCharPair, output);
        continue;
      }

      // Not a range. Parse a single atom.
      if (!prod_ClassAtom(p, output)) {
        break;
      }
    }
    // We always succeed, even if empty.
    return true;
  }

  // ES6 B.1.4 ClassAtomInRange
  static bool prod_ClassAtomInRange(Parser *p, CharList *output) {
    REGEX_TRACE();
    if (term_Literal<'-'>(p)) {
      output->push_back('-');
      return true;
    }
    return p->run(prod_ClassAtomNoDashInRange, output);
  }

  // ES6 B.1.4 ClassAtomNoDashInRange
  static bool prod_ClassAtomNoDashInRange(Parser *p, CharList *output) {
    REGEX_TRACE();
    // Note: the spec does not match v8 behavior here. Specifically it has the
    // production "\ ClassEscape but only if ClassEscape evaluates to a CharSet
    // with exactly one character", followed by IdentityEscape. Following this
    // literally, a regexp like /[\d-z]/ should reject ClassEscape (because
    // Digits has more than one character) and then fall through to
    // IdentityEscape, being equivalent to /[d-z]/ But what actually happens is
    // that v8 parses out the class escape and then treats that as an atom, i.e.
    // /[\d-z]/ is equivalent to /[-\dz]/. Implement that here by simply
    // returning false if this is a character class escape.
    Optional<CharacterClass> multiCharClass;
    if (p->sequence<Optional<CharacterClass> *>(
            {term_Literal<'\\'>, term_CharacterClassEscape}, &multiCharClass))
      return false;

    // "SourceCharacter but not one of \ or ] or -"
    auto notSpecial = [](CharT c) { return c != '\\' && c != ']' && c != '-'; };
    if (auto c = p->consumeCharIf(notSpecial)) {
      output->push_back(*c);
      return true;
    }

    // "\ ClassEscape but only if ClassEscape evaluates to a CharSet with
    // exactly one character"
    // We rejected multi-character ClassEscapes above, so any remaining
    // ClassEscapes must be single character.
    ClassAtom escape;
    if (p->sequence<ClassAtom *>(
            {term_Literal<'\\'>, prod_ClassEscape}, &escape)) {
      assert(
          !escape.charClass_ && escape.singletons_.size() == 1 &&
          "Unexpected escape produced from prod_ClassEscape");
      output->push_back(escape.singletons_.front());
      return true;
    }

    // "\ IdentityEscape"
    // This is what happens to class escapes like \w in ranges.
    // Note that this never actually succeeds, because it will have been already
    // explored through ClassEscape -> CharacterEscape -> IdentityEscape.
    return p->sequence<CharList *>(
        {term_Literal<'\\'>, term_IdentityEscape}, output);
  }

  // ES6 B.1.4 ClassAtom
  static bool prod_ClassAtom(Parser *p, ClassAtomList *output) {
    REGEX_TRACE();
    if (term_Literal<'-'>(p)) {
      output->emplace_back('-');
      return true;
    }
    return p->run(prod_ClassAtomNoDash, output);
  }

  // ES6 B.1.4 ClassAtomNoDash
  static bool prod_ClassAtomNoDash(Parser *p, ClassAtomList *output) {
    REGEX_TRACE();
    // "SourceCharacter but not one of \ or ] or -" where source characters are
    // any Unicode code point.
    auto notSpecial = [](CharT c) { return c != '\\' && c != ']' && c != '-'; };
    if (auto c = p->consumeCharIf(notSpecial)) {
      output->push_back(ClassAtom(*c));
      return true;
    }

    // \ ClassEscape
    ClassAtom atom;
    if (p->sequence<ClassAtom *>(
            {term_Literal<'\\'>, prod_ClassEscape}, &atom)) {
      output->push_back(std::move(atom));
      return true;
    }
    return false;
  }

  // ES6 B.1.4 ClassEscape
  // Note upcoming spec removes DecimalEscape production.
  static bool prod_ClassEscape(Parser *p, ClassAtom *output) {
    // 'b'
    // CharacterEscape
    // CharacterClassEscape

    if (term_Literal<'b'>(p)) {
      // "Return the CharSet containing the single character <BS> U+0008
      // (BACKSPACE)"
      output->singletons_.push_back(CharT(0x08));
      return true;
    }

    if (p->run(term_CharacterClassEscape, &output->charClass_))
      return true;

    return p->run(prod_CharacterEscape, &output->singletons_);
  }

  // ES6 21.2.1 CharacterClassEscape
  static bool term_CharacterClassEscape(
      Parser *p,
      Optional<CharacterClass> *output) {
    REGEX_TRACE();
    if (term_Literal<'d'>(p)) {
      output->emplace(CharacterClass::Digits, false);
    } else if (term_Literal<'D'>(p)) {
      output->emplace(CharacterClass::Digits, true);
    } else if (term_Literal<'s'>(p)) {
      output->emplace(CharacterClass::Spaces, false);
    } else if (term_Literal<'S'>(p)) {
      output->emplace(CharacterClass::Spaces, true);
    } else if (term_Literal<'w'>(p)) {
      output->emplace(CharacterClass::Words, false);
    } else if (term_Literal<'W'>(p)) {
      output->emplace(CharacterClass::Words, true);
    } else {
      return false;
    }
    return true;
  }

  // ES6 B.1.4 DecimalEscape within ClassEscape
  static bool prod_DecimalEscape_inClassEscape(Parser *p, CharList *output) {
    REGEX_TRACE();
    uint32_t val = 0;
    // Note that we do not stop after consuming a leading 0, even though the
    // spec says to do so, because v8 does not either: /[\063]/ matches octal
    // 063 "3" and not "\0 or 6 or 3".
    if (p->consumeDecimalIntegerLiteral(&val)) {
      // "but only if the integer value of DecimalEscape is 0"
      if (val != 0)
        return false;
      output->push_back(static_cast<CharT>(val));
      return true;
    }
    return false;
  }

  // ES6 A.8 CharacterClass
  // This is square brackets, e.g. [a-z_].
  static bool prod_CharacterClass(Parser *p) {
    REGEX_TRACE();
    if (!term_Literal<'['>(p))
      return false;
    bool negate = bool(term_Literal<'^'>(p));
    std::vector<ClassAtom> atoms;
    if (!p->run(prod_ClassRanges, &atoms))
      return false;
    if (!term_Literal<']'>(p))
      return p->setError(constants::ErrorType::UnbalancedBracket);

    // We parsed the character class, so add it to a bracket expression.
    auto bracket = p->re_->startBracketList(negate);
    for (const ClassAtom &atom : atoms) {
      atom.addToExpression(bracket);
    }
    return true;
  }

 public:
  /// Constructor from a regular expression \p re and regexp input string range
  /// \p start to \p end. Escapes like \123 are considered to be backrefs if
  /// they do not exceed backRefLimit.
  Parser(
      RegexType *re,
      ForwardIterator start,
      ForwardIterator end,
      uint32_t backRefLimit)
      : re_(re),
        start_(start),
        current_(start),
        end_(end),
        backRefLimit_(backRefLimit) {}

  /// Parsing entry point. Parse the input, returning either an error or the
  /// point where parsing finished. \return the result of the parse,
  /// containing either an error or the location where parsing finished.
  ParseResult<ForwardIterator> performParse() {
    Parser::prod_Pattern(this);
    if (errored()) {
      return error_;
    } else if (current_ != end_) {
      // We are top level and did not consume all the input.
      // The only way this can happen is if we have an unbalanced ).
      // TODO: express that directly in the grammar, so that the grammar always
      // consumes all input or errors.
      return constants::ErrorType::UnbalancedParenthesis;
    } else {
      return current_;
    }
  }

  /// \return the maximum number of backreferences encountered
  uint32_t maxBackRef() const {
    return maxBackRef_;
  }
};

template <typename Receiver>
ParseResult<const char16_t *> parseRegex(
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
template ParseResult<const char16_t *> parseRegex(
    const char16_t *start,
    const char16_t *end,
    Regex<U16RegexTraits> *receiver,
    uint32_t backRefLimit,
    uint32_t *outMaxBackRef);

} // namespace regex
} // namespace hermes
