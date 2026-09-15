/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Per spec §27.1.6.2.1 step 9.b, the async iterator wrapper's .next()
// must always return a Promise. When the underlying sync iterator's
// .next() returns a non-object (e.g. null), the wrapper must reject
// the returned promise with TypeError — NOT throw synchronously.
//
// Detection: schedule microtasks before for-await runs. The await on
// next()'s rejected promise yields to microtasks, so they run between
// "before" and "caught". A buggy sync throw skips the await hop and
// runs "caught" before the microtasks.

var bad = {
    [Symbol.iterator]() { return { next() { return null; } }; }
};

var log = [];
Promise.resolve().then(function() { log.push("m1"); });
Promise.resolve().then(function() { log.push("m2"); });

function testRejectedValueCloseGetterError() {
    var closeGetterCalled = false;
    var iterable = {
        [Symbol.iterator]: function() {
            return {
                next: function() {
                    return {
                        done: false,
                        value: Promise.reject("value-reject"),
                    };
                },
                get return() {
                    closeGetterCalled = true;
                    throw "return-getter";
                },
            };
        },
    };

    return (async function() {
        try {
            for await (var v of iterable) {}
            print("test2: resolved");
        } catch (e) {
            print("test2:", e, closeGetterCalled);
        }
    })();
}

function testPoisonedPromiseResolveCloses() {
    function PoisonedConstructorError() {}
    var poisonedError = new PoisonedConstructorError();
    var returnCalled = false;
    var iterable = {
        [Symbol.iterator]: function() {
            return {
                next: function() {
                    var value = Promise.resolve("value");
                    Object.defineProperty(value, "constructor", {
                        get: function() { throw poisonedError; },
                    });
                    return {done: false, value: value};
                },
                return: function() {
                    returnCalled = true;
                    return {done: true};
                },
            };
        },
    };

    return (async function() {
        try {
            for await (var v of iterable) {}
            print("test3: resolved");
        } catch (e) {
            print("test3:", e instanceof PoisonedConstructorError, returnCalled);
        }
    })();
}

(async function() {
    log.push("before");
    try {
        for await (var v of bad) {}
    } catch (e) {
        log.push("caught:" + (e instanceof TypeError));
    }
    log.push("after");
})().then(function() {
    print(log.join(","));
    return testRejectedValueCloseGetterError();
}).then(function() {
    return testPoisonedPromiseResolveCloses();
});

// CHECK: before,m1,m2,caught:true,after
// CHECK-NEXT: test2: value-reject true
// CHECK-NEXT: test3: true true
