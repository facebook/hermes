/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

var x = null;

function doAlloc() {
    for (var i = 0; i < 100000; i++) {
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
    }
}

function allocNTimes(n) {
    for (var i = 0; i < n; i++) {
        doAlloc()
    }
}

print(allocNTimes(100));
