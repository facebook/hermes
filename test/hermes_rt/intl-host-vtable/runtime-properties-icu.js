// Run the upstream intl/runtime-properties-icu.js test via host-provided ICU vtable.
// RUN: %hermes_rt --intl-provider=host-vtable %S/../../hermes/intl/runtime-properties-icu.js | %FileCheck %S/checks/runtime-properties-icu.host-vtable
// REQUIRES: intl
