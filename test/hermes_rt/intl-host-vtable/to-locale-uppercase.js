// Run the upstream intl/to-locale-uppercase.js test via host-provided ICU vtable.
// RUN: %hermes_rt --intl-provider=host-vtable %S/../../hermes/intl/to-locale-uppercase.js | %FileCheck --match-full-lines %S/../../hermes/intl/to-locale-uppercase.js
// REQUIRES: intl
