var initAsyncIterators = function() {
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

    var HermesAsyncIteratorsInternal = {
        _makeAsyncIterator
    };
    Object.freeze(HermesAsyncIteratorsInternal);
    globalThis.HermesAsyncIteratorsInternal = HermesAsyncIteratorsInternal;
};
