/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s
// REQUIRES: intl

function assert(pred, str) {
  if (!pred) {
    throw new Error('assertion failed' + (str === undefined ? '' : (': ' + str)));
  }
}

let resolvedOptions;

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD' }).resolvedOptions();
assert(resolvedOptions.minimumSignificantDigits === undefined);
assert(resolvedOptions.maximumSignificantDigits === undefined);

//
// Validate minimumSignificantDigits logic
//
try { new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumSignificantDigits: -1 }) }
catch (e) { assert(e.message.includes('minimumSignificantDigits value is invalid.')) }

try { new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumSignificantDigits: 22 }) }
catch (e) { assert(e.message.includes('minimumSignificantDigits value is invalid.')) }

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumSignificantDigits: 1 }).resolvedOptions();
assert(resolvedOptions.minimumSignificantDigits === 1);
assert(resolvedOptions.maximumSignificantDigits === 21);

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumSignificantDigits: 21 }).resolvedOptions();
assert(resolvedOptions.minimumSignificantDigits === 21);
assert(resolvedOptions.maximumSignificantDigits === 21);

//
// Validate maximumSignificantDigits logic
//
try { new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', maximumSignificantDigits: -1 }) }
catch (e) { assert(e.message.includes('maximumSignificantDigits value is invalid.')) }

try { new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', maximumSignificantDigits: 22 }) }
catch (e) { assert(e.message.includes('maximumSignificantDigits value is invalid.')) }

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', maximumSignificantDigits: 1 }).resolvedOptions();
assert(resolvedOptions.minimumSignificantDigits === 1);
assert(resolvedOptions.maximumSignificantDigits === 1);

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', maximumSignificantDigits: 21 }).resolvedOptions();
assert(resolvedOptions.minimumSignificantDigits === 1);
assert(resolvedOptions.maximumSignificantDigits === 21);

//
// Validate when both are set
//
try { new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumSignificantDigits: 5, maximumSignificantDigits: 2 }) }
catch (e) { assert(e.message.includes('maximumSignificantDigits value is invalid.')) }

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumSignificantDigits: 3, maximumSignificantDigits: 5 }).resolvedOptions();
assert(resolvedOptions.minimumSignificantDigits === 3);
assert(resolvedOptions.maximumSignificantDigits === 5);
