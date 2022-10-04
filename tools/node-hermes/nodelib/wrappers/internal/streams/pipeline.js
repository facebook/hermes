// Ported from https://github.com/mafintosh/pump with
// permission from the author, Mathias Buus (@mafintosh).
'use strict';

function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) { try { var info = gen[key](arg); var value = info.value; } catch (error) { reject(error); return; } if (info.done) { resolve(value); } else { Promise.resolve(value).then(_next, _throw); } }

function _asyncToGenerator(fn) { return function () { var self = this, args = arguments; return new Promise(function (resolve, reject) { var gen = fn.apply(self, args); function _next(value) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value); } function _throw(err) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err); } _next(undefined); }); }; }

function _wrapAsyncGenerator(fn) { return function () { return new _AsyncGenerator(fn.apply(this, arguments)); }; }

function _AsyncGenerator(gen) { var front, back; function send(key, arg) { return new Promise(function (resolve, reject) { var request = { key: key, arg: arg, resolve: resolve, reject: reject, next: null }; if (back) { back = back.next = request; } else { front = back = request; resume(key, arg); } }); } function resume(key, arg) { try { var result = gen[key](arg); var value = result.value; var wrappedAwait = value instanceof _AwaitValue; Promise.resolve(wrappedAwait ? value.wrapped : value).then(function (arg) { if (wrappedAwait) { resume(key === "return" ? "return" : "next", arg); return; } settle(result.done ? "return" : "normal", arg); }, function (err) { resume("throw", err); }); } catch (err) { settle("throw", err); } } function settle(type, value) { switch (type) { case "return": front.resolve({ value: value, done: true }); break; case "throw": front.reject(value); break; default: front.resolve({ value: value, done: false }); break; } front = front.next; if (front) { resume(front.key, front.arg); } else { back = null; } } this._invoke = send; if (typeof gen["return"] !== "function") { this["return"] = undefined; } }

_AsyncGenerator.prototype[typeof Symbol === "function" && Symbol.asyncIterator || "@@asyncIterator"] = function () { return this; };

_AsyncGenerator.prototype.next = function (arg) { return this._invoke("next", arg); };

_AsyncGenerator.prototype["throw"] = function (arg) { return this._invoke("throw", arg); };

_AsyncGenerator.prototype["return"] = function (arg) { return this._invoke("return", arg); };

function _awaitAsyncGenerator(value) { return new _AwaitValue(value); }

function _AwaitValue(value) { this.wrapped = value; }

function _asyncIterator(iterable) { var method; if (typeof Symbol !== "undefined") { if (Symbol.asyncIterator) method = iterable[Symbol.asyncIterator]; if (method == null && Symbol.iterator) method = iterable[Symbol.iterator]; } if (method == null) method = iterable["@@asyncIterator"]; if (method == null) method = iterable["@@iterator"]; if (method == null) throw new TypeError("Object is not async iterable"); return method.call(iterable); }

function _asyncGeneratorDelegate(inner, awaitWrap) { var iter = {}, waiting = false; function pump(key, value) { waiting = true; value = new Promise(function (resolve) { resolve(inner[key](value)); }); return { done: false, value: awaitWrap(value) }; } ; iter[typeof Symbol !== "undefined" && Symbol.iterator || "@@iterator"] = function () { return this; }; iter.next = function (value) { if (waiting) { waiting = false; return value; } return pump("next", value); }; if (typeof inner["throw"] === "function") { iter["throw"] = function (value) { if (waiting) { waiting = false; throw value; } return pump("throw", value); }; } if (typeof inner["return"] === "function") { iter["return"] = function (value) { if (waiting) { waiting = false; return value; } return pump("return", value); }; } return iter; }

var _primordials = primordials,
    ArrayIsArray = _primordials.ArrayIsArray,
    _Promise = _primordials.Promise,
    SymbolAsyncIterator = _primordials.SymbolAsyncIterator;

var eos = require('internal/streams/end-of-stream');

var _require = require('internal/util'),
    once = _require.once;

var destroyImpl = require('internal/streams/destroy');

var _require2 = require('internal/errors'),
    aggregateTwoErrors = _require2.aggregateTwoErrors,
    _require2$codes = _require2.codes,
    ERR_INVALID_ARG_TYPE = _require2$codes.ERR_INVALID_ARG_TYPE,
    ERR_INVALID_RETURN_VALUE = _require2$codes.ERR_INVALID_RETURN_VALUE,
    ERR_MISSING_ARGS = _require2$codes.ERR_MISSING_ARGS,
    ERR_STREAM_DESTROYED = _require2$codes.ERR_STREAM_DESTROYED;

var _require3 = require('internal/validators'),
    validateCallback = _require3.validateCallback;

var _require4 = require('internal/streams/utils'),
    isIterable = _require4.isIterable,
    isReadableNodeStream = _require4.isReadableNodeStream,
    isNodeStream = _require4.isNodeStream;

var PassThrough;
var Readable;

function destroyer(stream, reading, writing, callback) {
  callback = once(callback);
  var finished = false;
  stream.on('close', function () {
    finished = true;
  });
  eos(stream, {
    readable: reading,
    writable: writing
  }, function (err) {
    finished = !err;
    var rState = stream._readableState;

    if (err && err.code === 'ERR_STREAM_PREMATURE_CLOSE' && reading && rState && rState.ended && !rState.errored && !rState.errorEmitted) {
      // Some readable streams will emit 'close' before 'end'. However, since
      // this is on the readable side 'end' should still be emitted if the
      // stream has been ended and no error emitted. This should be allowed in
      // favor of backwards compatibility. Since the stream is piped to a
      // destination this should not result in any observable difference.
      // We don't need to check if this is a writable premature close since
      // eos will only fail with premature close on the reading side for
      // duplex streams.
      stream.once('end', callback).once('error', callback);
    } else {
      callback(err);
    }
  });
  return function (err) {
    if (finished) return;
    finished = true;
    destroyImpl.destroyer(stream, err);
    callback(err || new ERR_STREAM_DESTROYED('pipe'));
  };
}

function popCallback(streams) {
  // Streams should never be an empty array. It should always contain at least
  // a single stream. Therefore optimize for the average case instead of
  // checking for length === 0 as well.
  validateCallback(streams[streams.length - 1]);
  return streams.pop();
}

function makeAsyncIterable(val) {
  if (isIterable(val)) {
    return val;
  } else if (isReadableNodeStream(val)) {
    // Legacy streams are not Iterable.
    return fromReadable(val);
  }

  throw new ERR_INVALID_ARG_TYPE('val', ['Readable', 'Iterable', 'AsyncIterable'], val);
}

function fromReadable(_x) {
  return _fromReadable.apply(this, arguments);
}

function _fromReadable() {
  _fromReadable = _wrapAsyncGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee(val) {
    return regeneratorRuntime.wrap(function _callee$(_context) {
      while (1) {
        switch (_context.prev = _context.next) {
          case 0:
            if (!Readable) {
              Readable = require('internal/streams/readable');
            }

            return _context.delegateYield(_asyncGeneratorDelegate(_asyncIterator(Readable.prototype[SymbolAsyncIterator].call(val)), _awaitAsyncGenerator), "t0", 2);

          case 2:
          case "end":
            return _context.stop();
        }
      }
    }, _callee);
  }));
  return _fromReadable.apply(this, arguments);
}

function pump(_x2, _x3, _x4) {
  return _pump.apply(this, arguments);
}

function _pump() {
  _pump = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee2(iterable, writable, finish) {
    var error, onresolve, resume, wait, cleanup, _iteratorNormalCompletion, _didIteratorError, _iteratorError, _iterator, _step, _value, chunk;

    return regeneratorRuntime.wrap(function _callee2$(_context2) {
      while (1) {
        switch (_context2.prev = _context2.next) {
          case 0:
            onresolve = null;

            resume = function resume(err) {
              if (err) {
                error = err;
              }

              if (onresolve) {
                var callback = onresolve;
                onresolve = null;
                callback();
              }
            };

            wait = function wait() {
              return new _Promise(function (resolve, reject) {
                if (error) {
                  reject(error);
                } else {
                  onresolve = function onresolve() {
                    if (error) {
                      reject(error);
                    } else {
                      resolve();
                    }
                  };
                }
              });
            };

            writable.on('drain', resume);
            cleanup = eos(writable, {
              readable: false
            }, resume);
            _context2.prev = 5;

            if (!writable.writableNeedDrain) {
              _context2.next = 9;
              break;
            }

            _context2.next = 9;
            return wait();

          case 9:
            _iteratorNormalCompletion = true;
            _didIteratorError = false;
            _context2.prev = 11;
            _iterator = _asyncIterator(iterable);

          case 13:
            _context2.next = 15;
            return _iterator.next();

          case 15:
            _step = _context2.sent;
            _iteratorNormalCompletion = _step.done;
            _context2.next = 19;
            return _step.value;

          case 19:
            _value = _context2.sent;

            if (_iteratorNormalCompletion) {
              _context2.next = 28;
              break;
            }

            chunk = _value;

            if (writable.write(chunk)) {
              _context2.next = 25;
              break;
            }

            _context2.next = 25;
            return wait();

          case 25:
            _iteratorNormalCompletion = true;
            _context2.next = 13;
            break;

          case 28:
            _context2.next = 34;
            break;

          case 30:
            _context2.prev = 30;
            _context2.t0 = _context2["catch"](11);
            _didIteratorError = true;
            _iteratorError = _context2.t0;

          case 34:
            _context2.prev = 34;
            _context2.prev = 35;

            if (!(!_iteratorNormalCompletion && _iterator["return"] != null)) {
              _context2.next = 39;
              break;
            }

            _context2.next = 39;
            return _iterator["return"]();

          case 39:
            _context2.prev = 39;

            if (!_didIteratorError) {
              _context2.next = 42;
              break;
            }

            throw _iteratorError;

          case 42:
            return _context2.finish(39);

          case 43:
            return _context2.finish(34);

          case 44:
            writable.end();
            _context2.next = 47;
            return wait();

          case 47:
            finish();
            _context2.next = 53;
            break;

          case 50:
            _context2.prev = 50;
            _context2.t1 = _context2["catch"](5);
            finish(error !== _context2.t1 ? aggregateTwoErrors(error, _context2.t1) : _context2.t1);

          case 53:
            _context2.prev = 53;
            cleanup();
            writable.off('drain', resume);
            return _context2.finish(53);

          case 57:
          case "end":
            return _context2.stop();
        }
      }
    }, _callee2, null, [[5, 50, 53, 57], [11, 30, 34, 44], [35,, 39, 43]]);
  }));
  return _pump.apply(this, arguments);
}

function pipeline() {
  for (var _len = arguments.length, streams = new Array(_len), _key = 0; _key < _len; _key++) {
    streams[_key] = arguments[_key];
  }

  var callback = once(popCallback(streams)); // stream.pipeline(streams, callback)

  if (ArrayIsArray(streams[0]) && streams.length === 1) {
    streams = streams[0];
  }

  if (streams.length < 2) {
    throw new ERR_MISSING_ARGS('streams');
  }

  var error;
  var value;
  var destroys = [];
  var finishCount = 0;

  function finish(err) {
    var _final = --finishCount === 0;

    if (err && (!error || error.code === 'ERR_STREAM_PREMATURE_CLOSE')) {
      error = err;
    }

    if (!error && !_final) {
      return;
    }

    while (destroys.length) {
      destroys.shift()(error);
    }

    if (_final) {
      callback(error, value);
    }
  }

  var ret;

  var _loop = function _loop(i) {
    var stream = streams[i];
    var reading = i < streams.length - 1;
    var writing = i > 0;

    if (isNodeStream(stream)) {
      finishCount++;
      destroys.push(destroyer(stream, reading, writing, finish));
    }

    if (i === 0) {
      if (typeof stream === 'function') {
        ret = stream();

        if (!isIterable(ret)) {
          throw new ERR_INVALID_RETURN_VALUE('Iterable, AsyncIterable or Stream', 'source', ret);
        }
      } else if (isIterable(stream) || isReadableNodeStream(stream)) {
        ret = stream;
      } else {
        throw new ERR_INVALID_ARG_TYPE('source', ['Stream', 'Iterable', 'AsyncIterable', 'Function'], stream);
      }
    } else if (typeof stream === 'function') {
      ret = makeAsyncIterable(ret);
      ret = stream(ret);

      if (reading) {
        if (!isIterable(ret, true)) {
          throw new ERR_INVALID_RETURN_VALUE('AsyncIterable', "transform[".concat(i - 1, "]"), ret);
        }
      } else {
        var _ret;

        if (!PassThrough) {
          PassThrough = require('internal/streams/passthrough');
        } // If the last argument to pipeline is not a stream
        // we must create a proxy stream so that pipeline(...)
        // always returns a stream which can be further
        // composed through `.pipe(stream)`.


        var pt = new PassThrough({
          objectMode: true
        }); // Handle Promises/A+ spec, `then` could be a getter that throws on
        // second use.

        var then = (_ret = ret) === null || _ret === void 0 ? void 0 : _ret.then;

        if (typeof then === 'function') {
          then.call(ret, function (val) {
            value = val;
            pt.end(val);
          }, function (err) {
            pt.destroy(err);
          });
        } else if (isIterable(ret, true)) {
          finishCount++;
          pump(ret, pt, finish);
        } else {
          throw new ERR_INVALID_RETURN_VALUE('AsyncIterable or Promise', 'destination', ret);
        }

        ret = pt;
        finishCount++;
        destroys.push(destroyer(ret, false, true, finish));
      }
    } else if (isNodeStream(stream)) {
      if (isReadableNodeStream(ret)) {
        ret.pipe(stream); // Compat. Before node v10.12.0 stdio used to throw an error so
        // pipe() did/does not end() stdio destinations.
        // Now they allow it but "secretly" don't close the underlying fd.

        if (stream === process.stdout || stream === process.stderr) {
          ret.on('end', function () {
            return stream.end();
          });
        }
      } else {
        ret = makeAsyncIterable(ret);
        finishCount++;
        pump(ret, stream, finish);
      }

      ret = stream;
    } else {
      var name = reading ? "transform[".concat(i - 1, "]") : 'destination';
      throw new ERR_INVALID_ARG_TYPE(name, ['Stream', 'Function'], stream);
    }
  };

  for (var i = 0; i < streams.length; i++) {
    _loop(i);
  } // TODO(ronag): Consider returning a Duplex proxy if the first argument
  // is a writable. Would improve composability.
  // See, https://github.com/nodejs/node/issues/32020


  return ret;
}

module.exports = pipeline;
