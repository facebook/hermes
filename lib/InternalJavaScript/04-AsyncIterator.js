// References:
// https://github.com/babel/babel/blob/dcfa42b933299644c4e78168cb4678ecbfae67ed/packages/babel-runtime-corejs3/helpers/esm/OverloadYield.js
// https://github.com/babel/babel/blob/dcfa42b933299644c4e78168cb4678ecbfae67ed/packages/babel-runtime-corejs3/helpers/esm/wrapAsyncGenerator.js
// https://github.com/babel/babel/blob/dcfa42b933299644c4e78168cb4678ecbfae67ed/packages/babel-runtime-corejs3/helpers/esm/awaitAsyncGenerator.js
// https://babel.dev/repl#?browsers=chrome%2062&build=&builtIns=false&corejs=false&spec=false&loose=false&code_lz=IYZwngdgxgBAZgV2gFwJYHsICp7vQCgEoYBvAKBhmAHdhVkYAFAJ3QFtUQBTAOma5DoANgDcu-AOTAJhANwBICjDCouQgCYwAjAAZZSlWs006DFu069-g0eIkAjGfsqGNVWvSasO3PgOFiklBOBqpuACxkAL5kQA&debug=false&forceAllTransforms=true&modules=false&shippedProposals=true&evaluate=false&fileSize=true&timeTravel=false&sourceType=module&lineWrap=true&presets=env&prettier=true&targets=&version=7.13.15&externalPlugins=&assumptions=%7B%7D

(function() {
    function _OverloadYield(e, d) {
        this.v = e, this.k = d;
    }

    function _AsyncGenerator(e) {
        var r, t;
        function resume(r, t) {
            try {
                var n = e[r](t),
                    o = n.value,
                    u = o instanceof _OverloadYield;
                Promise.resolve(u ? o.v : o).then(function (t) {
                    if (u) {
                        var i = "return" === r ? "return" : "next";
                        if (!o.k || t.done) return resume(i, t);
                        t = e[i](t).value;
                    }
                    settle(n.done ? "return" : "normal", t);
                }, function (e) {
                    resume("throw", e);
                });
            } catch (e) {
                settle("throw", e);
            }
        }
        function settle(e, n) {
            switch (e) {
                case "return":
                    r.resolve({
                        value: n,
                        done: !0
                    });
                    break;
                case "throw":
                    r.reject(n);
                    break;
                default:
                    r.resolve({
                        value: n,
                        done: !1
                    });
            }
            (r = r.next) ? resume(r.key, r.arg) : t = null;
        }
        this._invoke = function (e, n) {
            return new Promise(function (o, u) {
                var i = {
                    key: e,
                    arg: n,
                    resolve: o,
                    reject: u,
                    next: null
                };
                t ? t = t.next = i : (r = t = i, resume(e, n));
            });
        }, "function" != typeof e["return"] && (this["return"] = void 0);
    }

    _AsyncGenerator.prototype[Symbol.asyncIterator] = function () {
        return this;
    }, _AsyncGenerator.prototype.next = function (e) {
        return this._invoke("next", e);
    }, _AsyncGenerator.prototype["throw"] = function (e) {
        return this._invoke("throw", e);
    }, _AsyncGenerator.prototype["return"] = function (e) {
        return this._invoke("return", e);
    };

    function _wrapAsyncGenerator(e) {
        return function () {
            return new _AsyncGenerator(e.apply(this, arguments));
        };
    }

    function _awaitAsyncGenerator(e) {
        return new _OverloadYield(e, 0);
    }

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
            // If the value is a promise, wait for it to resolve.
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
        _makeAsyncIterator,
        _wrapAsyncGenerator,
        _awaitAsyncGenerator,
    };
    Object.freeze(HermesAsyncIteratorsInternal);
    globalThis.HermesAsyncIteratorsInternal = HermesAsyncIteratorsInternal;
})();
