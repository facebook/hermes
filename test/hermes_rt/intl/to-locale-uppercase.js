// Run the upstream intl/to-locale-uppercase.js test via hermes.dll C API.
// RUN: %hermes_rt %S/../../hermes/intl/to-locale-uppercase.js | %FileCheck --match-full-lines %S/../../hermes/intl/to-locale-uppercase.js
// RUN: %hermes_rt --intl-provider=winglob %S/../../hermes/intl/to-locale-uppercase.js | %FileCheck --match-full-lines %S/checks/to-locale-uppercase.winglob
// RUN: %hermes_rt --intl-provider=system-icu %S/../../hermes/intl/to-locale-uppercase.js | %FileCheck --match-full-lines %S/../../hermes/intl/to-locale-uppercase.js
// REQUIRES: intl
