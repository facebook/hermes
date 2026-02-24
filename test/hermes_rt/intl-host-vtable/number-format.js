// Run the upstream intl/number-format.js test via host-provided ICU vtable.
// RUN: %hermes_rt --intl-provider=host-vtable %S/../../hermes/intl/number-format.js | %FileCheck --match-full-lines %S/../../hermes/intl/number-format.js
// REQUIRES: intl
