# ICU-free `convertToCase`

Date: 2026-08-27
Status: approved, ready for implementation planning

Sub-project 2 of 4 in the ICU4C removal. See
`doc/superpowers/specs/2026-08-26-icu-free-normalize-design.md` for the overall
decomposition and the shared conventions this follows.

## Context

`convertToCase` is one of the four functions Hermes still needs ICU for on
Linux and Windows. It backs `String.prototype.toUpperCase`,
`toLowerCase`, `toLocaleUpperCase` and `toLocaleLowerCase`.

`String.cpp:947` has an ASCII fast path that avoids the backend entirely, but
only when `useCurrentLocale` is false, so `toLocaleUpperCase("abc")` reaches
the backend today even though nothing about it needs a locale.

### Locale-sensitive casing is a deliberate cross-platform contract

Every backend that implements this function honors `useCurrentLocale`:

| Platform | Backend | Locale source |
| --- | --- | --- |
| Android | Java via JNI | `Locale.getDefault()` |
| iOS, macOS | CoreFoundation | `CFLocaleCopyCurrent()`, overridable by `_HERMES_TEST_LOCALE` |
| Linux, Windows | ICU | `uloc_getDefault()` |
| Emscripten | none | no-op, `// FIXME: implement this` |
| LITE | none | no-op |

`AndroidUnicodeUtils.java` carries an explicit comment about Turkish dotless i,
so the behavior is intentional rather than incidental.

`test/hermes/string-locale.js` tests it directly, running the same script under
`LC_ALL=en_US`, `LC_ALL=tr_TR` and bare `LC_ALL=UTF-8` with a separate check
prefix for each, asserting `'aeiou'.toLocaleUpperCase()` is `AEİOU` under
Turkish. That test is live: mutating the Turkish expectation fails
`buck2 test //xplat/static_h:lit`.

This spec therefore **preserves** locale-sensitive casing. Dropping it was
considered and rejected: it would break a checked-in test and make Linux the
only platform that ignores the user's locale for casing.

### Measured behavior of the code being replaced

Established empirically against the current ICU build, not inferred:

- Turkish and Azeri: `upper("i")` is U+0130, `lower("I")` is U+0131.
- Lithuanian: `lower("Ì")` is `0069 0307 0300` (dot added),
  `upper("i̇")` is `0049` (dot removed).
- Locale precedence is `LC_ALL`, then `LC_MESSAGES`, then `LANG`. **`LC_CTYPE`
  is ignored**, despite POSIX convention; `LC_CTYPE=en_US` with `LANG=tr_TR`
  still yields Turkish casing.
- No system locale data is required. `tr_TR` is not installed on the
  development host, yet Turkish casing works, because ICU parses the
  environment string against its own bundled data.
- Accepted formats include `tr`, `tr_TR`, `tr_TR.UTF-8` and `tr-TR`.
- The locale *argument* is ignored: `"i".toLocaleUpperCase("en")` returns
  U+0130 under `LANG=tr_TR`. This is correct for a non-ECMA-402
  implementation, which ECMA-262 directs to the host environment's current
  locale.

## Scope

Replace `convertToCase` with a self-contained implementation on every platform
that lacks a better one: Linux, Windows, Emscripten and LITE. Android and Apple
keep their platform backends.

### Non-goals

- Changing the Android or Apple backends.
- Implementing Intl, or honoring the locale argument. When
  `HERMES_ENABLE_INTL` is on, `intlStringPrototypeToLocaleUpperCase` already
  takes over and does this properly from the argument.
- Regenerating `UnicodeData.inc`.
- Removing the ICU dependency, which happens in sub-project 4.

### Deliberate behavior changes

- **LITE and Emscripten gain working case conversion.** Both return their input
  unchanged today, so this is a substantial improvement rather than a
  regression.
- **`_HERMES_TEST_LOCALE` starts working on Linux and Windows.** It is
  currently honored only by `PlatformUnicodeCF.cpp:26`. Supporting it uniformly
  lets the lit tests drive the locale directly instead of depending on
  `LC_ALL` resolving, which matters because `date-locale.js` is already skipped
  under ubsan.
- **`toLocaleUpperCase` on ASCII stops allocating** when the host locale does
  not affect casing, which is the common case.

## Approach

Delta-range blocks for the simple mappings plus a side table for the full
ones, reusing the `UnicodeTransformRange` encoding that `UNICODE_FOLDS`
already uses and the `DeltaMapBlock` machinery already in
`utils/genUnicodeTable.py`.

Rejected alternatives:

- **Flat sorted `cp -> mapping` arrays.** Roughly 24 KB instead of 8 KB, for a
  simpler generator, while abandoning an encoding the codebase already applies
  to this exact shape of data.
- **Two-stage trie.** Same reasoning as the normalizer: a data structure found
  nowhere else here, for a function that is not hot.

### Measured table sizes

| Table | Encoding | Size |
| --- | --- | --- |
| `toUpper` simple mappings | ~130 delta blocks | 1.0 KB |
| `toLower` simple mappings | ~130 delta blocks | 1.0 KB |
| Full mappings (102 upper, 1 lower) | 103 entries plus `char16_t` pool | 1.3 KB |
| `Cased` | 158 ranges | 1.2 KB |
| `Case_Ignorable` | 464 ranges | 3.6 KB |
| `Soft_Dotted` | 34 ranges | 0.3 KB |
| Canonical combining class | reused from the normalizer | 0 |
| **Total** | | **~8.4 KB** |

1,505 simple uppercase mappings compress to roughly 130 blocks, the same ratio
`UNICODE_FOLDS` achieves. These are projections from measured input counts and
the existing encoding's behavior; the figure is to be confirmed once the tables
exist, as it was for the normalizer.

## Architecture

### Shared layer

`include/hermes/Platform/Unicode/UnicodeCaseConversion.h`:

```cpp
namespace hermes {
namespace unicode {

/// Options for case conversion.
enum class CaseConversion { ToUpper, ToLower };

/// The language-specific casing rules to apply. Azerbaijani shares Turkish's
/// rules, so it has no separate enumerator.
enum class CaseLocale { Root, Turkish, Lithuanian };

/// Convert \p buf to \p targetCase in place, applying the full Unicode case
/// mappings from SpecialCasing.txt and the conditional rules for \p locale.
void convertCaseUTF16(
    llvh::SmallVectorImpl<char16_t> &buf,
    CaseConversion targetCase,
    CaseLocale locale);

} // namespace unicode
} // namespace hermes
```

`platform_unicode` keeps a `using CaseConversion = unicode::CaseConversion;`
alias so existing call sites are unchanged, mirroring what `NormalizationForm`
already does.

The function is named `convertCaseUTF16` rather than `convertToCase` for the
same reason `normalizeUTF16` is not `normalize`: with `CaseConversion` living
in `hermes::unicode`, argument-dependent lookup finds that namespace from every
unqualified call and would collide with the identically-signatured
`platform_unicode::convertToCase`.

### Locale detection belongs to the backend

`convertCaseUTF16` takes an explicit `CaseLocale` and is pure. Deciding what
the host locale is stays in the backend, which is where the other platforms
already put it:

- `PlatformUnicodeHermes.cpp` reads the environment.
- `PlatformUnicodeLite.cpp` passes `Root` unconditionally, preserving its
  contract of not depending on system state.
- `PlatformUnicodeEmscripten.cpp` passes `Root`.

Detection replicates ICU's observed behavior exactly: `_HERMES_TEST_LOCALE`,
then `LC_ALL`, then `LC_MESSAGES`, then `LANG`; first non-empty wins. It
deliberately does **not** consult `LC_CTYPE`, even though POSIX convention
would suggest it for a character-handling category, because diverging from ICU
here would be a silent behavior change for anyone who sets it. The language
subtag is the text before the first `_`, `-`, `.` or `@`; `tr` and `az` map to
`Turkish`, `lt` to `Lithuanian`, everything else to `Root`.

The result is computed once and cached, since it cannot change within a
process.

### Conditional rules

From `SpecialCasing.txt`, 16 conditional entries:

- **`Final_Sigma`** (1 entry, locale-independent, always applied). Lowercasing
  U+03A3 yields U+03C2 when preceded by a `Cased` character, ignoring any
  `Case_Ignorable` characters between, and not followed by one. Required
  regardless of the locale decision, and already covered by
  `PlatformUnicodeTest.cpp:29` and test262.
- **Turkish and Azeri** (7 entries): `After_I` and `Not_Before_Dot`, plus the
  unconditional dotted and dotless i mappings.
- **Lithuanian** (8 entries): `More_Above`, `After_Soft_Dotted`, and the rules
  adding or removing U+0307.

`More_Above` and `After_Soft_Dotted` need the canonical combining class, which
the normalizer already ships. `UnicodeNormalization.h` gains a
`getCanonicalCombiningClass(uint32_t)` accessor rather than duplicating the
table.

### ASCII fast path

`platform_unicode` gains:

```cpp
/// \return true if the host locale changes case conversion results, i.e. it
/// is Turkish, Azerbaijani or Lithuanian. Cached.
bool localeAffectsCasing();
```

`String.cpp:947` then guards the ASCII fast path with
`!useCurrentLocale || !localeAffectsCasing()` instead of `!useCurrentLocale`,
so `toLocaleUpperCase` on ASCII avoids the copy for everyone outside those
three languages. Android and Apple implement it against their own locale APIs.

## Generator changes

`utils/genUnicodeTable.py` gains `--table casing`, emitting
`lib/Platform/Unicode/CaseData.inc`:

- `TO_UPPER_DELTAS[]`, `TO_LOWER_DELTAS[]` as `UnicodeTransformRange`, built
  with the existing `DeltaMapBlock`.
- `FULL_UPPER[]`, `FULL_LOWER[]` mapping a code point to an offset and length
  in a shared `char16_t` pool, for the unconditional multi-character mappings.
- `CASED_RANGES[]`, `CASE_IGNORABLE_RANGES[]`, `SOFT_DOTTED_RANGES[]`.

The existing `CaseMap` class is **not** reused. It is built for RegExp
canonicalization and deliberately discards exactly what is needed here: it
skips any mapping producing more than one character, and ignores every
conditional `SpecialCasing.txt` entry.

`DerivedCoreProperties.txt` and `PropList.txt` are already fetched.
`CASING_FILES` joins the existing per-table file lists so the provenance header
records only what this table derives from.

## Testing

There is no single UCD conformance file for casing, so coverage comes from
several directions:

- **Generated exhaustive mapping check.** For every code point with a
  non-identity simple mapping, assert the compressed delta-block lookup agrees
  with an expectation derived independently from `UnicodeData.txt`. The delta
  encoding is where bugs hide, and this is the only test that isolates it.
- **Targeted gtests per conditional rule**, covering each of `Final_Sigma`,
  `After_I`, `Not_Before_Dot`, `More_Above` and `After_Soft_Dotted`, in both
  the matching and non-matching direction.
- **A populated-data guard**, as the normalizer has, so empty tables cannot
  make the rest pass vacuously.
- **Existing coverage already in the suites**: test262's `special_casing.js`,
  `special_casing_conditional.js`, `Final_Sigma_U180E.js` and
  `supplementary_plane.js`; and the lit test `string-locale.js`, which
  exercises Turkish through `LC_ALL`.
- **Mutation check.** As with the normalizer, the suite must be shown to fail
  when the implementation is broken, rather than assumed to.

`buck2 test //xplat/static_h:lit` is part of the gate for this sub-project. It
was not run during the normalizer work, which was a gap; `string-locale.js` is
the test most likely to catch a regression here.

## Diff breakdown

1. Generator: `--table casing`, emitting `CaseData.inc`. Generated file checked
   in. No behavior change.
2. `UnicodeCaseConversion.{h,cpp}` and its tests, wired into the LITE and
   Emscripten backends whose `convertToCase` is a no-op today. Adds
   `getCanonicalCombiningClass` to the normalizer's header.
3. Locale detection and `localeAffectsCasing` in `PlatformUnicodeHermes.cpp`,
   switching Linux and Windows off `icu_impl::convertToCase`. The `lit` and
   `testsuite_tests` runs are the gate.
4. The `String.cpp` ASCII fast-path guard, with the Android and Apple
   implementations of `localeAffectsCasing`.

## Risks

- **Locale detection diverges from ICU** in a case not covered by the
  measurements, silently changing behavior. Mitigation: the precedence and
  format handling above are measured, not assumed; `string-locale.js` covers
  the main path, and diff 3 should re-run the measurement matrix against the
  new implementation.
- **A conditional rule is subtly wrong**, which unit tests might miss because
  the rules are context-sensitive. Mitigation: per-rule tests in both
  directions, plus test262's conditional coverage and the mutation check.
- **Table size exceeds the projection.** Lower risk than the normalizer, since
  the encoding is already in use on comparable data, but the figure is a
  projection and will be measured.
- **`String.cpp` fast-path change alters behavior** for a locale that affects
  casing beyond ASCII. Mitigation: `localeAffectsCasing` returns true for all
  three special languages, so the fast path is only taken when the locale is
  `Root`, where ASCII casing is unambiguous.

## Success criteria

- The exhaustive mapping check and per-rule tests pass.
- `buck2 test //xplat/static_h:HermesUnitTests`, `:lit` and `:testsuite_tests`
  all pass.
- The measured locale matrix behaves identically to the ICU build: Turkish and
  Azeri dotted and dotless i, the Lithuanian dot rules, and the documented
  environment variable precedence.
- `convertToCase` calls no ICU symbol on Linux or Windows.
- LITE and Emscripten perform real case conversion.
- Table size is measured and recorded.
