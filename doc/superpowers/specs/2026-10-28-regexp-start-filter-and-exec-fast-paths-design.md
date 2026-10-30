# RegExp start-position filter and builtin-exec fast paths

Status: design. Branch `regexp-speedup`, on top of the regexp bytecode
sharing work (`ea916884b`) and `static_h` `f837b9dd8`.

Two independent changes, one commit each:

- **Part A**, a start-position filter in the regex engine.
- **Part B**, fast paths in the RegExp library glue for a regexp whose
  `exec` is the builtin.

## Problem

Measured on a release build (`-g -fno-omit-frame-pointer`, x86-64) at
`de0922711`, best of three:

| Benchmark | Result |
|---|---|
| Octane RegExp | 956 (score) |
| `micros/stringMatch.js` | 1195 ms |
| `micros/stringSearch.js` | 1186 ms |
| `micros/regExpSearch.js` | 700 ms |
| `micros/regExpMatch.js` | 495 ms |
| `micros/regExpSplit.js` | 436 ms |
| `micros/regExpReplace.js` | 293 ms |

**The matcher tries every start position.** `Context::match`
(`lib/Regex/Executor.cpp:1063`) loops over `locsToCheckCount` positions
(`:1087`, `:1115-1120`) and runs the whole instruction loop at each.
Nothing skips a position whose first code unit cannot begin a match.
Temporary counters in that loop, over one Octane RegExp run, gave:

- 5.9M searches tried 282M start positions.
- 278M of those failed. 200.6M failed after one dispatched instruction,
  and 218.2M after at most two.
- Failed positions account for 534M of 560M dispatched instructions
  (95%).

`perf` puts `Context::match`, `backtrack`, `matchWidth1Loop` and
`bracketMatchesChar` at about 58% of Octane RegExp, and at 85-90% of
`stringMatch` and `regExpSearch`.

**The glue builds arrays nobody sees.** In `regExpMatch`, the top
symbols are `directRegExpExec`, `getNamedDescriptorUnsafe`,
`createNoAllocPropStorage`, `_setOwnIndexedImpl`, `slice`,
`SmallVector` copies and `executeCall1`. A global `@@match` runs
`regExpExec` per match (`lib/VM/JSLib/RegExp.cpp:1858`): an uncached
`Get(rx, "exec")` (`:1158`), a JS-level call to the native exec
(`:1171`), and a full result array with `index`, `input` and `groups`
(`directRegExpExec`, `:1013-1105`), of which it reads only element 0
(`:1873`). `@@replace` keeps every result array until phase 2 and then
reads them back through generic `Get`s (`:2083-2264`).

## Goals

- **A.** Skip start positions that cannot begin a match, computed once
  at regex compile time.
- **B.** When `exec` is the builtin, run the builtin exec's effects
  without a JS call or a result array the caller would only read back.
- Neither changes any observable behavior, including the existing
  deviations listed under Non-goals, with one stated exception: a regex
  resource limit (A.5).

## Non-goals

- Fixing existing spec deviations: `test` never looks up `exec`
  (`RegExp.cpp:1239-1247`); `@@search` always sets and restores
  `lastIndex` (`:1953`, `:1966`); `@@match` and `@@replace` read
  `global`/`unicode` rather than `flags`; `@@split` and `@@matchAll`
  have no `SpeciesConstructor`. Each stays as it is.
- `@@split` (`:2380-2665`). It already calls `JSRegExp::search` on a
  private splitter without `exec`; it only gets the cheaper search of
  B.4.
- Literal prefixes longer than one character, and Boyer-Moore.
- Regenerating the sandbox. Its generated C already carries bytecode
  version 99 against the tree's 100, so `verify_generated.py` fails today
  ("the bytecode version check uses BYTECODE_VERSION 100", twice); the
  last bump, `defe64b07`, also changed only the version line. A
  regeneration is a separate task.
- A regex JIT.

## Part A: start-position filter

### The start set

A **start set** describes which code units can be the first unit of a
match:

- `ascii[16]`: a 128-bit bitmap. Bit `u` is set if a match can begin
  with code unit `u`.
- `nonASCII`: true if a match can begin with a unit >= 0x80.

The set may be a superset of the truth, never a subset. A regex has
**no filter** when it can match the empty string, or when the analysis
cannot bound its first unit. Every non-ASCII unit is treated alike; this
keeps the `/u` invariant that the matcher never starts inside a
surrogate pair except at the first position (A.4).

### Computing it

The set is computed in `Regex::parseWithBackRefLimit`, after
`optimizeNodeList` and next to `matchConstraints_`
(`include/hermes/Regex/Regex.h:357-370`), and stored in a new member.
`compile()` (`Regex.h:142-158`) writes it into the header.

Node fields are private and there is no RTTI or node-kind enum
(`include/hermes/Regex/RegexNode.h:47-148`), so the analysis is a new
virtual method on `Node`:

```cpp
/// Add to \p set every code unit that can begin a match of this node.
/// \return true if the node can match without consuming input, so the
/// caller must also add the start set of what follows.
/// \p depth bounds the recursion; at kMaxStartSetDepth the caller gives
/// up (no filter).
virtual StartSetResult addStartSet(StartSet &set, unsigned depth) const;
```

`StartSetResult` is `{Consumes, Nullable, Unknown}`. A list is walked
in order: each node's set is added, and the walk stops at the first
node that returns `Consumes`. A list whose every node is `Nullable` is
`Nullable`. `Unknown` anywhere makes the whole result `Unknown`.

Rules per node, with the bytecode each emits:

| Node | Result |
|---|---|
| base `Node` (the leading no-op, `Regex.h:349`) | `Nullable`, adds nothing |
| `GoalNode` | `Nullable`. Reaching it at top level means the regex can match empty: no filter |
| `LeftAnchorNode`, `RightAnchorNode`, `WordBoundaryNode` | `Nullable`, adds nothing (zero-width) |
| `LookaroundNode`, every kind | `Nullable`, adds nothing, does not descend. A lookbehind body is reversed and reads text to the left (`Regex.h:478-491`) |
| `BackRefNode` | `Unknown`. It can repeat text captured inside a lookahead, e.g. `/(?=(a))\1/` |
| `MarkedSubexpressionNode` | the result of its contents |
| `AlternationNode` | the union over the alternatives; `Nullable` if any is; `Unknown` if any is. An alternative may be empty (`/a\|/`) |
| `LoopNode` | its loopee's set; `Nullable` if `min_ == 0` or the loopee is nullable. `{0}` adds the loopee's set anyway (a superset is safe) |
| `MatchAnyNode` | `Consumes`. dotAll: all of `ascii`, `nonASCII`. Otherwise all ASCII except 0x0A and 0x0D, plus `nonASCII` (U+2028/2029 are the other line terminators, `Executor.cpp:573-576`) |
| `MatchCharNode` | `Consumes`; see below |
| `BracketNode` | `Consumes`; see below |

Depth: there is no nesting limit, and `test/hermes/regexp.js:94` nests
50000 groups. Each descent into a child list adds one to `depth`; at
`kMaxStartSetDepth = 64` the result is `Unknown`. The parser and
`Node::compile` are iterative for the same reason
(`RegexParser.cpp:172-173`, `RegexNode.h:64-74`).

**`MatchCharNode`.** The first unit is `chars_[0]`, or its high
surrogate when it is astral under `/u`. A lone surrogate under `/u`
emits `U16MatchChar32` (`RegexNode.h:662-674`) and is non-ASCII.

- Case-sensitive: add that unit.
- `icase_` (node-local; modifier groups change it, `Regex.h:49-54`):
  `chars_[0]` is already canonical (`Regex.h:406-411`). Add every ASCII
  unit `u` with `canonicalize(u, unicode) == chars_[0]`, the executor's
  own test (`Executor.cpp:892-904`). If `chars_[0]` is non-ASCII, set
  `nonASCII`. If the regex is `/u`, set `nonASCII` too: under `/iu`,
  U+017F and U+212A fold to `s` and `k` (the only two non-ASCII-to-ASCII
  folds in `UNICODE_FOLDS`; `LEGACY_CANONS` has none). Setting it for
  every `/iu` char is a deliberate superset.

**`BracketNode`.** For each ASCII unit `u`, the bit is the executor's
answer for `u`: `bracketMatchesChar` (`Executor.cpp:722-757`) on the
emitted ranges, which under `icase` are closed with
`makeCanonicallyEquivalent` at emit time (`RegexNode.h:829-831`). The
closure moves into a `BracketNode` helper that `emitStep` and
`addStartSet` both call, so the two cannot diverge. Character classes
use the same `characterHasType`, with the input canonicalized first
under `/iu` (`Executor.cpp:738-740`). `nonASCII` is set if the bracket
is negated, if any emitted range reaches 0x80 or above, or if it has any
class other than a positive `\d` (a negative class such as `\D` or
`\W` sets it). Classes and negation are thus a superset on the
non-ASCII side and exact on the ASCII side.

### Storing it

The header is a packed 6-byte struct (`RegexBytecodeHeader`,
`include/hermes/Regex/RegexBytecode.h:285-297`). It grows by 18 bytes:

```cpp
/// Start filter flags: StartFilterPresent, StartFilterNonASCII,
/// StartFilterSingleChar.
uint8_t startFlags;
/// The only start unit when StartFilterSingleChar is set.
uint8_t startChar;
/// Bit u (u < 128) set if a match can begin with code unit u.
uint8_t startASCII[16];
```

`StartFilterSingleChar` is set when exactly one ASCII bit is set;
`startChar` is that unit. `nonASCII` may be set as well.

Instruction offsets exclude the header (`RegexBytecode.h:373-377`;
`Executor.cpp:1073`), so no jump target and no dumped offset moves.
HBC and SH store the bytes as opaque blobs (`BytecodeStream.cpp:430-438`;
`SH.cpp:192-203`). The bytes format changes, so `BYTECODE_VERSION`
goes from 100 to 101 (`include/hermes/BCGen/HBC/BytecodeVersion.h`).
SH units carry no version; generated C must be rebuilt, as for any SH
ABI change.

`dumpRegexBytecode` (`lib/Regex/RegexSerialization.cpp:342-352`) prints
a new line after `Header:` when a filter is present:

```
  StartSet: 0x61-0x63 0x6b nonascii
```

ASCII members print as hex ranges in increasing order; `nonascii` is
printed when the flag is set.

### Using it

In `Context::match` (`Executor.cpp:1115-1120`), when `!onlyAtStart` and
the filter is present, the loop advances `locIndex` to the first
position `p < charsToRight` whose unit passes the filter before
`c.setCurrentPointer`. If none does, `match` returns no match. The extra
end-of-input position (`locsToCheckCount = 1 + charsToRight`, `:1087`)
needs no check: a filtered regex cannot match empty.

- One predicate decides whether a unit passes: a unit >= 0x80 passes
  iff `nonASCII`; an ASCII unit passes iff its bitmap bit is set. Both
  input widths use it. The `char` path is not always 7-bit:
  `JSRegExp::search` passes only ASCII strings there, but
  `GraphQLDependencyExtractor.h:43` passes raw UTF-8.
- The current position's unit is tested inline. Only when it fails does
  the loop scan forward from the next position: `memchr` for
  `StartFilterSingleChar` on `char` input, otherwise a loop over the
  predicate.
- When the filter rarely rejects (e.g. `[a-z]` over lowercase text), the
  per-position test is pure overhead. Once 16 attempted positions in a
  row have passed since the last skip, the filter is turned off for the
  rest of that `match` call. A filtered search still visits a subset of the
  unfiltered positions, so A.5 holds unchanged.
- `onlyAtStart` is true for sticky, for `AnchoredAtStart`
  (`searchWithBytecodeImpl`, `:1667-1668`) and for every lookaround
  recursion (`:1437`), which also runs backwards cursors. The filter is
  never applied there. Sticky and anchored searches try one position,
  and the first instruction already rejects it cheaply.
- After the skip, the loop's own `advanceStringIndex` (`:1035-1061`)
  continues as now. Under `/u`, a scan stops on a high surrogate before
  its low surrogate whenever `nonASCII` is set, and skips both when it
  is not, so it never lands inside a pair it did not start at.
- `backtracksRemaining_` is shared across positions (`:404`); skipping
  positions only saves budget.

### A.5 Resource limits

A skipped position is not free today: it can push backtrack records, and
so spend budget (`pushBacktrack`, `Executor.cpp:457-467`) or stack depth,
before its first unit is tested, e.g. by entering a capture group or a
mandatory loop. `/(){999999999}a/.exec("b")` overflows today
(`RegexTest.cpp:97` expects overflow for `(){999999999}`) and finds no
match with the filter, because the filter skips every position.

For every unfiltered search that completes without `StackOverflow`, the
filtered search performs a subsequence of its per-position matcher work:
the positions it visits run exactly as before, with at least as much
budget left. Each position resets the cursor and `ip_`
(`Executor.cpp:1116-1120`), and an exhausted position drains its
backtrack records, restoring captures and loop data (`:803-871`). So
**the results differ only when the unfiltered search ends in
`StackOverflow`** (a `RangeError`, "Maximum regex stack depth reached",
`JSRegExp.cpp:310-311`). The filter may avoid such a limit, never cause
one. Keeping the limit exact would forbid the filter for any regex that
begins with a group, so this is accepted as the one behavior change.

`searchWithBytecode` gains a flag, `matchNoStartFilter`, that disables
the filter. Only tests use it, to compare filtered and unfiltered
results.

## Part B: builtin-exec fast paths

### B.1 The builtin exec core

`directRegExpExec` (`RegExp.cpp:899-1110`) is split:

```cpp
/// RegExpBuiltinExec up to, but not including, building the result array:
/// Get and ToLength lastIndex, search, Set lastIndex, and the unicode
/// adjustment of the ranges. \return false on no match; on a match fill
/// \p match.
static CallResult<bool> regExpBuiltinExecMatch(
    Handle<JSRegExp> regexp, Runtime &runtime, Handle<StringPrimitive> S,
    RegExpMatch &match);
```

`directRegExpExec` becomes `regExpBuiltinExecMatch` followed by the
existing array building (`:1013-1105`). The observable operations are
exactly today's: the `Get` and `ToLength` of `lastIndex` (`:918-928`),
and one `Set` of `lastIndex` to 0 or `e` (`:977-1011`). `JSRegExp::search`
still updates the legacy statics on every success
(`lib/VM/JSRegExp.cpp:376-383`).

`test` (`RegExp.cpp:1239-1247`) calls `regExpBuiltinExecMatch` and
never builds the array. It already requires a `JSRegExp` and never
looks up `exec`; that stays.

### B.2 Recognizing the builtin exec

`Runtime` saves the builtin: `regExpPrototypeExec`, a new
`RUNTIME_HV_FIELD` (`include/hermes/VM/RuntimeHermesValueFields.def`),
assigned from `defineMethod` (`RegExp.cpp:82-87`), as
`arrayPrototypeValues` is (`lib/VM/JSLib/Array.cpp:146`).

```cpp
/// \return true if \p R is a JSRegExp and Get(R, "exec") would return the
/// builtin exec from a plain data property, with no proxy, host object or
/// accessor on the lookup path.
static bool hasBuiltinExec(Runtime &runtime, Handle<JSObject> R);
```

It follows the existing precedent (`lib/VM/JSLib/HermesBuiltin.cpp:590-606`,
`Interpreter-slowpaths.cpp:330-348`): `getNamedDescriptorPredefined`
for `exec`, bail if no property, `desc.flags.proxyObject`,
`desc.flags.hostObject` or `desc.flags.accessor`, then compare the slot
value with `runtime.regExpPrototypeExec`. A data-property `Get` runs no
user code, so skipping it is unobservable. A hidden-class check alone
would not do: `RegExp.prototype.exec = f` rewrites the slot without a
new class (`lib/VM/JSObject.cpp:3254-3283`).

### B.3 Where it is used

**The exec frame stays.** Calling the builtin's effects without calling
the builtin would drop the native `exec` frame. Stack traces record it
(`JSError.cpp:355`, `:708`), both for user code that runs inside exec
(`ToLength` of a `lastIndex` object, `RegExp.cpp:923`) and for errors the
engine raises there: the `TypeError` of a non-writable `lastIndex`
(`:977`, `:1005`), and the regex limit `RangeError`. So every site that
today reaches the builtin through `executeCall1` (inside `regExpExec`,
`RegExp.cpp:1168`) runs its builtin-exec work inside a frame for the
builtin, built the way a call builds it. Those sites are `regExpExec`'s
direct dispatch, the global `@@match` loop, `@@replace` phase 1 and
`@@search`. Sites that call the C++ exec directly today and so have no
exec frame keep none: `test` (`RegExp.cpp:1239-1247`), the fallback for
a non-callable `exec` (`:1187-1194`), and the shared core itself (no
double frame). The frame is:

- a `ScopedNativeCallFrame` with one argument, callee
  `runtime.regExpPrototypeExec`, `this` = `R` and argument `S`, with its
  overflow check, exactly as `Callable::executeCall1` builds it
  (`lib/VM/Callable.cpp:322-339`);
- then, as `Callable::call` does before dispatch, `potentiallyMoveHeap()`
  once the frame's arguments are set (`include/hermes/VM/Callable.h:282`),
  with the callee rooted and reloaded across it and across the body;
- then the body of `NativeFunction::_nativeCall`
  (`include/hermes/VM/Callable.h:730-764`): the native depth tracker,
  `validateSavedIPBeforeCall`, `setCurrentFrameToTopOfStack`, and
  `restoreStackAndPreviousFrame`, plus the `HERMESVM_PROFILER_NATIVECALL`
  counters. `_nativeCall` is split into a template that runs a given body
  inside that setup, and itself calls it with `functionPtr_`, so the fast
  paths share the code instead of copying it.

What the fast paths skip is the `exec` lookup, the dispatch through
`Callable::call`, the native's `ToString` of a string, and the result
array. `NativeFunction::_callImpl` only forwards to `_nativeCall`
(`lib/VM/Callable.cpp:1189`), and debugger and sampling-profiler stack
walks read the runtime frames (`Debugger.cpp:649`,
`SamplingProfiler.cpp:81`), so the frame is the same to all of them.

**`regExpExec`** (`RegExp.cpp:1149-1195`) keeps its `Get(R, "exec")`.
When the value is `runtime.regExpPrototypeExec` and `R` is a
`JSRegExp`, it runs `regExpBuiltinExec` in the exec frame instead of
`executeCall1`. The native exec would only run `ToString` on
a string (a no-op) and the same `directRegExpExec`. This covers
non-global `@@match`, `@@search` and the `matchAll` iterator
(`JSRegExpStringIterator.cpp:87`).

**Loops.** The other callers run `regExpExec` in a loop, or once, with
no user code between iterations: each iteration's only other operations
are `ToString` of a string from the builtin exec, and a `Get`/`ToLength`
of a `lastIndex` that holds a number the builtin exec wrote. So `exec`
cannot change during the loop, and one `hasBuiltinExec` check, made
after the last user-code step before the loop, covers every iteration.

- **Global `@@match`** (`RegExp.cpp:1781-1917`). After `Get global`,
  `Get unicode` and `Set lastIndex = 0` (`:1811-1834`), if
  `hasBuiltinExec(rx)`, the loop runs `regExpBuiltinExecMatch` in the
  exec frame and
  appends a slice of range 0 to `A`. Empty matches do the same
  `Get`/`ToLength`/`advanceStringIndex`/`Set` of `lastIndex` as now
  (`:1889-1910`).
- **`@@replace`** (`RegExp.cpp:1988-2374`). After `ToString(S)`,
  `ToString(replaceValue)` when not callable, `Get global`,
  `Get unicode` and the `Set` (`:2021-2068`), if `hasBuiltinExec(rx)`

  phase 1 runs each builtin exec in the exec frame and stores each `RegExpMatch` in a C++ vector instead of a result
  array. Phase 2 is factored into one step per match that takes
  `matched`, `position`, the captures list and `namedCaptures`. The
  slow path extracts them from the result array with today's `Get`s
  (`:2157-2264`); the fast path builds them from the ranges:
  `matched` and the captures are slices (`undefined` for unmatched
  groups), `position` is range 0's location, and `namedCaptures` is a
  groups object built from the regexp's group-name mapping
  (`getGroupNameMappings`, `JSRegExp.cpp:261-267`) and the captures.
  `buildGroupsObject` (`RegExp.cpp:694-763`) is generalized to read
  captures from either source. The `Get`s it replaces are on own
  properties of an array that user code never sees, so they are
  unobservable. The callback still runs in phase 2, after every exec,
  as now.
- **`@@search`** (`RegExp.cpp:1920-1985`). Between its two `Set`s of
  `lastIndex`, if `hasBuiltinExec(rx)`, it runs
  `regExpBuiltinExecMatch` in the exec frame and returns range 0's location, instead of
  `Get(result, "index")` on an array.

`lastIndex` is still read and written exactly as today on every path.
If `lastIndex` is non-writable, the `Set` before each loop throws
TypeError as now (`setLastIndex`, `lib/VM/JSLib/JSLibInternal.h:300-311`;
`Runtime.cpp:211-243`).

### B.4 Fewer copies per search

`performSearch` (`JSRegExp.cpp:293-332`) allocates a
`std::vector<regex::CapturedRange>` per search and copies it into a
`RegExpMatch` (`SmallVector<OptValue<RegExpMatchRange>, 4>`,
`include/hermes/VM/RegExpMatch.h:30`). The vector becomes a
`Runtime`-owned buffer that keeps its capacity; it is used only between
the executor's return and the copy into the `RegExpMatch`, and the
executor runs no JS and no GC allocation, so it is never reentered. The
`@@split` loop moves its `RegExpMatch` instead of copying it
(`RegExp.cpp:2531`).

### GC safety

- `RegExpMatch` holds integers; it survives any allocation.
- Slices come from `S`, which `JSRegExp::search` flattens
  (`StringPrimitive::createStringView` → `ensureFlat`), and `slice`
  reads the characters after it allocates (`StringPrimitive.cpp:350-382`).
- No raw character pointer or `StringView` iterator is held across a
  `slice`, `setElementAt`, `push_back` or call.
- Loops keep today's `GCScope` markers; the phase-2 step keeps
  `marker1`/`marker2` (`RegExp.cpp:2154`, `:2226`).
- The C++ vector of matches in `@@replace` is malloc'd; it is not
  reachable from the GC and holds no GC pointers.
- `RegExpMatch` vectors and `Locals` are destroyed normally; nothing
  RAII spans a call that can throw by longjmp beyond what the existing
  code already holds.

## Tests

### Part A

- **Header contents** (`unittests/Support/RegexTest.cpp`, which builds
  `cregex(u"pat", u"flags")` and can read the header from `compile()`):
  a table of patterns with the expected `startFlags`, `startChar` and
  bitmap. It includes: a literal (`/abc/` → single char `a`); `/i`
  (`/a/i` → `a`, `A`); `/iu` with `s` and `k` (nonASCII set); `/i`
  without `u` on `s` (nonASCII clear); `ſ` under `/i` (nonASCII only);
  alternation (`/ab|cd/`); an empty alternative (`/a|/`, no filter);
  `x*y` (both); `x+y` (`x` only); a leading anchor, `\b`, lookahead and
  lookbehind (transparent); a back-reference first (no filter); `.`
  with and without `s`; brackets, negated brackets, `\d`, `\w`, `\s`,
  `[^a]`; an astral literal under `/u` (nonASCII); `(?i:a)b` and
  `a(?i:b)` (node-local icase); 100 nested groups (no filter, depth
  cap); `/^a/` (anchored; set computed but never used).
  Fails before: the fields do not exist.
- **Differential** (same file): a corpus of the patterns above plus
  `\s`-heavy, multiline and sticky ones, each searched over ASCII
  `char` input and UTF-16 input (with U+017F, U+212A, U+2028, a
  surrogate pair, a lone surrogate, and a start index inside a pair),
  at every start index, with and without `matchNoStartFilter`. The
  helper compares the whole `MatchRuntimeResult` and the captures (the
  existing boolean helper, `RegexTest.cpp:22-30`, folds overflow into
  no match). They must be equal unless the unfiltered result is
  `StackOverflow` (A.5); `/(){999999999}a/` over `"b"` is in the corpus
  and pins that case: unfiltered overflows, filtered is `NoMatch`. Mutations it must catch: drop the other
  case of an `/i` letter; clear `nonASCII` for `/iu` `s`; treat a
  lookahead's body as consuming; let `MatchAny` exclude a non-ASCII
  unit; apply the filter when `onlyAtStart`.
- **Dump** (`test/hermes/regexp_bytecode.js`): its 28 `Header:` CHECK
  lines gain the `StartSet:` line where a filter exists; at least one
  case each for single char, a range, and `nonascii`. Fails before: the
  line is not printed.
- **Existing**: `test/hermes/regexp*.js`, `test/hermes/regex*.js`, the
  50000-deep nesting at `regexp.js:94`, and the whole `check-hermes`.

### Part B

A new `test/hermes/regexp-exec-fast-path.js`, run under `%hermes`,
`%hermes -lazy`, `-gc-sanitize-handles=1` and `%shermes -exec`. Each
case states the observable that the fast path must not change:

- `RegExp.prototype.exec` replaced by a counting function: global
  `match`, `replace` (string and callback), `search` and non-global
  `match` call it, with the right arguments and count. An own `exec` on
  the instance, and on a subclass prototype, likewise.
- `exec` as an accessor on `RegExp.prototype`: the getter runs once per
  iteration, as now.
- A regexp whose prototype is a `Proxy` of `RegExp.prototype`: the
  proxy's `get` trap sees each `exec` lookup.
- A frozen global regexp: `match` and `replace` throw TypeError from
  the pre-loop `Set`, so the error's stack has no `exec` frame.
- `lastIndex` holding an object with `valueOf` on a non-global regexp:
  `valueOf` runs once per `exec`/`test`, as now.
- Legacy statics (`RegExp.$1`, `lastMatch`, `leftContext`,
  `rightContext`) after `test`, global `match`, `search` and `replace`,
  and inside a replace callback.
- Global `match` and `replace` with empty matches, under `/u` over a
  surrogate pair (advance by code point) and without `/u`.
- Sticky plus global `match` and `replace`.
- `replace` with `$<name>`, `$1`, `` $` ``, `$'`, `$&`, and a callback
  receiving captures, position, the string and `groups`; unmatched
  groups as `undefined`.
- `hasIndices` on non-global `match` (array built in full).

- Stack observation, each showing the native `exec` frame as today:
  a non-global regexp whose `lastIndex` is an object whose `valueOf`
  records `new Error().stack`, under `match` and `replace` (`search`
  overwrites `lastIndex` with 0 before exec, `RegExp.cpp:1953`, so it
  never coerces it); a frozen sticky non-global regexp under `match`,
  whose exec `Set` throws; a global `match` whose regex hits the stack
  limit inside exec; and `@@search` with `/(){999999999}/`, whose limit
  `RangeError` comes from inside exec (the pattern is nullable, so the
  start filter does not apply). `test` with a `lastIndex.valueOf` still
  shows no `exec` frame, as today.

Every case has a mutation that it, and not only some other case, fails
under:

| Case | Mutation |
|---|---|
| replaced / own / subclass `exec` | `hasBuiltinExec` always true |
| `exec` accessor | `hasBuiltinExec` does one ordinary `Get`, accepts an accessor that returns the builtin, and the result is hoisted out of the loop (a counting accessor that returns the builtin then sees 1 lookup instead of one per iteration). Deleting the `desc.flags.accessor` test alone is masked: the slot holds a `PropertyAccessor`, never equal to the builtin (`JSObject.cpp:3278`) |
| `Proxy` prototype | the `proxyObject` test removed |
| frozen regexp | the fast path skips the pre-loop `Set`; the TypeError then comes from inside exec and its stack gains the `exec` frame |
| `lastIndex.valueOf` count | `regExpBuiltinExecMatch` skips `ToLength` for a non-global, non-sticky regexp |
| stack observation | each framed site in turn (direct dispatch, `@@match`, `@@replace`, `@@search`) runs the core without the exec frame; `test` given a frame |
| statics | `test` searches without recording statics |
| empty matches under `/u` | the fast loop advances by one code unit regardless of `unicode` |
| sticky plus global | the fast loop clears sticky before searching |
| replacement patterns and callback arguments | phase 2 uses range 0 for every capture; unmatched groups become `""` |
| `hasIndices` | non-global `match` takes the array-free path |

Each mutation is run and its failing case recorded in the plan.

## Measurements

The benchmarks of the Problem section are re-run after each part, on the
same release build configuration, best of three, and reported in the
commit message. There is no pass/fail threshold.

## Risks

- **A superset rule written as a subset loses matches.** The
  differential test is the guard; the case analysis for `icase` and
  brackets reuses the executor's own predicates rather than restating
  them.
- **Bytecode format.** Old HBC files are rejected by the version check;
  old SH-generated C is not checked and would misread the header.
- **Fast paths and user code.** B relies on no user code running
  inside the loops. Every user-code step (`ToString`, `Get global`,
  `Get unicode`, `Get flags`) happens before the check; the callback of
  `@@replace` runs after phase 1. A future edit that adds user code
  inside a loop must move the check.
