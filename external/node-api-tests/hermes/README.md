# Hermes Node.js NAPI Test Harness

This directory contains the harness for running the Node.js NAPI test
suite under Hermes. The test sources in `../js-native-api/` and
`../node-api/` are vendored from Node.js.

## Patches to Vendored Tests

The following vendored test files have been patched for Hermes
compatibility. All patches are marked with `[Hermes patch]` comments.

### V8-specific error message regexes

Hermes and V8 use different error message formats for strict-mode
TypeError. For example:

    V8:     "Cannot assign to read only property 'x' of object '#<Object>'"
    Hermes: "Cannot assign to read-only property 'x'"

    V8:     "Cannot set property x of #<Object> which has only a getter"
    Hermes: "Cannot assign to property 'x' which has only a getter"

The `assert.throws()` regexes in these tests were relaxed to accept both
formats while still verifying the correct error type and property name:

- `js-native-api/test_properties/test.js`
- `js-native-api/test_constructor/test.js`
- `js-native-api/6_object_wrap/test.js`

## Strict Mode Detection

The harness prepends assert/common/require code before each test, which
moves `'use strict'` directives out of top-of-script position. To
compensate, `run-node-tests.cmake` detects `'use strict'` at the top of
the original test source (after leading comments) and passes `-strict`
to hermes.
