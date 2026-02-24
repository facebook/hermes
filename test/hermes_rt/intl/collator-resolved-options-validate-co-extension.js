// Run the upstream test via hermes.dll C API.
// RUN: %hermes_rt %S/../../hermes/intl/collator-resolved-options-validate-co-extension.js
// RUN: %hermes_rt --intl-provider=system-icu %S/../../hermes/intl/collator-resolved-options-validate-co-extension.js
// REQUIRES: intl
