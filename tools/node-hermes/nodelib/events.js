// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.
'use strict';

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) { try { var info = gen[key](arg); var value = info.value; } catch (error) { reject(error); return; } if (info.done) { resolve(value); } else { Promise.resolve(value).then(_next, _throw); } }

function _asyncToGenerator(fn) { return function () { var self = this, args = arguments; return new Promise(function (resolve, reject) { var gen = fn.apply(self, args); function _next(value) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value); } function _throw(err) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err); } _next(undefined); }); }; }

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _toConsumableArray(arr) { return _arrayWithoutHoles(arr) || _iterableToArray(arr) || _unsupportedIterableToArray(arr) || _nonIterableSpread(); }

function _nonIterableSpread() { throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _iterableToArray(iter) { if (typeof Symbol !== "undefined" && iter[Symbol.iterator] != null || iter["@@iterator"] != null) return Array.from(iter); }

function _arrayWithoutHoles(arr) { if (Array.isArray(arr)) return _arrayLikeToArray(arr); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function _awaitAsyncGenerator(value) { return new _AwaitValue(value); }

function _wrapAsyncGenerator(fn) { return function () { return new _AsyncGenerator(fn.apply(this, arguments)); }; }

function _AsyncGenerator(gen) { var front, back; function send(key, arg) { return new Promise(function (resolve, reject) { var request = { key: key, arg: arg, resolve: resolve, reject: reject, next: null }; if (back) { back = back.next = request; } else { front = back = request; resume(key, arg); } }); } function resume(key, arg) { try { var result = gen[key](arg); var value = result.value; var wrappedAwait = value instanceof _AwaitValue; Promise.resolve(wrappedAwait ? value.wrapped : value).then(function (arg) { if (wrappedAwait) { resume(key === "return" ? "return" : "next", arg); return; } settle(result.done ? "return" : "normal", arg); }, function (err) { resume("throw", err); }); } catch (err) { settle("throw", err); } } function settle(type, value) { switch (type) { case "return": front.resolve({ value: value, done: true }); break; case "throw": front.reject(value); break; default: front.resolve({ value: value, done: false }); break; } front = front.next; if (front) { resume(front.key, front.arg); } else { back = null; } } this._invoke = send; if (typeof gen["return"] !== "function") { this["return"] = undefined; } }

_AsyncGenerator.prototype[typeof Symbol === "function" && Symbol.asyncIterator || "@@asyncIterator"] = function () { return this; };

_AsyncGenerator.prototype.next = function (arg) { return this._invoke("next", arg); };

_AsyncGenerator.prototype["throw"] = function (arg) { return this._invoke("throw", arg); };

_AsyncGenerator.prototype["return"] = function (arg) { return this._invoke("return", arg); };

function _AwaitValue(value) { this.wrapped = value; }

var _primordials = primordials,
    ArrayPrototypeIndexOf = _primordials.ArrayPrototypeIndexOf,
    ArrayPrototypeJoin = _primordials.ArrayPrototypeJoin,
    ArrayPrototypeShift = _primordials.ArrayPrototypeShift,
    ArrayPrototypeSlice = _primordials.ArrayPrototypeSlice,
    ArrayPrototypeSplice = _primordials.ArrayPrototypeSplice,
    Boolean = _primordials.Boolean,
    Error = _primordials.Error,
    ErrorCaptureStackTrace = _primordials.ErrorCaptureStackTrace,
    FunctionPrototypeBind = _primordials.FunctionPrototypeBind,
    FunctionPrototypeCall = _primordials.FunctionPrototypeCall,
    MathMin = _primordials.MathMin,
    NumberIsNaN = _primordials.NumberIsNaN,
    ObjectCreate = _primordials.ObjectCreate,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    ObjectDefineProperties = _primordials.ObjectDefineProperties,
    ObjectGetPrototypeOf = _primordials.ObjectGetPrototypeOf,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    _Promise = _primordials.Promise,
    PromiseReject = _primordials.PromiseReject,
    PromiseResolve = _primordials.PromiseResolve,
    ReflectOwnKeys = _primordials.ReflectOwnKeys,
    String = _primordials.String,
    StringPrototypeSplit = _primordials.StringPrototypeSplit,
    _Symbol = _primordials.Symbol,
    SymbolFor = _primordials.SymbolFor,
    SymbolAsyncIterator = _primordials.SymbolAsyncIterator;
var kRejection = SymbolFor('nodejs.rejection');
var spliceOne;

var _require = require('internal/errors'),
    AbortError = _require.AbortError,
    kEnhanceStackBeforeInspector = _require.kEnhanceStackBeforeInspector,
    _require$codes = _require.codes,
    ERR_INVALID_ARG_TYPE = _require$codes.ERR_INVALID_ARG_TYPE,
    ERR_OUT_OF_RANGE = _require$codes.ERR_OUT_OF_RANGE,
    ERR_UNHANDLED_ERROR = _require$codes.ERR_UNHANDLED_ERROR;

var _require2 = require('internal/util/inspect'),
    inspect = _require2.inspect;

var _require3 = require('internal/validators'),
    validateAbortSignal = _require3.validateAbortSignal,
    validateBoolean = _require3.validateBoolean,
    validateFunction = _require3.validateFunction;

var kCapture = _Symbol('kCapture');

var kErrorMonitor = _Symbol('events.errorMonitor');

var kMaxEventTargetListeners = _Symbol('events.maxEventTargetListeners');

var kMaxEventTargetListenersWarned = _Symbol('events.maxEventTargetListenersWarned');
/**
 * Creates a new `EventEmitter` instance.
 * @param {{ captureRejections?: boolean; }} [opts]
 * @returns {EventEmitter}
 */


function EventEmitter(opts) {
  EventEmitter.init.call(this, opts);
}

module.exports = EventEmitter;
module.exports.once = once;
module.exports.on = on;
module.exports.getEventListeners = getEventListeners; // Backwards-compat with node 0.10.x

EventEmitter.EventEmitter = EventEmitter;
EventEmitter.usingDomains = false;
EventEmitter.captureRejectionSymbol = kRejection;
ObjectDefineProperty(EventEmitter, 'captureRejections', {
  get: function get() {
    return EventEmitter.prototype[kCapture];
  },
  set: function set(value) {
    validateBoolean(value, 'EventEmitter.captureRejections');
    EventEmitter.prototype[kCapture] = value;
  },
  enumerable: true
});
EventEmitter.errorMonitor = kErrorMonitor; // The default for captureRejections is false

ObjectDefineProperty(EventEmitter.prototype, kCapture, {
  value: false,
  writable: true,
  enumerable: false
});
EventEmitter.prototype._events = undefined;
EventEmitter.prototype._eventsCount = 0;
EventEmitter.prototype._maxListeners = undefined; // By default EventEmitters will print a warning if more than 10 listeners are
// added to it. This is a useful default which helps finding memory leaks.

var defaultMaxListeners = 10;
var isEventTarget;

function checkListener(listener) {
  validateFunction(listener, 'listener');
}

ObjectDefineProperty(EventEmitter, 'defaultMaxListeners', {
  enumerable: true,
  get: function get() {
    return defaultMaxListeners;
  },
  set: function set(arg) {
    if (typeof arg !== 'number' || arg < 0 || NumberIsNaN(arg)) {
      throw new ERR_OUT_OF_RANGE('defaultMaxListeners', 'a non-negative number', arg);
    }

    defaultMaxListeners = arg;
  }
});
ObjectDefineProperties(EventEmitter, {
  kMaxEventTargetListeners: {
    value: kMaxEventTargetListeners,
    enumerable: false,
    configurable: false,
    writable: false
  },
  kMaxEventTargetListenersWarned: {
    value: kMaxEventTargetListenersWarned,
    enumerable: false,
    configurable: false,
    writable: false
  }
});
/**
 * Sets the max listeners.
 * @param {number} n
 * @param {EventTarget[] | EventEmitter[]} [eventTargets]
 * @returns {void}
 */

EventEmitter.setMaxListeners = function () {
  var n = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : defaultMaxListeners;
  if (typeof n !== 'number' || n < 0 || NumberIsNaN(n)) throw new ERR_OUT_OF_RANGE('n', 'a non-negative number', n);

  if ((arguments.length <= 1 ? 0 : arguments.length - 1) === 0) {
    defaultMaxListeners = n;
  } else {
    if (isEventTarget === undefined) isEventTarget = require('internal/event_target').isEventTarget;

    for (var i = 0; i < (arguments.length <= 1 ? 0 : arguments.length - 1); i++) {
      var target = i + 1 < 1 || arguments.length <= i + 1 ? undefined : arguments[i + 1];

      if (isEventTarget(target)) {
        target[kMaxEventTargetListeners] = n;
        target[kMaxEventTargetListenersWarned] = false;
      } else if (typeof target.setMaxListeners === 'function') {
        target.setMaxListeners(n);
      } else {
        throw new ERR_INVALID_ARG_TYPE('eventTargets', ['EventEmitter', 'EventTarget'], target);
      }
    }
  }
};

EventEmitter.init = function (opts) {
  if (this._events === undefined || this._events === ObjectGetPrototypeOf(this)._events) {
    this._events = ObjectCreate(null);
    this._eventsCount = 0;
  }

  this._maxListeners = this._maxListeners || undefined;

  if (opts !== null && opts !== void 0 && opts.captureRejections) {
    validateBoolean(opts.captureRejections, 'options.captureRejections');
    this[kCapture] = Boolean(opts.captureRejections);
  } else {
    // Assigning the kCapture property directly saves an expensive
    // prototype lookup in a very sensitive hot path.
    this[kCapture] = EventEmitter.prototype[kCapture];
  }
};

function addCatch(that, promise, type, args) {
  if (!that[kCapture]) {
    return;
  } // Handle Promises/A+ spec, then could be a getter
  // that throws on second use.


  try {
    var then = promise.then;

    if (typeof then === 'function') {
      then.call(promise, undefined, function (err) {
        // The callback is called with nextTick to avoid a follow-up
        // rejection from this promise.
        process.nextTick(emitUnhandledRejectionOrErr, that, err, type, args);
      });
    }
  } catch (err) {
    that.emit('error', err);
  }
}

function emitUnhandledRejectionOrErr(ee, err, type, args) {
  if (typeof ee[kRejection] === 'function') {
    ee[kRejection].apply(ee, [err, type].concat(_toConsumableArray(args)));
  } else {
    // We have to disable the capture rejections mechanism, otherwise
    // we might end up in an infinite loop.
    var prev = ee[kCapture]; // If the error handler throws, it is not catcheable and it
    // will end up in 'uncaughtException'. We restore the previous
    // value of kCapture in case the uncaughtException is present
    // and the exception is handled.

    try {
      ee[kCapture] = false;
      ee.emit('error', err);
    } finally {
      ee[kCapture] = prev;
    }
  }
}
/**
 * Increases the max listeners of the event emitter.
 * @param {number} n
 * @returns {EventEmitter}
 */


EventEmitter.prototype.setMaxListeners = function setMaxListeners(n) {
  if (typeof n !== 'number' || n < 0 || NumberIsNaN(n)) {
    throw new ERR_OUT_OF_RANGE('n', 'a non-negative number', n);
  }

  this._maxListeners = n;
  return this;
};

function _getMaxListeners(that) {
  if (that._maxListeners === undefined) return EventEmitter.defaultMaxListeners;
  return that._maxListeners;
}
/**
 * Returns the current max listener value for the event emitter.
 * @returns {number}
 */


EventEmitter.prototype.getMaxListeners = function getMaxListeners() {
  return _getMaxListeners(this);
}; // Returns the length and line number of the first sequence of `a` that fully
// appears in `b` with a length of at least 4.


function identicalSequenceRange(a, b) {
  for (var i = 0; i < a.length - 3; i++) {
    // Find the first entry of b that matches the current entry of a.
    var pos = ArrayPrototypeIndexOf(b, a[i]);

    if (pos !== -1) {
      var rest = b.length - pos;

      if (rest > 3) {
        var len = 1;
        var maxLen = MathMin(a.length - i, rest); // Count the number of consecutive entries.

        while (maxLen > len && a[i + len] === b[pos + len]) {
          len++;
        }

        if (len > 3) {
          return [len, i];
        }
      }
    }
  }

  return [0, 0];
}

function enhanceStackTrace(err, own) {
  var ctorInfo = '';

  try {
    var name = this.constructor.name;
    if (name !== 'EventEmitter') ctorInfo = " on ".concat(name, " instance");
  } catch (_unused) {}

  var sep = "\nEmitted 'error' event".concat(ctorInfo, " at:\n");
  var errStack = ArrayPrototypeSlice(StringPrototypeSplit(err.stack, '\n'), 1);
  var ownStack = ArrayPrototypeSlice(StringPrototypeSplit(own.stack, '\n'), 1);

  var _identicalSequenceRan = identicalSequenceRange(ownStack, errStack),
      len = _identicalSequenceRan[0],
      off = _identicalSequenceRan[1];

  if (len > 0) {
    ArrayPrototypeSplice(ownStack, off + 1, len - 2, '    [... lines matching original stack trace ...]');
  }

  return err.stack + sep + ArrayPrototypeJoin(ownStack, '\n');
}
/**
 * Synchronously calls each of the listeners registered
 * for the event.
 * @param {string | symbol} type
 * @param {...any} [args]
 * @returns {boolean}
 */


EventEmitter.prototype.emit = function emit(type) {
  var doError = type === 'error';
  var events = this._events;

  for (var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
    args[_key - 1] = arguments[_key];
  }

  if (events !== undefined) {
    if (doError && events[kErrorMonitor] !== undefined) this.emit.apply(this, [kErrorMonitor].concat(args));
    doError = doError && events.error === undefined;
  } else if (!doError) return false; // If there is no 'error' event listener then throw.


  if (doError) {
    var er;
    if (args.length > 0) er = args[0];

    if (er instanceof Error) {
      try {
        var capture = {};
        ErrorCaptureStackTrace(capture, EventEmitter.prototype.emit);
        ObjectDefineProperty(er, kEnhanceStackBeforeInspector, {
          value: FunctionPrototypeBind(enhanceStackTrace, this, er, capture),
          configurable: true
        });
      } catch (_unused2) {} // Note: The comments on the `throw` lines are intentional, they show
      // up in Node's output if this results in an unhandled exception.


      throw er; // Unhandled 'error' event
    }

    var stringifiedEr;

    var _require4 = require('internal/util/inspect'),
        _inspect = _require4.inspect;

    try {
      stringifiedEr = _inspect(er);
    } catch (_unused3) {
      stringifiedEr = er;
    } // At least give some kind of context to the user


    var err = new ERR_UNHANDLED_ERROR(stringifiedEr);
    err.context = er;
    throw err; // Unhandled 'error' event
  }

  var handler = events[type];
  if (handler === undefined) return false;

  if (typeof handler === 'function') {
    var result = handler.apply(this, args); // We check if result is undefined first because that
    // is the most common case so we do not pay any perf
    // penalty

    if (result !== undefined && result !== null) {
      addCatch(this, result, type, args);
    }
  } else {
    var len = handler.length;
    var listeners = arrayClone(handler);

    for (var i = 0; i < len; ++i) {
      var _result = listeners[i].apply(this, args); // We check if result is undefined first because that
      // is the most common case so we do not pay any perf
      // penalty.
      // This code is duplicated because extracting it away
      // would make it non-inlineable.


      if (_result !== undefined && _result !== null) {
        addCatch(this, _result, type, args);
      }
    }
  }

  return true;
};

function _addListener(target, type, listener, prepend) {
  var m;
  var events;
  var existing;
  checkListener(listener);
  events = target._events;

  if (events === undefined) {
    events = target._events = ObjectCreate(null);
    target._eventsCount = 0;
  } else {
    // To avoid recursion in the case that type === "newListener"! Before
    // adding it to the listeners, first emit "newListener".
    if (events.newListener !== undefined) {
      var _listener$listener;

      target.emit('newListener', type, (_listener$listener = listener.listener) !== null && _listener$listener !== void 0 ? _listener$listener : listener); // Re-assign `events` because a newListener handler could have caused the
      // this._events to be assigned to a new object

      events = target._events;
    }

    existing = events[type];
  }

  if (existing === undefined) {
    // Optimize the case of one listener. Don't need the extra array object.
    events[type] = listener;
    ++target._eventsCount;
  } else {
    if (typeof existing === 'function') {
      // Adding the second element, need to change to array.
      existing = events[type] = prepend ? [listener, existing] : [existing, listener]; // If we've already got an array, just append.
    } else if (prepend) {
      existing.unshift(listener);
    } else {
      existing.push(listener);
    } // Check for listener leak


    m = _getMaxListeners(target);

    if (m > 0 && existing.length > m && !existing.warned) {
      existing.warned = true; // No error code for this since it is a Warning
      // eslint-disable-next-line no-restricted-syntax

      var w = new Error('Possible EventEmitter memory leak detected. ' + "".concat(existing.length, " ").concat(String(type), " listeners ") + "added to ".concat(inspect(target, {
        depth: -1
      }), ". Use ") + 'emitter.setMaxListeners() to increase limit');
      w.name = 'MaxListenersExceededWarning';
      w.emitter = target;
      w.type = type;
      w.count = existing.length;
      process.emitWarning(w);
    }
  }

  return target;
}
/**
 * Adds a listener to the event emitter.
 * @param {string | symbol} type
 * @param {Function} listener
 * @returns {EventEmitter}
 */


EventEmitter.prototype.addListener = function addListener(type, listener) {
  return _addListener(this, type, listener, false);
};

EventEmitter.prototype.on = EventEmitter.prototype.addListener;
/**
 * Adds the `listener` function to the beginning of
 * the listeners array.
 * @param {string | symbol} type
 * @param {Function} listener
 * @returns {EventEmitter}
 */

EventEmitter.prototype.prependListener = function prependListener(type, listener) {
  return _addListener(this, type, listener, true);
};

function onceWrapper() {
  if (!this.fired) {
    this.target.removeListener(this.type, this.wrapFn);
    this.fired = true;
    if (arguments.length === 0) return this.listener.call(this.target);
    return this.listener.apply(this.target, arguments);
  }
}

function _onceWrap(target, type, listener) {
  var state = {
    fired: false,
    wrapFn: undefined,
    target: target,
    type: type,
    listener: listener
  };
  var wrapped = onceWrapper.bind(state);
  wrapped.listener = listener;
  state.wrapFn = wrapped;
  return wrapped;
}
/**
 * Adds a one-time `listener` function to the event emitter.
 * @param {string | symbol} type
 * @param {Function} listener
 * @returns {EventEmitter}
 */


EventEmitter.prototype.once = function once(type, listener) {
  checkListener(listener);
  this.on(type, _onceWrap(this, type, listener));
  return this;
};
/**
 * Adds a one-time `listener` function to the beginning of
 * the listeners array.
 * @param {string | symbol} type
 * @param {Function} listener
 * @returns {EventEmitter}
 */


EventEmitter.prototype.prependOnceListener = function prependOnceListener(type, listener) {
  checkListener(listener);
  this.prependListener(type, _onceWrap(this, type, listener));
  return this;
};
/**
 * Removes the specified `listener` from the listeners array.
 * @param {string | symbol} type
 * @param {Function} listener
 * @returns {EventEmitter}
 */


EventEmitter.prototype.removeListener = function removeListener(type, listener) {
  checkListener(listener);
  var events = this._events;
  if (events === undefined) return this;
  var list = events[type];
  if (list === undefined) return this;

  if (list === listener || list.listener === listener) {
    if (--this._eventsCount === 0) this._events = ObjectCreate(null);else {
      delete events[type];
      if (events.removeListener) this.emit('removeListener', type, list.listener || listener);
    }
  } else if (typeof list !== 'function') {
    var position = -1;

    for (var i = list.length - 1; i >= 0; i--) {
      if (list[i] === listener || list[i].listener === listener) {
        position = i;
        break;
      }
    }

    if (position < 0) return this;
    if (position === 0) list.shift();else {
      if (spliceOne === undefined) spliceOne = require('internal/util').spliceOne;
      spliceOne(list, position);
    }
    if (list.length === 1) events[type] = list[0];
    if (events.removeListener !== undefined) this.emit('removeListener', type, listener);
  }

  return this;
};

EventEmitter.prototype.off = EventEmitter.prototype.removeListener;
/**
 * Removes all listeners from the event emitter. (Only
 * removes listeners for a specific event name if specified
 * as `type`).
 * @param {string | symbol} [type]
 * @returns {EventEmitter}
 */

EventEmitter.prototype.removeAllListeners = function removeAllListeners(type) {
  var events = this._events;
  if (events === undefined) return this; // Not listening for removeListener, no need to emit

  if (events.removeListener === undefined) {
    if (arguments.length === 0) {
      this._events = ObjectCreate(null);
      this._eventsCount = 0;
    } else if (events[type] !== undefined) {
      if (--this._eventsCount === 0) this._events = ObjectCreate(null);else delete events[type];
    }

    return this;
  } // Emit removeListener for all listeners on all events


  if (arguments.length === 0) {
    var _iterator = _createForOfIteratorHelper(ReflectOwnKeys(events)),
        _step;

    try {
      for (_iterator.s(); !(_step = _iterator.n()).done;) {
        var key = _step.value;
        if (key === 'removeListener') continue;
        this.removeAllListeners(key);
      }
    } catch (err) {
      _iterator.e(err);
    } finally {
      _iterator.f();
    }

    this.removeAllListeners('removeListener');
    this._events = ObjectCreate(null);
    this._eventsCount = 0;
    return this;
  }

  var listeners = events[type];

  if (typeof listeners === 'function') {
    this.removeListener(type, listeners);
  } else if (listeners !== undefined) {
    // LIFO order
    for (var i = listeners.length - 1; i >= 0; i--) {
      this.removeListener(type, listeners[i]);
    }
  }

  return this;
};

function _listeners(target, type, unwrap) {
  var events = target._events;
  if (events === undefined) return [];
  var evlistener = events[type];
  if (evlistener === undefined) return [];
  if (typeof evlistener === 'function') return unwrap ? [evlistener.listener || evlistener] : [evlistener];
  return unwrap ? unwrapListeners(evlistener) : arrayClone(evlistener);
}
/**
 * Returns a copy of the array of listeners for the event name
 * specified as `type`.
 * @param {string | symbol} type
 * @returns {Function[]}
 */


EventEmitter.prototype.listeners = function listeners(type) {
  return _listeners(this, type, true);
};
/**
 * Returns a copy of the array of listeners and wrappers for
 * the event name specified as `type`.
 * @param {string | symbol} type
 * @returns {Function[]}
 */


EventEmitter.prototype.rawListeners = function rawListeners(type) {
  return _listeners(this, type, false);
};
/**
 * Returns the number of listeners listening to the event name
 * specified as `type`.
 * @deprecated since v3.2.0
 * @param {EventEmitter} emitter
 * @param {string | symbol} type
 * @returns {number}
 */


EventEmitter.listenerCount = function (emitter, type) {
  if (typeof emitter.listenerCount === 'function') {
    return emitter.listenerCount(type);
  }

  return FunctionPrototypeCall(listenerCount, emitter, type);
};

EventEmitter.prototype.listenerCount = listenerCount;
/**
 * Returns the number of listeners listening to event name
 * specified as `type`.
 * @param {string | symbol} type
 * @returns {number}
 */

function listenerCount(type) {
  var events = this._events;

  if (events !== undefined) {
    var evlistener = events[type];

    if (typeof evlistener === 'function') {
      return 1;
    } else if (evlistener !== undefined) {
      return evlistener.length;
    }
  }

  return 0;
}
/**
 * Returns an array listing the events for which
 * the emitter has registered listeners.
 * @returns {any[]}
 */


EventEmitter.prototype.eventNames = function eventNames() {
  return this._eventsCount > 0 ? ReflectOwnKeys(this._events) : [];
};

function arrayClone(arr) {
  // At least since V8 8.3, this implementation is faster than the previous
  // which always used a simple for-loop
  switch (arr.length) {
    case 2:
      return [arr[0], arr[1]];

    case 3:
      return [arr[0], arr[1], arr[2]];

    case 4:
      return [arr[0], arr[1], arr[2], arr[3]];

    case 5:
      return [arr[0], arr[1], arr[2], arr[3], arr[4]];

    case 6:
      return [arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]];
  }

  return ArrayPrototypeSlice(arr);
}

function unwrapListeners(arr) {
  var ret = arrayClone(arr);

  for (var i = 0; i < ret.length; ++i) {
    var orig = ret[i].listener;
    if (typeof orig === 'function') ret[i] = orig;
  }

  return ret;
}
/**
 * Returns a copy of the array of listeners for the event name
 * specified as `type`.
 * @param {EventEmitter | EventTarget} emitterOrTarget
 * @param {string | symbol} type
 * @returns {Function[]}
 */


function getEventListeners(emitterOrTarget, type) {
  // First check if EventEmitter
  if (typeof emitterOrTarget.listeners === 'function') {
    return emitterOrTarget.listeners(type);
  } // Require event target lazily to avoid always loading it


  var _require5 = require('internal/event_target'),
      isEventTarget = _require5.isEventTarget,
      kEvents = _require5.kEvents;

  if (isEventTarget(emitterOrTarget)) {
    var root = emitterOrTarget[kEvents].get(type);
    var listeners = [];
    var handler = root === null || root === void 0 ? void 0 : root.next;

    while (((_handler = handler) === null || _handler === void 0 ? void 0 : _handler.listener) !== undefined) {
      var _handler, _handler$listener;

      var listener = (_handler$listener = handler.listener) !== null && _handler$listener !== void 0 && _handler$listener.deref ? handler.listener.deref() : handler.listener;
      listeners.push(listener);
      handler = handler.next;
    }

    return listeners;
  }

  throw new ERR_INVALID_ARG_TYPE('emitter', ['EventEmitter', 'EventTarget'], emitterOrTarget);
}
/**
 * Creates a `Promise` that is fulfilled when the emitter
 * emits the given event.
 * @param {EventEmitter} emitter
 * @param {string} name
 * @param {{ signal: AbortSignal; }} [options]
 * @returns {Promise}
 */


function once(_x, _x2) {
  return _once.apply(this, arguments);
}

function _once() {
  _once = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee2(emitter, name) {
    var options,
        signal,
        _args2 = arguments;
    return regeneratorRuntime.wrap(function _callee2$(_context2) {
      while (1) {
        switch (_context2.prev = _context2.next) {
          case 0:
            options = _args2.length > 2 && _args2[2] !== undefined ? _args2[2] : {};
            signal = options === null || options === void 0 ? void 0 : options.signal;
            validateAbortSignal(signal, 'options.signal');

            if (!(signal !== null && signal !== void 0 && signal.aborted)) {
              _context2.next = 5;
              break;
            }

            throw new AbortError();

          case 5:
            return _context2.abrupt("return", new _Promise(function (resolve, reject) {
              var errorListener = function errorListener(err) {
                emitter.removeListener(name, resolver);

                if (signal != null) {
                  eventTargetAgnosticRemoveListener(signal, 'abort', abortListener);
                }

                reject(err);
              };

              var resolver = function resolver() {
                if (typeof emitter.removeListener === 'function') {
                  emitter.removeListener('error', errorListener);
                }

                if (signal != null) {
                  eventTargetAgnosticRemoveListener(signal, 'abort', abortListener);
                }

                for (var _len3 = arguments.length, args = new Array(_len3), _key3 = 0; _key3 < _len3; _key3++) {
                  args[_key3] = arguments[_key3];
                }

                resolve(args);
              };

              eventTargetAgnosticAddListener(emitter, name, resolver, {
                once: true
              });

              if (name !== 'error' && typeof emitter.once === 'function') {
                emitter.once('error', errorListener);
              }

              function abortListener() {
                eventTargetAgnosticRemoveListener(emitter, name, resolver);
                eventTargetAgnosticRemoveListener(emitter, 'error', errorListener);
                reject(new AbortError());
              }

              if (signal != null) {
                eventTargetAgnosticAddListener(signal, 'abort', abortListener, {
                  once: true
                });
              }
            }));

          case 6:
          case "end":
            return _context2.stop();
        }
      }
    }, _callee2);
  }));
  return _once.apply(this, arguments);
}

// var AsyncIteratorPrototype = ObjectGetPrototypeOf(ObjectGetPrototypeOf( /*#__PURE__*/_wrapAsyncGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee() {
//   return regeneratorRuntime.wrap(function _callee$(_context) {
//     while (1) {
//       switch (_context.prev = _context.next) {
//         case 0:
//         case "end":
//           return _context.stop();
//       }
//     }
//   }, _callee);
// }))).prototype);

function createIterResult(value, done) {
  return {
    value: value,
    done: done
  };
}

function eventTargetAgnosticRemoveListener(emitter, name, listener, flags) {
  if (typeof emitter.removeListener === 'function') {
    emitter.removeListener(name, listener);
  } else if (typeof emitter.removeEventListener === 'function') {
    emitter.removeEventListener(name, listener, flags);
  } else {
    throw new ERR_INVALID_ARG_TYPE('emitter', 'EventEmitter', emitter);
  }
}

function eventTargetAgnosticAddListener(emitter, name, listener, flags) {
  if (typeof emitter.on === 'function') {
    if (flags !== null && flags !== void 0 && flags.once) {
      emitter.once(name, listener);
    } else {
      emitter.on(name, listener);
    }
  } else if (typeof emitter.addEventListener === 'function') {
    // EventTarget does not have `error` event semantics like Node
    // EventEmitters, we do not listen to `error` events here.
    emitter.addEventListener(name, function (arg) {
      listener(arg);
    }, flags);
  } else {
    throw new ERR_INVALID_ARG_TYPE('emitter', 'EventEmitter', emitter);
  }
}
/**
 * Returns an `AsyncIterator` that iterates `event` events.
 * @param {EventEmitter} emitter
 * @param {string | symbol} event
 * @param {{ signal: AbortSignal; }} [options]
 * @returns {AsyncIterator}
 */


function on(emitter, event, options) {
  var signal = options === null || options === void 0 ? void 0 : options.signal;
  validateAbortSignal(signal, 'options.signal');
  if (signal !== null && signal !== void 0 && signal.aborted) throw new AbortError();
  var unconsumedEvents = [];
  var unconsumedPromises = [];
  var error = null;
  var finished = false;
  var iterator = ObjectSetPrototypeOf(_defineProperty({
    next: function next() {
      // First, we consume all unread events
      var value = unconsumedEvents.shift();

      if (value) {
        return PromiseResolve(createIterResult(value, false));
      } // Then we error, if an error happened
      // This happens one time if at all, because after 'error'
      // we stop listening


      if (error) {
        var p = PromiseReject(error); // Only the first element errors

        error = null;
        return p;
      } // If the iterator is finished, resolve to done


      if (finished) {
        return PromiseResolve(createIterResult(undefined, true));
      } // Wait until an event happens


      return new _Promise(function (resolve, reject) {
        unconsumedPromises.push({
          resolve: resolve,
          reject: reject
        });
      });
    },
    "return": function _return() {
      eventTargetAgnosticRemoveListener(emitter, event, eventHandler);
      eventTargetAgnosticRemoveListener(emitter, 'error', errorHandler);

      if (signal) {
        eventTargetAgnosticRemoveListener(signal, 'abort', abortListener, {
          once: true
        });
      }

      finished = true;

      var _iterator2 = _createForOfIteratorHelper(unconsumedPromises),
          _step2;

      try {
        for (_iterator2.s(); !(_step2 = _iterator2.n()).done;) {
          var promise = _step2.value;
          promise.resolve(createIterResult(undefined, true));
        }
      } catch (err) {
        _iterator2.e(err);
      } finally {
        _iterator2.f();
      }

      return PromiseResolve(createIterResult(undefined, true));
    },
    "throw": function _throw(err) {
      if (!err || !(err instanceof Error)) {
        throw new ERR_INVALID_ARG_TYPE('EventEmitter.AsyncIterator', 'Error', err);
      }

      error = err;
      eventTargetAgnosticRemoveListener(emitter, event, eventHandler);
      eventTargetAgnosticRemoveListener(emitter, 'error', errorHandler);
    }
  }, SymbolAsyncIterator, function () {
    return this;
  }), AsyncIteratorPrototype);
  eventTargetAgnosticAddListener(emitter, event, eventHandler);

  if (event !== 'error' && typeof emitter.on === 'function') {
    emitter.on('error', errorHandler);
  }

  if (signal) {
    eventTargetAgnosticAddListener(signal, 'abort', abortListener, {
      once: true
    });
  }

  return iterator;

  function abortListener() {
    errorHandler(new AbortError());
  }

  function eventHandler() {
    var promise = ArrayPrototypeShift(unconsumedPromises);

    for (var _len2 = arguments.length, args = new Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
      args[_key2] = arguments[_key2];
    }

    if (promise) {
      promise.resolve(createIterResult(args, false));
    } else {
      unconsumedEvents.push(args);
    }
  }

  function errorHandler(err) {
    finished = true;
    var toError = ArrayPrototypeShift(unconsumedPromises);

    if (toError) {
      toError.reject(err);
    } else {
      // The next time we call next()
      error = err;
    }

    iterator["return"]();
  }
}
