// Run the upstream intl/intl.js test via hermes.dll C API.
// RUN: %hermes_rt %S/../../hermes/intl/intl.js
// RUN: %hermes_rt --intl-provider=winglob %S/../../hermes/intl/intl.js
// RUN: %hermes_rt --intl-provider=system-icu %S/../../hermes/intl/intl.js
// REQUIRES: intl
