# ICU-free `localeCompare`

Date: 2026-08-27
Status: approved, ready for implementation planning

Sub-project 3 of 4 in the ICU4C removal. See
`doc/superpowers/specs/2026-08-26-icu-free-normalize-design.md` for the overall
decomposition and the shared conventions this follows.

## Context

`localeCompare` backs `String.prototype.localeCompare` and is the third of the
four functions Hermes still needs ICU for on Linux and Windows. There is
exactly one caller, `String.cpp:1323`, and when `HERMES_ENABLE_INTL` is on it
never runs at all: `Intl.Collator` takes over at `Intl.cpp:1625`. Intl is
Android-only, so this spec concerns the non-Intl path.

### What the current implementation does

`PlatformUnicodeICU.cpp:37-58` opens one `UCollator` for `uloc_getDefault()`,
falling back to the root locale, and sets `UCOL_NORMALIZATION_MODE` to
`UCOL_ON` so that canonically equivalent strings compare equal.
`ucol_strcoll` then takes explicit lengths, which is why embedded U+0000 is
handled correctly today.

The buck build passes ICU the **full** `icudt` rather than the reduced
English-only data file (`BUCK:1923`, `//third-party/icu:icudt`), so the CLDR
locale tailorings really are present at runtime: under `LANG=sv_SE` a Linux
user currently gets Swedish collation, in which `ä` sorts after `z`.

### Why not libc

Recorded in the normalize spec and not relitigated here. `strcoll_l` is
NUL-terminated, so embedded U+0000 truncates the comparison; unpaired
surrogates have no valid UTF-8 encoding; glibc does no normalization
preprocessing, silently dropping the `UCOL_NORMALIZATION_MODE` behavior; and
results vary with the host's installed locale archive, degrading to byte order
on musl and in minimal containers.

### Existing coverage

`test/hermes/string-functions.js:596-613` is the live test. It runs under
`LANG=en_US.UTF-8` and covers six cases, written here in escape form because
the interesting pairs are visually identical: `'abc'` against itself and
against `'def'`; `'a'` against `'Z'`; precomposed `'\u00F6'` against
decomposed `'o\u0308'`; `'a\u0323\u0308'` against
`'a\u0308\u0323'`; the Angstrom sign `'\u212B'` against
`'A\u030A'`; and diacritic reordering, `'S\u0323\u0307'`
against `'S\u0307\u0323'`.
`test/BCGen/HBC/deltamode/string-functions-update.js:454-471` is a near-copy for
delta-bytecode mode. Both must keep passing.

`test/hermes/string-locale.js` covers only casing; it does **not** test
`localeCompare`, so the locale-tailoring behavior described above is untested.
`utils/testsuite/skiplist.json` has no `localeCompare` entries, so whatever
test262 covers today passes today. There are no unit tests.

## Scope

Replace `localeCompare` with a self-contained UTS #10 root collation on the
Hermes and LITE backends.

### Non-goals

- Changing the Android, Apple or Emscripten backends. Emscripten defers to the
  host JavaScript engine's collator, which is better than anything shipped
  here.
- Implementing Intl or ECMA-402 `Intl.Collator`.
- Honoring the `locales` argument. ECMA-262 directs a non-ECMA-402
  implementation to the host environment's current locale, and this
  implementation has no locale at all; see below.
- CLDR locale tailorings.
- Regenerating `UnicodeData.inc`.
- Removing the ICU dependency, which happens in sub-project 4.

### Deliberate behavior changes

- **Root collation replaces host-locale tailoring on Linux and Windows.** A
  user under `LANG=sv_SE` stops getting Swedish ordering. Tailorings are
  roughly a hundred locales of CLDR rule data with their own rule parser and no
  conformance suite, which is a larger project than the other three
  sub-projects combined. ECMA-262 without Intl makes the result
  implementation-defined, and no test covers the tailored behavior.
- **LITE gains real collation.** It compares UTF-16 code units today
  (`PlatformUnicodeLite.cpp:15`), so this is a substantial improvement. LITE's
  contract is *no dependencies*, not small size, and it is expected to become
  unnecessary now that the Hermes backend exists; table size is therefore not
  an argument against wiring it in.
- **Ordering diverges from ICU on the CLDR root deltas.** ICU uses CLDR root,
  which adjusts DUCET in a handful of documented ways, mostly the placement of
  variable characters and a few script reorderings. Letters and digits are
  unaffected.

## Approach

DUCET root collation per UTS #10, three levels, non-ignorable variable
weighting.

Non-ignorable is ICU's default and is also the simpler algorithm: variable
characters keep their primary weights, so there is no fourth level and no
shifting logic.

Rejected alternatives:

- **NFD followed by code point comparison.** Costs no new data at all and
  satisfies the ECMA-262 recommendation that canonically equivalent strings
  compare equal, but makes `'a'.localeCompare('B')` positive. Sorting
  user-visible strings is the overwhelmingly common use of this function, and
  this would leave Linux as the only platform without real collation.
- **Primary weights only, tie-breaking by code point.** Roughly a third the
  table size and gets base-letter ordering right, but accent and case ordering
  fall back to code point order, and it fails the UCA conformance suite by
  construction, leaving this project's "show the suite fail" discipline with
  nothing to hang on.
- **`FractionalUCA.txt` from `third-party/tp2/icu/78.1rc`.** Already in the
  repo at exactly UCA 17.0.0, and reproduces ICU's ordering rather than raw
  DUCET. Rejected because the format is ICU-internal — variable-length
  fractional primaries, script reorder groups, prefix rules — so parsing it
  means reimplementing part of ICU's data model instead of the published
  algorithm, and its matching conformance suite is CLDR's, which still has to
  be fetched.
- **`allkeys.txt` from `third-party/tp2/nunicode`.** Also already in the repo
  and needs no download, but it is DUCET 13.0.0. It would pair 13.0.0
  collation weights with 17.0.0 normalization and casing tables in one binary,
  and validating it would need a 13.0.0-era conformance file, which is itself a
  fetch. It trades the blocker for a permanent inconsistency.

### Input data

`allkeys.txt` and the `CollationTest_*.txt` conformance files from
`https://www.unicode.org/Public/UCA/17.0.0/`. Neither is in the repo and
unicode.org is blocked by the agent proxy, so both are fetched externally and
unpacked into `/tmp/ucd` alongside the UCD files, exactly as the UCD data was.
Their SHA1s are recorded in the generated file's provenance header:

```
7843ad89a16c33ba1ff5140eb3e2cee4cc6886e5  allkeys.txt
07b4ea4038f8621cd6e6d503ebd78ea64ce921a8  CollationTest_NON_IGNORABLE_SHORT.txt
9f95424a421d150aca5d8187936d8aff835f3de6  CollationTest_NON_IGNORABLE.txt
f8854c3264f54301ebb1160784162fb51b92b3be  CollationTest_SHIFTED_SHORT.txt
65aae53557ea9403ac9d692180e66abeb55e9097  CollationTest_SHIFTED.txt
```

UCA 17.0.0 matches the UCD 17.0.0 the other tables are pinned to. The
checked-in ICU 77.1 implements Unicode 16.0, so the same version skew already
documented for normalize applies here; our data being newer is the correct
direction.

### Shape of the data, measured

From `allkeys.txt` 17.0.0, before any compression:

| | Count |
| --- | --- |
| Mapping lines | 39,749 |
| Single code point, one collation element | 34,787 |
| Single code point, expansion to 2-18 elements | 3,998 |
| Contractions, all 2 or 3 code points | 964 |
| `@implicitweights` ranges (Tangut, Nushu, Khitan) | 6 |
| Distinct (variable, secondary, tertiary) triples | 307 |

Han is **not** listed and must come from the implicit weight formula; there is
no large block of algorithmically-derivable entries to drop, which is what the
earlier 60 KB end of the estimate assumed.

Two properties of the data drive the encoding. Secondary and tertiary weights
are drawn from a tiny alphabet — 307 distinct combinations, with
`(non-variable, 0x0020, 0x0002)` alone covering 21,229 entries — so they
compress to a one-byte index into a palette. And primary weights advance in
step with the code point within a script block, so the single-element mappings
collapse from 34,787 entries to 6,655 runs.

### Measured table size

**123 KB** for a first-cut encoding, against the roughly 150 KB threshold at
which the fidelity decision was to be revisited:

| Component | Encoding | Size |
| --- | --- | --- |
| Single-element runs | 6,655 x 10 B | 65.0 KB |
| Style palette | 307 x 4 B | 1.2 KB |
| Expansions | 2,008 headers + 4,933 pooled elements | 35.0 KB |
| Contractions | 953 headers + 1,836 pooled elements | 22.1 KB |
| **Total** | | **123.2 KB** |

This is a floor on what the generator must beat, not a projection: the field
widths above are deliberately naive, storing each run's start code point in a
full four bytes when runs are sorted and the deltas would fit in two, and
giving each contraction a 16-byte header when the keys are at most three code
points. Both prior sub-projects found comparable slack.

### Only NFD-reachable entries are generated

The measurement above already assumes an optimization worth stating
explicitly. Because step 1 normalizes to NFD, no canonically decomposable code
point can ever reach the table. Dropping those entries costs nothing and
removes 1,990 of the 3,998 expansions, halving that table from 67 KB to 35 KB.
It removes only 91 single-element entries and 11 contractions, because
precomposed characters are overwhelmingly the ones with expansions.

The filter is *canonical* decomposition only. Compatibility decompositions
survive NFD, so the ligatures and compatibility forms stay in the table. Hangul
syllables decompose arithmetically and are not listed in `UnicodeData.txt`, so
the generator must exclude `AC00..D7A3` explicitly.

This makes "the input has been normalized to NFD" a hard precondition of the
lookup rather than an implementation detail. Any fast path that skips
normalization must be restricted to code points that are NFD-stable; ASCII
qualifies, which is the only fast path contemplated here.

## Architecture

### Shared layer

`include/hermes/Platform/Unicode/UnicodeCollation.h`:

```cpp
namespace hermes {
namespace unicode {

/// Compare \p left and \p right under the DUCET root collation of UTS #10,
/// using non-ignorable variable weighting and comparing through the tertiary
/// level. \return -1, 0, or 1 corresponding to whether \p left compares less
/// than, equal to, or greater than \p right.
///
/// Both arguments are treated as WTF-16: unpaired surrogates are compared by
/// their implicit weights, and U+0000 is an ordinary character rather than a
/// terminator.
int compareUTF16(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right);

} // namespace unicode
} // namespace hermes
```

Named `compareUTF16` for consistency with `normalizeUTF16` and
`convertCaseUTF16`. The ADL hazard that forced those names does not arise here,
since no argument type lives in `hermes::unicode`, but diverging from the
established convention for that reason alone would be worse.

The function is pure and takes no locale, which is what makes the Hermes and
LITE backends able to share one call.

### Algorithm

1. **Normalize to NFD.** UTS #10 step 1, via the existing
   `normalizeUTF16(buf, NormalizationForm::D)`. This simultaneously reproduces
   the `UCOL_NORMALIZATION_MODE` behavior the ICU code opts into, which is why
   the normalizer was built first.
2. **Build the collation element array.** Walk the NFD code points, taking the
   longest match against the contraction table and applying the discontiguous
   contraction rule of UTS #10 S2.1.1-S2.1.3. Expansions come from a shared
   collation element pool. Han, Tangut, Nushu and unassigned code points use
   the implicit weight formula of UTS #10 section 10.1.3 rather than table
   entries.
3. **Compare by level.** Level 1 across all non-zero primaries, then level 2,
   then level 3. Sort keys are never materialized; each level is a single pass
   over the two collation element arrays.
4. **Return -1, 0, or 1.**

Strings equal through the tertiary level return 0. This matches ICU with the
identical level off, so `"a\u0000b"` and `"ab"` compare equal there as they do
today. The result remains a consistent comparison function, which is all
ECMA-262 requires.

### Backends

- `PlatformUnicodeHermes.cpp` calls `unicode::compareUTF16`, dropping the
  `icu_impl::localeCompare` forward and its `TODO(icu-removal)`.
- `PlatformUnicodeLite.cpp` calls the same function, replacing its code-unit
  comparison.
- Emscripten, Android and Apple are untouched.

`UnicodeCollation.cpp` joins `UnicodeNormalization.cpp` and
`UnicodeCaseConversion.cpp` in the conditional source list in
`lib/Platform/Unicode/CMakeLists.txt` and the corresponding BUCK target, so
Android and Apple do not compile the table.

### Shared UTF-16 decode helper

`nextCodePoint` is currently duplicated at `UnicodeNormalization.cpp:139` and
`UnicodeCaseConversion.cpp:319`. This is the third consumer, which is the point
at which the extraction was deferred to.

It moves to `include/hermes/Platform/Unicode/CharacterProperties.h`, in
namespace `hermes`, next to the `isHighSurrogate`, `isLowSurrogate` and
`utf16SurrogatePairToCodePoint` helpers it is built from. No new header is
needed, and the two existing copies are deleted.

## Generator changes

`utils/genUnicodeTable.py` gains two `--table` choices, following the pattern
`normalization`/`normtest` and `casing`/`casetest` already establish:

- `--table collation`, emitting `lib/Platform/Unicode/CollationData.inc`: the
  run-encoded single-element mappings, the style palette, an expansion pool of
  collation elements indexed by offset and length in the manner of
  `FULL_UPPER`, and the contraction table. Canonically decomposable code points
  are omitted per the NFD precondition above, and the six `@implicitweights`
  ranges are emitted as ranges rather than expanded.
- `--table collationtest`, emitting the conformance data described below.

Two generator-side assertions matter, because both encode assumptions that
would fail silently. The run encoder must verify that every run it emits
reproduces the input weights exactly, rather than trusting the monotonicity it
exploits. And the `@implicitweights` ranges must be checked against the
formula in UTS #10 section 10.1.3, not assumed to match it.

Contraction keys are at most three code points in this version of the data,
which the generator should assert rather than hardcode, since a longer key
would silently truncate.

`allkeys.txt` joins the `URLS` dict, and a `COLLATION_FILES` list records the
provenance of the new tables.

Note the existing defect recorded in the handoff: `normtest` is dispatched from
a bare `else:` while the other choices use explicit `elif`. Adding a sixth and
seventh choice makes that latent bug reachable, so it is fixed as part of this
work rather than left as a follow-up.

## Testing

- **UCA conformance suite.** The file asserts that each row sorts at or after
  the row before it. Rows are emitted as string literals in the upstream
  format and parsed by the test, as `NormalizationConformanceTest.cpp` does.

  Read `CollationTest_NON_IGNORABLE_SHORT.txt`, but note that despite the name
  it is **not** a subset: it carries the same 208,070 rows as the full file and
  is smaller only because it omits the per-row sort key comments, 2.2 MB
  against 17.7 MB. Rows average 2.07 code points, so all 208,070 emit as
  roughly 3.7 MB of `.inc` source. That is 2.6 times the normalizer's 1.4 MB
  data file and is the recommended starting point; if it costs too much build
  time, the fallback is a stride-2 sample at 1.8 MB, which matches the
  normalizer's precedent. Sampling is the fallback rather than the default
  because adjacent rows are the most similar pairs in the file, so dropping
  every other row discards exactly the comparisons that discriminate hardest.

  The file's ordering assumes an appended **identical level** — a final code
  point tie-break that this implementation deliberately does not have. The
  harness must apply that tie-break itself when `compareUTF16` returns 0, or
  the suite fails on rows that are correctly equal at the tertiary level.

  `CollationTest_SHIFTED.txt` is not used: it tests shifted variable
  weighting, which this implementation does not offer.

- **Generated weight lookup check.** For a sample of code points spanning the
  compressed encoding's boundaries, assert the table lookup agrees with an
  expectation derived independently from `allkeys.txt`. The compression is
  where bugs hide, and this is the only test that isolates it from the
  algorithm.

- **Targeted gtests**, one area each: contractions including the discontiguous
  case, expansions, implicit weights for Han and for unassigned code points,
  variable characters under non-ignorable weighting, canonical equivalence,
  unpaired surrogates, embedded U+0000, and supplementary-plane code points.

- **A populated-data guard**, as the normalizer and case converter have, so
  empty tables cannot make the rest pass vacuously.

- **Existing suites.** `test/hermes/string-functions.js` and its delta-mode
  copy, plus whatever test262 covers, all of which pass today.

- **Mutation check.** Every suite must be shown to fail when the
  implementation is broken. One test in an earlier plan was caught asserting
  nothing at all before this discipline was applied.

Gates for the sub-project: `buck2 test //xplat/static_h:HermesUnitTests`,
`:lit` and `:testsuite_tests`, plus `arc lint`.

## Diff breakdown

1. Generator `--table collation` and the checked-in `CollationData.inc`, with
   the `else:`/`elif` fix. No behavior change. The 123 KB above is the floor
   this diff must come in at or under; coming in materially over it is the
   signal to stop and revisit fidelity before writing any implementation.
2. `UnicodeCollation.{h,cpp}` and its unit tests, plus the `nextCodePoint`
   extraction into `CharacterProperties.h`. Not yet wired to any backend, so
   still no behavior change.
3. Generator `--table collationtest`, the conformance data, and
   `CollationConformanceTest.cpp`, shown to fail under mutation.
4. Wire `PlatformUnicodeHermes.cpp` and `PlatformUnicodeLite.cpp` to
   `compareUTF16`, dropping `icu_impl::localeCompare`. The `lit` and
   `testsuite_tests` runs are the gate.

## Risks

- **Table size exceeds the measurement.** Largely retired: 123 KB is measured
  from the real data rather than projected, and the naive field widths leave
  visible slack. What remains is the risk that the generated encoding is
  *worse* than the hand-measured one. Mitigation: diff 1 still stands alone and
  reports its number.
- **The NFD-reachability filter is wrong.** Dropping 1,990 expansion entries is
  only safe if every lookup path normalizes first, and a mistake here produces
  wrong weights for real characters rather than a crash. Mitigation: the
  precondition is stated in the header, the filter uses canonical
  decompositions only, and the conformance suite exercises precomposed input
  directly.
- **Contraction matching is subtly wrong.** The discontiguous contraction rule
  is the most intricate part of UTS #10 and the easiest place to be
  plausibly-but-quietly incorrect. Mitigation: the conformance suite is the
  real defense here; targeted tests alone would not catch it.
- **The conformance harness masks a defect.** If the identical-level tie-break
  is applied too eagerly it can turn genuine ordering failures into passes.
  Mitigation: the mutation check must specifically break primary, secondary
  and tertiary comparison in turn and confirm the suite catches each.
- **Divergence from ICU that matters more than expected.** Root collation is
  not CLDR root, and tailorings are gone. Mitigation: the affected orderings
  are documented above; the difference is confined to variable characters,
  script order, and locales with tailorings, none of which any test covers.
- **The `.inc` files are large enough to slow the build.** The conformance data
  is 3.7 MB against the normalizer's 1.4 MB, and it is checked in. Mitigation:
  emit rows as string literals parsed at runtime rather than pre-expanded
  arrays, which was measured on the normalizer at a third of the source size;
  measure compile time in diff 3 and fall back to a stride-2 sample if it is
  material.

## Success criteria

- The UCA conformance suite passes, and is shown to fail when primary,
  secondary and tertiary comparison are each broken in turn.
- The targeted gtests and the weight lookup check pass.
- `buck2 test //xplat/static_h:HermesUnitTests`, `:lit` and
  `:testsuite_tests` all pass, including the six existing `localeCompare`
  cases in `string-functions.js`.
- `localeCompare` calls no ICU symbol on Linux or Windows.
- LITE performs real collation.
- `nextCodePoint` exists once.
- The generated table comes in at or under the measured 123 KB floor.
