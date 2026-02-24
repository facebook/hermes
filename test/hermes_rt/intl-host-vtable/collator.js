// Run the upstream intl/collator.js test via host-provided ICU vtable.
// RUN: %hermes_rt --intl-provider=host-vtable %S/../../hermes/intl/collator.js | %FileCheck --match-full-lines %S/../../hermes/intl/collator.js
// REQUIRES: intl
