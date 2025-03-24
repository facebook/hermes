function initAsyncIterators() {
    function _makeAsyncIterator(iterable, asyncIterMethod, syncIterMethod) {
        if (asyncIterMethod) {
            return asyncIterMethod.call(iterable);
        }

        let syncIterator = syncIterMethod.call(iterable);
        async function handleResult(result) {
            if (result.done) {
                return result;
            }
            const value = result.value;
            const resolvedValue = value instanceof Promise ? await value : value;
            return { done: false, value: resolvedValue };
        }

        return {
            async next() {
                const result = syncIterator.next();
                return handleResult(result);
            },
            async return(value) {
                const result = typeof syncIterator.return === 'function' ? syncIterator.return(value) : { done: true, value };
                return handleResult(result);
            },
            async throw(error) {
                const result = typeof syncIterator.throw === 'function' ? syncIterator.throw(error) : { done: true, value: error };
                return handleResult(result);
            }
        };
    }

    // Register as "makeAsyncIterator"
    internalBytecodeResult.makeAsyncIterator = _makeAsyncIterator;
}

// Async operations require Promise support
if (HermesInternal?.hasPromise?.()) {
    initAsyncIterators();
} else {
    // Ensure the invariant is maintained and error if Promise is unavailable
    internalBytecodeResult.makeAsyncIterator = function() {
        throw Error("Async iterators cannot be used with Promise disabled. makeAsyncIterator not registered.");
    };
}