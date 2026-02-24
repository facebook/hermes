// Run the upstream test via external unicode.org ICU DLLs.
// RUN: %hermes_rt --intl-icu-path=%external_icu_dir --intl-icu-version=%external_icu_version %S/../../hermes/intl/collator-resolved-options-validate-co-extension.js
// REQUIRES: intl, external_icu
