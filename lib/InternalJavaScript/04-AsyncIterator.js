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
        async function handleResult(result) {
            if (result.done) {
                return result;
            }
            var value = result.value;
            var resolvedValue = value instanceof HermesPromise ? await value : value;
            return { done: false, value: resolvedValue };
        }

        return {
            async next() {
                var result = syncIterator.next();
                return handleResult(result);
            },
            async return(value) {
                var result = typeof syncIterator.return === 'function' ? syncIterator.return(value) : { done: true, value };
                return handleResult(result);
            },
            async throw(error) {
                var result = typeof syncIterator.throw === 'function' ? syncIterator.throw(error) : { done: true, value: error };
                return handleResult(result);
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
                        if (!iterationValue.kind || resolvedValue.done) {
                            return resume(nextKey, resolvedValue);
                        }
                        resolvedValue = generatorFunction[nextKey](resolvedValue).value;
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

    // Since async generators require an AST transformation, using internalBytecodeResult is not feasible.
    var HermesAsyncIteratorsInternal = {
        _wrapAsyncGenerator,
        _awaitAsyncGenerator,
    };
    Object.freeze(HermesAsyncIteratorsInternal);
    globalThis.HermesAsyncIteratorsInternal = HermesAsyncIteratorsInternal;
}

initAsyncIterators();
initAsyncGenerators();
