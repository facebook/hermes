/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function initAsyncIterators() {
    var HermesPromise = globalThis.Promise;

    /**
     * Creates an asynchronous iterator for the given iterable.
     *
     * This function checks if the iterable already has an async iterator.
     * If it does, it returns the existing async iterator.
     * If not, it constructs an async iterator from the iterable's synchronous iterator.
     * The constructed async iterator resolves any promise values within its methods.
     *
     * @param {Iterable} iterable - The iterable object for which to create an async iterator.
     * @returns {AsyncIterator} - An asynchronous iterator for the given iterable.
     */
    function _makeAsyncIterator(iterable) {
        if (iterable[Symbol.asyncIterator]) {
            return iterable[Symbol.asyncIterator]();
        }

        var syncIterator = iterable[Symbol.iterator]();

        // Close the sync iterator, ignoring errors from .return().
        // Used by close-on-rejection paths where IteratorClose is
        // called with an abrupt completion: per §7.4.11 step 5, the
        // original abrupt completion preempts any close error.
        function closeIterator() {
            try {
                var ret = syncIterator.return;
                if (typeof ret === 'function') {
                    ret.call(syncIterator);
                }
            } catch (e) {}
        }

        // Per spec (AsyncFromSyncIteratorContinuation):
        // 1. Read .done and .value from the sync result
        // 2. Wrap the value via PromiseResolve(%Promise%, value)
        // 3. If done is false and closeOnRejection is true, attach an
        //    onRejected handler that calls IteratorClose before propagating
        function handleResult(result, closeOnRejection) {
            // IteratorComplete: result must be an object (functions
            // are objects per ES2025 §6.1.7).
            if (result === null ||
                (typeof result !== 'object' && typeof result !== 'function')) {
                throw new TypeError(
                    'iterator result is not an object');
            }
            var done = result.done;
            var value = result.value;
            if (done) {
                return HermesPromise.resolve(value).then(function(v) {
                    return { done: true, value: v };
                });
            }
            // Wrap value via PromiseResolve. If the value is a rejected
            // promise or a thenable whose .then throws, the onRejected
            // handler closes the sync iterator per spec step 13.
            var valueWrapper;
            try {
                valueWrapper = HermesPromise.resolve(value);
            } catch (e) {
                if (closeOnRejection) {
                    closeIterator();
                }
                throw e;
            }
            return valueWrapper.then(function(v) {
                return { done: false, value: v };
            }, closeOnRejection ? function(error) {
                closeIterator();
                throw error;
            } : undefined);
        }

        return {
            next() {
                try {
                    return handleResult(syncIterator.next(), true);
                } catch (e) {
                    return HermesPromise.reject(e);
                }
            },
            // Per spec §27.1.6.2.2 step 9-10: if value is not present,
            // call return with no arguments.
            return() {
                try {
                    var hasArg = arguments.length > 0;
                    var arg = arguments[0];
                    // Spec GetMethod reads `.return` once; cache to
                    // avoid invoking a getter twice.
                    var retMethod = syncIterator.return;
                    var result;
                    if (typeof retMethod === 'function') {
                        result = hasArg
                            ? retMethod.call(syncIterator, arg)
                            : retMethod.call(syncIterator);
                    } else {
                        result = { done: true, value: arg };
                    }
                    return handleResult(result, false);
                } catch (e) {
                    return HermesPromise.reject(e);
                }
            },
            throw(error) {
                try {
                    // Spec GetMethod reads `.throw` once; cache to
                    // avoid invoking a getter twice.
                    var throwMethod = syncIterator.throw;
                    if (typeof throwMethod !== 'function') {
                        // Per spec §27.1.6.2.3 step 8: close the
                        // iterator, then throw TypeError. The close
                        // is invoked with NormalCompletion, so per
                        // §7.4.11 step 4.d / step 6, a throw from
                        // .return() OR a non-Object result preempts
                        // the "no throw method" TypeError (step 8.d's
                        // IfAbruptRejectPromise on IteratorClose).
                        var ret = syncIterator.return;
                        if (typeof ret === 'function') {
                            var closeResult = ret.call(syncIterator);
                            if (closeResult === null ||
                                (typeof closeResult !== 'object' &&
                                 typeof closeResult !== 'function')) {
                                throw new TypeError(
                                    "iterator.return() did not return an object");
                            }
                        }
                        throw new TypeError(
                            "The iterator does not provide a 'throw' method.");
                    }
                    return handleResult(
                        throwMethod.call(syncIterator, error),
                        true);
                } catch (e) {
                    return HermesPromise.reject(e);
                }
            }
        };
    }

    // Register as "makeAsyncIterator"
    internalBytecodeResult.makeAsyncIterator = _makeAsyncIterator;
}

function initAsyncGenerators() {
    var HermesPromise = globalThis.Promise;

    // https://github.com/babel/babel/blob/dcfa42b933299644c4e78168cb4678ecbfae67ed/packages/babel-runtime-corejs3/helpers/esm/wrapAsyncGenerator.js
    function OverloadYield(value, kind) {
        this.value = value, this.kind = kind;
    }

    /**
     * Implements an asynchronous generator to handle async iteration.
     *
     * This function manages state transitions (next, throw, return)
     * through promises to mimic JavaScript's native async generator behavior.
     * https://github.com/babel/babel/blob/dcfa42b933299644c4e78168cb4678ecbfae67ed/packages/babel-runtime-corejs3/helpers/esm/awaitAsyncGenerator.js
     */
    function AsyncGenerator(generatorFunction) {
        var currentPromise, resolveQueue;

        function resume(key, argument) {
            try {
                var nextIteration = generatorFunction[key](argument),
                    iterationValue = nextIteration.value,
                    isOverloadYield = iterationValue instanceof OverloadYield;
                HermesPromise.resolve(isOverloadYield ? iterationValue.value : iterationValue).then(function (resolvedValue) {
                    if (isOverloadYield) {
                        var nextKey = "return" === key ? "return" : "next";
                        if (!iterationValue.kind) {
                            // kind=0 (await): always resume with the
                            // resolved value.
                            return resume(nextKey, resolvedValue);
                        }
                        // kind=1 (delegate yield*). Per spec §15.5.5
                        // (yield * AssignmentExpression) step 8.a.iv reads
                        // .done once via IteratorComplete; step 8.a.v.1
                        // (if done) or step 8.a.vi (if not, inside
                        // AsyncGeneratorYield(? IteratorValue(...))) reads
                        // .value once. Validate the result is an object
                        // (TypeError if not, per step 8.a.iii) and cache
                        // both .done and .value up front so the regular
                        // generator's yield* protocol below sees them as
                        // plain data properties on a fresh object instead
                        // of re-invoking the original getters. If
                        // validation or .done/.value throws, catch it and
                        // throw into the generator so yield* can unwind
                        // properly and the generator enters completed
                        // state.
                        try {
                            if (resolvedValue === null ||
                                (typeof resolvedValue !== 'object' &&
                                 typeof resolvedValue !== 'function')) {
                                throw new TypeError(
                                    "iterator.next() did not return an object");
                            }
                            var done = resolvedValue.done;
                            var value = resolvedValue.value;
                        } catch (e) {
                            resume("throw", e);
                            return;
                        }
                        if (done) {
                            return resume(nextKey, { done: done, value: value });
                        }
                        resolvedValue = generatorFunction[nextKey]({ done: done, value: value }).value;
                    }
                    conclude(nextIteration.done ? "return" : "normal", resolvedValue);
                }, function (error) {
                    resume("throw", error);
                });
            } catch (error) {
                conclude("throw", error);
            }
        }

        function conclude(type, result) {
            switch (type) {
                case "return":
                    currentPromise.resolve({ value: result, done: true });
                    break;
                case "throw":
                    currentPromise.reject(result);
                    break;
                default:
                    currentPromise.resolve({ value: result, done: false });
            }
            (currentPromise = currentPromise.next) ? resume(currentPromise.key, currentPromise.arg) : resolveQueue = null;
        }

        this._invoke = function (key, argument) {
            return new HermesPromise(function (resolve, reject) {
                var promiseCapability = {
                    key: key,
                    arg: argument,
                    resolve: resolve,
                    reject: reject,
                    next: null
                };
                resolveQueue ? resolveQueue = resolveQueue.next = promiseCapability : (currentPromise = resolveQueue = promiseCapability, resume(key, argument));
            });
        };

        if (typeof generatorFunction["return"] !== "function") {
            this["return"] = void 0;
        }
    }

    AsyncGenerator.prototype[Symbol.asyncIterator] = function () {
        return this;
    };
    AsyncGenerator.prototype.next = function (value) {
        return this._invoke("next", value);
    };
    AsyncGenerator.prototype["throw"] = function (error) {
        return this._invoke("throw", error);
    };
    AsyncGenerator.prototype["return"] = function (value) {
        return this._invoke("return", value);
    };

    // https://github.com/babel/babel/blob/dcfa42b933299644c4e78168cb4678ecbfae67ed/packages/babel-runtime-corejs3/helpers/esm/wrapAsyncGenerator.js
    function _wrapAsyncGenerator(generatorFunction) {
        return function () {
            return new AsyncGenerator(generatorFunction.apply(this, arguments));
        };
    }

    // https://github.com/babel/babel/blob/dcfa42b933299644c4e78168cb4678ecbfae67ed/packages/babel-runtime-corejs3/helpers/esm/awaitAsyncGenerator.js
    function _awaitAsyncGenerator(value) {
        return new OverloadYield(value, 0);
    }

    // https://github.com/babel/babel/blob/dcfa42b933299644c4e78168cb4678ecbfae67ed/packages/babel-helpers/src/helpers/asyncGeneratorDelegate.ts
    function _asyncGeneratorDelegate(iterable) {
        var inner;
        var isSyncIterator;
        var asyncIteratorMethod = iterable[Symbol.asyncIterator];
        if (asyncIteratorMethod != null) {
            inner = asyncIteratorMethod.call(iterable);
            isSyncIterator = false;
        } else {
            inner = iterable[Symbol.iterator]();
            isSyncIterator = true;
        }
        // Per spec §7.4.2 GetIteratorDirect step 1, cache the `next`
        // method at iterator creation in IteratorRecord.[[NextMethod]];
        // subsequent yield* iterations call the cached reference rather
        // than re-reading from the iterator object. `throw` and `return`
        // are NOT cached by spec — they are looked up fresh via
        // GetMethod at each use site (§15.5.5 steps 8.b.i / 8.c.ii).
        var nextMethod = inner.next;
        var iter = {}, waiting = false;
        function getMethod(obj, key) {
            var method = obj[key];
            if (method == null) {
                return undefined;
            }
            if (typeof method !== "function") {
                throw new TypeError(key + " is not a function");
            }
            return method;
        }
        function closeSyncIteratorFromRejectedValue(error) {
            // IteratorClose with a throw completion still performs the
            // `return` lookup/call for side effects, but the original throw
            // completion wins over any abrupt completion from closing.
            try {
                var retMethod = getMethod(inner, "return");
                if (retMethod !== undefined) {
                    try {
                        retMethod.call(inner);
                    } catch (e) {
                        // IteratorClose with a throw completion preserves
                        // the original completion.
                    }
                }
            } catch (e) {
                // IteratorClose with a throw completion preserves the
                // original completion.
            }
            throw error;
        }
        // Per spec §27.1.6.4 AsyncFromSyncIteratorContinuation: when
        // wrapping a sync iterator as async, the result is validated as
        // an Object (steps 1-4), the value is wrapped via
        // PromiseResolve(%Promise%, value) (step 7), and an unwrap
        // closure that returns CreateIterResultObject is attached via
        // PerformPromiseThen (step 17). Both done=true and done=false
        // paths Await the value.
        function wrapSyncResult(result, closeOnRejection) {
            if (result === null ||
                (typeof result !== 'object' &&
                 typeof result !== 'function')) {
                throw new TypeError(
                    "iterator.next() did not return an object");
            }
            var done = result.done;
            var value = result.value;
            var valueWrapper;
            try {
                valueWrapper = HermesPromise.resolve(value);
            } catch (e) {
                if (closeOnRejection && !done) {
                    closeSyncIteratorFromRejectedValue(e);
                }
                throw e;
            }
            var onFulfilled = function (v) {
                return { done: done, value: v };
            };
            if (done || !closeOnRejection) {
                return valueWrapper.then(onFulfilled);
            }
            return valueWrapper.then(
                onFulfilled,
                closeSyncIteratorFromRejectedValue);
        }
        function pump(key, value) {
            waiting = true;
            value = new HermesPromise(function (resolve) {
                var result = key === "next"
                    ? nextMethod.call(inner, value)
                    : pumpThrowReturn(key, value);
                // For sync inner, route results through
                // wrapSyncResult so the value is Awaited per
                // AsyncFromSyncIteratorContinuation. For async inner,
                // pumpThrowReturn / nextMethod returns a Promise that
                // already encapsulates the spec semantics.
                resolve(
                    isSyncIterator
                        ? wrapSyncResult(result, key !== "return")
                        : result);
            });
            return { done: false, value: new OverloadYield(value, 1) };
        }
        // Handle throw/return with lazy method lookup per spec.
        // For sync iterators wrapped as async delegates, the spec's
        // %AsyncFromSyncIteratorPrototype% checks for .throw/.return
        // at call time, not at construction time.
        function pumpThrowReturn(key, value) {
            var method = getMethod(inner, key);
            if (method === undefined) {
                if (key === "throw") {
                    // Spec §15.5.5 step 8.b.iii: close the iterator
                    // first, then throw TypeError. For async inner
                    // (step 8.b.iii.3 AsyncIteratorClose §7.4.13) the
                    // close result is Awaited and validated as Object
                    // per AsyncIteratorClose step 4.e; for sync inner
                    // (step 8.b.iii.4 IteratorClose §7.4.11) the close
                    // is sync with the same Object validation at
                    // IteratorClose §7.4.11 step 4.d. In both shapes, close
                    // errors preempt the "no throw" TypeError per
                    // IfAbruptRejectPromise semantics.
                    var retMethod = getMethod(inner, "return");
                    if (retMethod === undefined) {
                        throw new TypeError(
                            "yield* delegate must have a .throw() method");
                    }
                    if (isSyncIterator) {
                        var retResult = retMethod.call(inner);
                        if (retResult === null ||
                            (typeof retResult !== "object" &&
                             typeof retResult !== "function")) {
                            throw new TypeError(
                                "iterator.return() did not return an object");
                        }
                        throw new TypeError(
                            "yield* delegate must have a .throw() method");
                    }
                    return HermesPromise.resolve(retMethod.call(inner)).then(
                        function (result) {
                            if (result === null ||
                                (typeof result !== "object" &&
                                 typeof result !== "function")) {
                                throw new TypeError(
                                    "iterator.return() did not return an object");
                            }
                            throw new TypeError(
                                "yield* delegate must have a .throw() method");
                        });
                }
                // key === "return" and no .return method.
                // Spec §15.5.5 step 8.c.iii.2: for async generatorKind
                // value is Awaited before being wrapped in
                // ReturnCompletion. For async inner we Await here; for
                // sync inner we return a sync result so wrapSyncResult
                // can apply the required Await uniformly with the next()
                // path.
                if (isSyncIterator) {
                    return { value: value, done: true };
                }
                return HermesPromise.resolve(value).then(function (awaited) {
                    return { value: awaited, done: true };
                });
            }
            return method.call(inner, value);
        }
        iter[Symbol.iterator] = function () { return this; };
        iter.next = function (value) {
            if (waiting) { waiting = false; return value; }
            return pump("next", value);
        };
        // Always install throw and return so the regular generator's
        // yield* protocol can forward them. The lazy lookup in
        // pumpThrowReturn handles the case where the inner iterator
        // doesn't have the method.
        iter["throw"] = function (value) {
            if (waiting) { waiting = false; throw value; }
            return pump("throw", value);
        };
        iter["return"] = function (value) {
            if (waiting) { waiting = false; return value; }
            return pump("return", value);
        };
        return iter;
    }

    // Register _awaitAsyncGenerator so it can be called via GetBuiltinClosure
    // from IRGen for `for await...of` inside async generators.
    internalBytecodeResult.awaitAsyncGenerator = _awaitAsyncGenerator;

    // Since async generators require an AST transformation, using internalBytecodeResult is not feasible.
    var HermesAsyncIteratorsInternal = {
        _wrapAsyncGenerator,
        _awaitAsyncGenerator,
        _asyncGeneratorDelegate,
    };
    Object.freeze(HermesAsyncIteratorsInternal);
    globalThis.HermesAsyncIteratorsInternal = HermesAsyncIteratorsInternal;
}

initAsyncIterators();
initAsyncGenerators();
