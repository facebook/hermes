/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

(function() {

function foo() { 'noinline'; }

var N = 10_000_000;
var start = Date.now();
for (var i = 0; i < N; ++i) foo.apply(this, arguments);
var end = Date.now();
print("Time: " + (end - start));

})();
