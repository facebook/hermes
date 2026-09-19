# ICU-free `normalize` on Linux

Date: 2026-08-26
Status: approved, ready for implementation planning

## Context

On Linux, Hermes depends on ICU4C for exactly four functions, all in
`lib/Platform/Unicode/PlatformUnicodeICU.cpp`:

| Function | ICU symbols | JS surface |
| --- | --- | --- |
| `localeCompare` | `ucol_open`, `ucol_setAttribute`, `ucol_strcoll`, `uloc_getDefault` | `String.prototype.localeCompare` |
| `dateFormat` | `udat_open`, `udat_format`, `udat_close`, `uloc_getDefault` | `Date.prototype.toLocale{,Date,Time}String` |
| `convertToCase` | `u_strToUpper`, `u_strToLower`, `uloc_getDefault` | `String.prototype.to{Upper,Lower}Case` (non-ASCII only) and `toLocale{Upper,Lower}Case` (always) |
| `normalize` | `unorm2_get{NFC,NFD,NFKC,NFKD}Instance`, `unorm2_normalize` | `String.prototype.normalize` |

`BUCK:858` selects the ICU backend for Linux and `BUCK:1116-1117` pulls in the
checked-in ICU 77.1 from `//third-party/icu`. CMake hard-errors when no ICU is
found (`CMakeLists.txt:686`).

Intl is not a factor on Linux. `-DHERMES_ENABLE_INTL` is set only for Android
(`BUCK:964-966`), the internal build pins `intl_enabled = "0"` (`BUCK:1818`),
and CMake defaults `HERMES_ENABLE_INTL` to OFF (`CMakeLists.txt:288`), so
`PlatformIntlICU.cpp` and `lib/Platform/Intl/impl_icu/` never compile there.

Removing ICU drops a large dependency, removes the `shermes` shipping tax at
`BUCK:1320-1345` (which copies the entire ICU library directory next to
`shermes` output), and retires the license obligation tracked in
`doc/icu_license.txt`.

### Why not libc

`strcoll_l` and friends were considered as a replacement for the locale-aware
functions and rejected for `localeCompare` on correctness grounds: it is
NUL-terminated so embedded U+0000 truncates the comparison (ICU's
`ucol_strcoll` takes explicit lengths, `PlatformUnicodeICU.cpp:63-69`),
unpaired surrogates have no valid UTF-8 encoding, and glibc performs no
normalization preprocessing, silently dropping the `UCOL_NORMALIZATION_MODE`
behavior the current code deliberately opts into at
`PlatformUnicodeICU.cpp:50`. It is also non-deterministic across machines,
degrading to byte order on musl and in images without a locale archive.
`dateFormat` has no table-driven alternative and will use `strftime_l`, which
is addressed in its own spec.

## Scope

The ICU removal is decomposed into four sub-projects, each with its own spec,
plan, and diff stack:

1. **`normalize`** (this spec)
2. `convertToCase`
3. `localeCompare`, which reuses the normalization tables from (1) to preserve
   the normalization-insensitive comparison ICU gives today
4. `dateFormat`

ICU stays linked until (4) lands; the dependency drop is a single change at the
end.

### Non-goals

- Changing the Android (Java) or Apple (CoreFoundation) backends. These have
  real platform normalizers and do not compile the tables.
- Regenerating `lib/Platform/Unicode/UnicodeData.inc`.
- Implementing Intl.
- Any behavior change on a platform that already had a working `normalize`,
  beyond the UCD version skew described below. Note that LITE and Emscripten
  did **not** have one — both were no-ops — so for them this is a deliberate
  and substantial behavior change, from returning the input untouched to
  actually normalizing it.

### UCD version skew against ICU

The checked-in ICU 77.1 implements Unicode 16.0
(`third-party/icu/72.1/v77.1/common/unicode/uchar.h:64`), while
`genUnicodeTable.py` is pinned to UCD 17.0.0. Output is therefore *not*
byte-identical to ICU in every case.

The divergence is tightly bounded. Unicode's Normalization Stability Policy
guarantees that the decomposition of an already-encoded character never
changes and that no decomposition is ever added to an existing character, so
every code point assigned in 16.0 normalizes identically under both. The only
possible differences are code points newly assigned in 17.0 that carry a
decomposition mapping: ICU sees them as unassigned and passes them through,
whereas these tables decompose them correctly.

This is the correct direction to diverge, and the codebase already has this
skew — `UnicodeData.inc` has been generated at 17.0.0 against the same
Unicode 16.0 ICU since 2026-04-30. It does mean the conformance suite is the
authority for correctness, not a differential comparison against ICU.

## Increment model

The normalizer is a plain function, `hermes::unicode::normalizeUTF16`, with no
build mode of its own. Every backend for a platform that lacks a
higher-quality normalizer calls it. That is a wider set than Linux and Windows:
`PlatformUnicodeLite.cpp` and `PlatformUnicodeEmscripten.cpp` both had
`normalize` as an outright no-op, and LITE's documented purpose in BUCK and
CMake is to avoid depending on system libraries rather than to be small, so a
self-contained normalizer is exactly what it always wanted. Android and Apple
keep their platform normalizers and never compile it.

For Linux and Windows specifically, a new `HERMES_PLATFORM_UNICODE_HERMES`
backend becomes the default in the last diff of this sub-project. It calls the
shared normalizer and forwards the other three functions to the existing ICU
code. Each later sub-project replaces one forwarder; the final one deletes the
forwarding layer and the `//third-party/icu` dependencies.

This means every increment is covered by the existing test262 and lit runs the
moment it lands, with no parallel build configuration to maintain. The cost is
a transient `#include` of the ICU headers in the new backend for three diffs.

Once all four functions are native, `HERMES_PLATFORM_UNICODE_LITE` and
`HERMES_PLATFORM_UNICODE_HERMES` become the same thing and should be merged.

The shared function is named `normalizeUTF16` rather than `normalize` because
`NormalizationForm` lives in `hermes::unicode`, so argument-dependent lookup
finds that namespace from every unqualified `normalize(buf, form)` call and
collides with the identically-signatured `platform_unicode::normalize`.

## Approach

Sorted-range tables with binary search, reusing the `std::binary_search` and
`UnicodeRangeComp` idiom already established in `CharacterProperties.cpp:32`.

Rejected alternatives:

- **Two-stage trie (ICU `UTrie` shape).** O(1) lookups and likely smaller for
  the dense CCC data, but introduces a data structure found nowhere else in the
  codebase and complicates the generator, for a function that is not hot. If
  `localeCompare` profiling later demands faster CCC lookups, a single table
  can be upgraded in isolation without touching the algorithm.
- **Porting a third-party normalizer's tables (utf8proc-style).** Adds a
  dependency in order to remove one, with its own licensing and UCD-version
  story.

Two optimizations are included because they are nearly free:

- **Quick_Check fast path.** If every character is `QC=Yes` for the target form
  and no canonical ordering violation is present, return the input untouched
  with no allocation. This is the common case and yields an all-ASCII fast path
  for free, since ASCII is uniformly `QC=Yes` and `CCC=0`.
- **Pre-expanded decompositions.** Recursive decomposition is resolved at
  table-generation time, so the runtime never recurses.

The tables were estimated at 40-60 KB before they existed. The straightforward
encoding measured 147,920 bytes, roughly two and a half times that, with the
two decomposition tables and their pool accounting for 120 KB of it. Note that
the mitigation this spec originally proposed, a trie for the CCC and
Quick_Check tables, targeted the wrong tables: those are only 13 KB combined.

Three changes brought it to **86,336 bytes**, a 42% reduction:

- `DecompEntry` packed into 8 bytes rather than the natural 12, as
  `uint32_t cp : 21; uint32_t length : 5; uint32_t wide : 1; uint32_t offset;`.
- `COMPAT_DECOMP` stores only the 3,849 mappings that differ from their
  canonical counterpart, down from 5,914, with lookup falling back to
  `CANON_DECOMP`.
- Sequences interned into a `char16_t` pool (6,542 values) or, when they
  contain a supplementary-plane code point, a `char32_t` one (221 values).

86 KB is a rounding error against the roughly 30 MB of ICU it displaces on
Linux and Windows, and Android and Apple do not compile it at all.

## File structure

New files:

```
include/hermes/Platform/Unicode/UnicodeNormalization.h
lib/Platform/Unicode/UnicodeNormalization.cpp
lib/Platform/Unicode/NormalizationData.inc                  generated, checked in
lib/Platform/Unicode/PlatformUnicodeHermes.cpp
lib/Platform/Unicode/PlatformUnicodeICUImpl.h               temporary
unittests/PlatformUnicode/NormalizationConformanceTest.cpp
unittests/PlatformUnicode/NormalizationTestData.inc         generated, checked in
```

`UnicodeNormalization.cpp` includes `NormalizationData.inc` inside its
namespace, mirroring `CharacterProperties.cpp:20`.

### `NormalizationForm` placement

`NormalizationForm` currently lives in `PlatformUnicode.h`, the backend
*selection* header. Having the low-level normalizer include that header would
invert the layering, so the enum moves into `UnicodeNormalization.h` and
`platform_unicode` retains a `using NormalizationForm = unicode::NormalizationForm;`
alias. Every existing call site continues to compile unchanged.

**The enumerator order must not change.** `PlatformUnicodeJava.cpp` passes the
ordinal across JNI; this is the constraint the comment at `PlatformUnicode.h:64`
refers to, and `AndroidUnicodeUtils.java` depends on it.

### ICU delegation shim

Rather than add a source file, `PlatformUnicodeICU.cpp` keeps its four function
bodies but moves them into `namespace icu_impl`, guarded by
`#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_ICU || HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_HERMES`.
The public `platform_unicode` entry points remain in the same file behind
`#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_ICU`.
`PlatformUnicodeICUImpl.h` declares the `icu_impl` functions so
`PlatformUnicodeHermes.cpp` can call them. Both are deleted by sub-project (4).

## Generator changes

`utils/genUnicodeTable.py` is extended rather than supplemented with a second
script, so the UCD version pin and the download cache stay in one place. That
is what makes it impossible for the tables and the conformance data to
reference different UCD versions.

- Add `NormalizationTest.txt` to `UnicodeDataFiles.URLS` at the existing pinned
  `VERSION = "17.0.0"`. `UnicodeData.txt` and `DerivedNormalizationProps.txt`
  are already fetched.
- Add `--table {properties,normalization,normtest}`, defaulting to `properties`
  so the documented `genUnicodeTable.py | clang-format > UnicodeData.inc`
  invocation is unchanged.
- Emit the same SHA1 and version provenance block that `UnicodeData.inc`
  already carries into every generated file, so a partial UCD bump is visible
  in review.

`UnicodeData.inc` is not regenerated. It was generated 2026-04-30 at UCD
17.0.0; regenerating it risks shifting RegExp property-escape ranges and
churning unrelated tests. The new tables are generated at that same pinned
version into separate files.

### Extracted data

From `UnicodeData.txt`:

- Canonical decompositions (field 5, no `<tag>`), recursively pre-expanded.
- Compatibility decompositions (field 5, with `<tag>`), recursively
  pre-expanded, and including the canonical mappings so that NFKD is a superset
  of NFD.
- Canonical combining class (field 3).

From `DerivedNormalizationProps.txt`:

- `Full_Composition_Exclusion`.
- `NFC_QC`, `NFD_QC`, `NFKC_QC`, `NFKD_QC`, each Yes, No, or Maybe.

The canonical composition table is the inverse of the canonical decomposition
mappings, minus full composition exclusions, singleton decompositions, and
non-starter decompositions (those whose first character has `CCC != 0`).

Hangul is handled arithmetically per UAX #15 and contributes no table entries.

### Runtime representation

- Decomposition mappings: `CANON_DECOMP` and `COMPAT_DECOMP`, each sorted by
  code point, each entry packed into 8 bytes holding the code point, a length,
  a `wide` bit selecting the pool, and an offset into it. `COMPAT_DECOMP` omits
  any mapping equal to the canonical one, so a compatibility lookup that misses
  falls back to `CANON_DECOMP`.
- Decomposition pools: `DECOMP_POOL16` (`char16_t`) and `DECOMP_POOL32`
  (`char32_t`), with identical sequences interned across both tables.
- CCC and the four Quick_Check properties: sorted range arrays, `CCCRange` and
  `NormRange`. These deliberately do not reuse `UnicodeRange`, which is
  declared inside `UnicodeData.inc` and so is not visible outside
  `CharacterProperties.cpp`; keeping the two generated files independent means
  neither can break the other.
- Canonical composition: array sorted by `(starter, combining)` yielding the
  composite.

## Algorithm

Standard UAX #15:

1. Quick-check the input against the target form's `*_QC` property and
   canonical ordering. If it is already normalized, return with no allocation.
2. Decompose into a `SmallVector<char32_t>` scratch buffer, emitting each code
   point's pre-expanded mapping for the form's decomposition type, or the code
   point itself. Hangul syllables decompose arithmetically.
3. Canonically order: stable-sort each maximal run of `CCC != 0` by CCC.
4. For forms C and KC, apply the UAX #15 canonical composition algorithm.
   Hangul composes arithmetically.
5. Re-encode to UTF-16 into the caller's buffer.

### Correctness requirements

These are the WTF-16 hazards that motivated rejecting the libc approach, and
they must hold here:

- **Unpaired surrogates pass through unchanged.** They have `CCC=0`, `QC=Yes`,
  and no decomposition. The implementation must not crash, drop them, or
  substitute U+FFFD.
- **Embedded NUL is a non-event.** The API operates on
  `SmallVectorImpl<char16_t>` with an explicit length; no code path may treat
  U+0000 as a terminator.
- **Supplementary-plane decompositions work.** Surrogate pairs are decoded to
  code points before lookup and re-encoded afterwards, so characters such as
  the musical symbols at U+1D15E decompose correctly rather than being skipped.

## Testing

- **Conformance suite.** `NormalizationConformanceTest.cpp` drives all 20,034
  rows of `NormalizationTest.txt`, asserting the UAX #15 invariants across all
  four forms, including the Part 1 invariant that code points not listed in the
  file normalize to themselves. The rows are emitted as string literals in the
  upstream format and parsed by the test rather than pre-expanded into arrays:
  that is a third the source size, compiles far faster, stays greppable against
  the upstream file, and lets a failure print the row verbatim.
- **A populated-data guard.** `TestDataIsPopulated` asserts the generated
  tables are non-empty, so a generator bug cannot make every other assertion in
  the file pass vacuously.
- **Targeted gtests** for each correctness requirement above, plus the empty
  string, a string that grows under NFD, one that shrinks under NFC, Hangul
  round-tripping, and the compatibility-to-canonical fallback.
- **test262** `String.prototype.normalize` through the existing runner. Because
  the Hermes backend becomes the Linux default in the final diff of this
  sub-project, this exercises the new code end to end.

A conformance suite that cannot fail is worthless, so the suite is checked
against a deliberately broken build: short-circuiting `composeInPlace` must
produce mismatches. It does, from row 0, failing three tests.

## Diff breakdown

1. Generator: `--table` flag and `NormalizationTest.txt` fetch; both generated
   files checked in. No behavior change.
2. `UnicodeNormalization.{h,cpp}` and the conformance test, wired into the LITE
   and Emscripten backends, whose `normalize` was a no-op. Linux and Windows
   still use ICU at this point.
3. `PlatformUnicodeICU.cpp` refactored into `icu_impl` plus
   `PlatformUnicodeICUImpl.h`. Pure refactor, no behavior change.
4. `PlatformUnicodeHermes.cpp` and the BUCK and CMake wiring. Linux and Windows
   switch to the new backend; `normalize` is table-driven and the other three
   functions forward to ICU. The full test262 run is the gate.

## Risks

- ~~**Table size exceeds the estimate.**~~ Realized and resolved: the tables
  measured 147,920 bytes against a 40-60 KB estimate, and were compacted to
  86,336 bytes. They are compiled only for the backends that call them, so
  Android and Apple pay nothing. See the Approach section.
- **Quick_Check fast path is subtly wrong**, returning unnormalized strings
  unchanged. Mitigation: the conformance suite runs every case through the full
  path and the fast path; a `QC=Maybe` code point must never take the fast
  path.
- **UCD version drift** between `UnicodeData.inc` and the new tables. Mitigation:
  the shared version pin and the provenance block in every generated file.
- **A test262 case covers a code point newly assigned in UCD 17.0** and encodes
  the Unicode 16.0 answer, producing a spurious failure against the ICU
  baseline. Mitigation: expected to be unlikely, since the test262 normalize
  tests are largely version-agnostic. If it occurs, the resolution is to keep
  the 17.0 behavior and record the divergence, not to regress the tables.

## Success criteria

- The conformance suite passes for all four normalization forms.
- test262 `String.prototype.normalize` results are unchanged from the ICU
  baseline, or any difference is traced to the documented UCD 17.0 versus
  Unicode 16.0 skew and accepted.
- `check-hermes` and `//xplat/static_h:testsuite_tests` pass on Linux with the
  Hermes backend as the default.
- `normalize` no longer calls any ICU symbol on Linux or Windows.
