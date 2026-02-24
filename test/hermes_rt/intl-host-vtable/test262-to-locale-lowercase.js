// Run the upstream intl/test262-to-locale-lowercase.js test via host-provided ICU vtable.
// RUN: %hermes_rt --intl-provider=host-vtable %S/../../hermes/intl/test262-to-locale-lowercase.js | %FileCheck --match-full-lines %S/../../hermes/intl/test262-to-locale-lowercase.js
// REQUIRES: intl
