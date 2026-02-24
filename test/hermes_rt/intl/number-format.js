// Run the upstream intl/number-format.js test via hermes.dll C API.
// RUN: %hermes_rt %S/../../hermes/intl/number-format.js | %FileCheck --match-full-lines %S/../../hermes/intl/number-format.js
// RUN: %hermes_rt --intl-provider=winglob %S/../../hermes/intl/number-format.js | %FileCheck --match-full-lines %S/checks/number-format.winglob
// RUN: %hermes_rt --intl-provider=system-icu %S/../../hermes/intl/number-format.js | %FileCheck --match-full-lines %S/../../hermes/intl/number-format.js
// REQUIRES: intl
