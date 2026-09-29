# ICU-free `dateFormat`

Date: 2026-08-29
Status: approved, ready for implementation planning

Sub-project 4 of 4 in the ICU4C removal, and the last one. See
`doc/superpowers/specs/2026-08-26-icu-free-normalize-design.md` for the overall
decomposition and the shared conventions this follows.

## Context

`dateFormat` is the final function Hermes needs ICU for. It backs
`Date.prototype.toLocaleString`, `toLocaleDateString` and `toLocaleTimeString`
through three one-line wrappers at `lib/VM/JSLib/DateUtil.cpp:561-571`, whose
only caller is `datePrototypeToLocaleStringHelper` at
`lib/VM/JSLib/Date.cpp:588`.

When `HERMES_ENABLE_INTL` is on, that helper dispatches to `Intl.DateTimeFormat`
and `dateFormat` is never reached at all.

### This sub-project is not shaped like the other three

Normalize, casing and collation were each a Unicode algorithm with a normative
specification, a UCD data file, and a conformance suite that could prove the
implementation correct. `dateFormat` has none of those.

`PlatformUnicodeICU.cpp:81` calls `udat_open` at `UDAT_MEDIUM` against
`uloc_getDefault()`. Its output *is* CLDR locale data — month names, field
order, 12- versus 24-hour clock, the glue between date and time. There is no
algorithm to reimplement. Every option therefore gives something up, and the
design question is which.

ECMA-262 makes this easy to answer in principle: without ECMA-402 the result of
`toLocaleString` is implementation-defined, "a convenient, human-readable form".

### What the code does today, measured

Run against the current binary on this host, `TZ=EST+5`, `new Date(112)`:

| Locale | `toLocaleString()` |
| --- | --- |
| `en_US` | `Dec 31, 1969, 7:00:00 PM` |
| `tr_TR` | `31 Ara 1969 19:00:00` |

Note the Turkish output differs in month name, field order, clock, and the
date/time separator.

**ICU produced that Turkish output on a machine where `tr_TR` is not an
installed system locale.** This host has 38 locales, all English variants.
`strftime` under `LC_ALL=tr_TR` here prints `Dec`. ICU does not consult the
system locale archive; it parses the environment string against its own bundled
CLDR data, exactly as the convertToCase spec found for casing.

### Why not libc

`strftime_l` was the approach the normalize spec pencilled in. It is rejected:

- **It cannot reproduce the current output.** Turkish month names require the
  locale to be installed. On this development host it is not, so the checked-in
  Turkish expectations would fail immediately.
- **It does not match ICU even when the locale is present.** For `en_US` the
  locale's own `%x` gives `12/31/1969` against ICU's `Dec 31, 1969`, and `%X`
  gives `07:00:00 PM` against `7:00:00 PM`.
- **It is host-dependent.** Results vary with the installed locale archive and
  degrade on musl and in minimal containers — the same objection this project
  accepted when it rejected `strcoll_l` for collation.

A hybrid — fixed field order with locale-supplied month names — was also
rejected. It matches `en_US` exactly and yields `Ara` where Turkish is
installed, but produces `Ara 31, 1969` on a 12-hour clock: neither the ICU
output nor idiomatic Turkish, and still host-dependent.

### Existing coverage is thinner than it looks

`test/hermes/date-locale.js` is the only test that pins any output. It carries
`UNSUPPORTED: ubsan || intl || unicode_lite`, so it does not run in the
default asan-ubsan build. `utils/testsuite/skiplist.json` has no
`Date/prototype/toLocale` entries, and no C++ unit test touches this
function. Nothing in `doc/` documents the non-Intl path or the LITE stub.

So the entire user-visible contract is one lit test that the default build
skips.

## Scope

Replace `dateFormat` with a fixed, locale-independent English formatter on every
backend that lacks a platform implementation, and remove ICU from the
`platform_unicode` layer.

### Non-goals

- Changing the Apple (CoreFoundation) or Android (Java) backends.
- Implementing Intl or ECMA-402.
- Honoring the `locales` argument, or any host locale.
- Shipping CLDR data.
- Deleting the LITE backend. It is expected to go away now that the Hermes
  backend exists, but that is its own change.
- Removing the ICU-backed Intl implementation. See below.

### Deliberate behavior changes

- **Output stops depending on the host locale on Linux and Windows.** A user
  under `LANG=sv_SE` or `LC_ALL=tr_TR` gets English. This is the same trade
  already accepted for root collation in sub-project 3.
- **LITE and Emscripten gain a working implementation.** Both return the literal
  string `"dateFormat not implemented"` today
  (`PlatformUnicodeLite.cpp:31`, `PlatformUnicodeEmscripten.cpp:43`), so this is
  a substantial improvement rather than a regression.
- **The ICU `platform_unicode` backend is deleted.** Building with
  `-c hermes.platform_unicode=ICU` stops being a valid configuration.

## Approach

A fixed English format, chosen to match what ICU emits for `en_US` today so the
existing expectations stay byte-identical:

| Call | Output |
| --- | --- |
| `toLocaleDateString()` | `Dec 31, 1969` |
| `toLocaleTimeString()` | `7:00:00 PM` |
| `toLocaleString()` | `Dec 31, 1969, 7:00:00 PM` |

Precisely: three-letter English month abbreviation; day of month with no
leading zero; four-digit year, widening for years outside 1000-9999; 12-hour
clock with no leading zero on the hour; two-digit minutes and seconds; a single
ASCII space before `AM`/`PM`; and `, ` joining date and time.

`Invalid Date` is produced by the caller at `Date.cpp:606-634` for non-finite
timestamps and never reaches the backend.

## Architecture

### Shared layer

`include/hermes/Platform/Unicode/PlatformDateFormat.h`:

```cpp
namespace hermes {
namespace platform_unicode {

/// Format \p unixtimeMs into \p buf in a fixed, locale-independent English
/// form, including the date when \p formatDate and the time when
/// \p formatTime. The timestamp is interpreted in the host's local timezone.
///
/// This is the fallback for platforms with no locale-aware date formatter of
/// their own. It deliberately ignores the host locale, so its output is
/// identical on every machine and in every container. Platforms that have a
/// real formatter -- Apple through CoreFoundation, Android through
/// java.text.DateFormat -- do not call it.
void formatDateTimeFixed(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf);

} // namespace platform_unicode
} // namespace hermes
```

Implementation in `lib/Platform/Unicode/PlatformDateFormat.cpp`, compiled for
the same backends as the other self-contained sources.

### Date arithmetic is self-contained

`include/hermes/VM/JSLib/DateUtil.h` already has every ECMA-262 helper this
needs -- `yearFromTime`, `monthFromTime`, `dateFromTime`, `hourFromTime`,
`weekDay`, `localTime`, `equivalentTime`. It is deliberately **not** used:
`lib/Platform` depending on `lib/VM` is a layering inversion, and its only
precedent is `lib/Platform/Intl/impl_icu/LocaleBCP47Object.h`, which is
deprecated code.

The formatter therefore does its own civil-from-days conversion. The local UTC
offset comes from `localtime_r`. Because `time_t` cannot represent the whole
±8.64e15 ms JavaScript date range, out-of-range timestamps are mapped to an
equivalent year with the same weekday and leap-year character before the offset
is queried, then the real year is substituted back — the technique
`DateUtil.h:117 equivalentTime` already uses for the same reason.

### Backends

- `PlatformUnicodeHermes.cpp`, `PlatformUnicodeLite.cpp` and
  `PlatformUnicodeEmscripten.cpp` call `formatDateTimeFixed`.
- `PlatformUnicodeCF.cpp` and `PlatformUnicodeJava.cpp` are untouched.

### ICU comes out of the platform_unicode layer

Deleted: `lib/Platform/Unicode/PlatformUnicodeICU.cpp`,
`include/hermes/Platform/Unicode/PlatformUnicodeICUImpl.h`, the `icu_impl`
namespace, the `platform_unicode_icu` config in `BUCK`, the `USE_ICU` CMake
flag, and the `//third-party/icu` dependencies of the `platform_unicode`
target. Every remaining `TODO(icu-removal)` marker in that layer goes with them.

**`lib/Platform/Intl/impl_icu/` is out of scope.** Those four files
(`Collator.h`, `LocaleConverter.cpp`, `LocaleResolver.cpp`, and
`PlatformIntlICU.cpp`) are the only other ICU includers in the tree, and they
are compiled only in Intl builds. That implementation is deprecated and slated
for deletion on its own schedule. This sub-project must not break it, and must
not invest in it. Concretely: with Intl disabled — the default everywhere except
Android — nothing links ICU once this lands; with Intl enabled, ICU remains for
`Intl.Collator` and friends.

## Testing

### A lit feature for the unicode backend

`test/lit.cfg` gains a feature naming the resolved `platform_unicode` backend,
generalizing the `unicode_lite` feature that already exists. Today lit can
distinguish `apple`, `linux`, `windows` and `unicode_lite`, but cannot tell the
ICU backend from the Hermes backend on Linux — so gating by platform alone would
silently mis-fire for `-c hermes.platform_unicode=ICU`.

Both the ICU and LITE backends are expected to be deleted. The feature should
therefore be defined so their clauses can be removed alongside them without a
second rewrite of the tests: the split is between backends with a locale-aware
platform formatter and backends using the fixed one, and after both deletions
that reduces to Apple and Android versus everything else.

### Tests

- **A new lit test for the fixed format.** It runs the same script under
  `LC_ALL=en_US` and `LC_ALL=tr_TR` and asserts **identical English output**
  from
  both. That converts the locale dimension from something ICU's data satisfied
  into an active check that the host locale is ignored, which would catch a
  `strftime_l` reintroduction.
- **`test/hermes/date-locale.js` is retained for the locale-aware backends**,
  with its expectations unchanged, gated on the new feature.
- **C++ unit tests** for `formatDateTimeFixed`: the epoch; pre-1970 negative
  timestamps; all twelve month abbreviations; the 12-hour boundaries at midnight
  and noon, where a naive implementation prints `0:00:00 AM` or `12:00:00 PM`
  incorrectly; the extremes of the JavaScript date range, where `time_t`
  overflow
  and the equivalent-year remapping are exercised; and each of the three
  `formatDate`/`formatTime` flag combinations.
- **The stale `UNSUPPORTED: ubsan` marker is removed.** It is already liftable
  before any change here: the test runs clean against the asan+ubsan binary
  under
  `UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=0`, in both the `en_US` and
  `tr_TR` cases. `sl` history traces the exclusion to `32a1c90aa246`, the 2022
  "Copy Hermes into xplat/static_h" import, so it is inherited upstream baggage
  rather than anything this tree caused. Lifting it gains coverage in the build
  developers actually run.
- **A `doc/SpecIncompat.md` entry**, recording that `toLocale*String` ignores
  the
  `locales` argument and the host locale and emits a fixed English format. That
  file currently says nothing about `Date`.

### Mutation check

As with every suite in this stack, the new tests must be shown to fail against a
broken implementation rather than assumed to. The locale-independence assertion
in particular should be checked by making the formatter consult the host locale
and confirming the test catches it.

## Diff breakdown

1. `PlatformDateFormat.{h,cpp}` and its unit tests. Not wired to any backend, so
   no behavior change.
2. The lit backend feature, and the test split. Still no behavior change: the
   existing test keeps running against the existing backends.
3. Wire the Hermes, LITE and Emscripten backends to `formatDateTimeFixed`. This
   is the behavior change; `lit` and `testsuite_tests` are the gate. Lift the
   `ubsan` marker here, and add the `SpecIncompat.md` entry.
4. Delete the ICU backend and the `platform_unicode` layer's ICU dependencies.

## Risks

- **The Apple and Android paths cannot be exercised from this tree.** It is
  Linux-only on EdenFS and builds with buck2. The Apple half of the test split
  is
  inherited unchanged and unverified. Mitigation: do not modify those backends
  or
  their expectations at all, so the risk is confined to the gating.
- **Hand-rolled civil-date conversion is where this will go wrong if it goes
  wrong.** The JavaScript date range is far wider than `time_t`. Mitigation:
  unit
  tests aimed directly at the extremes and at the equivalent-year remapping.
- **Deleting the ICU backend removes a configuration in active use** — this
  project's own stack has been building `-c hermes.platform_unicode=ICU` as a
  smoke check throughout. Mitigation: it is deleted in its own diff, last, after
  everything else is green.
- **Losing locale-sensitive dates may surprise someone.** Mitigation: Hermes on
  Linux and Windows is largely a development and shermes surface; Android and
  Apple keep real localization; Intl remains the supported answer. Documented in
  `SpecIncompat.md` rather than only in this spec.

## Success criteria

- `toLocale{,Date,Time}String` produce the fixed format on the Hermes, LITE and
  Emscripten backends, identically under any `LC_ALL`.
- The Apple and Android backends are byte-for-byte unchanged.
- No file under `lib/Platform/Unicode/` or `include/hermes/Platform/Unicode/`
  references ICU, and the `platform_unicode` target has no ICU dependency.
- `lib/Platform/Intl/impl_icu/` still compiles and behaves as it does today.
- `buck2 test //xplat/static_h:HermesUnitTests`, `:lit` and `:testsuite_tests`
  all pass, with `date-locale.js` no longer skipped under ubsan.
- The new tests are shown to fail against a broken implementation.
