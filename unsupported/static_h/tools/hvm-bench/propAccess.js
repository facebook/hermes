/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function access16Times(o) {
    sum = 0;
    sum += o.p0;
    sum += o.p1;
    sum += o.p2;
    sum += o.p3;
    sum += o.p0;
    sum += o.p1;
    sum += o.p2;
    sum += o.p3;
    sum += o.p0;
    sum += o.p1;
    sum += o.p2;
    sum += o.p3;
    sum += o.p0;
    sum += o.p1;
    sum += o.p2;
    sum += o.p3;
    return sum;
}

function access16NTimes(n) {
    sum = 0;
    o = {p0: 10, p1: 11, p2: 12, p3: 13};
    for (var i = 0; i < n; i++) {
        sum += access16Times(o)
    }
    return sum;
}

print(access16NTimes(10000000));
