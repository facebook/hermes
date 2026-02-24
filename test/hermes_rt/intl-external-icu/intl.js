// Run the upstream intl/intl.js test via external unicode.org ICU DLLs.
// RUN: %hermes_rt --intl-icu-path=%external_icu_dir --intl-icu-version=%external_icu_version %S/../../hermes/intl/intl.js
// REQUIRES: intl, external_icu
