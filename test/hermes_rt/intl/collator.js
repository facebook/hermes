// Run the upstream intl/collator.js test via hermes.dll C API.
// RUN: %hermes_rt %S/../../hermes/intl/collator.js | %FileCheck --match-full-lines %S/../../hermes/intl/collator.js
// RUN: %hermes_rt --intl-provider=winglob %S/../../hermes/intl/collator.js | %FileCheck --match-full-lines %S/checks/collator.winglob
// RUN: %hermes_rt --intl-provider=system-icu %S/../../hermes/intl/collator.js | %FileCheck --match-full-lines %S/../../hermes/intl/collator.js
// REQUIRES: intl
