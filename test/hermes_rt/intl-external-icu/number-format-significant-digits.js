// Run the upstream test via external unicode.org ICU DLLs.
// RUN: %hermes_rt --intl-icu-path=%external_icu_dir --intl-icu-version=%external_icu_version %S/../../hermes/intl/number-format-significant-digits.js
// REQUIRES: intl, external_icu
