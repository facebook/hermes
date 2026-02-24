// Run the upstream intl/collator.js test via external unicode.org ICU DLLs.
// RUN: %hermes_rt --intl-icu-path=%external_icu_dir --intl-icu-version=%external_icu_version %S/../../hermes/intl/collator.js | %FileCheck --match-full-lines %S/../../hermes/intl/collator.js
// REQUIRES: intl, external_icu
