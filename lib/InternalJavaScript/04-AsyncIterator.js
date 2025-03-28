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

initAsyncIterators();