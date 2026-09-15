/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * Hermes implements async functions by lowering each one to an ordinary
 * function that passes an inner generator to spawnAsync. Conceptually:
 *
 *   async function f() {
 *     return await value;
 *   }
 *
 * becomes:
 *
 *   function f() {
 *     return spawnAsync(function* () {
 *       return yield value;
 *     }, this, arguments);
 *   }
 *
 * The generator runs synchronously until it yields at an await boundary.
 * spawnAsync normalizes the yielded value to a Promise, then injects its
 * fulfillment back with generator.next() or its rejection with
 * generator.throw(). A final generator return resolves the async function's
 * Promise, while an uncaught throw rejects it.
 *
 * An invocation creates one Promise reaction when it first reaches an await
 * boundary, then reuses it for all subsequent awaits. Functions that complete
 * synchronously do not allocate a reaction at all.
 */

function initAsyncFn() {
  // promiseCapability.[[Promise]]
  var HermesPromise = globalThis.Promise;

  // Capture the polyfill's internal then-equivalent (exposed by
  // 01-Promise.js). Unlike Promise.prototype.then, it skips IsPromise
  // and SpeciesConstructor checks, so the only user-observable
  // `.constructor` read in Await comes from the explicit check below
  // (matching spec §27.2.1.6 PromiseResolve step 1).
  var performInternalThen = internalBytecodeResult.performInternalThen;
  var createInternalHandler = internalBytecodeResult.createInternalHandler;
  var createPendingPromiseCore =
    internalBytecodeResult.createPendingPromiseCore;
  var resolvePromiseCore = internalBytecodeResult.resolvePromiseCore;
  var rejectPromiseCore = internalBytecodeResult.rejectPromiseCore;
  // PromiseResolve after the exact-Promise check below has failed.
  var promiseResolveCoreNonExact =
    internalBytecodeResult.promiseResolveCoreNonExact;

  // Build the reaction pair for one invocation. Kept out of resume() so that
  // resume() captures nothing and so needs no environment of its own.
  function makeHandler(gen, resultPromise) {
    var handler = createInternalHandler(
      function (value) {
        resume(gen, resultPromise, handler, false, value);
      },
      function (reason) {
        resume(gen, resultPromise, handler, true, reason);
      },
      resultPromise
    );
    return handler;
  }

  // Close a generator that can no longer be resumed so that `finally` blocks
  // in the original function body still run, then report the failure on the
  // async function's Promise. A `finally` that throws replaces the original
  // error, as it would for an ordinary return; one that awaits cannot be
  // driven any further and is dropped.
  function abandon(gen, resultPromise, error) {
    try {
      gen.return(undefined);
    } catch (e) {
      error = e;
    }
    rejectPromiseCore(resultPromise, error);
  }

  // Advance an async function's generator by one step. A fulfilled await
  // resumes normally; a rejected await is thrown at the suspended yield so
  // that try/catch in the original async function can observe it.
  function resume(gen, resultPromise, handler, isThrow, value) {
    while (true) {
      var next;
      try {
        next = isThrow ? gen.throw(value) : gen.next(value);
      } catch (e) {
        // This includes uncaught exceptions from the original function body.
        rejectPromiseCore(resultPromise, e);
        return;
      }
      if (next.done) {
        // The generator's return value is the async function's result.
        resolvePromiseCore(resultPromise, next.value);
        return;
      }

      // A non-final generator result represents an await boundary. Its value
      // is the operand produced by the lowered `yield`.
      // Per spec §27.7.5.3 Await + §27.2.1.6 PromiseResolve step 1:
      // return val as-is only when IsPromise(val) AND
      // val.constructor === %Promise%. Subclass instances and thenables fall
      // through to the wrap path so their custom .then / .constructor are
      // observed via the Promise Resolution Procedure.
      var val = next.value;
      var p;
      try {
        if (
          val instanceof HermesPromise &&
          val.constructor === HermesPromise
        ) {
          p = val;
        } else {
          p = promiseResolveCoreNonExact(val);
        }
      } catch (e) {
        // PromiseResolve errors happen at the await expression. Feed them back
        // through the generator so the original function's try/catch sees
        // them. Looping avoids growing the native stack if that catch reaches
        // another await whose normalization also throws.
        isThrow = true;
        value = e;
        continue;
      }
      if (handler === null) {
        // Delay this allocation until the generator actually suspends.
        handler = makeHandler(gen, resultPromise);
      }
      // Promise reactions enqueue the next generator step. The same handler
      // is reused when that step reaches another await, since an async
      // function has at most one await outstanding.
      try {
        performInternalThen(p, handler);
      } catch (e) {
        // Nothing will resume the generator now that attaching its
        // continuation has failed.
        abandon(gen, resultPromise, e);
      }
      return;
    }
  }

  // This spawn function is borrowed from the
  // [original proposal](https://github.com/tc39/proposal-async-await),
  // then modified to use captured Promise operations that user code cannot
  // replace and to receive the original arguments as a third parameter.
  function spawn(genF, self, args) {
    // Async execution has one terminal completion, so its result can use a
    // pending core Promise directly without allocating resolving functions.
    var resultPromise = createPendingPromiseCore();
    var gen;

    try {
      // IRGen passes the inner generator function along with the async
      // function's original receiver and arguments.
      gen = genF.apply(self, args);
      // The first next() argument is ignored by a generator. This starts the
      // original function body synchronously and runs it to its first await.
      // Reaction callbacks are created only if the generator suspends.
      resume(gen, resultPromise, null, false, undefined);
    } catch (e) {
      rejectPromiseCore(resultPromise, e);
    }
    return resultPromise;
  }

  // register as "spawnAsync".
  internalBytecodeResult.spawnAsync = spawn;
}

initAsyncFn();
