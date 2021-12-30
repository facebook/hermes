/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function wbPerf(n) {
    var ogObj = { x: 2 };
    // Make sure the property storage exists before YG collection,
    // so it is promoted by the YG collection we cause below.
    ogObj.p = {};
    // Get it to the old gen: allocate enough to cause at least one YG GC.
    var a;
    for (var i = 0; i < 1000; i++) {
        a = new Array(1000);
    }
    var ygObj = { x: 1 };
    // Make sure the property storage exists.
    ygObj.p = {};
    for (var i = 0; i < n; i++) {
        for (var j = 0; j < 1000000; j++) {
            // We do 10 property stores in total.
            // 7 will write to YG locations: 6 with YG values, 1 with an OG value.
            ygObj.p = ygObj;
            ygObj.p = ygObj;
            ygObj.p = ygObj;
            ygObj.p = ygObj;
            ygObj.p = ygObj;
            ygObj.p = ygObj;

            ygObj.p = ogObj;

            // 3 write to OG location; 2 with YG values, one with an OG value.
            ogObj.p = ygObj;
            ogObj.p = ygObj;

            ogObj.p = ogObj;
        }
    }
    return ygObj.p.x + ogObj.p.x;
}

print(wbPerf(200));
