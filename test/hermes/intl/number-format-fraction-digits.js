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
assert(resolvedOptions.minimumFractionDigits === 2);
assert(resolvedOptions.maximumFractionDigits === 2);

//
// Validate minimumFractionDigits logic
//
try { new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumFractionDigits: -1 }) }
catch (e) { assert(e.message.includes('minimumFractionDigits value is invalid.')) }

try { new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumFractionDigits: 21 }) }
catch (e) { assert(e.message.includes('minimumFractionDigits value is invalid.')) }

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumFractionDigits: 0 }).resolvedOptions();
assert(resolvedOptions.minimumFractionDigits === 0);
assert(resolvedOptions.maximumFractionDigits === 2);

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumFractionDigits: 20 }).resolvedOptions();
assert(resolvedOptions.minimumFractionDigits === 20);
assert(resolvedOptions.maximumFractionDigits === 20);

//
// Validate maximumFractionDigits logic
//
try { new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', maximumFractionDigits: -1 }) }
catch (e) { assert(e.message.includes('maximumFractionDigits value is invalid.')) }

try { new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', maximumFractionDigits: 21 }) }
catch (e) { assert(e.message.includes('maximumFractionDigits value is invalid.')) }

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', maximumFractionDigits: 0 }).resolvedOptions();
assert(resolvedOptions.minimumFractionDigits === 0);
assert(resolvedOptions.maximumFractionDigits === 0);

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', maximumFractionDigits: 20 }).resolvedOptions();
assert(resolvedOptions.minimumFractionDigits === 2);
assert(resolvedOptions.maximumFractionDigits === 20);

//
// Validate when both are set
//
try { new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumFractionDigits: 5, maximumFractionDigits: 2 }) }
catch (e) { assert(e.message.includes('minimumFractionDigits is greater than maximumFractionDigits')) }

resolvedOptions = new Intl.NumberFormat('en-US', {style: 'currency', currency: 'USD', minimumFractionDigits: 3, maximumFractionDigits: 5 }).resolvedOptions();
assert(resolvedOptions.minimumFractionDigits === 3);
assert(resolvedOptions.maximumFractionDigits === 5);
