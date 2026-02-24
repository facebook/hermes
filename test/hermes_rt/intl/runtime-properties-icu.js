// Run the upstream intl/runtime-properties-icu.js test via hermes.dll C API.
// RUN: %hermes_rt %S/../../hermes/intl/runtime-properties-icu.js | %FileCheck %S/../../hermes/intl/runtime-properties-icu.js
// RUN: %hermes_rt --intl-provider=winglob %S/../../hermes/intl/runtime-properties-icu.js | %FileCheck %S/checks/runtime-properties-icu.winglob
// RUN: %hermes_rt --intl-provider=system-icu %S/../../hermes/intl/runtime-properties-icu.js | %FileCheck %S/checks/runtime-properties-icu.system-icu
// REQUIRES: intl
