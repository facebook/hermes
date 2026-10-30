# RegExp Start Filter and Builtin-Exec Fast Paths Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> superpowers:subagent-driven-development (recommended) or
> superpowers:executing-plans to implement this plan task-by-task. Steps
> use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Skip regex start positions whose first code unit cannot begin a
match (Part A), and run the builtin `RegExp.prototype.exec` without a
JS-level call or an unread result array when `exec` is the builtin
(Part B), with no observable change except the one resource-limit case
the spec accepts (A.5).

**Architecture:**
- Part A: a new virtual, `Node::addStartSet`, computes at parse time a
  superset of the code units that can begin a match. `Regex::compile()`
  stores it in three new header fields (`startFlags`, `startChar`,
  `startASCII[16]`). `Context::match` uses it to skip positions when it
  tries more than one; `memchr` handles the single-character case on
  ASCII input.
- Part B: `directRegExpExec` is split into `regExpBuiltinExecMatch` (the
  observable part of RegExpBuiltinExec) and the array building. The
  global `@@match` loop, `@@replace` phase 1, `@@search` and
  `regExpExec`'s direct dispatch run it inside a native frame for the
  builtin exec, built exactly as a real call builds it. `@@replace`
  phase 2 is factored per match so the fast path can feed it from match
  ranges.

**Tech Stack:** C++17, lit/FileCheck, GoogleTest, ASan, MallocGC with
handle sanitization.

**Spec:**
`doc/superpowers/specs/2026-10-28-regexp-start-filter-and-exec-fast-paths-design.md`
(binding). Read it in full before Task 1, and read
`/home/tmikov/work/hermes-x86-jit/DESIGN-PRINCIPLES.md`. Where this plan
settles a question the spec leaves open, the task says "Plan ruling";
the last section lists them all.

## Global Constraints

- **Repository.** The worktree is `/home/tmikov/work/hermes-regex-jit`
  (branch `regexp-speedup`). The shell's working directory is a different
  checkout (`/home/tmikov/work/hermes`). Every path, `git` and `cmake`
  command uses `R=/home/tmikov/work/hermes-regex-jit` explicitly
  (`git -C $R ...`). Never bare `cd`; use `( cd dir; cmd; )` if it cannot
  be avoided.
- **Scratch.** `S` is the session scratchpad directory (the harness
  gives its path). Temporary files go there, never in the repo.
- **Two parts.** Tasks 1-3 are Part A, Tasks 4-7 are Part B. They must
  squash into two commits, so no Part B task touches a Part A file
  except where stated, and Part A never touches `lib/VM`.
- **Non-goals** (spec, "Non-goals"):
  - existing spec deviations stay: `test` never looks up `exec`
    (`lib/VM/JSLib/RegExp.cpp:1239-1247`); `@@search` always sets and
    restores `lastIndex` (`:1953`, `:1966`); `@@match`/`@@replace` read
    `global`/`unicode`, not `flags`; no `SpeciesConstructor` in
    `@@split`/`@@matchAll`;
  - `@@split` gets only B.4;
  - no prefixes longer than one character, no Boyer-Moore;
  - no sandbox regeneration (`verify_generated.py` already fails on the
    version, spec "Non-goals");
  - no regex JIT.
- **Names** (spec-fixed): `Node::addStartSet(StartSet &, unsigned) const`,
  `StartSetResult {Consumes, Nullable, Unknown}`,
  `kMaxStartSetDepth = 64`; header fields `startFlags`, `startChar`,
  `startASCII[16]`; flags `StartFilterPresent`, `StartFilterNonASCII`,
  `StartFilterSingleChar`; match flag `matchNoStartFilter`;
  `BYTECODE_VERSION` 101; `regExpBuiltinExecMatch`; `hasBuiltinExec`;
  `RUNTIME_HV_FIELD(regExpPrototypeExec, NativeFunction)`.
- **GC-safe coding.** Every change under `lib/VM/` or `include/hermes/VM/`
  is written after invoking the `gc-safe-coding` skill.
  - Root every value that crosses an allocation or call; never compare
    raw pointers across one.
  - Every RAII object the new code creates (`LocalsRAII`, `GCScope`,
    `GCScopeMarkerRAII`, `ScopedNativeCallFrame`,
    `ScopedNativeDepthTracker`) spans only calls that report exceptions
    through `ExecutionStatus`, exactly like the code it replaces. No new
    kind of RAII crosses a call.
- **Style.** 80 columns, 2-space indent, a doc comment on every
  declaration, only trivial one-line methods inside a class body, the
  Meta copyright header on new files (CLAUDE.md).
- **Build trees** (configured; clang, Debug, `-O1`, JIT off):
  - `$R/cmake-build-asan`: ASan, Hades; the reference tree;
  - `$R/cmake-build-hsan`: ASan, MallocGC, `HERMESVM_SANITIZE_HANDLES`;
  - `$R/cmake-build-hv32`: ASan, `HEAP_HV_PREFER32`;
  - `$R/cmake-build-release`: Release, `-g -fno-omit-frame-pointer`, for
    measurements only.

  Rebuild a tree before running its tests, and after every mutation and
  every restore:

  ```bash
  cmake --build $R/cmake-build-asan --target hermes hermesc shermes \
    HermesSupportTests
  ```
- **Tests.**
  - Regex unit tests:
    `$R/cmake-build-asan/unittests/Support/HermesSupportTests
    --gtest_filter='<pattern>'`.
  - One lit test, by its RUN lines through lit:
    `LIT_FILTER='<regex>' cmake --build $R/cmake-build-asan --target
    check-hermes`.
  - One lit test, directly (faster for mutations):
    `$R/cmake-build-asan/bin/hermes <args> <file> |
    $R/cmake-build-asan/bin/FileCheck --match-full-lines <file>`.
  - Full suite: `cmake --build $R/cmake-build-<t> --target check-hermes`.
    Task 1 Step 1 records the baseline counts on `cmake-build-asan`.
- **Prove-can-fail** (DESIGN-PRINCIPLES 6). Every new test is shown to
  fail before the feature, or, for a regression guard, under each
  mutation the task names. A mutation is a concrete code edit. Apply it,
  rebuild, run the named command, record the failure, restore the code
  (`git -C $R diff` must show only the task's change), rebuild and re-run
  green.
- **Mutation diff** (Part B). FileCheck stops at the first mismatch, but
  one mutation can change several cases. So Part B mutations compare the
  whole output: Task 4 Step 3 writes the unmutated output to
  `$S/fast-path.expected`, and a mutation is run as

  ```bash
  timeout 120 $R/cmake-build-asan/bin/hermes \
    $R/test/hermes/regexp-exec-fast-path.js > $S/fast-path.out 2>&1
  echo "exit $?"; diff $S/fast-path.expected $S/fast-path.out
  ```

  The task names the lines that must differ. Record every differing
  line.
- **Commits.**
  - One commit per task, from a message file:
    `git -C $R commit -F $S/commit-<N>.txt`. The message text is given
    in the task. Code in a message goes in ``` fences, never in indented
    blocks. Every message ends with exactly:
    ```
    Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
    Claude-Session: https://claude.ai/code/session_019iPixSEVBGc2qFmM3Mq1Js
    ```
  - Stage by explicit path only: never `git add -A`, never
    `git add dz/issues`.
  - Never push.
- **Reporting.** Each task's report lists: the commit hash, the trees
  built, every suite run with its pass/unsupported/fail counts, and each
  mutation with the failure it produced.
- **BASE.** Record `git -C $R rev-parse HEAD` before Task 1 as BASE
  (Task 1 Step 1). Task 8 measures it.

## Review Focus

Inputs the spec implies but its test list does not name, most likely to
bite first. Each has a test in the owning task.

1. **UTF-16 strings whose units are all ASCII.** A string can be stored
   as UTF-16 and still hold only ASCII units, and then it takes the
   `char16_t` path without `matchInputAllAscii`
   (`lib/VM/JSRegExp.cpp:357-374`). The filter must behave as on the
   ASCII path. Test: the differential input `u"abcAB 1\nxyz"` (Task 3).
2. **Unicode property escapes in brackets.** `\p{Lu}`, `\P{Lu}` and
   `\p{Lu}` under `/iu` are ranges added while parsing
   (`lib/Regex/RegexParser.cpp:1344-1366`), not classes. Their ASCII bits
   and non-ASCII bit must be right. Tests: header expectations and corpus
   entries, guarded by `HERMES_ENABLE_UNICODE_REGEXP_PROPERTY_ESCAPES`
   (Tasks 1, 3).
3. **Empty and full brackets.** `[]` matches nothing and `[^]` matches
   any unit. The first gives a present but empty filter that must reject
   every position; the second must not lose non-ASCII units. Tests:
   header expectations and corpus entries (Tasks 1, 3).
4. **A `unicode` property that disagrees with the flag.** `@@match` and
   `@@replace` advance past an empty match by `Get(rx, "unicode")`
   (`RegExp.cpp:1825-1830`, `:2056-2061`), not by the regexp's flag. The
   fast loops must keep using the value they read. Test: `unicodeGetter`
   in `regexp-exec-fast-path.js` (Tasks 6, 7).
5. **A replace callback that uses the same global regexp.** The
   callback runs in phase 2 and resets `lastIndex` and the statics. The
   matches stored by phase 1 must be unaffected. Test:
   `reentrantCallback` in `regexp-exec-fast-path.js` (Task 7).

---

## File Structure

- Part A, `lib/Regex` and its headers:
  - `include/hermes/Regex/RegexBytecode.h`: header fields, flag enum,
    `startSetHasASCII()`;
  - `include/hermes/Regex/RegexNode.h`: `StartSet`, `StartSetResult`,
    `addStartSet` per node, the bracket closure helper;
  - `include/hermes/Regex/RegexTraits.h`: `bracketMatchesCodePoint`,
    shared by the executor and the analysis;
  - `include/hermes/Regex/Regex.h`: computing and storing the set;
  - `include/hermes/Regex/RegexTypes.h`: `matchNoStartFilter`;
  - `lib/Regex/Executor.cpp`: the filter;
  - `lib/Regex/RegexSerialization.cpp`: the `StartSet:` dump line;
  - `include/hermes/BCGen/HBC/BytecodeVersion.h`: 100 to 101;
  - tests: `unittests/Support/RegexTest.cpp`,
    `test/hermes/regexp_bytecode.js`, `test/BCGen/HBC/store_to_env.js`.
- Part B, `lib/VM`:
  - `include/hermes/VM/Callable.h`: `NativeFunction::runNativeCallBody`;
  - `include/hermes/VM/RuntimeHermesValueFields.def`:
    `regExpPrototypeExec`;
  - `include/hermes/VM/Runtime.h`: `regExpSearchCaptures`;
  - `lib/VM/JSRegExp.cpp`: `performSearch` uses the buffer;
  - `lib/VM/JSLib/RegExp.cpp`: everything else;
  - test: `test/hermes/regexp-exec-fast-path.js` (new).

---

### Task 1: Start-set analysis and header fields

**Files:**
- Modify: `include/hermes/Regex/RegexBytecode.h:284-299`
- Modify: `include/hermes/Regex/RegexTraits.h` (after `:154`)
- Modify: `include/hermes/Regex/RegexNode.h` (includes `:21-30`; before
  `class Node` at `:47`; every node class `:47-978`; after `:1000`)
- Modify: `include/hermes/Regex/Regex.h:73-74`, `:142-158`, `:347-372`
- Modify: `lib/Regex/Executor.cpp:719-757`
- Modify: `include/hermes/BCGen/HBC/BytecodeVersion.h:21-22`
- Test: `unittests/Support/RegexTest.cpp` (append before `:824`)

**Interfaces:**
- **Produces:**
  - `enum StartFilterFlags : uint8_t {StartFilterPresent = 1,
    StartFilterNonASCII = 2, StartFilterSingleChar = 4}` and
    `bool startSetHasASCII(const RegexBytecodeHeader &, uint32_t u)` in
    `RegexBytecode.h`;
  - `RegexBytecodeHeader::startFlags`, `startChar`, `startASCII[16]`;
    `sizeof(RegexBytecodeHeader) == 24`;
  - `class StartSet` with `addASCII`, `addNonASCII`, `hasASCII`,
    `hasNonASCII`, `writeTo(RegexBytecodeHeader &)`;
  - `enum class StartSetResult`, `kMaxStartSetDepth`,
    `Node::addStartSetForList(const NodeList &, StartSet &, unsigned)`;
  - `template <class Traits> bool bracketMatchesCodePoint(...)` in
    `RegexTraits.h`.
  - Tasks 2 and 3 read the header fields and `startSetHasASCII`.

**Facts this task relies on** (read before coding):
- Node fields are private and there is no RTTI or kind enum
  (`RegexNode.h:47-148`), so the analysis is a virtual.
- The root list starts with a no-op `Node` and ends with a `GoalNode`
  (`Regex.h:349`, `:362`); coalescing runs before the constraints
  (`Regex.h:363`, `:370`).
- `icase_` is node-local (`Regex.h:49-54`); `pushChar` canonicalizes
  icase characters (`Regex.h:406-411`); `unicode` is global
  (`Regex.h:407`, `:436`, `:473`).
- The executor accepts an icase unit `c` for instruction char `k` iff
  `c == k || canonicalize(c) == k` (`Executor.cpp:892-904`,
  `:648-660`).
- Bracket ranges are closed with `makeCanonicallyEquivalent` at emit time
  (`RegexNode.h:828-831`); the executor tests the closed ranges raw and
  canonicalizes the input for classes only under `/iu`
  (`Executor.cpp:738-755`).
- Lookbehind bodies are reversed (`Regex.h:478-491`).
- `parse()` may call `parseWithBackRefLimit` twice (`Regex.h:317-335`).
- Instruction offsets exclude the header (`RegexBytecode.h:371-377`,
  `Executor.cpp:1073`), so growing the header moves nothing.

- [ ] **Step 1: Record BASE and the baseline suite**

```bash
git -C $R rev-parse HEAD        # record as BASE
cmake --build $R/cmake-build-asan --target check-hermes
```

Record BASE and the counts (expected passes, expected failures,
unsupported, failures). There must be 0 failures; if not, STOP and
report.

- [ ] **Step 2: Write the header-contents test**

Append to `unittests/Support/RegexTest.cpp`, just before
`} // end anonymous namespace` (`:824`). Add `#include <cstdio>` and
`#include <cstring>` after `#include <string>` (`:12`).

```cpp
/// \return the start filter in the header of \p bytecode as a string:
/// "none" if absent; otherwise the ASCII members as hex ranges in
/// increasing order, then "nonascii" if StartFilterNonASCII is set, then
/// "single" if StartFilterSingleChar is set; "empty" for a present filter
/// with no members. Checks that StartFilterSingleChar and startChar agree
/// with the bitmap.
std::string describeStartFilter(const std::vector<uint8_t> &bytecode) {
  RegexBytecodeHeader header;
  EXPECT_GE(bytecode.size(), sizeof(header));
  std::memcpy(&header, bytecode.data(), sizeof(header));
  if (!(header.startFlags & StartFilterPresent)) {
    EXPECT_EQ(0u, header.startFlags);
    return "none";
  }
  std::string out;
  unsigned count = 0;
  for (uint32_t u = 0; u < 128;) {
    if (!startSetHasASCII(header, u)) {
      ++u;
      continue;
    }
    uint32_t last = u;
    while (last + 1 < 128 && startSetHasASCII(header, last + 1))
      ++last;
    char buf[16];
    if (last == u)
      snprintf(buf, sizeof(buf), "0x%02x", u);
    else
      snprintf(buf, sizeof(buf), "0x%02x-0x%02x", u, last);
    if (!out.empty())
      out += ' ';
    out += buf;
    count += last - u + 1;
    u = last + 1;
  }
  if (header.startFlags & StartFilterNonASCII) {
    if (!out.empty())
      out += ' ';
    out += "nonascii";
  }
  const bool single = header.startFlags & StartFilterSingleChar;
  EXPECT_EQ(count == 1, single) << out;
  if (single) {
    EXPECT_TRUE(startSetHasASCII(header, header.startChar)) << out;
    out += " single";
  }
  return out.empty() ? "empty" : out;
}

/// \return describeStartFilter() of the regex \p pattern with \p flags.
std::string startFilterOf(
    const std::u16string &pattern,
    const char16_t *flags = u"") {
  cregex re(pattern.c_str(), flags);
  EXPECT_TRUE(re.valid());
  if (!re.valid())
    return "invalid";
  return describeStartFilter(re.compile());
}

/// \return \p depth nested capture groups around the pattern "a".
std::u16string nestedGroups(unsigned depth) {
  return std::u16string(depth, u'(') + u"a" + std::u16string(depth, u')');
}

TEST(Regex, StartFilterHeader) {
  // Literals. /abc/ is coalesced into one MatchCharNode.
  EXPECT_EQ("0x61 single", startFilterOf(u"abc"));
  EXPECT_EQ("0x61 single", startFilterOf(u"(){999999999}a"));

  // Case folding: both cases of an ASCII letter; under /iu also every
  // non-ASCII unit, for U+017F -> 's' and U+212A -> 'k'.
  EXPECT_EQ("0x41 0x61", startFilterOf(u"a", u"i"));
  EXPECT_EQ("0x41 0x61 nonascii", startFilterOf(u"a", u"iu"));
  EXPECT_EQ("0x53 0x73 nonascii", startFilterOf(u"s", u"iu"));
  EXPECT_EQ("0x4b 0x6b nonascii", startFilterOf(u"k", u"iu"));
  EXPECT_EQ("0x53 0x73", startFilterOf(u"s", u"i"));
  EXPECT_EQ("nonascii", startFilterOf(u"\\u017f", u"i"));
  EXPECT_EQ("0x53 0x73 nonascii", startFilterOf(u"\\u017f", u"iu"));

  // Alternation, including an empty alternative.
  EXPECT_EQ("0x61 0x63", startFilterOf(u"ab|cd"));
  EXPECT_EQ("none", startFilterOf(u"a|"));

  // Loops.
  EXPECT_EQ("0x78-0x79", startFilterOf(u"x*y"));
  EXPECT_EQ("0x78 single", startFilterOf(u"x+y"));

  // Zero-width nodes are transparent; lookarounds are not descended into.
  EXPECT_EQ("0x61 single", startFilterOf(u"^a"));
  EXPECT_EQ("0x61 single", startFilterOf(u"\\ba"));
  EXPECT_EQ("0x61 single", startFilterOf(u"(?=b)a"));
  EXPECT_EQ("0x61 single", startFilterOf(u"(?!b)a"));
  EXPECT_EQ("0x61 single", startFilterOf(u"(?<=b)a"));
  EXPECT_EQ("none", startFilterOf(u"\\b"));

  // A back-reference first gives up; after a consuming node it is never
  // reached.
  EXPECT_EQ("none", startFilterOf(u"\\1(a)"));
  EXPECT_EQ("none", startFilterOf(u"(?=(a))\\1"));
  EXPECT_EQ("0x61 single", startFilterOf(u"(a)\\1"));

  // Any character.
  EXPECT_EQ("0x00-0x09 0x0b-0x0c 0x0e-0x7f nonascii", startFilterOf(u"."));
  EXPECT_EQ("0x00-0x7f nonascii", startFilterOf(u".", u"s"));

  // Brackets and classes: exact on ASCII, a superset on non-ASCII.
  EXPECT_EQ("0x61-0x63", startFilterOf(u"[a-c]"));
  EXPECT_EQ("0x00-0x60 0x62-0x7f nonascii", startFilterOf(u"[^a]"));
  EXPECT_EQ("0x30-0x39", startFilterOf(u"\\d"));
  EXPECT_EQ("0x30-0x39", startFilterOf(u"[\\d]"));
  EXPECT_EQ("0x00-0x2f 0x3a-0x7f nonascii", startFilterOf(u"\\D"));
  EXPECT_EQ(
      "0x30-0x39 0x41-0x5a 0x5f 0x61-0x7a nonascii", startFilterOf(u"\\w"));
  EXPECT_EQ("0x09-0x0d 0x20 nonascii", startFilterOf(u"\\s"));
  EXPECT_EQ(
      "0x00-0x2f 0x3a-0x40 0x5b-0x5e 0x60 0x7b-0x7f nonascii",
      startFilterOf(u"\\W", u"iu"));
  EXPECT_EQ("0x61 nonascii single", startFilterOf(u"[a\\u00e9]"));
  EXPECT_EQ("0x41-0x5a 0x61-0x7a", startFilterOf(u"[a-z]", u"i"));
  EXPECT_EQ("0x41-0x5a 0x61-0x7a nonascii", startFilterOf(u"[a-z]", u"iu"));
  // Review Focus 3.
  EXPECT_EQ("empty", startFilterOf(u"[]"));
  EXPECT_EQ("0x00-0x7f nonascii", startFilterOf(u"[^]"));
#ifdef HERMES_ENABLE_UNICODE_REGEXP_PROPERTY_ESCAPES
  // Review Focus 2.
  EXPECT_EQ("0x41-0x5a nonascii", startFilterOf(u"\\p{Lu}", u"u"));
  EXPECT_EQ("0x00-0x40 0x5b-0x7f nonascii", startFilterOf(u"\\P{Lu}", u"u"));
  EXPECT_EQ("0x41-0x5a 0x61-0x7a nonascii", startFilterOf(u"\\p{Lu}", u"iu"));
#endif

  // Surrogates under /u: an astral literal starts with its high surrogate;
  // a lone surrogate is itself non-ASCII.
  EXPECT_EQ("nonascii", startFilterOf(u"\\u{1F600}", u"u"));
  EXPECT_EQ("nonascii", startFilterOf(u"\\uD83D", u"u"));

  // Modifier groups make icase node-local.
  EXPECT_EQ("0x41 0x61", startFilterOf(u"(?i:a)b"));
  EXPECT_EQ("0x61 single", startFilterOf(u"a(?i:b)"));

  // The depth cap: the innermost list of N nested groups is at depth N.
  EXPECT_EQ("0x61 single", startFilterOf(nestedGroups(63)));
  EXPECT_EQ("none", startFilterOf(nestedGroups(64)));
  EXPECT_EQ("none", startFilterOf(nestedGroups(100)));
}
```

Derivations, for the reviewer:
- `/a/i`: `pushChar` stores `canonicalize('a', false) == 'A'`; the ASCII
  units with `canonicalize(u, false) == 'A'` are 0x41 and 0x61.
- `/\u017f/i`: legacy canonicalization never maps non-ASCII to ASCII, so
  the char stays 0x17f; no ASCII unit canonicalizes to it.
- `/\u017f/iu`: `canonicalize(0x17f, true) == 's'`.
- `\s`: `isWhiteSpaceChar` and `isLineTerminatorChar` give 0x09-0x0d and
  0x20 in ASCII (`RegexTraits.h:52-62`).
- `[a-z]/iu`: the closure adds A-Z, U+017F and U+212A (a range of 0x80 or
  more sets `nonascii`).
- `\W/iu`: the negative class matches the ASCII non-word units; the
  class sets `nonascii`.

- [ ] **Step 3: See it fail**

```bash
cmake --build $R/cmake-build-asan --target HermesSupportTests
```

Expected: compile errors, `StartFilterPresent`, `startSetHasASCII` and
`startFlags` undeclared.

- [ ] **Step 4: Header fields** (`include/hermes/Regex/RegexBytecode.h`)

Replace `RegexBytecodeHeader` (`:284-297`) with:

```cpp
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

  /// The start filter: a set of StartFilterFlags.
  uint8_t startFlags;

  /// The only ASCII member of the start set when StartFilterSingleChar is
  /// set, 0 otherwise.
  uint8_t startChar;

  /// The ASCII members of the start set: bit u % 8 of startASCII[u / 8] is
  /// set if a match can begin with code unit u.
  uint8_t startASCII[16];
};
```

After `LLVM_PACKED_END;` (`:299`) add:

```cpp
static_assert(
    sizeof(RegexBytecodeHeader) == 24,
    "RegexBytecodeHeader must stay packed");

/// Flags of RegexBytecodeHeader::startFlags.
enum StartFilterFlags : uint8_t {
  /// The header holds a start filter: every match begins with a code unit
  /// of the start set. Such a regex cannot match the empty string.
  StartFilterPresent = 1 << 0,

  /// A match may begin with any code unit >= 0x80.
  StartFilterNonASCII = 1 << 1,

  /// The start set has exactly one ASCII member, startChar.
  StartFilterSingleChar = 1 << 2,
};

/// \return whether the ASCII code unit \p u is in the start set of
/// \p header.
inline bool startSetHasASCII(const RegexBytecodeHeader &header, uint32_t u) {
  assert(u < 128 && "not an ASCII code unit");
  return (header.startASCII[u >> 3] >> (u & 7)) & 1;
}
```

Add `#include <cassert>` after `#include <cstdint>` (`:15`).

- [ ] **Step 5: The shared bracket predicate**
  (`include/hermes/Regex/RegexTraits.h`, after `ASCIIRegexTraits`,
  `:154`)

```cpp
/// \return whether the code point \p ch matches a bracket whose character
/// classes are \p positiveClasses and \p negativeClasses (sets of
/// CharacterClass::Type bits), whose ranges are \p ranges (closed under
/// canonical equivalence when \p icase), and which is negated if \p negate.
/// \p unicode is the regex's unicode flag. The executor and the start-set
/// analysis both call this, so that they agree on every code unit.
template <class Traits>
bool bracketMatchesCodePoint(
    const Traits &traits,
    uint8_t positiveClasses,
    uint8_t negativeClasses,
    bool negate,
    llvh::ArrayRef<BracketRange32> ranges,
    typename Traits::CodePoint ch,
    bool icase,
    bool unicode) {
  // When both ignoreCase and unicode are set (/iu), canonicalize the
  // character before testing, because per ES spec 22.2.2.9.2
  // (WordCharacters), \w with /iu must match any character whose canonical
  // form is a word character (e.g. U+212A KELVIN SIGN canonicalizes to 'k').
  if (positiveClasses || negativeClasses) {
    auto testCh = icase && unicode ? Traits::canonicalize(ch, true) : ch;
    for (auto charClass :
         {CharacterClass::Digits,
          CharacterClass::Spaces,
          CharacterClass::Words}) {
      if ((positiveClasses & charClass) &&
          traits.characterHasType(testCh, charClass))
        return !negate;
      if ((negativeClasses & charClass) &&
          !traits.characterHasType(testCh, charClass))
        return !negate;
    }
  }
  return traits.rangesContain(ranges, ch) != negate;
}
```

In `lib/Regex/Executor.cpp`, replace the body of `bracketMatchesChar`
(`:729-756`) with:

```cpp
  return bracketMatchesCodePoint(
      ctx.traits_,
      insn->positiveCharClasses,
      insn->negativeCharClasses,
      insn->negate,
      llvh::makeArrayRef(ranges, insn->rangeCount),
      ch,
      icase,
      ctx.syntaxFlags_.unicode);
```

Keep its doc comment (`:719-721`). `true ^ insn->negate` was `!negate`
and `contained ^ insn->negate` was `contained != negate` for the 1-bit
`negate`, so the executor is unchanged.

- [ ] **Step 6: `StartSet`, `StartSetResult` and the list walk**
  (`include/hermes/Regex/RegexNode.h`)

Add `#include "hermes/Regex/RegexTraits.h"` after
`#include "hermes/Regex/RegexBytecode.h"` (`:24`), and `#include <cstring>`
after `#include <string>` (`:29`).

Before `/// Base class representing some part...` (`:42`) add:

```cpp
/// The result of Node::addStartSet().
enum class StartSetResult : uint8_t {
  /// Every match of the node consumes input and begins with a code unit of
  /// the set.
  Consumes,
  /// The node can match without consuming input, so what follows it also
  /// contributes to the start set.
  Nullable,
  /// The analysis cannot bound the first code unit: no filter.
  Unknown,
};

/// The nesting depth of node lists at which the start-set analysis gives
/// up. The node tree has no depth limit (test/hermes/regexp.js:94 nests
/// 50000 groups), so the recursive analysis is bounded instead.
constexpr unsigned kMaxStartSetDepth = 64;

/// A superset of the code units that can begin a match: exact for ASCII
/// code units, and one bit standing for every code unit >= 0x80. Treating
/// every non-ASCII unit alike keeps a /u search from stopping on a low
/// surrogate whose high surrogate it skipped.
class StartSet {
 public:
  /// Add the ASCII code unit \p u.
  inline void addASCII(uint32_t u);

  /// Add every code unit >= 0x80.
  void addNonASCII() {
    nonASCII_ = true;
  }

  /// \return whether the ASCII code unit \p u is in the set.
  inline bool hasASCII(uint32_t u) const;

  /// \return whether the code units >= 0x80 are in the set.
  bool hasNonASCII() const {
    return nonASCII_;
  }

  /// Store this set as the start filter of \p header.
  inline void writeTo(RegexBytecodeHeader &header) const;

 private:
  /// Bit u % 8 of ascii_[u / 8] is set if the ASCII unit u is in the set.
  uint8_t ascii_[16] = {};

  /// Whether every code unit >= 0x80 is in the set.
  bool nonASCII_ = false;
};

void StartSet::addASCII(uint32_t u) {
  assert(u < 128 && "not an ASCII code unit");
  ascii_[u >> 3] |= 1u << (u & 7);
}

bool StartSet::hasASCII(uint32_t u) const {
  assert(u < 128 && "not an ASCII code unit");
  return (ascii_[u >> 3] >> (u & 7)) & 1;
}

void StartSet::writeTo(RegexBytecodeHeader &header) const {
  header.startFlags = StartFilterPresent;
  if (nonASCII_)
    header.startFlags |= StartFilterNonASCII;
  unsigned count = 0;
  uint32_t only = 0;
  for (uint32_t u = 0; u < 128; ++u) {
    if (hasASCII(u)) {
      ++count;
      only = u;
    }
  }
  if (count == 1) {
    header.startFlags |= StartFilterSingleChar;
    header.startChar = static_cast<uint8_t>(only);
  }
  static_assert(
      sizeof(header.startASCII) == sizeof(ascii_), "bitmap size mismatch");
  std::memcpy(header.startASCII, ascii_, sizeof(ascii_));
}
```

In `class Node`, after `optimizeNodeList`'s declaration (`:89-92`), add:

```cpp
  /// Add to \p set the start sets of the nodes of \p nodes, in order, up to
  /// and including the first node that consumes input. \p depth is the
  /// nesting depth of \p nodes: 0 for the root list.
  /// \return Consumes if a node of the list consumes, Nullable if none
  /// does, Unknown if the analysis gave up.
  inline static StartSetResult
  addStartSetForList(const NodeList &nodes, StartSet &set, unsigned depth);
```

In the `protected:` section of `Node`, after `matchConstraints()`
(`:117-119`), add:

```cpp
  /// Add to \p set every code unit that can begin a match of this node.
  /// \p depth is the nesting depth of the list holding this node; a node
  /// that descends into a child list passes depth + 1.
  /// \return Nullable if the node can match without consuming input, so
  /// the caller must also add the start set of what follows; Consumes if
  /// every match consumes; Unknown to give up (no filter).
  /// The base Node is the leading no-op node: it is nullable and adds
  /// nothing. Every subclass overrides this.
  virtual StartSetResult addStartSet(StartSet &set, unsigned depth) const {
    return StartSetResult::Nullable;
  }
```

After `Node::reverseNodeList` (`:980-1000`) add:

```cpp
StartSetResult Node::addStartSetForList(
    const NodeList &nodes,
    StartSet &set,
    unsigned depth) {
  if (depth >= kMaxStartSetDepth)
    return StartSetResult::Unknown;
  for (const Node *node : nodes) {
    StartSetResult result = node->addStartSet(set, depth);
    if (result != StartSetResult::Nullable)
      return result;
  }
  return StartSetResult::Nullable;
}
```

- [ ] **Step 7: The per-node rules** (`RegexNode.h`; one override per
  class, added to the class's `protected:` section, creating one where the
  class has none)

`GoalNode` (`:151-162`):

```cpp
  /// Reaching the goal means the list can match the empty string.
  StartSetResult addStartSet(StartSet &, unsigned) const override {
    return StartSetResult::Nullable;
  }
```

`LoopNode` (`:164-306`):

```cpp
  /// The loopee's set is added even for {0}: a superset is safe. The loop is
  /// nullable if it may run zero times or its loopee is nullable.
  StartSetResult addStartSet(StartSet &set, unsigned depth) const override {
    StartSetResult result = addStartSetForList(loopee_, set, depth + 1);
    if (result == StartSetResult::Unknown)
      return result;
    return min_ == 0 ? StartSetResult::Nullable : result;
  }
```

`AlternationNode` (`:309-422`):

```cpp
  /// The union over the alternatives. An alternative may be empty (/a|/).
  StartSetResult addStartSet(StartSet &set, unsigned depth) const override {
    StartSetResult result = StartSetResult::Consumes;
    for (const NodeList &alternative : alternatives_) {
      StartSetResult altResult =
          addStartSetForList(alternative, set, depth + 1);
      if (altResult == StartSetResult::Unknown)
        return altResult;
      if (altResult == StartSetResult::Nullable)
        result = StartSetResult::Nullable;
    }
    return result;
  }
```

`MarkedSubexpressionNode` (`:425-469`):

```cpp
  /// A capture group matches what its contents match.
  StartSetResult addStartSet(StartSet &set, unsigned depth) const override {
    return addStartSetForList(contents_, set, depth + 1);
  }
```

`BackRefNode` (`:472-495`):

```cpp
  /// A back-reference can repeat text captured inside a lookahead, e.g.
  /// /(?=(a))\1/, so its first unit is unknown.
  StartSetResult addStartSet(StartSet &, unsigned) const override {
    return StartSetResult::Unknown;
  }
```

`WordBoundaryNode` (`:498-518`), `LeftAnchorNode` (`:521-547`),
`RightAnchorNode` (`:550-566`), each:

```cpp
  /// A zero-width assertion consumes nothing.
  StartSetResult addStartSet(StartSet &, unsigned) const override {
    return StartSetResult::Nullable;
  }
```

`LookaroundNode` (`:898-978`):

```cpp
  /// Every lookaround is zero-width at the match start. Its body is not
  /// descended into: a lookbehind body is reversed and reads the text to
  /// the left.
  StartSetResult addStartSet(StartSet &, unsigned) const override {
    return StartSetResult::Nullable;
  }
```

`MatchAnyNode` (`:570-609`):

```cpp
  /// Without dotAll, '.' rejects the line terminators: 0x0A and 0x0D in
  /// ASCII, U+2028 and U+2029 beyond (Executor.cpp isLineTerminator).
  StartSetResult addStartSet(StartSet &set, unsigned) const override {
    for (uint32_t u = 0; u < 128; ++u) {
      if (dotAll_ || (u != u'\n' && u != u'\r'))
        set.addASCII(u);
    }
    set.addNonASCII();
    return StartSetResult::Consumes;
  }
```

`MatchCharNode` (`:613-756`):

```cpp
  /// The first unit of a match is the first unit of chars_[0], which is
  /// already canonical when icase_ (Regex::pushChar).
  StartSetResult addStartSet(StartSet &set, unsigned) const override {
    assert(!chars_.empty() && "MatchCharNode without characters");
    const CodePoint c = chars_.front();
    if (!isASCII(c)) {
      // An astral character starts with its high surrogate, a lone
      // surrogate is non-ASCII, and no ASCII unit canonicalizes to a
      // non-ASCII character.
      set.addNonASCII();
    } else if (!icase_) {
      set.addASCII(c);
    } else {
      // Every ASCII unit the executor accepts for c: u == c or
      // canonicalize(u) == c (MatchCharICase8, MatchNCharICase8).
      for (uint32_t u = 0; u < 128; ++u) {
        if (u == c || UTF16RegexTraits::canonicalize(u, unicode_) == c)
          set.addASCII(u);
      }
      // Under /iu, U+017F and U+212A canonicalize to 's' and 'k'. Setting
      // the bit for every /iu character is a deliberate superset.
      if (unicode_)
        set.addNonASCII();
    }
    return StartSetResult::Consumes;
  }
```

`BracketNode<Traits>` (`:759-895`). Add, before `populateInstruction`
(`:814`):

```cpp
  /// \return the code point set this bracket emits: the parsed set, closed
  /// under canonical equivalence when icase_. emitStep() and addStartSet()
  /// both use it, so they cannot diverge.
  CodePointSet emittedCodePointSet() const {
    return icase_ ? makeCanonicallyEquivalent(codePointSet_, unicode_)
                  : codePointSet_;
  }
```

In `populateInstruction`, replace `:828-831` with:

```cpp
    CodePointSet cps = emittedCodePointSet();
```

Add to the class (a `protected:` section after `matchesExactlyOneCharacter`,
`:875-878`):

```cpp
  /// The ASCII bits are the executor's own answers: bracketMatchesCodePoint
  /// on the emitted ranges. Non-ASCII units are a superset: they are in the
  /// set unless the bracket is not negated, has no class other than a
  /// positive \d, and has no emitted range reaching 0x80.
  StartSetResult addStartSet(StartSet &set, unsigned) const override {
    const CodePointSet cps = emittedCodePointSet();
    llvh::SmallVector<BracketRange32, 8> ranges;
    for (const CodePointRange &range : cps.ranges())
      ranges.push_back(
          BracketRange32{range.first, range.first + range.length - 1});
    uint8_t positive = 0;
    uint8_t negative = 0;
    for (CharacterClass cc : classes_) {
      if (cc.inverted_)
        negative |= cc.type_;
      else
        positive |= cc.type_;
    }
    for (uint32_t u = 0; u < 128; ++u) {
      if (bracketMatchesCodePoint(
              traits_,
              positive,
              negative,
              negate_,
              ranges,
              u,
              icase_,
              unicode_))
        set.addASCII(u);
    }
    if (negate_ || negative != 0 ||
        (positive & ~uint8_t(CharacterClass::Digits)) != 0 ||
        (!cps.ranges().empty() && cps.end() > 0x80))
      set.addNonASCII();
    return StartSetResult::Consumes;
  }
```

- [ ] **Step 8: Compute, store and write the set** (`Regex.h`)

After `matchConstraints_` (`:73-74`) add:

```cpp
  /// The code units that can begin a match, or None if there is no start
  /// filter: the regex can match the empty string, or the analysis gave up.
  llvh::Optional<StartSet> startSet_;
```

In `parseWithBackRefLimit`, after `nodes_.clear();` (`:348`) add
`startSet_ = llvh::None;`, and after
`matchConstraints_ = Node::matchConstraintsForList(nodes_);` (`:370`) add:

```cpp
  // Compute the start-position filter.
  if (result == constants::ErrorType::None) {
    StartSet set;
    if (Node::addStartSetForList(nodes_, set, 0) == StartSetResult::Consumes)
      startSet_ = set;
  }
```

In `compile()`, replace the header construction (`:150-154`) with:

```cpp
    RegexBytecodeHeader header{};
    header.markedCount = markedCount_;
    header.loopCount = static_cast<uint16_t>(loopCount_);
    header.syntaxFlags = globalFlags_.toByte();
    header.constraints = matchConstraints_;
    if (startSet_)
      startSet_->writeTo(header);
```

- [ ] **Step 9: Bump the bytecode version**
  (`include/hermes/BCGen/HBC/BytecodeVersion.h:21-22`)

```cpp
// Bytecode version generated by this version of the compiler.
// Updated: Oct 28, 2026
const static uint32_t BYTECODE_VERSION = 101;
```

- [ ] **Step 10: Build and run**

```bash
cmake --build $R/cmake-build-asan --target hermes hermesc shermes \
  HermesSupportTests
$R/cmake-build-asan/unittests/Support/HermesSupportTests \
  --gtest_filter='Regex.*'
```

Expected: every `Regex.*` test passes, including `StartFilterHeader`.
If an expectation differs, re-derive it from the rules above before
changing the code or the expectation; report any expectation changed.

- [ ] **Step 11: Prove the header test can fail** (rebuild
  `HermesSupportTests` after each edit and each restore; run the Step 10
  command)
  - M1, drop the other case: in `MatchCharNode::addStartSet` change
    `if (u == c || UTF16RegexTraits::canonicalize(u, unicode_) == c)` to
    `if (u == c)`. Fails: `startFilterOf(u"a", u"i")` gives
    `"0x41 single"`.
  - M2, no `nonascii` for `/iu`: delete
    `if (unicode_) set.addNonASCII();` in `MatchCharNode`. Fails:
    `u"s", u"iu"` gives `"0x53 0x73"`.
  - M3, lookaround body as consuming: make `LookaroundNode::addStartSet`
    `{ addStartSetForList(exp_, set, depth + 1); return
    StartSetResult::Consumes; }` (name the parameters). Fails:
    `u"(?=b)a"` gives `"0x62 single"`.
  - M4, `.` without non-ASCII: in `MatchAnyNode`, change
    `set.addNonASCII();` to `if (dotAll_) set.addNonASCII();`. Fails:
    `u"."`.
  - M5, depth cap: change `kMaxStartSetDepth` to 65. Fails:
    `nestedGroups(64)` gives `"0x61 single"`.
  - M6, back-reference as nullable: return `StartSetResult::Nullable` in
    `BackRefNode`. Fails: `u"\\1(a)"` gives `"0x61 single"`.
  - M7, negation ignored on the non-ASCII side: drop `negate_ ||` from
    the `addNonASCII` condition in `BracketNode`. Fails: `u"[^a]"`.

- [ ] **Step 12: Full suite**

```bash
cmake --build $R/cmake-build-asan --target check-hermes
```

Expected: 0 failures, and the Step 1 counts with one more expected pass:
check-hermes runs `unittests/` too and counts each GoogleTest case, and
this task adds `Regex.StartFilterHeader`. The header grows silently: no
offset moves and the dump is unchanged until Task 2. The 50000-deep
nesting in `test/hermes/regexp.js:94` must pass (the analysis stops at
depth 64).

- [ ] **Step 13: Commit**

```bash
git -C $R add include/hermes/Regex/RegexBytecode.h \
  include/hermes/Regex/RegexTraits.h include/hermes/Regex/RegexNode.h \
  include/hermes/Regex/Regex.h lib/Regex/Executor.cpp \
  include/hermes/BCGen/HBC/BytecodeVersion.h unittests/Support/RegexTest.cpp
git -C $R commit -F $S/commit-1.txt
```

`$S/commit-1.txt`:

````
Regex: compute a start set per regex and store it in the header

Each regex now records at compile time which code units can begin a
match: an exact bitmap of the ASCII units and one bit for all units of
0x80 and above. A regex that can match the empty string, or whose first
unit cannot be bounded, has no set.

A new virtual, Node::addStartSet, walks the node tree with a depth cap
of 64, since the tree has no nesting limit:

```
virtual StartSetResult addStartSet(StartSet &set, unsigned depth) const;
```

Zero-width nodes are transparent, lookarounds are not descended into,
and a back-reference gives up. Case-insensitive characters add every
ASCII unit the executor accepts for them, and under /iu also the
non-ASCII bit, for U+017F and U+212A. A bracket's ASCII bits come from
the executor's own predicate, now shared as bracketMatchesCodePoint, on
the ranges it emits, now computed by one helper for emission and
analysis.

RegexBytecodeHeader grows from 6 to 24 bytes: startFlags, startChar and
startASCII[16]. Instruction offsets exclude the header, so no jump
target or dumped offset moves. The bytes format changes, so
BYTECODE_VERSION goes from 100 to 101. SH-generated C must be rebuilt.
Nothing reads the set yet.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_019iPixSEVBGc2qFmM3Mq1Js
````

---

### Task 2: Print the start set in bytecode dumps

**Files:**
- Modify: `lib/Regex/RegexSerialization.cpp:343-353`
- Modify: `test/hermes/regexp_bytecode.js` (21 `Header:` lines),
  `test/BCGen/HBC/store_to_env.js:108`

**Interfaces:**
- **Consumes:** Task 1's header fields and `startSetHasASCII`.
- **Produces:** the dump line
  `  StartSet: <hex ranges>[ nonascii]` after `  Header:` when a filter
  is present (spec, "Storing it").

- [ ] **Step 1: Update the expected dumps**

The test uses `--match-full-lines` and `CHECK-NEXT`
(`test/hermes/regexp_bytecode.js:8`), so a new line must be expected
wherever a filter exists. FileCheck ignores leading whitespace here.
Run this script; it inserts after the given 1-based line numbers of the
current file, bottom-up:

```bash
python3 - $R/test/hermes/regexp_bytecode.js <<'EOF'
import sys
path = sys.argv[1]
lines = open(path).read().split('\n')
# line number of the case's Header CHECK -> StartSet value
inserts = {
    15: "0x41 0x61",                 # 0: /a\x01\u017f/i
    23: "0x61",                      # 1: /^a\u017f\x01$/
    33: "0x61-0x62",                 # 2: /^a|b/
    43: "0x61-0x7a",                 # 3: /[a-z].../
    52: "0x61",                      # 4: /a(b(c)(d))e\1\2/
    76: "0x41 0x61",                 # 6: /abc(?=^)(?!def)/i
    88: "0x61",                      # 7: /ab*c+d{3,5}/
    100: "0x61",                     # 8: /a((b+){3})*/
    116: "0x62",                     # 9: /(^b)+(c)*?/
    132: "nonascii",                 # 10: /[\u017f]/i
    145: "0x61",                     # 12: /a+/
    191: "0x41 0x61",                # 18: /(a)(?=(.))/i
    204: "0x41 0x61",                # 19: /(a)(?<!(.))/i
    218: "0x61",                     # 20: /a{255}overflow/
    225: "0x41 0x61",                # 21: /a{255}overflow/i
    232: "0x00-0x09 0x0b-0x0c 0x0e-0x7f nonascii",  # 22: /./
    238: "0x00-0x09 0x0b-0x0c 0x0e-0x7f nonascii",  # 23: /./u
    244: "0x00-0x7f nonascii",       # 24: /./s
    250: "0x00-0x7f nonascii",       # 25: /./us
    256: "0x61-0x66",                # 26: /a|b|c|d|e|f/
    277: "0x61 0x64",                # 27: /(abc|def)/
}
for n in sorted(inserts, reverse=True):
    assert 'Header:' in lines[n - 1], (n, lines[n - 1])
    lines.insert(n, '// CHECK-NEXT:    StartSet: ' + inserts[n])
open(path, 'w').write('\n'.join(lines))
EOF
```

Cases 5 (`/\b\B/`), 11 (`/a*/`), 13, 14, 15, 16 and 17 can match the
empty string and get no line. Derivations: icase letters are stored
uppercase without `/u`, so `'A'` gives 0x41 and 0x61; `/[\u017f]/i`
emits only `[0x17f]` (`:133`); `.` excludes 0x0A and 0x0D unless `s`.

In `test/BCGen/HBC/store_to_env.js`, after line 108
(`// CHECK-NEXT:  Header: marked: 0 loops: 0 flags: 1 constraints: 4`,
for `/Hermes/i`) insert:

```js
// CHECK-NEXT:  StartSet: 0x48 0x68
```

- [ ] **Step 2: See them fail**

```bash
LIT_FILTER='regexp_bytecode|store_to_env' \
  cmake --build $R/cmake-build-asan --target check-hermes
```

Expected: 2 failures, each `CHECK-NEXT: expected string not found in
input` on the first `StartSet:` line.

- [ ] **Step 3: Print the line** (`lib/Regex/RegexSerialization.cpp`,
  after the `Header:` print, `:352`)

```cpp
  if (header->startFlags & regex::StartFilterPresent) {
    // ASCII members as hex ranges in increasing order.
    OS << "  StartSet:";
    for (uint32_t u = 0; u < 128;) {
      if (!regex::startSetHasASCII(*header, u)) {
        ++u;
        continue;
      }
      uint32_t last = u;
      while (last + 1 < 128 && regex::startSetHasASCII(*header, last + 1))
        ++last;
      OS << llvh::format(" 0x%02x", u);
      if (last != u)
        OS << llvh::format("-0x%02x", last);
      u = last + 1;
    }
    if (header->startFlags & regex::StartFilterNonASCII)
      OS << " nonascii";
    OS << '\n';
  }
```

- [ ] **Step 4: Run**

```bash
cmake --build $R/cmake-build-asan --target hermes hermesc
LIT_FILTER='regexp_bytecode|store_to_env' \
  cmake --build $R/cmake-build-asan --target check-hermes
```

Expected: 2 passes.

- [ ] **Step 5: Prove it can fail.** Delete the whole statement
  `if (header->startFlags & regex::StartFilterNonASCII) OS << " nonascii";`
  (both lines; deleting only its body would leave the `if` controlling
  `OS << '\n';`). Rebuild, re-run Step 4. Expected: `regexp_bytecode.js`
  fails at case 10, the first case with `nonascii` in file order: the
  line prints `StartSet:` where `StartSet: nonascii` is expected.
  Restore, rebuild, re-run green.

- [ ] **Step 6: Full suite** on `cmake-build-asan`: 0 failures; expected
  passes still baseline + 1 (this task adds no test; it changes two
  existing lit files).

- [ ] **Step 7: Commit**

```bash
git -C $R add lib/Regex/RegexSerialization.cpp \
  test/hermes/regexp_bytecode.js test/BCGen/HBC/store_to_env.js
git -C $R commit -F $S/commit-2.txt
```

`$S/commit-2.txt`:

````
Regex: print the start set in regex bytecode dumps

dumpRegexBytecode prints a line after the header when the regex has a
start filter, with the ASCII members as hex ranges and "nonascii" when
non-ASCII units may begin a match:

```
  Header: marked: 0 loops: 0 flags: 0 constraints: 4
  StartSet: 0x61-0x63 0x6b nonascii
```

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_019iPixSEVBGc2qFmM3Mq1Js
````

---

### Task 3: Skip start positions in the executor

**Files:**
- Modify: `include/hermes/Regex/RegexTypes.h:33-48`
- Modify: `lib/Regex/Executor.cpp` (includes `:8-15`; before `match`,
  `:1063`; `match`, `:1063-1120`)
- Test: `unittests/Support/RegexTest.cpp`

**Interfaces:**
- **Consumes:** Task 1's header fields and `startSetHasASCII`.
- **Produces:** `constants::matchNoStartFilter = 1 << 4`, which disables
  the filter (tests only).

**Facts this task relies on:**
- `match` resets only the cursor and `ip_` per position
  (`Executor.cpp:1116-1120`); an exhausted position drains its backtrack
  records (`:803-871`); `backtracksRemaining_` is shared (`:404`).
- `onlyAtStart` is true for sticky, `AnchoredAtStart`
  (`:1665-1668`) and every lookaround recursion (`:1437`), which also
  runs backwards cursors (`:1089-1093`).
- The loop checks `1 + charsToRight` positions (`:1087`); a filtered
  regex cannot match empty, so the last one needs no attempt.
- `advanceStringIndex` steps over a surrogate pair under `/u`
  (`:1035-1061`).
- ASCII-path input is 7-bit (`lib/VM/JSRegExp.cpp:357-365`).
- `operator~` on `MatchFlagType` masks to 12 bits (`RegexTypes.h:50-52`),
  so `1 << 4` is safe.

- [ ] **Step 1: Write the differential and resource-limit tests**

Append to `unittests/Support/RegexTest.cpp`, before
`} // end anonymous namespace`:

```cpp
/// \return the outcome of searching \p text (\p length units) for
/// \p bytecode from \p start with \p flags: "nomatch", "overflow", or
/// "match" and the captures.
template <typename CharT>
std::string searchOutcome(
    const std::vector<uint8_t> &bytecode,
    const CharT *text,
    uint32_t length,
    uint32_t start,
    constants::MatchFlagType flags) {
  cmatch m;
  switch (searchWithBytecode(bytecode, text, start, length, &m, flags)) {
    case MatchRuntimeResult::Match:
      return "match " + flatten(m);
    case MatchRuntimeResult::NoMatch:
      return "nomatch";
    case MatchRuntimeResult::StackOverflow:
      return "overflow";
  }
  return "?";
}

/// \return \p s with each non-ASCII code unit written as \uXXXX.
std::string printable(const std::u16string &s) {
  std::string out;
  for (char16_t c : s) {
    if (c < 0x80) {
      out += static_cast<char>(c);
    } else {
      char buf[8];
      snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(c));
      out += buf;
    }
  }
  return out;
}

/// Check that the start filter of \p pattern with \p flags never changes a
/// search result: over UTF-16 and ASCII inputs, at every start index, with
/// and without matchOnlyAtStart (sticky), the filtered search and the
/// unfiltered one (matchNoStartFilter) give the same outcome, unless the
/// unfiltered one overflows (spec A.5).
void expectStartFilterTransparent(
    const std::u16string &pattern,
    const std::u16string &flags) {
  // Case folds to ASCII (U+017F, U+212A), line terminators, a surrogate
  // pair, lone surrogates, Latin-1, and ASCII units held as UTF-16
  // (Review Focus 1). \x escapes are split so no hex digit follows them.
  static const std::u16string utf16Inputs[] = {
      u"xaAbsSkK\u017f\u212a",
      u"b\u2028a\nc\rd e\tf_9",
      u"\xD83D\xDE00" u"a" u"\xD83D\xDE00",
      u"a\xD83D" u"b\xDE00" u"c",
      u"\u00e9\u00c9x1_9",
      u"abcAB 1\nxyz",
      u"",
  };
  static const std::string asciiInputs[] = {
      "xaAbsSkK",
      "b a\nc\rd\te_9",
      "abcabd012_",
      "",
  };
  cregex re(pattern.c_str(), flags.c_str());
  ASSERT_TRUE(re.valid()) << printable(pattern);
  const std::vector<uint8_t> bytecode = re.compile();
  for (constants::MatchFlagType mode :
       {constants::matchDefault, constants::matchOnlyAtStart}) {
    for (const std::u16string &in : utf16Inputs) {
      const uint32_t len = in.size();
      for (uint32_t start = 0; start <= len; ++start) {
        std::string unfiltered = searchOutcome(
            bytecode,
            in.data(),
            len,
            start,
            mode | constants::matchNoStartFilter);
        if (unfiltered == "overflow")
          continue;
        EXPECT_EQ(
            unfiltered, searchOutcome(bytecode, in.data(), len, start, mode))
            << "/" << printable(pattern) << "/" << printable(flags)
            << " on \"" << printable(in) << "\" from " << start
            << (mode ? " sticky" : "");
      }
    }
    for (const std::string &in : asciiInputs) {
      const uint32_t len = in.size();
      const constants::MatchFlagType asciiMode =
          mode | constants::matchInputAllAscii;
      for (uint32_t start = 0; start <= len; ++start) {
        std::string unfiltered = searchOutcome(
            bytecode,
            in.data(),
            len,
            start,
            asciiMode | constants::matchNoStartFilter);
        if (unfiltered == "overflow")
          continue;
        EXPECT_EQ(
            unfiltered,
            searchOutcome(bytecode, in.data(), len, start, asciiMode))
            << "/" << printable(pattern) << "/" << printable(flags)
            << " on ascii \"" << in << "\" from " << start
            << (mode ? " sticky" : "");
      }
    }
  }
}

TEST(Regex, StartFilterTransparent) {
  const std::pair<std::u16string, std::u16string> corpus[] = {
      {u"abc", u""},
      {u"a", u""},
      {u"a", u"i"},
      {u"a", u"iu"},
      {u"s", u"iu"},
      {u"k", u"iu"},
      {u"s", u"i"},
      {u"K", u"i"},
      {u"\\u017f", u"i"},
      {u"\\u017f", u"iu"},
      {u"\\u212a", u"iu"},
      {u"ab|cd", u""},
      {u"a|", u""},
      {u"x*y", u""},
      {u"x+y", u""},
      {u"^a", u""},
      {u"^b", u"m"},
      {u"a$", u"m"},
      {u"\\ba", u""},
      {u"\\bb", u"i"},
      {u"(?=b)a", u""},
      {u"(?<=a)b", u""},
      {u"(?!a)b", u""},
      {u"(?!a)\\w", u""},
      {u"(?<!a)\\w", u"u"},
      {u"\\1(a)", u""},
      {u"(?=(a))\\1", u""},
      {u"(a)\\1", u"i"},
      {u".", u""},
      {u".", u"s"},
      {u".", u"u"},
      {u".a", u"su"},
      {u"[a-c]", u""},
      {u"[^a]", u""},
      {u"[^a]", u"u"},
      {u"\\d", u""},
      {u"\\D", u""},
      {u"\\w", u""},
      {u"\\w", u"iu"},
      {u"\\W", u"iu"},
      {u"\\s", u""},
      {u"\\s+x", u""},
      {u"[\\s\\S]a", u""},
      {u"\\S", u""},
      {u"[a-z]", u"i"},
      {u"[a-z]", u"iu"},
      {u"[a\\u00e9]", u""},
      {u"[]", u""},
      {u"[^]", u""},
      {u"\\u{1F600}", u"u"},
      {u"[\\u{1F600}x]", u"u"},
      {u"\\uD83D", u"u"},
      {u"\\uDE00", u"u"},
      {u"\\uD83D", u""},
      {u"\\u2028|\\n", u""},
      {u"(?i:a)b", u""},
      {u"a(?i:b)", u""},
#ifdef HERMES_ENABLE_UNICODE_REGEXP_PROPERTY_ESCAPES
      {u"\\p{Lu}", u"u"},
      {u"\\P{Lu}", u"u"},
      {u"\\p{Lu}", u"iu"},
#endif
      {nestedGroups(63), u""},
      {nestedGroups(64), u""},
  };
  for (const auto &entry : corpus)
    expectStartFilterTransparent(entry.first, entry.second);
}

TEST(Regex, StartFilterAvoidsOverflow) {
  // Spec A.5: unfiltered, position 0 enters the loop and overflows the
  // backtrack stack before testing 'a'; filtered, no position is tried. It
  // runs once each way: an unfiltered run builds 2^24 backtrack records.
  const std::vector<uint8_t> bytecode = cregex(u"(){999999999}a").compile();
  const char16_t text[] = u"b";
  cmatch m;
  EXPECT_EQ(
      MatchRuntimeResult::StackOverflow,
      searchWithBytecode(
          bytecode, text, 0, 1, &m, constants::matchNoStartFilter));
  EXPECT_EQ(
      MatchRuntimeResult::NoMatch,
      searchWithBytecode(bytecode, text, 0, 1, &m, constants::matchDefault));
}
```

- [ ] **Step 2: See it fail**

```bash
cmake --build $R/cmake-build-asan --target HermesSupportTests
```

Expected: compile error, `matchNoStartFilter` is not a member of
`constants`.

- [ ] **Step 3: The flag** (`include/hermes/Regex/RegexTypes.h`, after
  `matchOnlyAtStart = 1 << 3,` at `:47`)

```cpp

  /// Do not use the start-position filter. Only tests use it, to compare
  /// filtered and unfiltered searches.
  matchNoStartFilter = 1 << 4,
```

- [ ] **Step 4: The filter** (`lib/Regex/Executor.cpp`)

Add `#include <cstring>` after the `llvh` includes (`:15`). Before
`template <class Traits> auto Context<Traits>::match` (`:1063`) add:

```cpp
/// \return the first index i in [\p from, \p length) such that the code
/// unit \p start[i] can begin a match according to the start filter of
/// \p header, or \p length if there is none. This is the ASCII-path
/// overload: every unit is 7-bit, so the non-ASCII bit is irrelevant.
static inline size_t findStartCandidate(
    const RegexBytecodeHeader &header,
    const char *start,
    size_t from,
    size_t length) {
  if (header.startFlags & StartFilterSingleChar) {
    const void *found =
        std::memchr(start + from, header.startChar, length - from);
    return found ? static_cast<const char *>(found) - start : length;
  }
  for (size_t i = from; i < length; ++i) {
    const uint8_t u = static_cast<uint8_t>(start[i]);
    assert(u < 128 && "ASCII-path input must be 7-bit");
    if (startSetHasASCII(header, u))
      return i;
  }
  return length;
}

/// \return the first index i in [\p from, \p length) such that the code
/// unit \p start[i] can begin a match according to the start filter of
/// \p header, or \p length if there is none. A unit >= 0x80 passes iff
/// StartFilterNonASCII is set.
static inline size_t findStartCandidate(
    const RegexBytecodeHeader &header,
    const char16_t *start,
    size_t from,
    size_t length) {
  const bool nonASCII = header.startFlags & StartFilterNonASCII;
  if (header.startFlags & StartFilterSingleChar) {
    const char16_t only = header.startChar;
    for (size_t i = from; i < length; ++i) {
      const char16_t u = start[i];
      if (u == only || (u >= 0x80 && nonASCII))
        return i;
    }
    return length;
  }
  for (size_t i = from; i < length; ++i) {
    const char16_t u = start[i];
    if (u >= 0x80 ? nonASCII : startSetHasASCII(header, u))
      return i;
  }
  return length;
}
```

In `match`, after `const size_t locsToCheckCount = ...;` (`:1087`) add:

```cpp
  // The start-position filter. Only a search that tries several positions
  // uses it: sticky and anchored searches and every lookaround recursion
  // (which may run a backwards cursor) pass onlyAtStart.
  const auto *header =
      reinterpret_cast<const RegexBytecodeHeader *>(bytecodeStream_.data());
  const bool useStartFilter = !onlyAtStart &&
      (header->startFlags & StartFilterPresent) &&
      !(flags_ & constants::matchNoStartFilter);
```

At the top of the loop body (`:1118`, before
`const CodeUnit *potentialMatchLocation = ...`) add:

```cpp
    if (useStartFilter) {
      // Skip to the first position whose unit can begin a match. A filtered
      // regex cannot match the empty string, so the position at the end of
      // the input needs no attempt. Under /u, every non-ASCII unit is
      // treated alike, so the scan stops on a high surrogate before its low
      // surrogate, and advanceStringIndex then steps over the pair.
      locIndex = findStartCandidate(*header, startLoc, locIndex, charsToRight);
      if (locIndex >= charsToRight)
        return nullptr;
    }
```

The `backtrackingSucceeded:` label (`:1121`) stays after these lines:
`BACKTRACK()` jumps there within the same position, past the skip.

- [ ] **Step 5: Run**

```bash
cmake --build $R/cmake-build-asan --target hermes hermesc shermes \
  HermesSupportTests
$R/cmake-build-asan/unittests/Support/HermesSupportTests \
  --gtest_filter='Regex.*'
```

Expected: all pass, including `StartFilterTransparent` and
`StartFilterAvoidsOverflow`.

- [ ] **Step 6: Prove the tests can fail** (rebuild after each edit and
  restore; run the Step 5 command). These are the spec's mutations
  ("Tests", Part A, "Mutations it must catch"):
  - M1 (Task 1 Step 11). Fails `StartFilterTransparent`, e.g. `/a/i` on
    `"xaAbsSkK..."` from 0: unfiltered `match (1-2)`, filtered
    `match (2-3)`.
  - M2. Fails: `/s/iu` on `"xaAbsSkK\u017f\u212a"` from 6: unfiltered
    `match (8-9)`, filtered `nomatch`.
  - M3. Fails: `/(?!a)b/` on `"xaAbsSkK..."` from 0: unfiltered
    `match (3-4)`, filtered `nomatch`.
  - M4. Fails: `/./` on `"\u00e9\u00c9x1_9"` from 0: unfiltered
    `match (0-1)`, filtered `match (2-3)`.
  - M8, filter when `onlyAtStart`: change the `useStartFilter`
    initializer to start `(header->startFlags & StartFilterPresent) &&`,
    dropping `!onlyAtStart &&`. Fails: `/a/i` sticky on `"xaAbsSkK..."`
    from 0: unfiltered `nomatch`, filtered `match (1-2)`.
  - M9, no filter: change the `useStartFilter` initializer to `false`.
    Fails `StartFilterAvoidsOverflow`: the second search is
    `StackOverflow`.

- [ ] **Step 7: Full suite** on `cmake-build-asan`: 0 failures; expected
  passes baseline + 3 (this task adds `Regex.StartFilterTransparent` and
  `Regex.StartFilterAvoidsOverflow`). This covers
  `test/hermes/regexp*.js`, `regex*.js`,
  `regress-regexp-*.js` and `test/shermes/regexp-aot.js`.

- [ ] **Step 8: Commit**

```bash
git -C $R add include/hermes/Regex/RegexTypes.h lib/Regex/Executor.cpp \
  unittests/Support/RegexTest.cpp
git -C $R commit -F $S/commit-3.txt
```

`$S/commit-3.txt` (Task 8 adds the measurements):

````
Regex: skip start positions that cannot begin a match

Context::match ran the whole instruction loop at every start position.
Over one Octane RegExp run, 278M of 282M positions failed, most after
one instruction, and failed positions took 95% of dispatched
instructions.

When a search tries more than one position and the regex has a start
filter, the loop now advances to the first position whose code unit is
in the start set, and returns no match when there is none. ASCII input
uses memchr for a single start character and the bitmap otherwise;
UTF-16 input accepts a unit of 0x80 or more iff the non-ASCII bit is
set. Sticky and anchored searches and lookaround recursion never use
it.

A position the filter skips could only have failed. It ran with no more
budget than the positions after it, so results change only where the
unfiltered search hit the regex resource limit: the filter may avoid a
"Maximum regex stack depth reached" RangeError, never cause one, e.g.

```
/(){999999999}a/.exec("b")   // was RangeError, now null
```

A new match flag, matchNoStartFilter, disables the filter; the unit
tests compare filtered and unfiltered searches over a corpus.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_019iPixSEVBGc2qFmM3Mq1Js
````

---

### Task 4: Split the builtin exec core from the result array

**Files:**
- Modify: `lib/VM/JSLib/RegExp.cpp:898-1111` (`directRegExpExec`),
  `:1235-1246` (`regExpPrototypeTest`), `:2531` (`@@split`)
- Modify: `include/hermes/VM/Runtime.h` (includes `:11-45`; after
  `regExpLastMatch`, `:935`)
- Modify: `lib/VM/JSRegExp.cpp:293-309` (`performSearch`)
- Create: `test/hermes/regexp-exec-fast-path.js`

**Interfaces:**
- **Produces:**
  ```cpp
  static CallResult<bool> regExpBuiltinExecMatch(
      Handle<JSRegExp> regexp, Runtime &runtime, Handle<StringPrimitive> S,
      RegExpMatch &match);
  ```
  in `RegExp.cpp`, defined before `directRegExpExec`; and
  `std::vector<regex::CapturedRange> Runtime::regExpSearchCaptures`.
  Tasks 5-7 call `regExpBuiltinExecMatch`.

**Facts this task relies on:**
- The observable operations of `directRegExpExec` are the `Get` and
  `ToLength` of `lastIndex` (`RegExp.cpp:917-927`) and one `Set` of
  `lastIndex` to 0 or `e` (`:977-1011`); the statics are written by
  `JSRegExp::search` on success (`lib/VM/JSRegExp.cpp:376-383`).
- `test` already requires a `JSRegExp`, never looks up `exec`, and runs
  `ToString` on its argument (`RegExp.cpp:1198-1246`).
- `performSearch` allocates a `std::vector<regex::CapturedRange>` per
  search (`JSRegExp.cpp:301`), and the executor clears and refills it
  (`lib/Regex/Executor.cpp:1676-1686`) without running JS or allocating
  on the GC heap.
- `@@split` copies its `RegExpMatch` (`RegExp.cpp:2531`).

**GC safety.** `regexp` and `S` are handles; `RegExpMatch` holds only
integers (`include/hermes/VM/RegExpMatch.h:18-30`). The core keeps the
`GCScope` that `directRegExpExec` opened (`:912`), so handles created by
`toLengthU64` and `JSRegExp::search` are freed on return.

- [ ] **Step 1: Invoke the gc-safe-coding skill.**

- [ ] **Step 2: Write the regression test**

Create `test/hermes/regexp-exec-fast-path.js`:

```js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// The RegExp library skips the JS-level call of RegExp.prototype.exec when
// exec is the builtin. Every observable effect of the call must remain: user
// replacements of exec, the exec lookups, lastIndex coercion and writes,
// the legacy statics, and the native exec frame in stack traces.

/// \return a printable form of \p v: strings quoted with non-ASCII escaped,
/// arrays bracketed, undefined and null by name.
function show(v) {
  if (v === undefined) return "undefined";
  if (v === null) return "null";
  if (typeof v === "string") {
    var out = '"';
    for (var i = 0; i < v.length; ++i) {
      var c = v.charCodeAt(i);
      out += c < 0x20 || c > 0x7e
          ? "\\u" + c.toString(16).padStart(4, "0") : v[i];
    }
    return out + '"';
  }
  if (Array.isArray(v)) return "[" + v.map(show).join(",") + "]";
  if (typeof v === "object") {
    return "{" + Object.keys(v).map(function (k) {
      return k + ":" + show(v[k]);
    }).join(",") + "}";
  }
  return String(v);
}

/// \return whether the stack trace \p stack has a frame for the native exec.
function hasExecFrame(stack) {
  return stack.indexOf("at exec (native)") >= 0;
}

/// Run \p body with RegExp.prototype.exec replaced by \p exec, restoring
/// the builtin afterwards.
function withProtoExec(exec, body) {
  var saved = RegExp.prototype.exec;
  RegExp.prototype.exec = exec;
  try {
    body();
  } finally {
    RegExp.prototype.exec = saved;
  }
}

function replacedExec(builtinExec) {
  print("replaced exec");
  var log = [];
  function counting(s) {
    log.push(this.source + "@" + this.lastIndex + ":" + s);
    return builtinExec.call(this, s);
  }
  withProtoExec(counting, function () {
    log = [];
    print(show("abab".match(/ab/g)), show(log));
    log = [];
    print(show("abab".replace(/ab/g, "x")), show(log));
    log = [];
    print(show("abab".replace(/ab/g, function (m) { return m.toUpperCase(); })),
          show(log));
    log = [];
    print("abab".search(/b/), show(log));
    log = [];
    print(show("abab".match(/b/)), show(log));
  });
}
// CHECK-LABEL: replaced exec
// CHECK-NEXT: ["ab","ab"] ["ab@0:abab","ab@2:abab","ab@4:abab"]
// CHECK-NEXT: "xx" ["ab@0:abab","ab@2:abab","ab@4:abab"]
// CHECK-NEXT: "ABAB" ["ab@0:abab","ab@2:abab","ab@4:abab"]
// CHECK-NEXT: 1 ["b@0:abab"]
// CHECK-NEXT: ["b"] ["b@0:abab"]

function ownAndSubclassExec(builtinExec) {
  print("own and subclass exec");
  var n = 0;
  var re = /a/g;
  re.exec = function (s) { ++n; return builtinExec.call(this, s); };
  print(show("aa".match(re)), show("aa".replace(re, "b")), "aa".search(re), n);
  class Sub extends RegExp {
    exec(s) { ++n; return super.exec(s); }
  }
  n = 0;
  var sub = new Sub("a", "g");
  print(show("aa".match(sub)), show("aa".replace(sub, "b")), "aa".search(sub),
        n);
}
// CHECK-LABEL: own and subclass exec
// CHECK-NEXT: ["a","a"] "bb" 0 7
// CHECK-NEXT: ["a","a"] "bb" 0 7

function execAccessor(builtinExec) {
  print("exec accessor");
  var gets = 0;
  Object.defineProperty(RegExp.prototype, "exec", {
    get: function () { ++gets; return builtinExec; },
    configurable: true,
  });
  try {
    print(show("abab".match(/ab/g)), gets);
    gets = 0;
    print(show("abab".replace(/ab/g, "x")), gets);
    gets = 0;
    print("abab".search(/b/), gets);
  } finally {
    Object.defineProperty(RegExp.prototype, "exec", {
      value: builtinExec,
      writable: true,
      configurable: true,
    });
  }
}
// CHECK-LABEL: exec accessor
// CHECK-NEXT: ["ab","ab"] 3
// CHECK-NEXT: "xx" 3
// CHECK-NEXT: 1 1

function proxyPrototype() {
  print("proxy prototype");
  var log = [];
  var proxy = new Proxy(RegExp.prototype, {
    get: function (target, key, receiver) {
      if (key === "exec") log.push("exec");
      return Reflect.get(target, key, receiver);
    },
  });
  var re = /ab/g;
  Object.setPrototypeOf(re, proxy);
  print(show(RegExp.prototype[Symbol.match].call(re, "abab")), log.length);
  log = [];
  print(show(RegExp.prototype[Symbol.replace].call(re, "abab", "x")),
        log.length);
  log = [];
  print(RegExp.prototype[Symbol.search].call(re, "abab"), log.length);
}
// CHECK-LABEL: proxy prototype
// CHECK-NEXT: ["ab","ab"] 3
// CHECK-NEXT: "xx" 3
// CHECK-NEXT: 0 1

function frozen() {
  print("frozen");
  var re = Object.freeze(/a/g);
  try {
    "aa".match(re);
  } catch (e) {
    print(e.name, hasExecFrame(e.stack));
  }
  try {
    "aa".replace(re, "b");
  } catch (e) {
    print(e.name, hasExecFrame(e.stack));
  }
}
// CHECK-LABEL: frozen
// CHECK-NEXT: TypeError false
// CHECK-NEXT: TypeError false

function lastIndexValueOf() {
  print("lastIndex valueOf");
  var n = 0;
  var re = /a/;
  re.lastIndex = { valueOf: function () { ++n; return 0; } };
  re.exec("a");
  re.test("a");
  "a".match(re);
  "a".replace(re, "b");
  "a".search(re);
  print(n, typeof re.lastIndex);
}
// CHECK-LABEL: lastIndex valueOf
// CHECK-NEXT: 4 object

function statics() {
  print("statics");
  function dump() {
    return [RegExp.$1, RegExp.lastMatch, RegExp.leftContext,
            RegExp.rightContext].join("|");
  }
  /(z)/.exec("z");
  /(\d)(\d)/.test("x12y");
  print(dump(), RegExp.$2);
  "a1b2c".match(/(\d)/g);
  print(dump());
  "xx3y".search(/(\d)/);
  print(dump());
  "p4q5r".replace(/(\d)/g, "-");
  print(dump());
  var seen = [];
  "p6q7r".replace(/(\d)/g, function (m) { seen.push(RegExp.$1); return m; });
  print(seen.join(","), dump());
}
// CHECK-LABEL: statics
// CHECK-NEXT: 1|12|x|y 2
// CHECK-NEXT: 2|2|a1b|c
// CHECK-NEXT: 3|3|xx|y
// CHECK-NEXT: 5|5|p4q|r
// CHECK-NEXT: 7,7 7|7|p6q|r

function emptyMatches() {
  print("empty matches");
  // Under /u the loop steps over the pair. The \uDE00 alternative matches
  // only a lone low surrogate, which a search started inside the pair sees:
  // so a loop that stepped by one code unit would match the pair (index
  // moved back by GetStringIndex) instead of looping on /(?:)/gu forever.
  var s = "\uD83D\uDE00x";
  print(show(s.match(/\uDE00|(?:)/gu)), show(s.match(/(?:)/g)));
  print(show(s.replace(/\uDE00|(?:)/gu, "-")), show(s.replace(/(?:)/g, "-")));
}
// CHECK-LABEL: empty matches
// CHECK-NEXT: ["","",""] ["","","",""]
// CHECK-NEXT: "-\ud83d\ude00-x-" "-\ud83d-\ude00-x-"

function stickyGlobal() {
  print("sticky global");
  print(show("aab".match(/a/gy)), show("baa".match(/a/gy)));
  print(show("aaba".replace(/a/gy, "x")), show("baa".replace(/a/gy, "x")));
}
// CHECK-LABEL: sticky global
// CHECK-NEXT: ["a","a"] null
// CHECK-NEXT: "xxba" "baa"

function replacements() {
  print("replacements");
  print(show("John Smith".replace(/(?<first>\w+)\s(?<last>\w+)/,
                                  "$<last>, $<first> [$1|$2|$&|$`|$']")));
  print(show("x-ab-y".replace(/(a)(z)?(b)/, "[$1|$2|$3|$&|$`|$']")));
  print(show("ab".replace(/(a)|(b)/g, "[$1|$2]")));
  "x-ab-y".replace(/(a)(z)?(b)/, function () {
    print(show(Array.prototype.slice.call(arguments)));
    return "";
  });
  "x-ab-y".replace(/(?<p>a)(?<q>z)?b/g, function () {
    print(show(Array.prototype.slice.call(arguments)));
    return "";
  });
}
// CHECK-LABEL: replacements
// CHECK-NEXT: "Smith, John [John|Smith|John Smith||]"
// CHECK-NEXT: "x-[a||b|ab|x-|-y]-y"
// CHECK-NEXT: "[a|][|b]"
// CHECK-NEXT: ["ab","a",undefined,"b",2,"x-ab-y"]
// CHECK-NEXT: ["ab","a",undefined,2,"x-ab-y",{p:"a",q:undefined}]

function hasIndices() {
  print("hasIndices");
  var m = "xab".match(/a(b)/d);
  print(show(m), m.index, show(m.indices));
}
// CHECK-LABEL: hasIndices
// CHECK-NEXT: ["ab","b"] 1 {{\[}}[1,3],[2,3]]

function unicodeGetter() {
  print("unicode getter");
  // @@match and @@replace advance past an empty match by Get(rx, "unicode"),
  // not by the regexp's own flag.
  var re = /(?:)/g;
  Object.defineProperty(re, "unicode", { value: true });
  var s = "\uD83D\uDE00x";
  print(show(s.match(re)), show(s.replace(re, "-")));
}
// CHECK-LABEL: unicode getter
// CHECK-NEXT: ["","",""] "-\ud83d\ude00-x-"

function reentrantCallback() {
  print("reentrant callback");
  // The callback runs after every exec of phase 1, and may use the same
  // regexp.
  var re = /\d/g;
  var out = "a1b2c".replace(re, function (d) {
    var inner = "7z8".replace(re, "q");
    return "<" + d + inner + re.lastIndex + RegExp.lastMatch + ">";
  });
  print(out, re.lastIndex, RegExp.lastMatch);
}
// CHECK-LABEL: reentrant callback
// CHECK-NEXT: a<1qzq08>b<2qzq08>c 0 8

function stacks() {
  print("stacks");
  var stack = "";
  var re = /a/;
  re.lastIndex = {
    valueOf: function () { stack = new Error().stack; return 0; },
  };
  "xa".match(re);
  print("match", hasExecFrame(stack));
  stack = "";
  "xa".replace(re, "b");
  print("replace", hasExecFrame(stack));
  stack = "";
  re.test("xa");
  print("test", hasExecFrame(stack));
  try {
    Object.freeze(/a/y)[Symbol.match]("a");
  } catch (e) {
    print("sticky", e.name, hasExecFrame(e.stack));
  }
  try {
    "b".match(/(){999999999}/g);
  } catch (e) {
    print("global match limit", e.name, hasExecFrame(e.stack));
  }
  try {
    "b".search(/(){999999999}/);
  } catch (e) {
    print("search limit", e.name, hasExecFrame(e.stack));
  }
}
// CHECK-LABEL: stacks
// CHECK-NEXT: match true
// CHECK-NEXT: replace true
// CHECK-NEXT: test false
// CHECK-NEXT: sticky TypeError true
// CHECK-NEXT: global match limit RangeError true
// CHECK-NEXT: search limit RangeError true

function main() {
  var builtinExec = RegExp.prototype.exec;
  replacedExec(builtinExec);
  ownAndSubclassExec(builtinExec);
  execAccessor(builtinExec);
  proxyPrototype();
  frozen();
  lastIndexValueOf();
  statics();
  emptyMatches();
  stickyGlobal();
  replacements();
  hasIndices();
  unicodeGetter();
  reentrantCallback();
  stacks();
}

main();
```

Notes for the reviewer:
- The expected lines are today's output. This exact file, extracted
  from this plan, was run through FileCheck at BASE's code (b2e83b329,
  which has no source change from BASE) on `cmake-build-asan` under
  every RUN line (`hermes`, `-lazy`, `-gc-sanitize-handles=1`,
  `shermes -exec`, and also `-O`) and on `cmake-build-hsan` with
  `-gc-sanitize-handles=1`; all passed. Every string is printed through
  `show()`, which escapes each code unit above 0x7e, so no expected line
  holds non-ASCII. The test is a regression guard: it passes before Part B
  and must still pass after it.
- `(){999999999}` can match the empty string, so it has no start filter
  and still hits the limit inside exec after Part A.
- `@@search` overwrites `lastIndex` with 0 before exec
  (`RegExp.cpp:1953`), so its `lastIndex.valueOf` never runs: the count
  is 4 (exec, test, match, replace).
- `{{\[}}` keeps FileCheck from reading `[[` as a variable.
- The callbacks in `statics` see `7,7`: phase 2 runs after every exec.

- [ ] **Step 3: Run it at BASE and save the expected output**

```bash
LIT_FILTER='regexp-exec-fast-path' \
  cmake --build $R/cmake-build-asan --target check-hermes
$R/cmake-build-asan/bin/hermes $R/test/hermes/regexp-exec-fast-path.js \
  > $S/fast-path.expected
```

Expected: 1 pass. `$S/fast-path.expected` is the reference for every
Part B mutation diff.

- [ ] **Step 4: Split `directRegExpExec`** (`lib/VM/JSLib/RegExp.cpp`)

Insert before `// ES6 21.2.5.2.2` (`:898`):

```cpp
/// ES6 21.2.5.2.2 RegExpBuiltinExec ( R, S ) up to, but not including,
/// building the result array: the Get and ToLength of lastIndex, the
/// search, the Set of lastIndex, and the unicode adjustment of the ranges.
/// \return false on no match; on a match, set \p match to its ranges
/// (element 0 is the whole match) and \return true.
static CallResult<bool> regExpBuiltinExecMatch(
    Handle<JSRegExp> regexp,
    Runtime &runtime,
    Handle<StringPrimitive> S,
    RegExpMatch &match) {
  struct : public Locals {
    PinnedValue<> propHandle;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  GCScope gcScope{runtime};

  // Let length be the number of code units in S.
  const uint32_t length = S->getStringLength();

  // Let lastIndex be ? ToLength(? Get(R, "lastIndex")).
  auto propRes = runtime.getNamed(regexp, PropCacheID::RegExpLastIndex);
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.propHandle = std::move(*propRes);
  auto lengthRes = toLengthU64(runtime, lv.propHandle);
  if (LLVM_UNLIKELY(lengthRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t lastIndex = *lengthRes;

  // Let flags be R.[[OriginalFlags]]
  const regex::SyntaxFlags flags = JSRegExp::getSyntaxFlags(regexp.get());

  // If flags contains "g", let global be true, else let global be false
  // If flags contains "y", let sticky be true, else let sticky be false.
  // If flags contains "u", let fullUnicode be true, else let fullUnicode be
  // false.
  const bool global = flags.global;
  const bool sticky = flags.sticky;
  const bool fullUnicode = flags.unicode;

  // If global is false and sticky is false, set lastIndex to 0.
  if (!global && !sticky) {
    lastIndex = 0;
  }

  // The ES6 spec is cast in terms of a loop on "matchSucceeded". This is
  // handled internally by JSRegExp::search for efficiency reasons.
  // (Note that lastIndex == length is allowed: it may match an empty range at
  // the end of the string.)
  match.clear();
  if (lastIndex <= length) {
    assert(
        lastIndex <= UINT32_MAX &&
        "lastIndex should fit in u32 because it is <= a string length");
    uint32_t searchStartOffset = static_cast<uint32_t>(lastIndex);
    auto matchResult = JSRegExp::search(regexp, runtime, S, searchStartOffset);
    if (LLVM_UNLIKELY(matchResult == ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    match = std::move(*matchResult);
  }

  if (match.empty()) {
    // No match. The following implements both:
    //   If lastIndex > length, then
    //     If global is true or sticky is true, then
    //         Let setStatus be Set(R, "lastIndex", 0, true);
    //         ReturnIfAbrupt(setStatus)
    //     return null
    // and also:
    //   If r is failure, then
    //     If sticky is true, then
    //       Let setStatus be Set(R, "lastIndex", 0, true);
    //       ReturnIfAbrupt(setStatus)
    //     Return null
    if (global || sticky) {
      if (LLVM_UNLIKELY(
              setLastIndex(regexp, runtime, 0) == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }
    return false;
  }

  // We have a match!
  // If using unicode, ensure that no indices can be set to a trailing
  // surrogate.
  if (fullUnicode) {
    for (auto &mg : match) {
      if (!mg)
        continue;
      auto captureStart = getStringIndex(S, mg->location);
      auto captureEnd = getStringIndex(S, mg->location + mg->length);
      mg = RegExpMatchRange{captureStart, captureEnd - captureStart};
    }
  }

  // "If global is true or sticky is true, Let setStatus be Set(R, "lastIndex",
  // e, true)."
  // Here 'e' is the end of the total match.
  if (global || sticky) {
    auto totalMatch = *match.front();
    uint32_t e = totalMatch.location + totalMatch.length;
    if (LLVM_UNLIKELY(
            setLastIndex(regexp, runtime, e) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  return true;
}
```

Then make `directRegExpExec` the core followed by the array building.
Replace its Locals and everything from `GCScope gcScope{runtime};`
(`:912`) through the `lastIndex` `Set` (`:1012`) with:

```cpp
  struct : public Locals {
    PinnedValue<JSArray> A;
    PinnedValue<StringPrimitive> strHandle;
    PinnedValue<JSArray> indicesArray;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  GCScope gcScope{runtime};

  RegExpMatch match;
  auto matchRes = regExpBuiltinExecMatch(regexp, runtime, S, match);
  if (LLVM_UNLIKELY(matchRes == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  if (!*matchRes) {
    resultOut.clear();
    return ExecutionStatus::RETURNED;
  }

  // If flags contains "d", let hasIndices be true, else let hasIndices be
  // false.
  const bool hasIndices = JSRegExp::getSyntaxFlags(regexp.get()).hasIndices;
```

Everything from `auto arrRes = JSArray::createAndAllocPropStorage(`
(`:1014`) to the end of the function stays as it is (it uses `match`,
`hasIndices`, `lv.A`, `lv.strHandle`, `lv.indicesArray` and `gcScope`).
`lv.propHandle` moved into the core.

- [ ] **Step 5: `test` uses the core unframed** (replace
  `regExpPrototypeTest`, `:1235-1246`)

```cpp
/// Implementation of RegExp.prototype.test
/// Returns true if a match is found, false otherwise. It runs the effects
/// of the builtin exec (lastIndex, the statics) without building the
/// result array. As before, it never looks up exec.
CallResult<HermesValue> regExpPrototypeTest(void *context, Runtime &runtime) {
  struct : public Locals {
    PinnedValue<StringPrimitive> S;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  Handle<JSRegExp> regexp = args.dyncastThis<JSRegExp>();
  if (!regexp) {
    return runtime.raiseTypeError(
        "RegExp function called on non-RegExp object");
  }

  auto strRes = toString_RJS(runtime, args.getArgHandle(0));
  if (strRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.S = std::move(*strRes);

  RegExpMatch match;
  auto matchRes = regExpBuiltinExecMatch(regexp, runtime, lv.S, match);
  if (LLVM_UNLIKELY(matchRes == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return HermesValue::encodeBoolValue(*matchRes);
}
```

- [ ] **Step 6: The Runtime-owned capture buffer** (B.4)

In `include/hermes/VM/Runtime.h`, add
`#include "hermes/Regex/Executor.h"` before
`#include "hermes/Support/Compiler.h"` (`:15`), and after
`RegExpMatch regExpLastMatch{};` (`:935`):

```cpp
  /// The capture ranges of the latest regex search, reused by every search
  /// so that it keeps its capacity. It is used only between the executor's
  /// return and the copy into a RegExpMatch (performSearch in
  /// JSRegExp.cpp); the executor runs no JS and allocates nothing on the GC
  /// heap, so it is never in use twice at once.
  std::vector<regex::CapturedRange> regExpSearchCaptures{};
```

In `lib/VM/JSRegExp.cpp`, `performSearch`, replace
`std::vector<regex::CapturedRange> nativeMatchRanges;` (`:301`) with:

```cpp
  std::vector<regex::CapturedRange> &nativeMatchRanges =
      runtime.regExpSearchCaptures;
```

In `@@split` (`RegExp.cpp:2531`), replace `auto match = *matchResult;`
with:

```cpp
    RegExpMatch match = std::move(*matchResult);
```

- [ ] **Step 7: Build and run**

```bash
cmake --build $R/cmake-build-asan --target hermes hermesc shermes
LIT_FILTER='regexp|regex|string-functions' \
  cmake --build $R/cmake-build-asan --target check-hermes
```

Expected: 0 failures, including `regexp-exec-fast-path.js`.

- [ ] **Step 8: Prove the guard can fail** (mutation diff; rebuild
  `hermes` after each edit and restore)
  - `lastIndex.valueOf` count: in `regExpBuiltinExecMatch`, move
    `const regex::SyntaxFlags flags = ...` above the `Get`, declare
    `uint64_t lastIndex = 0;`, and wrap the `Get`/`ToLength` block in
    `if (flags.global || flags.sticky) { ... lastIndex = *lengthRes; }`.
    Expected diff: `4 object` becomes `0 object`; `match true` and
    `replace true` become `false` (the `valueOf` never runs).
  - statics after `test`: in `regExpPrototypeTest`, around the core call,
    save and restore the statics:
    ```cpp
    struct : public Locals {
      PinnedValue<StringPrimitive> savedInput;
    } lvm;
    LocalsRAII lraiim(runtime, &lvm);
    lvm.savedInput = runtime.regExpLastInput;
    RegExpMatch savedMatch = runtime.regExpLastMatch;
    // ... the regExpBuiltinExecMatch call ...
    runtime.regExpLastInput = lvm.savedInput;
    runtime.regExpLastMatch = savedMatch;
    ```
    Expected diff: `1|12|x|y 2` changes to the statics of `/(z)/.exec("z")`
    (`z|z||` followed by an empty `$2`).

- [ ] **Step 9: Full suite** on `cmake-build-asan`: 0 failures; expected
  passes baseline + 4 (lit counts `regexp-exec-fast-path.js` once, whatever
  its number of RUN lines). Tasks 5-7 add no test: each later full suite
  expects baseline + 4.

- [ ] **Step 10: Commit**

```bash
git -C $R add lib/VM/JSLib/RegExp.cpp include/hermes/VM/Runtime.h \
  lib/VM/JSRegExp.cpp test/hermes/regexp-exec-fast-path.js
git -C $R commit -F $S/commit-4.txt
```

`$S/commit-4.txt`:

````
VM: split RegExpBuiltinExec from building the exec result array

directRegExpExec did the observable part of RegExpBuiltinExec (Get and
ToLength of lastIndex, the search, the Set of lastIndex) and then built
a result array. The first part is now its own function, which the fast
paths that follow will use:

```
static CallResult<bool> regExpBuiltinExecMatch(
    Handle<JSRegExp> regexp, Runtime &runtime, Handle<StringPrimitive> S,
    RegExpMatch &match);
```

RegExp.prototype.test calls it and no longer builds an array it throws
away. It still never looks up exec.

Each search also allocated a vector of capture ranges and copied it into
the RegExpMatch. The vector is now a Runtime buffer that keeps its
capacity, and the @@split loop moves its RegExpMatch instead of copying
it.

A new test, regexp-exec-fast-path.js, pins what the fast paths must not
change: replaced, own, subclass and accessor exec, a Proxy prototype,
lastIndex coercion and writes, the legacy statics, empty and sticky
matches, replacement patterns and callback arguments, hasIndices, and
the native exec frame in stack traces.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_019iPixSEVBGc2qFmM3Mq1Js
````

---

### Task 5: The exec frame and direct dispatch in `regExpExec`

**Files:**
- Modify: `include/hermes/VM/Callable.h:727-765` and after the
  `NativeFunction` class
- Modify: `include/hermes/VM/RuntimeHermesValueFields.def:71`
- Modify: `lib/VM/JSLib/RegExp.cpp:82-88`, after `:1146`, `:1164-1165`

**Interfaces:**
- **Consumes:** none from Part B beyond Task 4.
- **Produces:**
  - `template <typename Body> static CallResult<PseudoHandle<>>
    NativeFunction::runNativeCallBody(NativeFunction *self, Runtime &,
    const Body &body)`; `body()` returns `CallResult<HermesValue>`;
  - `runtime.regExpPrototypeExec` (`PinnedValue<NativeFunction>`);
  - in `RegExp.cpp`:
    ```cpp
    template <typename Body>
    static CallResult<HermesValue> runInBuiltinExecFrame(
        Runtime &runtime, Handle<JSObject> R, Handle<StringPrimitive> S,
        const Body &body);
    ```
  - Tasks 6 and 7 call `runInBuiltinExecFrame`.

**Facts this task relies on:**
- A real call builds: a `ScopedNativeCallFrame` with one argument and its
  overflow check (`lib/VM/Callable.cpp:322-339`); `potentiallyMoveHeap()`
  before dispatch (`include/hermes/VM/Callable.h:279-287`); then the body
  of `_nativeCall` (`Callable.h:730-765`). `NativeFunction::_callImpl`
  only forwards to `_nativeCall` (`lib/VM/Callable.cpp:1189-1193`).
- Stack traces record native frames from the runtime frames
  (`lib/VM/JSError.cpp:355`, `:708-712`); the debugger and the sampling
  profiler walk the same frames.
- The builtin exec runs `ToString` on its argument, a no-op on a string,
  then `directRegExpExec` (`RegExp.cpp:1198-1228`); its result is an
  object or null, so `regExpExec`'s type check (`:1175-1178`) always
  passes for it.
- The precedent for saving a builtin is `runtime.arrayPrototypeValues =
  defineMethod(...)` (`lib/VM/JSLib/Array.cpp:146`).

**GC safety.**
- `R` and `S` are handles owned by the caller.
- `runtime.regExpPrototypeExec` is a root. `runInBuiltinExecFrame` reads
  it once for the frame and again after `potentiallyMoveHeap()`, so the
  callee pointer passed on is current.
- `runNativeCallBody` never uses `self` after `body()` without reloading
  it: in `HERMESVM_PROFILER_NATIVECALL` builds it reloads the callee from
  the native frame's callee slot (`getCalleeClosureOrCBRef()`,
  `include/hermes/VM/StackFrame.h:112`), a root the GC updates, before
  updating the counters (Plan ruling 9). This also fixes the stale `self`
  that `_nativeCall` dereferenced after the native returned.
- The direct-dispatch comparison is between two rooted values, with no
  allocation between the reads.

- [ ] **Step 1: Invoke the gc-safe-coding skill.**

- [ ] **Step 2: Split `_nativeCall`** (`include/hermes/VM/Callable.h`)

Replace `_nativeCall` (`:727-765`) with:

```cpp
  /// Run \p body, which takes no arguments and returns
  /// CallResult<HermesValue>, as the body of a call of \p self: inside the
  /// native depth check and the frame setup and teardown that every call of
  /// a NativeFunction performs. The caller has pushed the frame (callee
  /// \p self, this and arguments), as for _nativeCall.
  template <typename Body>
  static inline CallResult<PseudoHandle<>> runNativeCallBody(
      NativeFunction *self,
      Runtime &runtime,
      const Body &body);

  /// This is a lightweight and unsafe wrapper intended to be used only by the
  /// interpreter. Its purpose is to avoid needlessly exposing the private
  /// fields.
  static CallResult<PseudoHandle<>> _nativeCall(
      NativeFunction *self,
      Runtime &runtime) {
    NativeFunctionPtr functionPtr = self->functionPtr_;
    void *context = self->context_;
    return runNativeCallBody(self, runtime, [functionPtr, context, &runtime]() {
      return functionPtr(context, runtime);
    });
  }
```

After the closing `};` of `class NativeFunction`, add:

```cpp
template <typename Body>
inline CallResult<PseudoHandle<>> NativeFunction::runNativeCallBody(
    NativeFunction *self,
    Runtime &runtime,
    const Body &body) {
  ScopedNativeDepthTracker depthTracker{runtime};
  if (LLVM_UNLIKELY(depthTracker.overflowed())) {
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  }

  // Ensure the IP was correctly saved before we create the new frame.
  runtime.validateSavedIPBeforeCall();
  auto newFrame = runtime.setCurrentFrameToTopOfStack();
  // We don't need to allocate CalleeExtraRegistersAtStart here because the
  // only register is DebugEnvironment, which is not useful in a
  // NativeFunction.
  static_assert(
      StackFrameLayout::FirstLocal - 1 == StackFrameLayout::DebugEnvironment &&
      StackFrameLayout::CalleeExtraRegistersAtStart == 1);

#ifdef HERMESVM_PROFILER_NATIVECALL
  auto t1 = HERMESVM_RDTSC();
#endif

  CallResult<HermesValue> res = body();

#ifdef HERMESVM_PROFILER_NATIVECALL
  // body() may have moved the heap, so \p self may be stale. Reload the
  // callee from the frame's callee slot, which the GC updates.
  self = vmcast<NativeFunction>(newFrame.getCalleeClosureOrCBRef());
  self->callDuration_ = HERMESVM_RDTSC() - t1;
  ++self->callCount_;
#else
  (void)self;
#endif
  runtime.restoreStackAndPreviousFrame(newFrame);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return createPseudoHandle(*res);
}
```

- [ ] **Step 3: Save the builtin exec**

In `include/hermes/VM/RuntimeHermesValueFields.def`, after
`RUNTIME_HV_FIELD(regExpPrototype, JSObject)` (`:71`):

```
// RegExp.prototype.exec as defined, to recognize the builtin.
RUNTIME_HV_FIELD(regExpPrototypeExec, NativeFunction)
```

In `RegExp.cpp` (`:82-88`), change the `exec` definition to:

```cpp
  runtime.regExpPrototypeExec = defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::exec),
      nullptr,
      regExpPrototypeExec,
      1);
```

- [ ] **Step 4: The exec frame** (`RegExp.cpp`, after `advanceStringIndex`,
  `:1146`)

```cpp
/// Run \p body as the body of a call of the builtin RegExp.prototype.exec
/// with this \p R and the argument \p S: inside a native frame for
/// runtime.regExpPrototypeExec built as Callable::executeCall1,
/// Callable::call and NativeFunction::_nativeCall build it. Stack traces,
/// the debugger and the sampling profiler then see the same exec frame as
/// for a real call. \p body returns CallResult<HermesValue>.
template <typename Body>
static CallResult<HermesValue> runInBuiltinExecFrame(
    Runtime &runtime,
    Handle<JSObject> R,
    Handle<StringPrimitive> S,
    const Body &body) {
  // As Callable::executeCall1.
  ScopedNativeCallFrame newFrame{
      runtime,
      1,
      runtime.regExpPrototypeExec.getHermesValue(),
      HermesValue::encodeUndefinedValue(),
      R.getHermesValue()};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  newFrame->getArgRef(0) = S.getHermesValue();
  // As Callable::call. The callee is read again from its root below.
  runtime.potentiallyMoveHeap();
  // As NativeFunction::_nativeCall.
  auto res = NativeFunction::runNativeCallBody(
      *runtime.regExpPrototypeExec, runtime, body);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return res->get();
}
```

- [ ] **Step 5: Direct dispatch in `regExpExec`** (`RegExp.cpp`, inside
  `if (auto execCallable = ...) {` at `:1165`, before the existing
  `// a. Let result be Call(exec, R, «S»).`)

```cpp
    // The builtin exec on a JSRegExp would run ToString(S), a no-op on a
    // string, and then RegExpBuiltinExec, whose result is an object or null.
    // Run that in the exec frame without the dispatch through
    // Callable::call.
    if (lv.exec.getHermesValue().getRaw() ==
        runtime.regExpPrototypeExec.getHermesValue().getRaw()) {
      if (auto regExpObj = Handle<JSRegExp>::dyn_vmcast(R)) {
        return runInBuiltinExecFrame(runtime, R, S, [&]() {
          return regExpBuiltinExec(runtime, regExpObj, S);
        });
      }
    }
```

A builtin exec on a non-`JSRegExp` still goes through `executeCall1`,
where the builtin raises its TypeError inside its frame, as today. The
non-callable fallback (`:1182-1192`) still calls `regExpBuiltinExec`
with no frame, as today.

- [ ] **Step 6: Build and run**

```bash
cmake --build $R/cmake-build-asan --target hermes hermesc shermes
LIT_FILTER='regexp|regex|string-functions|stacktrace' \
  cmake --build $R/cmake-build-asan --target check-hermes
```

Expected: 0 failures.

- [ ] **Step 7: Prove the guard can fail** (mutation diff; rebuild after
  each edit and restore)
  - Direct dispatch unframed: replace the `return runInBuiltinExecFrame(
    ...);` statement with `return regExpBuiltinExec(runtime, regExpObj,
    S);`. Expected diff: in `stacks`, `match`, `replace`, `sticky`,
    `global match limit` and `search limit` become `false` (every site
    still reaches exec through `regExpExec` at this task).
  - `test` given a frame: in `regExpPrototypeTest`, replace the core call
    with
    ```cpp
    bool matched = false;
    auto frameRes = runInBuiltinExecFrame(runtime, regexp, lv.S, [&]()
        -> CallResult<HermesValue> {
      auto r = regExpBuiltinExecMatch(regexp, runtime, lv.S, match);
      if (r == ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      matched = *r;
      return HermesValue::encodeUndefinedValue();
    });
    if (frameRes == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    return HermesValue::encodeBoolValue(matched);
    ```
    (move `runInBuiltinExecFrame` above `regExpPrototypeTest` for the
    mutation if needed). Expected diff: `test false` becomes `test true`.
  - Builtin recognized by mistake: change the raw comparison to `true`.
    Expected diff: every line of `replaced exec` and `own and subclass
    exec` loses the user exec's calls (the logs are `[]`, the counts 0).

- [ ] **Step 8: Full suite** on `cmake-build-asan`: 0 failures.

- [ ] **Step 9: Commit**

```bash
git -C $R add include/hermes/VM/Callable.h \
  include/hermes/VM/RuntimeHermesValueFields.def lib/VM/JSLib/RegExp.cpp
git -C $R commit -F $S/commit-5.txt
```

`$S/commit-5.txt`:

````
VM: run the builtin exec directly in its own frame from RegExpExec

RegExpExec called the exec it looked up through executeCall1 even when
it was the builtin, which then ran ToString on a string and
directRegExpExec. When exec is the builtin and R is a JSRegExp, it now
runs regExpBuiltinExec directly.

Stack traces, the debugger and the sampling profiler must still see the
native exec frame: user code can run inside exec (ToLength of a
lastIndex object), and errors can be raised there. So the call runs in
a frame for the builtin, built as executeCall1, Callable::call and
_nativeCall build it. _nativeCall is split so that both share the frame
code:

```
template <typename Body>
static CallResult<PseudoHandle<>> runNativeCallBody(
    NativeFunction *self, Runtime &runtime, const Body &body);
```

Runtime saves the builtin as regExpPrototypeExec. Every caller of
RegExpExec benefits; for non-global @@match and the matchAll iterator,
which need the result array, this is the whole fast path.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_019iPixSEVBGc2qFmM3Mq1Js
````

---

### Task 6: `hasBuiltinExec`, the global `@@match` loop and `@@search`

**Files:**
- Modify: `lib/VM/JSLib/RegExp.cpp` (new helpers before `:1779`;
  `regExpPrototypeSymbolMatch` `:1781-1917`;
  `regExpPrototypeSymbolSearch` `:1920-1985`)

**Interfaces:**
- **Consumes:** `regExpBuiltinExecMatch` (Task 4),
  `runInBuiltinExecFrame` and `runtime.regExpPrototypeExec` (Task 5).
- **Produces** (in `RegExp.cpp`, before `regExpPrototypeSymbolMatch`):
  - `static bool hasBuiltinExec(Runtime &runtime, Handle<JSObject> R);`
  - `static CallResult<bool> execMatchInBuiltinExecFrame(Runtime &runtime,
    Handle<JSRegExp> R, Handle<StringPrimitive> S, RegExpMatch &match);`
  - `static ExecutionStatus advanceLastIndexAfterEmptyMatch(Runtime
    &runtime, Handle<JSObject> rx, Handle<StringPrimitive> S, bool
    fullUnicode);`
  - Task 7 uses all three.

**Facts this task relies on:**
- `getNamedDescriptorPredefined` returns the owner and sets
  `proxyObject`/`hostObject` instead of a slot for proxies and host
  objects (`lib/VM/JSObject.cpp:1028-1065`); `getNamedSlotValueUnsafe`
  asserts it is not given either (`include/hermes/VM/JSObject.h:641-654`).
- Writing `RegExp.prototype.exec = f` rewrites the slot without a new
  class (`lib/VM/JSObject.cpp:3254-3283`), so the value must be compared.
- The precedents check `proxyObject` and compare the slot value with the
  saved builtin (`lib/VM/JSLib/HermesBuiltin.cpp:595-605`,
  `lib/VM/Interpreter-slowpaths.cpp:327-348`).
- Global `@@match` runs `Get global`, `Get unicode` and `Set lastIndex =
  0` before its loop (`RegExp.cpp:1811-1836`). In the loop, the only
  steps besides exec are `ToString` of a string from the builtin and the
  `Get`/`ToLength`/`Set` of a number `lastIndex` holds (`:1873-1911`):
  no user code, so `exec` cannot change during the loop.
- `@@search` reads only `index` of the result (`:1976-1984`).

**Counterexamples tried** (DESIGN-PRINCIPLES 3):
- A `lastIndex.valueOf` changing `exec` mid-loop: the pre-loop `Set`
  overwrote `lastIndex` with 0, and exec writes only numbers.
- An `exec` accessor that returns the builtin: rejected by
  `desc.flags.accessor`; the slow path runs the getter per iteration.
- A Proxy prototype: rejected by `proxyObject`; its `get` trap sees each
  lookup.
- An own `exec` holding the builtin: accepted; the builtin runs, as a
  `Get` would return it.

**GC safety.** `hasBuiltinExec` keeps the raw owner pointer only until
it reads the slot, with no allocation in between.
`getNamedDescriptorPredefined` may allocate (a lazy object), but `R` is
a handle. In the loops, `RegExpMatch` holds integers; the slice comes
from `S`, which `JSRegExp::search` flattened, and `slice` reads the
characters after it allocates (`lib/VM/StringPrimitive.cpp:350-382`).
The loop keeps its `flushToMarker` per iteration.

- [ ] **Step 1: Invoke the gc-safe-coding skill.**

- [ ] **Step 2: The helpers** (`RegExp.cpp`, before
  `// TODO: consider writing this in JS.` at `:1779`)

```cpp
/// \return true if \p R is a JSRegExp and Get(R, "exec") would return the
/// builtin exec from a plain data property, with no proxy, host object or
/// accessor on the lookup path. Such a Get runs no user code, so skipping
/// it is unobservable.
static bool hasBuiltinExec(Runtime &runtime, Handle<JSObject> R) {
  if (!vmisa<JSRegExp>(R.getHermesValue()))
    return false;
  NamedPropertyDescriptor desc;
  JSObject *owner = JSObject::getNamedDescriptorPredefined(
      R, runtime, Predefined::exec, desc);
  if (!owner || desc.flags.proxyObject || desc.flags.hostObject ||
      desc.flags.accessor)
    return false;
  SmallHermesValue slot =
      JSObject::getNamedSlotValueUnsafe(owner, runtime, desc);
  return slot.isObject() &&
      slot.getObject(runtime) == *runtime.regExpPrototypeExec;
}

/// Run regExpBuiltinExecMatch(\p R, \p S, \p match) inside a frame for the
/// builtin exec, as a call of RegExp.prototype.exec would. \return whether
/// it matched.
static CallResult<bool> execMatchInBuiltinExecFrame(
    Runtime &runtime,
    Handle<JSRegExp> R,
    Handle<StringPrimitive> S,
    RegExpMatch &match) {
  bool matched = false;
  auto res = runInBuiltinExecFrame(
      runtime, R, S, [&]() -> CallResult<HermesValue> {
        auto matchRes = regExpBuiltinExecMatch(R, runtime, S, match);
        if (LLVM_UNLIKELY(matchRes == ExecutionStatus::EXCEPTION))
          return ExecutionStatus::EXCEPTION;
        matched = *matchRes;
        return HermesValue::encodeUndefinedValue();
      });
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return matched;
}

/// After an empty match in the loop of @@match or @@replace: set lastIndex
/// of \p rx to AdvanceStringIndex(\p S, ToLength(Get(rx, "lastIndex")),
/// \p fullUnicode).
static ExecutionStatus advanceLastIndexAfterEmptyMatch(
    Runtime &runtime,
    Handle<JSObject> rx,
    Handle<StringPrimitive> S,
    bool fullUnicode) {
  struct : public Locals {
    PinnedValue<> lastIndex;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  // a. Let thisIndex be ToLength(Get(rx, "lastIndex")).
  // b. ReturnIfAbrupt(thisIndex).
  auto propRes = runtime.getNamed(rx, PropCacheID::RegExpLastIndex);
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  lv.lastIndex = std::move(*propRes);
  auto thisIndex = toLength(runtime, lv.lastIndex);
  if (LLVM_UNLIKELY(thisIndex == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  // c. Let nextIndex be AdvanceStringIndex(S, thisIndex, fullUnicode).
  double nextIndex = advanceStringIndex(
      S.get(), thisIndex->getNumberAs<uint64_t>(), fullUnicode);
  // d. Let setStatus be Set(rx, "lastIndex", nextIndex, true).
  // e. ReturnIfAbrupt(setStatus).
  return setLastIndex(rx, runtime, nextIndex);
}
```

- [ ] **Step 3: The global `@@match` loop**
  (`regExpPrototypeSymbolMatch`; everything up to `uint32_t n = 0;`,
  `:1844`, stays; replace from `// g. Repeat,` (`:1846`) to the end of
  the loop (`:1914`) with:)

```cpp
  // Whether exec is the builtin. Besides exec, every step of the loop below
  // runs no user code (ToString of a string, Get and ToLength of the number
  // that lastIndex holds, Set of lastIndex), so exec cannot change during
  // the loop and one check covers every iteration.
  const bool fast = hasBuiltinExec(runtime, rx);

  // g. Repeat,
  auto marker = gcScope.createMarker();
  RegExpMatch match;
  while (true) {
    gcScope.flushToMarker(marker);
    // i. Let result be RegExpExec(rx, S).
    // ii. ReturnIfAbrupt(result).
    bool matched;
    if (fast) {
      auto matchRes = execMatchInBuiltinExecFrame(
          runtime, Handle<JSRegExp>::vmcast(rx), lv.S, match);
      if (LLVM_UNLIKELY(matchRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      matched = *matchRes;
    } else {
      auto callRes = regExpExec(runtime, rx, lv.S);
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.result = callRes.getValue();
      matched = !lv.result->isNull();
    }
    // iii. If result is null, then
    if (!matched) {
      if (n == 0) {
        return HermesValue::encodeNullValue();
      } else {
        if (LLVM_UNLIKELY(
                JSArray::setLengthProperty(lv.A, runtime, n) ==
                ExecutionStatus::EXCEPTION))
          return ExecutionStatus::EXCEPTION;
        return lv.A.getHermesValue();
      }
    }
    // iv. Else result is not null,
    // 1. Let matchStr be ToString(Get(result, "0")).
    // 2. ReturnIfAbrupt(matchStr).
    if (fast) {
      // Element 0 of the builtin's result is this slice of S.
      auto strRes = StringPrimitive::slice(
          runtime, lv.S, match[0]->location, match[0]->length);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.matchStr.castAndSetHermesValue<StringPrimitive>(*strRes);
    } else {
      auto resultObj = Handle<JSObject>::vmcast(&lv.result);
      auto propRes2 = getIndexed_RJS(runtime, resultObj, 0);
      if (propRes2 == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.propValue = std::move(propRes2.getValue());
      auto strRes2 = toString_RJS(runtime, lv.propValue);
      if (strRes2 == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.matchStr = strRes2->get();
    }
    // 3. Let status be CreateDataProperty(A, ToString(n), matchStr).
    // 4. Assert: status is true.
    if (LLVM_UNLIKELY(
            JSArray::setElementAt(lv.A, runtime, n, lv.matchStr) ==
            ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    // 5. If matchStr is the empty String, then
    //   a-e. Set lastIndex to AdvanceStringIndex(S, thisIndex, fullUnicode).
    if (lv.matchStr->getStringLength() == 0) {
      if (LLVM_UNLIKELY(
              advanceLastIndexAfterEmptyMatch(
                  runtime, rx, lv.S, fullUnicode) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }
    // 6. Increment n.
    n++;
  }
```

The `llvm_unreachable` after the loop stays. `propRes` is still used by
the `Get global` above the loop, so it stays declared.

- [ ] **Step 4: `@@search`** (`regExpPrototypeSymbolSearch`; replace from
  `// 9. Let result be RegExpExec(rx, S).` (`:1958`) to the end of the
  function with:)

```cpp
  // 9. Let result be RegExpExec(rx, S).
  // 10. ReturnIfAbrupt(result).
  // When exec is the builtin, only the position of the match is needed.
  const bool fast = hasBuiltinExec(runtime, rx);
  RegExpMatch match;
  bool matched;
  if (fast) {
    auto matchRes = execMatchInBuiltinExecFrame(
        runtime, Handle<JSRegExp>::vmcast(rx), lv.S, match);
    if (LLVM_UNLIKELY(matchRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    matched = *matchRes;
  } else {
    auto execRes = regExpExec(runtime, rx, lv.S);
    if (LLVM_UNLIKELY(execRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv.result = execRes.getValue();
    matched = !lv.result.getHermesValue().isNull();
  }
  // 11. Let status be Set(rx, "lastIndex", previousLastIndex, true).
  auto previousLastIndexSHV = SmallHermesValue::encodeHermesValue(
      lv.previousLastIndex.getHermesValue(), runtime);
  status = setLastIndex(rx, runtime, previousLastIndexSHV);
  // 12. ReturnIfAbrupt(status).
  if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 13. If result is null, return -1.
  if (!matched) {
    return HermesValue::encodeTrustedNumberValue(-1);
  }
  // 14. Return Get(result, "index").
  if (fast) {
    // The builtin's index is the start of the whole match.
    return HermesValue::encodeTrustedNumberValue(match[0]->location);
  }
  auto resultObj = Handle<JSObject>::dyn_vmcast(Handle<>{lv.result});
  if (LLVM_UNLIKELY(!resultObj)) {
    return ExecutionStatus::EXCEPTION;
  }
  return JSObject::getNamed_RJS(
             resultObj, runtime, Predefined::getSymbolID(Predefined::index))
      .toCallResultHermesValue();
}
```

The check sits between the two `Set`s of `lastIndex`, as the spec
requires; the first `Set` writes an own data property of a `JSRegExp`
and runs no user code.

- [ ] **Step 5: Build and run**

```bash
cmake --build $R/cmake-build-asan --target hermes hermesc shermes
LIT_FILTER='regexp|regex|string-functions' \
  cmake --build $R/cmake-build-asan --target check-hermes
```

Expected: 0 failures.

- [ ] **Step 6: Prove the guard can fail** (spec table; mutation diff;
  rebuild after each edit and restore)
  - `hasBuiltinExec` always true: replace everything after the
    `vmisa<JSRegExp>` test with `return true;`. Expected diff: the global
    `match` and `search` lines of `replaced exec` and `own and subclass
    exec` lose the user exec's calls.
  - Accessor accepted and hoisted: replace the body after the
    `vmisa<JSRegExp>` test with
    ```cpp
    auto res = JSObject::getNamed_RJS(
        R, runtime, Predefined::getSymbolID(Predefined::exec));
    if (res == ExecutionStatus::EXCEPTION) {
      runtime.clearThrownValue();
      return false;
    }
    return res->get().getRaw() ==
        runtime.regExpPrototypeExec.getHermesValue().getRaw();
    ```
    Expected diff: `exec accessor` line 1 becomes `["ab","ab"] 1`. (The
    `search` line stays `1 1`: one hoisted `Get` replaces one exec
    `Get`.) Deleting only `desc.flags.accessor` is masked: the slot then
    holds a `PropertyAccessor`, never the builtin
    (`lib/VM/JSObject.cpp:3278`).
  - Proxy check removed: delete `desc.flags.proxyObject ||`. Expected: the
    run aborts in `proxy prototype` with the Debug assertion
    `getNamedSlotValueUnsafe called on a Proxy`
    (`include/hermes/VM/JSObject.h:645-648`); `exit` is non-zero.
  - Frozen, pre-loop `Set` skipped: move
    `const bool fast = hasBuiltinExec(runtime, rx);` above step c of
    `@@match`, and make the `Set` `if (!fast && setLastIndex(rx, runtime,
    0) == ExecutionStatus::EXCEPTION)`. Expected diff: `frozen` line 1
    becomes `TypeError true` (the TypeError now comes from inside exec).
  - Empty matches by code unit: in the `@@match` loop pass
    `fast ? false : fullUnicode` to `advanceLastIndexAfterEmptyMatch`.
    Expected diff: `empty matches` line 1 becomes
    `["","\ud83d\ude00","",""] ["","","",""]`, and `unicode getter`
    line 1 becomes `["","","",""] ...`. Derivation: after the empty match
    at 0, lastIndex becomes 1; the search from 1 matches the lone
    `\uDE00` at 1, which GetStringIndex reports as index 0 and length 2
    with lastIndex 2 (`RegExp.cpp:413-417`, `:995-1011`; checked at BASE
    with `re.lastIndex = 1; re.exec(s)` giving index 0, length 2,
    lastIndex 2); then empty matches at 2 and 3. The run terminates, so
    every later case stays visible. (The old `/(?:)/gu` loops forever
    under this mutation: the search from 1 moves lastIndex back to 0.)
  - Flag instead of `Get unicode` (Review Focus 4): pass
    `fast ? JSRegExp::getSyntaxFlags(vmcast<JSRegExp>(rx.get())).unicode
    : fullUnicode`. Expected diff: only `unicode getter` line 1, which
    becomes `["","","",""] ...` (the regexp has no `u` flag, so it steps
    by one unit and stops at the end). `empty matches` is unchanged: there
    the flag and `Get unicode` agree. So this case is observed
    independently of the code-unit mutation above.
  - Sticky cleared: before the `@@match` loop add
    `if (fast) { auto f = JSRegExp::getSyntaxFlags(vmcast<JSRegExp>(
    rx.get())); f.sticky = false; JSRegExp::setSyntaxFlags(
    vmcast<JSRegExp>(rx.get()), f); }`. Expected diff: `sticky global`
    line 1 becomes `["a","a"] ["a","a"]`.
  - `@@match` unframed: in the loop, call
    `regExpBuiltinExecMatch(Handle<JSRegExp>::vmcast(rx), runtime, lv.S,
    match)` instead of `execMatchInBuiltinExecFrame(...)`. Expected
    diff: `global match limit RangeError false`.
  - `@@search` unframed: the same edit in `@@search`. Expected diff:
    `search limit RangeError false`.
  - `hasIndices`, array-free non-global match: in `@@match`, before
    `return regExpExec(runtime, rx, lv.S);` (step 7.a) add
    ```cpp
    if (hasBuiltinExec(runtime, rx)) {
      RegExpMatch m1;
      auto r = regExpBuiltinExecMatch(
          Handle<JSRegExp>::vmcast(rx), runtime, lv.S, m1);
      if (r == ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      if (!*r)
        return HermesValue::encodeNullValue();
      auto s1 = StringPrimitive::slice(
          runtime, lv.S, m1[0]->location, m1[0]->length);
      if (s1 == ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      lv.matchStr.castAndSetHermesValue<StringPrimitive>(*s1);
      auto a1 = JSArray::create(runtime, 1, 1);
      if (a1 == ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      lv.A = std::move(*a1);
      if (JSArray::setElementAt(lv.A, runtime, 0, lv.matchStr) ==
          ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      return lv.A.getHermesValue();
    }
    ```
    Expected diff: `hasIndices` becomes `["ab"] undefined undefined`.

- [ ] **Step 7: Full suite** on `cmake-build-asan`: 0 failures.

- [ ] **Step 8: Commit**

```bash
git -C $R add lib/VM/JSLib/RegExp.cpp
git -C $R commit -F $S/commit-6.txt
```

`$S/commit-6.txt`:

````
VM: builtin-exec fast paths for global @@match and @@search

A global @@match ran RegExpExec per match: an uncached Get of exec, a
call to the native exec and a full result array with index, input and
groups, of which it read only element 0. @@search read only index.

When exec is the builtin, both now run regExpBuiltinExecMatch in the
exec frame and take the slice or position from the match ranges. The
check reads exec's descriptor and compares the value with the saved
builtin; a proxy, host object or accessor on the lookup path falls back
to the generic path:

```
static bool hasBuiltinExec(Runtime &runtime, Handle<JSObject> R);
```

One check before the loop covers every iteration: the only other steps
of an iteration are ToString of a string and the Get, ToLength and Set
of a number in lastIndex, so no user code can change exec. lastIndex is
read and written exactly as before.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_019iPixSEVBGc2qFmM3Mq1Js
````

---

### Task 7: The `@@replace` fast path

**Files:**
- Modify: `lib/VM/JSLib/RegExp.cpp` (`buildGroupsObject` `:691-763` and
  its callers `:790`, `:871`; new helpers before
  `regExpPrototypeSymbolReplace`, `:1987`; its body `:1989-2374`)

**Interfaces:**
- **Consumes:** `hasBuiltinExec`, `execMatchInBuiltinExecFrame`,
  `advanceLastIndexAfterEmptyMatch` (Task 6).
- **Produces** (all `static` in `RegExp.cpp`):
  - `template <typename GetCapture> CallResult<PseudoHandle<JSObject>>
    buildGroupsObject(Runtime &, Handle<JSObject> mappingObj, const
    GetCapture &getCapture)`, where `getCapture(uint32_t i)` returns the
    `SmallHermesValue` of element `i` of the match (0 = whole match);
  - `ExecutionStatus replaceOneMatch(...)`: steps 16.m-p for one match;
  - `ExecutionStatus replaceBuiltinMatch(...)`: builds `matched`, the
    captures and the groups object from a `RegExpMatch`, then calls
    `replaceOneMatch`.

**Facts this task relies on:**
- `@@replace` runs `ToString(S)`, `ToString(replaceValue)` when not
  callable, `Get global`, `Get unicode` and the `Set` before its loop
  (`RegExp.cpp:2020-2069`), keeps every result array
  (`:2099-2103`), and reads them back in phase 2 through `Get`s on own
  properties of arrays user code never sees (`:2156-2264`). The callback
  runs in phase 2 (`:2266-2316`).
- The group-name mapping maps a name to its index in the full match
  array (`include/hermes/Regex/Regex.h:497-503`;
  `lib/VM/JSRegExp.cpp:261-267`), which is what `buildGroupsObject`
  indexes (`RegExp.cpp:718-722`).
- `getSubstitution` takes captures as an `ArrayStorageSmall` and named
  captures as an object (`lib/VM/JSLib/JSLibInternal.h:282-289`).
- `stringView` holds a handle and survives allocations
  (`include/hermes/VM/StringView.h:38-57`); it is only sliced and
  appended (`RegExp.cpp:2356-2358`).

**Counterexamples tried:**
- A non-global `@@replace` has no pre-loop `Set`, so `lastIndex` may hold
  an object whose `valueOf` changes `exec`. There is one exec, and the
  slow path has already fetched `exec` before `valueOf` runs too, so both
  paths run the builtin once.
- A callback that reuses the regexp (Review Focus 5) runs after phase 1
  in both paths, on stored matches.

**GC safety.**
- `fastMatches` is a malloc'd vector of integers, unreachable from the
  GC.
- Every string is a slice of the flat `S`, made by `slice`, which reads
  the characters after allocating.
- `buildGroupsObject`'s `getCapture` reads through a handle and does not
  allocate.
- Phase 2 keeps `marker1` per match and a marker per capture
  (`:2154`, `:2226`).
- No raw pointer or `StringView` iterator is held across an allocation
  or call.

- [ ] **Step 1: Invoke the gc-safe-coding skill.**

- [ ] **Step 2: Generalize `buildGroupsObject`** (`:691-763`)

Change its doc comment, signature and the two reads:

```cpp
/// Build an object whose property keys are identical to \p mappingObj. Each
/// value for a given property key `k` in this object is
/// `getCapture(mappingObj[k])`: \p getCapture maps an index of the match
/// (0 is the whole match, n the n-th group) to that element as a
/// SmallHermesValue, and must not allocate. The result has a null
/// prototype.
template <typename GetCapture>
static CallResult<PseudoHandle<JSObject>> buildGroupsObject(
    Runtime &runtime,
    Handle<JSObject> mappingObj,
    const GetCapture &getCapture) {
```

In the non-dictionary path (`:718-722`):

```cpp
          auto idx = static_cast<uint32_t>(
              JSObject::getNamedSlotValueUnsafe(
                  *mappingObj, runtime, desc.slot)
                  .getNumber(runtime));
          JSObject::setNamedSlotValueUnsafe(
              lv.groupsObj.get(), runtime, desc.slot, getCapture(idx));
```

In the dictionary path (`:744-747`):

```cpp
        auto idx = static_cast<uint32_t>(
            JSObject::getNamedSlotValueUnsafe(*mappingObj, runtime, desc.slot)
                .getNumber(runtime));
        lv.tmpVal = getCapture(idx).unboxToHV(runtime);
```

Callers: in `setMatchGroups` (`:790`):

```cpp
  auto groupsRes = buildGroupsObject(
      runtime, mappingObj, [&](uint32_t i) -> SmallHermesValue {
        return matchObj->at(runtime, i);
      });
```

and in `makeMatchIndicesIndexPairArray` (`:871`):

```cpp
    auto groupsRes = buildGroupsObject(
        runtime, mappingObj, [&](uint32_t i) -> SmallHermesValue {
          return lv.A->at(runtime, i);
        });
```

- [ ] **Step 3: The per-match phase-2 helpers** (before
  `/// ES6.0 21.2.5.8` at `:1987`)

```cpp
/// ES6.0 21.2.5.8 steps 16.m-p of RegExp.prototype[@@replace], for one
/// match: compute the replacement of the match \p matched at \p position of
/// \p S, whose captures are \p captures (each a string or undefined) and
/// whose named captures are \p namedCaptures (undefined if none), by calling
/// \p replaceFn if it is not null and by GetSubstitution with
/// \p replaceValueStr otherwise. If \p position is not before
/// \p nextSourcePosition, append the text of \p stringView (a view of S)
/// from nextSourcePosition up to position, and the replacement, to
/// \p accumulatedResult, and move \p nextSourcePosition past the match.
static ExecutionStatus replaceOneMatch(
    Runtime &runtime,
    Handle<StringPrimitive> S,
    const StringView &stringView,
    Handle<Callable> replaceFn,
    Handle<StringPrimitive> replaceValueStr,
    Handle<StringPrimitive> matched,
    uint32_t position,
    Handle<ArrayStorageSmall> captures,
    Handle<> namedCaptures,
    SmallU16String<32> &accumulatedResult,
    uint32_t &nextSourcePosition) {
  struct : public Locals {
    PinnedValue<HermesValue> namedCapturesObj;
    PinnedValue<> replValue;
    PinnedValue<StringPrimitive> replacement;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  const bool hasNamedCaptures = !namedCaptures->isUndefined();
  // f. Let matchLength be the number of code units in matched.
  const uint32_t matchLength = matched->getStringLength();

  // m. If functionalReplace is true, then
  if (replaceFn) {
    CallResult<PseudoHandle<>> callRes{ExecutionStatus::EXCEPTION};
    {
      // i. Let replacerArgs be «matched».
      // Arguments: matched, captures, position, S (, groups).
      size_t replacerArgsCount =
          1 + (size_t)captures->size() + 2 + hasNamedCaptures;
      if (LLVM_UNLIKELY(replacerArgsCount >= UINT32_MAX))
        return runtime.raiseStackOverflow(
            Runtime::StackOverflowKind::JSRegisterStack);
      ScopedNativeCallFrame newFrame{
          runtime,
          static_cast<uint32_t>(replacerArgsCount),
          *replaceFn,
          false,
          HermesValue::encodeUndefinedValue()};
      if (LLVM_UNLIKELY(newFrame.overflowed()))
        return runtime.raiseStackOverflow(
            Runtime::StackOverflowKind::NativeStack);

      uint32_t argIdx = 0;
      newFrame->getArgRef(argIdx++) = matched.getHermesValue();
      // ii. Append in list order the elements of captures to the end of the
      // List replacerArgs.
      for (; argIdx <= captures->size(); ++argIdx) {
        newFrame->getArgRef(argIdx) =
            captures->at(argIdx - 1).unboxToHV(runtime);
      }
      // iii. Append position and S.
      newFrame->getArgRef(argIdx++) =
          HermesValue::encodeTrustedNumberValue(position);
      newFrame->getArgRef(argIdx++) = S.getHermesValue();
      // iv. If namedCaptures is not undefined, then
      if (hasNamedCaptures) {
        newFrame->getArgRef(argIdx++) = namedCaptures.getHermesValue();
      }
      // v. Let replValue be Call(replaceValue, undefined, replacerArgs).
      callRes = Callable::call(replaceFn, runtime);
      if (callRes == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }
    // vi. Let replacement be ToString(replValue).
    lv.replValue = std::move(callRes.getValue());
    auto strRes = toString_RJS(runtime, lv.replValue);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv.replacement = strRes->get();
  } else {
    // n. Else,
    // i. If namedCaptures is not undefined, then
    if (hasNamedCaptures) {
      // 1. Set namedCaptures to ? ToObject(namedCaptures).
      auto objRes = toObject(runtime, namedCaptures);
      if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.namedCapturesObj = *objRes;
    }
    // ii. Let replacement be GetSubstitution(matched, S, position, captures,
    // replaceValue).
    auto callRes = getSubstitution(
        runtime,
        matched,
        S,
        position,
        captures,
        hasNamedCaptures ? Handle<JSObject>::vmcast(&lv.namedCapturesObj)
                         : Runtime::makeNullHandle<JSObject>(),
        replaceValueStr);
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv.replacement.castAndSetHermesValue<StringPrimitive>(callRes.getValue());
  }
  // o. ReturnIfAbrupt(replacement).
  // p. If position ≥ nextSourcePosition, then
  if (position >= nextSourcePosition) {
    // i. NOTE position should not normally move backwards. If it does, it is
    // an indication of an ill-behaving RegExp subclass or use of an access
    // triggered side-effect to change the global flag or other
    // characteristics of rx. In such cases, the corresponding substitution is
    // ignored.
    // ii. Let accumulatedResult be the String formed by concatenating the
    // code units of the current value of accumulatedResult with the substring
    // of S consisting of the code units from nextSourcePosition (inclusive) up
    // to position (exclusive) and with the code units of replacement.
    stringView.slice(nextSourcePosition, position - nextSourcePosition)
        .appendUTF16String(accumulatedResult);
    lv.replacement->appendUTF16String(accumulatedResult);
    // iii. Let nextSourcePosition be position + matchLength.
    nextSourcePosition = position + matchLength;
  }
  return ExecutionStatus::RETURNED;
}

/// Phase 2 of RegExp.prototype[@@replace] for a match \p match of the
/// builtin exec on \p S: build matched, the captures and the groups object
/// (from \p groupNames, the regexp's group-name mapping, if not null) as
/// the builtin's result array would hold them, then run replaceOneMatch().
/// The Gets this replaces read own properties of an array user code never
/// sees, so skipping them is unobservable.
static ExecutionStatus replaceBuiltinMatch(
    Runtime &runtime,
    Handle<StringPrimitive> S,
    const StringView &stringView,
    Handle<Callable> replaceFn,
    Handle<StringPrimitive> replaceValueStr,
    Handle<JSObject> groupNames,
    const RegExpMatch &match,
    SmallU16String<32> &accumulatedResult,
    uint32_t &nextSourcePosition) {
  struct : public Locals {
    PinnedValue<StringPrimitive> matched;
    PinnedValue<ArrayStorageSmall> captures;
    PinnedValue<> capN;
    PinnedValue<> namedCaptures;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  // matched: element 0, the whole match.
  const RegExpMatchRange whole = *match[0];
  auto matchedRes =
      StringPrimitive::slice(runtime, S, whole.location, whole.length);
  if (LLVM_UNLIKELY(matchedRes == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  lv.matched.castAndSetHermesValue<StringPrimitive>(*matchedRes);

  // captures: a string per matched group, undefined per unmatched one.
  const uint32_t nCaptures = match.size() - 1;
  auto capturesRes = ArrayStorageSmall::create(runtime, nCaptures);
  if (LLVM_UNLIKELY(capturesRes == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  lv.captures.castAndSetHermesValue<ArrayStorageSmall>(*capturesRes);
  MutableHandle<ArrayStorageSmall> captures{lv.captures};
  for (uint32_t n = 1; n <= nCaptures; ++n) {
    GCScopeMarkerRAII marker{runtime};
    if (!match[n]) {
      lv.capN = HermesValue::encodeUndefinedValue();
    } else {
      auto capRes = StringPrimitive::slice(
          runtime, S, match[n]->location, match[n]->length);
      if (LLVM_UNLIKELY(capRes == ExecutionStatus::EXCEPTION))
        return ExecutionStatus::EXCEPTION;
      lv.capN = *capRes;
    }
    if (LLVM_UNLIKELY(
            ArrayStorageSmall::push_back(captures, runtime, lv.capN) ==
            ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
  }

  // namedCaptures: the groups object, or undefined without named groups.
  if (groupNames) {
    auto groupsRes = buildGroupsObject(
        runtime, groupNames, [&](uint32_t i) -> SmallHermesValue {
          return captures->at(i - 1);
        });
    if (LLVM_UNLIKELY(groupsRes == ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    lv.namedCaptures = groupsRes->getHermesValue();
  } else {
    lv.namedCaptures = HermesValue::encodeUndefinedValue();
  }

  return replaceOneMatch(
      runtime,
      S,
      stringView,
      replaceFn,
      replaceValueStr,
      lv.matched,
      whole.location,
      captures,
      lv.namedCaptures,
      accumulatedResult,
      nextSourcePosition);
}
```

- [ ] **Step 4: The `@@replace` body**

In the Locals of `regExpPrototypeSymbolReplace` (`:1989-2005`), delete
`PinnedValue<StringPrimitive> replacement;` and
`PinnedValue<> replValueHandle;` (now in `replaceOneMatch`). Keep
everything up to the end of `if (global) { ... }` (`:2069`). Replace
from `// 11. Let results be a new empty List.` (`:2070`) to the end of
the function (`:2374`) with:

```cpp
  // Whether exec is the builtin. From here to the end of phase 1 no user
  // code runs outside exec (ToString of a string, Get and ToLength of the
  // number that lastIndex holds, Set of lastIndex), so one check covers
  // every exec; callbacks run in phase 2, after the last exec.
  const bool fast = hasBuiltinExec(runtime, rx);
  // The matches of phase 1 when fast.
  std::vector<RegExpMatch> fastMatches;
  MutableHandle<ArrayStorageSmall> resultsHandle{lv.resultsHandle};
  if (!fast) {
    // 11. Let results be a new empty List.
    auto arrRes = ArrayStorageSmall::create(runtime, 16 /* capacity */);
    if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv.resultsHandle.castAndSetHermesValue<ArrayStorageSmall>(
        arrRes.getValue());
  }

  // 12. Let done be false.
  // 13. Repeat, while done is false
  auto stringView = StringPrimitive::createStringView(runtime, lv.S);
  while (true) {
    GCScopeMarkerRAII marker{runtime};
    bool emptyMatch = false;
    // a. Let result be RegExpExec(rx, S).
    // b. ReturnIfAbrupt(result).
    if (fast) {
      RegExpMatch match;
      auto matchRes = execMatchInBuiltinExecFrame(
          runtime, Handle<JSRegExp>::vmcast(rx), lv.S, match);
      if (LLVM_UNLIKELY(matchRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // c. If result is null, set done to true.
      if (!*matchRes)
        break;
      // Element 0 of the builtin's result is this range of S.
      emptyMatch = match[0]->length == 0;
      // d.i. Append result to the end of results.
      fastMatches.push_back(std::move(match));
      // ii. If global is false, set done to true.
      if (!global)
        break;
    } else {
      auto execRes = regExpExec(runtime, rx, lv.S);
      if (LLVM_UNLIKELY(execRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // c. If result is null, set done to true.
      if (execRes->isNull())
        break;
      // d. Else result is not null,
      // If result is not null, it must be an object.
      lv.result.castAndSetHermesValue<JSObject>(execRes.getValue());
      // i. Append result to the end of results.
      if (LLVM_UNLIKELY(
              ArrayStorageSmall::push_back(resultsHandle, runtime, lv.result) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // ii. If global is false, set done to true.
      if (!global)
        break;
      // iii. Else,
      // 1. Let matchStr be ToString(Get(result, "0")).
      // 2. ReturnIfAbrupt(matchStr).
      propRes = getIndexed_RJS(runtime, lv.result, 0);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.propValue = propRes->get();
      auto strRes = toString_RJS(runtime, lv.propValue);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.matchStr = strRes->get();
      emptyMatch = lv.matchStr->getStringLength() == 0;
    }
    // iii.3. If matchStr is the empty String, then
    //   a-e. Set lastIndex to AdvanceStringIndex(S, thisIndex, fullUnicode).
    if (emptyMatch) {
      if (LLVM_UNLIKELY(
              advanceLastIndexAfterEmptyMatch(
                  runtime, rx, lv.S, fullUnicode) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  }

  // 14. Let accumulatedResult be the empty String value.
  SmallU16String<32> accumulatedResult{};
  // 15. Let nextSourcePosition be 0.
  uint32_t nextSourcePosition = 0;
  // 16. Repeat, for each result in results,
  if (fast) {
    Handle<JSObject> groupNames =
        Handle<JSRegExp>::vmcast(rx)->getGroupNameMappings(runtime);
    for (const RegExpMatch &match : fastMatches) {
      GCScopeMarkerRAII marker1{runtime};
      if (LLVM_UNLIKELY(
              replaceBuiltinMatch(
                  runtime,
                  lv.S,
                  stringView,
                  replaceFn,
                  lv.replaceValueStr,
                  groupNames,
                  match,
                  accumulatedResult,
                  nextSourcePosition) == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  } else {
    for (uint32_t i = 0, size = resultsHandle->size(); i < size; ++i) {
      GCScopeMarkerRAII marker1{runtime};
      lv.result = vmcast<JSObject>(resultsHandle->at(i).getObject(runtime));
      // a. Let nCaptures be ToLength(Get(result, "length")).
      // b. ReturnIfAbrupt(nCaptures).
      propRes = JSObject::getNamed_RJS(
          lv.result, runtime, Predefined::getSymbolID(Predefined::length));
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.valueHandle = std::move(propRes.getValue());
      auto lengthRes = toLength(runtime, lv.valueHandle);
      if (LLVM_UNLIKELY(lengthRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // Use uint64_t so it can fit the result of toLength.
      uint64_t nCaptures = lengthRes->getNumberAs<uint64_t>();
      // c. Let nCaptures be max(nCaptures − 1, 0).
      nCaptures = nCaptures > 0 ? nCaptures - 1 : 0;

      // Throw RangeError if nCaptures exceeds uint32_t, since
      // ArrayStorageSmall::create() takes uint32_t capacity and the loop
      // below would run for an impractical number of iterations.
      if (LLVM_UNLIKELY(nCaptures > UINT32_MAX)) {
        return runtime.raiseRangeError(
            "Too many capture groups in exec result");
      }
      // d. Let matched be ToString(Get(result, "0")).
      // e. ReturnIfAbrupt(matched).
      propRes = getIndexed_RJS(runtime, lv.result, 0);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.indexedPropValue = std::move(propRes.getValue());
      auto strRes = toString_RJS(runtime, lv.indexedPropValue);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.matched = std::move(*strRes);
      // g. Let position be ToIntegerOrInfinity(Get(result, "index")).
      // h. ReturnIfAbrupt(position).
      propRes = JSObject::getNamed_RJS(
          lv.result, runtime, Predefined::getSymbolID(Predefined::index));
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.indexPropValue = std::move(propRes.getValue());
      auto intRes = toIntegerOrInfinity(runtime, lv.indexPropValue);
      if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // position could potentially be negative or Infinity here, so use
      // double.
      double positionDouble = intRes->getNumber();
      // i. Let position be max(min(position, lengthS), 0).
      // Now we can clamp to uint32_t because we've bounds checked
      // and `lengthS` is `uint32_t`.
      uint32_t position = (int64_t)std::fmax(
          std::fmin(positionDouble, (double)lengthS), (double)0);
      // j. Let n be 1.
      // Match the type of nCaptures.
      uint64_t n = 1;
      // k. Let captures be an empty List.
      auto capturesRes =
          ArrayStorageSmall::create(runtime, static_cast<uint32_t>(nCaptures));
      if (LLVM_UNLIKELY(capturesRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.capturesHandle.castAndSetHermesValue<ArrayStorageSmall>(
          capturesRes.getValue());
      MutableHandle<ArrayStorageSmall> capturesHandle{lv.capturesHandle};
      // l. Repeat while n ≤ nCaptures
      while (n <= nCaptures) {
        GCScopeMarkerRAII marker2{runtime};
        // i. Let capN be Get(result, ToString(n)).
        // ii. ReturnIfAbrupt(capN).
        propRes = getIndexed_RJS(runtime, lv.result, n);
        if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        lv.capN = std::move(propRes.getValue());
        // iii. If capN is not undefined, then
        if (!lv.capN->isUndefined()) {
          // 1. Let capN be ToString(capN).
          // 2. ReturnIfAbrupt(capN).
          auto capStrRes = toString_RJS(runtime, lv.capN);
          if (LLVM_UNLIKELY(capStrRes == ExecutionStatus::EXCEPTION)) {
            return ExecutionStatus::EXCEPTION;
          }
          lv.capN = capStrRes->getHermesValue();
        }
        // iv. Append capN as the last element of captures.
        if (LLVM_UNLIKELY(
                ArrayStorageSmall::push_back(
                    capturesHandle, runtime, lv.capN) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        // v. Let n be n+1
        n++;
      }

      // j. Let namedCaptures be ? Get(result, "groups").
      auto namedCapturesRes = JSObject::getNamed_RJS(
          lv.result, runtime, Predefined::getSymbolID(Predefined::groups));
      if (LLVM_UNLIKELY(namedCapturesRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.namedCaptures = std::move(*namedCapturesRes);

      // m-p.
      if (LLVM_UNLIKELY(
              replaceOneMatch(
                  runtime,
                  lv.S,
                  stringView,
                  replaceFn,
                  lv.replaceValueStr,
                  lv.matched,
                  position,
                  capturesHandle,
                  lv.namedCaptures,
                  accumulatedResult,
                  nextSourcePosition) == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  }

  // 17. If nextSourcePosition ≥ lengthS, return accumulatedResult.
  if (nextSourcePosition >= lengthS) {
    return StringPrimitive::createEfficient(runtime, accumulatedResult);
  }
  // 18. Return the String formed by concatenating the code units of
  // accumulatedResult with the substring of S consisting of the code units from
  // nextSourcePosition (inclusive) up through the final code unit of S
  // (inclusive).
  stringView.slice(nextSourcePosition).appendUTF16String(accumulatedResult);
  return StringPrimitive::createEfficient(runtime, accumulatedResult);
}
```

The slow path is today's code (`:2083-2361`) with three changes: the
empty-match block calls `advanceLastIndexAfterEmptyMatch`, steps m-p
call `replaceOneMatch`, and `namedCaptures` is passed as a value
(undefined when absent) instead of a flag plus a value.

- [ ] **Step 5: Build and run**

```bash
cmake --build $R/cmake-build-asan --target hermes hermesc shermes
LIT_FILTER='regexp|regex|string-functions|string-replace' \
  cmake --build $R/cmake-build-asan --target check-hermes
```

Expected: 0 failures, including `regexp-replace-truncation.js` (the
generic path's `length` RangeError) and `regexp.js:586-590` (an own
`exec` returning a huge `index`).

- [ ] **Step 6: Moving GC** (`-gc-sanitize-handles` coverage)

```bash
cmake --build $R/cmake-build-hsan --target hermes hermesc shermes
LIT_FILTER='regexp|regex|string-functions' \
  cmake --build $R/cmake-build-hsan --target check-hermes
```

Expected: 0 failures, no ASan report. The `-gc-sanitize-handles=1` RUN
line moves the heap at every allocation in this tree.

- [ ] **Step 7: Prove the guard can fail** (spec table; mutation diff on
  `cmake-build-asan`; rebuild after each edit and restore)
  - Replace fast path taken for any JSRegExp: change
    `const bool fast = hasBuiltinExec(runtime, rx);` in `@@replace` to
    `const bool fast = vmisa<JSRegExp>(rx.getHermesValue());`. Expected
    diff: `replaced exec` lines 2-3 have empty logs; `own and subclass
    exec` counts drop.
  - Frozen, pre-loop `Set` skipped: move the `fast` check above the
    `if (global)` block and make the `Set` `if (!fast)`-guarded. Expected
    diff: `frozen` line 2 becomes `TypeError true`.
  - Empty matches by code unit: pass `fast ? false : fullUnicode` in the
    `@@replace` loop. Expected diff: `empty matches` line 2 becomes
    `"---x-" "-\ud83d-\ude00-x-"`, and `unicode getter` becomes
    `["","",""] "-\ud83d-\ude00-x-"`. Derivation: phase 1 records
    (0,0), (0,2), (2,0), (3,0) (see the Task 6 derivation); phase 2
    appends `-` for each and the text before it, and the last append
    brings in `x`. The run terminates.
  - Flag instead of `Get unicode`: the Task 6 edit in the `@@replace`
    loop. Expected diff: only `unicode getter`, whose replace result
    becomes `"-\ud83d-\ude00-x-"`.
  - Sticky cleared: before the `@@replace` loop, clear `sticky` on the
    regexp when `fast` (the Task 6 edit). Expected diff: `sticky global`
    line 2 becomes `"xxbx" "bxx"`. Derivation: without sticky, the global
    search of `/a/g` on `"aaba"` finds `a` at 0, 1 and 3 (it skips the
    `b` at 2), and on `"baa"` at 1 and 2.
  - `@@replace` unframed: call `regExpBuiltinExecMatch(
    Handle<JSRegExp>::vmcast(rx), runtime, lv.S, match)` instead of
    `execMatchInBuiltinExecFrame(...)`. Expected diff: `replace false` in
    `stacks`.
  - Range 0 for every capture: in `replaceBuiltinMatch`, use `match[0]`
    in place of `match[n]` (both uses). Expected diff: `replacements`
    lines 1, 2, 4 and 5 change (every `$n`, `$<name>`, capture argument
    and group becomes the whole match).
  - Unmatched groups as `""`: in `replaceBuiltinMatch`, replace
    `lv.capN = HermesValue::encodeUndefinedValue();` with
    `lv.capN = HermesValue::encodeStringValue(
    runtime.getPredefinedString(Predefined::emptyString));`. Expected
    diff: `replacements` lines 4-5 show `""` for `undefined` (also
    `q:""`); line 2 is unchanged because `$2` substitutes `""` either
    way.
  - Callbacks interleaved with execs (Review Focus 5): move
    `SmallU16String<32> accumulatedResult{};` and
    `uint32_t nextSourcePosition = 0;` above the phase-1 loop, and in the
    fast branch replace `fastMatches.push_back(std::move(match));` with
    a call to `replaceBuiltinMatch(runtime, lv.S, stringView, replaceFn,
    lv.replaceValueStr, Handle<JSRegExp>::vmcast(rx)->
    getGroupNameMappings(runtime), match, accumulatedResult,
    nextSourcePosition)` (returning on exception). Expected: `statics`
    line 5 becomes `6,7 7|7|p6q|r`, and the run then loops in
    `reentrant callback` (the inner replace resets `lastIndex`) until
    `timeout` kills it (`exit 124`).

- [ ] **Step 8: Full suite** on `cmake-build-asan`: 0 failures.

- [ ] **Step 9: Commit**

```bash
git -C $R add lib/VM/JSLib/RegExp.cpp
git -C $R commit -F $S/commit-7.txt
```

`$S/commit-7.txt` (Task 8 adds the measurements):

````
VM: builtin-exec fast path for @@replace

@@replace kept every exec result array until phase 2, then read each
back through generic Gets of length, 0, index, every capture and
groups.

When exec is the builtin, phase 1 runs regExpBuiltinExecMatch in the
exec frame and keeps each match's ranges in a C++ vector. Phase 2 is
factored into one step per match that takes matched, position, the
captures and the named captures:

```
static ExecutionStatus replaceOneMatch(
    Runtime &runtime, Handle<StringPrimitive> S,
    const StringView &stringView, Handle<Callable> replaceFn,
    Handle<StringPrimitive> replaceValueStr,
    Handle<StringPrimitive> matched, uint32_t position,
    Handle<ArrayStorageSmall> captures, Handle<> namedCaptures,
    SmallU16String<32> &accumulatedResult, uint32_t &nextSourcePosition);
```

The generic path still extracts them with today's Gets. The fast path
builds them from the ranges: slices of S, undefined for unmatched
groups, and a groups object from the regexp's group-name mapping;
buildGroupsObject now takes a function that reads a capture. The
callback still runs in phase 2, after every exec.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_019iPixSEVBGc2qFmM3Mq1Js
````

---

### Task 8: Cross-configuration suites and measurements

**Files:**
- No source change. Amends the messages of the Task 3 and Task 7
  commits.

- [ ] **Step 1: Full suites**

```bash
for t in asan hsan hv32; do
  cmake --build $R/cmake-build-$t --target check-hermes
done
$R/cmake-build-asan/unittests/Support/HermesSupportTests \
  --gtest_filter='Regex.*'
```

Expected: 0 failures in every tree. On `cmake-build-asan`, expected
passes are the Task 1 baseline + 4 (unless a test is unsupported there).
`cmake-build-hsan` and `cmake-build-hv32` have no recorded baseline;
record their counts.

- [ ] **Step 2: The measurement script**

Write `$S/measure.sh`:

```bash
#!/bin/bash
# Usage: measure.sh <label>. Rebuilds the release hermes for the current
# checkout and runs each benchmark of the spec's Problem section 3 times.
set -e
R=/home/tmikov/work/hermes-regex-jit
cmake --build $R/cmake-build-release --target hermes > /dev/null
H=$R/cmake-build-release/bin/hermes
M=$R/benchmarks/bench-runner/resource/test-suites/micros
echo "== $1 ($(git -C $R rev-parse --short HEAD))"
for i in 1 2 3; do
  $H -O $R/benchmarks/octane/regexp.js | grep '^RegExp '
done
for b in stringMatch stringSearch regExpSearch regExpMatch regExpSplit \
    regExpReplace; do
  for i in 1 2 3; do
    echo "$b $($H -O $M/$b.js | grep '^Time:')"
  done
done
```

Best of three is the highest Octane score and the lowest micro time.

- [ ] **Step 3: Measure BASE, end of Part A, end of Part B**

The tree must be clean apart from untracked files
(`git -C $R status --short` shows only `??` lines). `A_END` is the Task 3
commit, `B_END` the Task 7 commit (HEAD).

```bash
A_END=$(git -C $R log --format=%H -1 \
  --grep='^Regex: skip start positions that cannot begin a match')
B_END=$(git -C $R rev-parse HEAD)
git -C $R checkout --detach <BASE>
bash $S/measure.sh BASE | tee $S/measure-base.txt
git -C $R checkout --detach $A_END
bash $S/measure.sh "Part A" | tee $S/measure-a.txt
git -C $R checkout regexp-speedup
bash $S/measure.sh "Part B" | tee $S/measure-b.txt
```

`<BASE>` is the hash recorded in Task 1 Step 1. Each checkout makes the
next build of the Debug trees rebuild more; that is expected.

- [ ] **Step 4: Record the numbers in the two commit messages**

Each part gets a block from its best-of-three numbers (no pass/fail
threshold, spec "Measurements"). The two blocks compare against
different revisions:

| Block  | Message of | "Before" column | "After" column | `<REF>` |
|--------|-----------|-----------------|----------------|---------|
| Part A | Task 3 commit | `measure-base.txt` | `measure-a.txt` | `BASE <BASE short hash>` |
| Part B | Task 7 commit | `measure-a.txt` | `measure-b.txt` | `the start-filter commit ("Regex: skip start positions that cannot begin a match")` |

Part B's reference names the Part A commit by subject, not by hash: the
rewrite below and the later squash both change that hash, while BASE is
an existing commit whose hash does not change. Template:

````
Measured on a release build (-g -fno-omit-frame-pointer, x86-64), best
of three, hermes -O, against <REF>:

```
Benchmark                Before    After
Octane RegExp (score)       ...      ...
micros/stringMatch.js    ... ms   ... ms
micros/stringSearch.js   ... ms   ... ms
micros/regExpSearch.js   ... ms   ... ms
micros/regExpMatch.js    ... ms   ... ms
micros/regExpSplit.js    ... ms   ... ms
micros/regExpReplace.js  ... ms   ... ms
```
````

Insert each block before the `Co-Authored-By:` line of its message.
Rewrite the Part A message first, then the Part B message (HEAD after
the rebase):

```bash
git -C $R log -1 --format=%B $A_END > $S/msg-a.txt
# edit $S/msg-a.txt: insert the Part A block before the trailers
GIT_SEQUENCE_EDITOR="sed -i '/^pick ${A_END:0:7}/a exec git commit --amend -F $S/msg-a.txt'" \
  git -C $R rebase -i ${A_END}~1
git -C $R log -1 --format=%B HEAD > $S/msg-b.txt
# edit $S/msg-b.txt: insert the Part B block before the trailers
git -C $R commit --amend -F $S/msg-b.txt
git -C $R diff $B_END HEAD --stat     # must be empty: only messages changed
git -C $R log -7 --format='%h %s'    # record the new hashes
```

See the `non-interactive-git-rebase` skill if the rebase stops. The
final `diff` must print nothing.

- [ ] **Step 5: Report**

The report lists: the per-tree suite counts; the three measurement
tables; the new hashes of all seven commits; every mutation and the
failure it produced (from Tasks 1-7); and every plan ruling used,
including any taken during implementation.

---

## Plan rulings

1. The executor's bracket predicate moves into
   `bracketMatchesCodePoint` (`RegexTraits.h`), shared by the executor
   and `BracketNode::addStartSet`. Beyond the spec's shared closure
   helper, this makes the ASCII bits the executor's own answers (spec,
   Risks) (Task 1).
2. The base `Node::addStartSet` returns `Nullable`, because the base
   `Node` is the leading no-op node; every subclass overrides it,
   including the zero-width ones, so each states its rule (Task 1).
3. `startSet_` is reset at the start of `parseWithBackRefLimit` and
   computed only on success, so a reparse (`Regex.h:317-335`) never
   keeps a stale set (Task 1).
4. `StartSet` is a class with an out-of-line `writeTo`; one helper,
   `startSetHasASCII`, reads the header bitmap for the executor, the
   dump and the tests; a `static_assert` pins the 24-byte header
   (Task 1).
5. The header test describes a filter as a string ("none", hex ranges,
   "nonascii", "single", "empty"), so one table checks flags, bitmap and
   `startChar` (Task 1).
6. A present filter with no members (only `[]`) dumps as `StartSet:`
   with nothing after it (Task 2).
7. The A.5 case `/(){999999999}a/` over `"b"` runs as its own test, once
   each way, not inside the corpus loop: an unfiltered run builds 2^24
   backtrack records (Task 3).
8. The differential corpus adds lookbehind, astral, multiline, `\s`,
   property-escape, `[]`/`[^]` and nested-group patterns, and a UTF-16
   input with only ASCII units (Review Focus 1-3) (Task 3).
9. Spec B.3 requires the callee rooted and reloaded across the body. In
   `HERMESVM_PROFILER_NATIVECALL` builds, `runNativeCallBody` reloads
   `self` from the native frame's callee slot after `body()` before
   updating the counters; `_nativeCall` gets the same fix. No tree here
   builds the profiler, so Task 5's report states that this path was
   compiled only by inspection, or, if the executor configures a scratch
   tree with `-DHERMESVM_PROFILER_NATIVECALL=ON`, that it built (Task 5).
10. `regExpBuiltinExecMatch` keeps the `GCScope` that `directRegExpExec`
    opened (`RegExp.cpp:912`), so the handles of `toLengthU64` and
    `JSRegExp::search` are freed in every caller, including `test` and
    the frame body (Task 4).
11. The empty-match `lastIndex` advance becomes
    `advanceLastIndexAfterEmptyMatch`, shared by the generic and fast
    loops of `@@match` and `@@replace`; each loop has one body with
    `fast` branches, so the steps cannot drift apart (Tasks 6, 7).
12. Phase 2 is `replaceOneMatch` (steps 16.m-p), shared by both paths;
    `replaceBuiltinMatch` builds its inputs from ranges.
    `buildGroupsObject` takes a capture-reading callable (Task 7).
13. The `@@replace` fast path does not create the results
    `ArrayStorageSmall` (Task 7).
14. Part B mutations are checked by diffing the whole test output against
    the unmutated output, since one mutation can change several cases
    and FileCheck stops at the first (Global Constraints).
15. The proxy mutation's failure is the Debug assertion in
    `getNamedSlotValueUnsafe` (`JSObject.h:645-648`), which every Debug
    tree here enables (Task 6).
16. The Part B test adds two Review Focus cases (`unicodeGetter`,
    `reentrantCallback`) and a mutation that interleaves callbacks with
    execs, beyond the spec's table (Tasks 4, 7).
17. Measurements use `hermes -O` and re-measure BASE in the same session,
    so before and after are compared like for like. Part A is compared
    with BASE, Part B with the end of Part A, which its message names by
    subject because rewriting and squashing change the hash. The numbers
    go into the last commit of each part (Tasks 3 and 7) by a
    non-interactive rebase, so the two-commit squash carries them
    (Task 8).
18. `Runtime.h` includes `hermes/Regex/Executor.h` for
    `regex::CapturedRange`, the buffer's element type (Task 4).
19. Header expectations for `\p{Lu}` sit under
    `HERMES_ENABLE_UNICODE_REGEXP_PROPERTY_ESCAPES` (on in every tree
    here), as the parser's support does (Tasks 1, 3).
