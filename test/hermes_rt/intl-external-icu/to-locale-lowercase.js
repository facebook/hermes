// Run the upstream intl/to-locale-lowercase.js test via external unicode.org ICU DLLs.
// RUN: %hermes_rt --intl-icu-path=%external_icu_dir --intl-icu-version=%external_icu_version %S/../../hermes/intl/to-locale-lowercase.js | %FileCheck --match-full-lines %S/../../hermes/intl/to-locale-lowercase.js
// REQUIRES: intl, external_icu
