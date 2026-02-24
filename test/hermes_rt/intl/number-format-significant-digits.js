// Run the upstream test via hermes.dll C API.
// RUN: %hermes_rt %S/../../hermes/intl/number-format-significant-digits.js
// RUN: %hermes_rt --intl-provider=system-icu %S/../../hermes/intl/number-format-significant-digits.js
// REQUIRES: intl
