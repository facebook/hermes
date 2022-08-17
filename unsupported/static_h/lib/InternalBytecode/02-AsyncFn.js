/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function initAsyncFn() {
  // promiseCapability.[[Promise]]
  var HermesPromise = globalThis.Promise;

  // %Promise_resolve%
  var HermesPromiseResolve = HermesPromise.resolve.bind(HermesPromise);

  // This spawn function is borrowed from the
  // [original proposal](https://github.com/tc39/proposal-async-await),
  // then it's modified to
  // - use the captured Promise and methods to immune from user-space hijacking.
  // - to take a third argument "args".
  // TODO(the Babel version seem to be a little bit faster.)
  function spawn(genF, self, args) {
    return new HermesPromise(function (resolve, reject) {
      var gen = genF.apply(self, args);
      function step(nextF) {
        var next;
        try {
          next = nextF();
        } catch (e) {
          // finished with failure, reject the promise
          reject(e);
          return;
        }
        if (next.done) {
          // finished with success, resolve the promise
          resolve(next.value);
          return;
        }
        // not finished, chain off the yielded promise and `step` again
        HermesPromiseResolve(next.value).then(
          function (v) {
            step(function () {
              return gen.next(v);
            });
          },
          function (e) {
            step(function () {
              return gen.throw(e);
            });
          }
        );
      }
      step(function () {
        return gen.next(undefined);
      });
    });
  }

  // register as "spawnAsync".
  internalBytecodeResult.spawnAsync = spawn;
}

// Async functions can only work with Promise enabled.
if (HermesInternal?.hasPromise?.()) {
  initAsyncFn();
} else {
  // Make sure we maintain the invariant that builtin is always
  // populated, and error out when Promise is disabled.
  internalBytecodeResult.spawnAsync = function (){
    throw Error("async function cannot be used with Promise disabled. spawnAsync not registered.");
  };
}
