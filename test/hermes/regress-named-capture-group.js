/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s

// Check Regex doesn't crash due to an underlying unitialized propStorage_.
// There was a bug that the hidden class of the named capture group mapping
// object would not have a properly initialized propStorage_.
// Thus, once the number of named groups exceeded the number of direct
// property slots, it crashed when trying to assign properties to
// the uninitialized propStorage_.

var s = '';
for (let i = 0; i < 1000; i++) {
  s += `(?<foo${i}>a)`;
}
var re = new RegExp(s);
print(re.exec(""))
