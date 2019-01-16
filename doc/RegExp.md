# RegExp Design

This document provides background into the design of Hermes's regular expression support, which is required by ECMA 5.1. Our implementation will be guided by Hermes principles:

- Prioritize correctness, strongly adhering to ECMA 5.1 spec.
- Prefer simplicity, deferring optimization until we have enough implemented to validate (or invalidate) our design decisions.
- Have outstanding test coverage.
- Design for a low-powered mobile execution environment.

A key design decision is whether to incorporate an external (e.g. third party) regex engine, or write our own.

## Initial Plan

The plan for the first half of 2017 is to use C++11's `std::regex`, [which has](http://en.cppreference.com/w/cpp/regex/ecmascript) `ECMAScript` as a `syntax_option_type`. This will allow us to get off the ground very quickly, unblocking further conformance testing.

We expect to hit limits:

1. Bugs in `std::regex` are likely, especially because we are using both libstdc++ and libc++.
2. `std::regex` has some "C++-only" extensions, such as extended character class syntax, which we may need to defeat.
3. Performance is unknown.

Once we understand the extent of the limits, we can decide whether to fork `std::regex` (from libstdc++, or libc++, or Boost) or go another direction (e.g. new engine, pcre2, irregexp).

## Unicode and ES6 Considerations

ES5 allows RegEx to be oblivious to non-BMP characters. For example, consider a regex intended to match cat face emoji. You might bravely try `/[😸‑🙀]/`. ES5 will interpret this as a range between the low surrogate of the first character, and the high surrogate of the second! While useless, this is nevertheless easy to implement.

ES6 provides a new RegExp flag `u` to opt into more Unicode savviness, by decoding surrogate pairs. This is the extent of the new Unicode support required by ES6 RegEx. There is no requirement to support canonical equivalence, Unicode-property-name based character classes, or other features of Unicode regexes. We do not anticipate needing to switch to a library like ICU for RegEx when adopting ES6.

## Feasibility of Authoring a New Engine

ES 5.1 contains a detailed grammar and execution specification for RegExp patterns. An unoptimized but conforming engine can be implemented mechanically.

We can estimate the complexity from [Rhino's RegExp engine](https://github.com/mozilla/rhino/blob/master/src/org/mozilla/javascript/regexp/NativeRegExp.java), which is similar to what we might build. This is 2982 lines.

## Requirements for 3rd Party Engines

Any third party engine must at a minimum adhere to the ES 5.1 spec. This implies:

- Supports UCS-2
- Support for backreferences (rules out some DFA-based engines)
- Support for the finicky details of ES regex semantics, e.g. behavior of repeated capture groups

Other requirements:

- Compatible license
- Binary size and memory usage appropriate for mobile
- Portable to macOS, iOS, Linux, and Android, with Windows compatibility a nice to have

## RegExp Engines of Other JS Implementations

- **JavaScriptCore** uses an internal library called "YARR!" (Yet Another Regex Runtime). This is a backtracking engine with JIT support.

- **V8** until January 2017 used an internal library "Irregxp". [Here is a post](https://blog.chromium.org/2009/02/irregexp-google-chromes-new-regexp.html) discussing it. This is a complicated engine that compiles a regexp to an automaton, then to bytecode, and then machine code. In January 2017, [V8 switched](https://v8project.blogspot.com/2017/01/speeding-up-v8-regular-expressions.html) to an engine that emits code using their "TurboFan" JS compilation tiers.

- **Firefox** appears to use irregexp.

- **Rhino** uses a [custom pure Java implementation](https://github.com/mozilla/rhino/blob/master/src/org/mozilla/javascript/regexp/NativeRegExp.java). It is a non-JIT backtracking engine.

## Other Engines (non-JS specific)

- **[xregexp](http://xregexp.com)** - Engine with ES6 support, but is implemented in terms of "native" (ES5) regexs, so cannot be used to bootstrap. However this might be a quick-n-dirty path to ES6 support.
- **[re2](https://github.com/google/re2)** - Popular engine but lacks backreference support.
- **[pcre2](http://www.pcre.org)** - ES5.1 [claims](https://www.ecma-international.org/ecma-262/5.1/#sec-15.10) its RegExs are modelled after Perl 5. However, there are some differences ([example](http://stackoverflow.com/questions/40679071/perl-vs-javascript-regular-expressions)) so PCRE would have to be forked. Still this is a viable option.
- **[LLVM Regex](http://llvm.org/docs/doxygen/html/classllvm_1_1Regex.html)** - Attractive because we are already using LLVM, but has POSIX instead of ES syntax, and no UCS-2 support.
- **[ICU](http://userguide.icu-project.org/strings/regexp)** implements [Unicode Regular Expressions](http://www.unicode.org/reports/tr18/), which is distinct from ES syntax. This is a heavyweight library.
- **[Folly](https://github.com/facebook/folly)**, Facebook's C++ foundations library, uses Boost regex.
