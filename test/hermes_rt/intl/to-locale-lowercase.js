// Run the upstream intl/to-locale-lowercase.js test via hermes.dll C API.
// RUN: %hermes_rt %S/../../hermes/intl/to-locale-lowercase.js | %FileCheck --match-full-lines %S/../../hermes/intl/to-locale-lowercase.js
// RUN: %hermes_rt --intl-provider=winglob %S/../../hermes/intl/to-locale-lowercase.js | %FileCheck --match-full-lines %S/checks/to-locale-lowercase.winglob
// RUN: %hermes_rt --intl-provider=system-icu %S/../../hermes/intl/to-locale-lowercase.js | %FileCheck --match-full-lines %S/../../hermes/intl/to-locale-lowercase.js
// REQUIRES: intl
