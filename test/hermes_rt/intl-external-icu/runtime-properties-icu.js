// Run the upstream intl/runtime-properties-icu.js test via external unicode.org ICU DLLs.
// RUN: %hermes_rt --intl-icu-path=%external_icu_dir --intl-icu-version=%external_icu_version %S/../../hermes/intl/runtime-properties-icu.js | %FileCheck %S/checks/runtime-properties-icu.external-icu
// REQUIRES: intl, external_icu
