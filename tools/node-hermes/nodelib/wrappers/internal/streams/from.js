'use strict';

function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) { try { var info = gen[key](arg); var value = info.value; } catch (error) { reject(error); return; } if (info.done) { resolve(value); } else { Promise.resolve(value).then(_next, _throw); } }

function _asyncToGenerator(fn) { return function () { var self = this, args = arguments; return new Promise(function (resolve, reject) { var gen = fn.apply(self, args); function _next(value) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value); } function _throw(err) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err); } _next(undefined); }); }; }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

var _primordials = primordials,
    PromisePrototypeThen = _primordials.PromisePrototypeThen,
    SymbolAsyncIterator = _primordials.SymbolAsyncIterator,
    SymbolIterator = _primordials.SymbolIterator;

var _require = require('buffer'),
    Buffer = _require.Buffer;

var _require$codes = require('internal/errors').codes,
    ERR_INVALID_ARG_TYPE = _require$codes.ERR_INVALID_ARG_TYPE,
    ERR_STREAM_NULL_VALUES = _require$codes.ERR_STREAM_NULL_VALUES;

function from(Readable, iterable, opts) {
  var iterator;

  if (typeof iterable === 'string' || iterable instanceof Buffer) {
    return new Readable(_objectSpread(_objectSpread({
      objectMode: true
    }, opts), {}, {
      read: function read() {
        this.push(iterable);
        this.push(null);
      }
    }));
  }

  var isAsync = false;

  if (iterable && iterable[SymbolAsyncIterator]) {
    isAsync = true;
    iterator = iterable[SymbolAsyncIterator]();
  } else if (iterable && iterable[SymbolIterator]) {
    isAsync = false;
    iterator = iterable[SymbolIterator]();
  } else {
    throw new ERR_INVALID_ARG_TYPE('iterable', ['Iterable'], iterable);
  }

  var readable = new Readable(_objectSpread({
    objectMode: true,
    highWaterMark: 1
  }, opts)); // Flag to protect against _read
  // being called before last iteration completion.

  var reading = false;

  readable._read = function () {
    if (!reading) {
      reading = true;
      next();
    }
  };

  readable._destroy = function (error, cb) {
    PromisePrototypeThen(close(error), function () {
      return process.nextTick(cb, error);
    }, // nextTick is here in case cb throws
    function (e) {
      return process.nextTick(cb, e || error);
    });
  };

  function close(_x) {
    return _close.apply(this, arguments);
  }

  function _close() {
    _close = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee(error) {
      var hadError, hasThrow, _yield$iterator$throw, value, done, _yield$iterator$retur, _value;

      return regeneratorRuntime.wrap(function _callee$(_context) {
        while (1) {
          switch (_context.prev = _context.next) {
            case 0:
              hadError = error !== undefined && error !== null;
              hasThrow = typeof iterator["throw"] === 'function';

              if (!(hadError && hasThrow)) {
                _context.next = 12;
                break;
              }

              _context.next = 5;
              return iterator["throw"](error);

            case 5:
              _yield$iterator$throw = _context.sent;
              value = _yield$iterator$throw.value;
              done = _yield$iterator$throw.done;
              _context.next = 10;
              return value;

            case 10:
              if (!done) {
                _context.next = 12;
                break;
              }

              return _context.abrupt("return");

            case 12:
              if (!(typeof iterator["return"] === 'function')) {
                _context.next = 19;
                break;
              }

              _context.next = 15;
              return iterator["return"]();

            case 15:
              _yield$iterator$retur = _context.sent;
              _value = _yield$iterator$retur.value;
              _context.next = 19;
              return _value;

            case 19:
            case "end":
              return _context.stop();
          }
        }
      }, _callee);
    }));
    return _close.apply(this, arguments);
  }

  function next() {
    return _next2.apply(this, arguments);
  }

  function _next2() {
    _next2 = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee2() {
      var _ref, value, done, res;

      return regeneratorRuntime.wrap(function _callee2$(_context2) {
        while (1) {
          switch (_context2.prev = _context2.next) {
            case 0:
              _context2.prev = 0;

              if (!isAsync) {
                _context2.next = 7;
                break;
              }

              _context2.next = 4;
              return iterator.next();

            case 4:
              _context2.t0 = _context2.sent;
              _context2.next = 8;
              break;

            case 7:
              _context2.t0 = iterator.next();

            case 8:
              _ref = _context2.t0;
              value = _ref.value;
              done = _ref.done;

              if (!done) {
                _context2.next = 15;
                break;
              }

              readable.push(null);
              _context2.next = 33;
              break;

            case 15:
              if (!(value && typeof value.then === 'function')) {
                _context2.next = 21;
                break;
              }

              _context2.next = 18;
              return value;

            case 18:
              _context2.t1 = _context2.sent;
              _context2.next = 22;
              break;

            case 21:
              _context2.t1 = value;

            case 22:
              res = _context2.t1;

              if (!(res === null)) {
                _context2.next = 28;
                break;
              }

              reading = false;
              throw new ERR_STREAM_NULL_VALUES();

            case 28:
              if (!readable.push(res)) {
                _context2.next = 32;
                break;
              }

              return _context2.abrupt("continue", 39);

            case 32:
              reading = false;

            case 33:
              _context2.next = 38;
              break;

            case 35:
              _context2.prev = 35;
              _context2.t2 = _context2["catch"](0);
              readable.destroy(_context2.t2);

            case 38:
              return _context2.abrupt("break", 41);

            case 39:
              _context2.next = 0;
              break;

            case 41:
            case "end":
              return _context2.stop();
          }
        }
      }, _callee2, null, [[0, 35]]);
    }));
    return _next2.apply(this, arguments);
  }

  return readable;
}

module.exports = from;
