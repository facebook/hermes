// @nolint
'use strict';

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

var _primordials = primordials,
    ArrayFrom = _primordials.ArrayFrom,
    ArrayIsArray = _primordials.ArrayIsArray,
    ArrayPrototypePush = _primordials.ArrayPrototypePush,
    ArrayPrototypeSlice = _primordials.ArrayPrototypeSlice,
    ArrayPrototypeSort = _primordials.ArrayPrototypeSort,
    Error = _primordials.Error,
    ObjectCreate = _primordials.ObjectCreate,
    ObjectDefineProperties = _primordials.ObjectDefineProperties,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    ObjectGetOwnPropertyDescriptor = _primordials.ObjectGetOwnPropertyDescriptor,
    ObjectGetOwnPropertyDescriptors = _primordials.ObjectGetOwnPropertyDescriptors,
    ObjectGetPrototypeOf = _primordials.ObjectGetPrototypeOf,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    Promise = _primordials.Promise,
    ReflectApply = _primordials.ReflectApply,
    ReflectConstruct = _primordials.ReflectConstruct,
    RegExpPrototypeTest = _primordials.RegExpPrototypeTest,
    SafeMap = _primordials.SafeMap,
    SafeSet = _primordials.SafeSet,
    StringPrototypeReplace = _primordials.StringPrototypeReplace,
    StringPrototypeToLowerCase = _primordials.StringPrototypeToLowerCase,
    StringPrototypeToUpperCase = _primordials.StringPrototypeToUpperCase,
    _Symbol = _primordials.Symbol,
    SymbolFor = _primordials.SymbolFor;

var _require = require('internal/errors'),
    _require$codes = _require.codes,
    ERR_INVALID_ARG_TYPE = _require$codes.ERR_INVALID_ARG_TYPE,
    ERR_NO_CRYPTO = _require$codes.ERR_NO_CRYPTO,
    ERR_UNKNOWN_SIGNAL = _require$codes.ERR_UNKNOWN_SIGNAL,
    uvErrmapGet = _require.uvErrmapGet,
    overrideStackTrace = _require.overrideStackTrace;

var signals = internalBinding('constants').os.signals;

// var _internalBinding = internalBinding('util'),
//     getHiddenValue = _internalBinding.getHiddenValue,
//     setHiddenValue = _internalBinding.setHiddenValue,
//     kArrowMessagePrivateSymbolIndex = _internalBinding.arrow_message_private_symbol,
//     kDecoratedPrivateSymbolIndex = _internalBinding.decorated_private_symbol,
//     _sleep = _internalBinding.sleep;

// var _internalBinding2 = internalBinding('types'),
//     isNativeError = _internalBinding2.isNativeError;

var noCrypto = false; //!process.versions.openssl;
var experimentalWarnings = new SafeSet();
var colorRegExp = /\u001b\[\d\d?m/g; // eslint-disable-line no-control-regex

var uvBinding;

function lazyUv() {
  var _uvBinding;

  (_uvBinding = uvBinding) !== null && _uvBinding !== void 0 ? _uvBinding : uvBinding = internalBinding('uv');
  return uvBinding;
}

function removeColors(str) {
  return StringPrototypeReplace(str, colorRegExp, '');
}

function isError(e) {
  // An error could be an instance of Error while not being a native error
  // or could be from a different realm and not be instance of Error but still
  // be a native error.
  return isNativeError(e) || e instanceof Error;
} // Keep a list of deprecation codes that have been warned on so we only warn on
// each one once.


var codesWarned = new SafeSet(); // Mark that a method should not be used.
// Returns a modified function which warns once by default.
// If --no-deprecation is set, then it is a no-op.

function deprecate(fn, msg, code) {
  if (process.noDeprecation === true) {
    return fn;
  }

  if (code !== undefined && typeof code !== 'string') throw new ERR_INVALID_ARG_TYPE('code', 'string', code);
  var warned = false;

  function deprecated() {
    if (!warned) {
      warned = true;

      if (code !== undefined) {
        if (!codesWarned.has(code)) {
          process.emitWarning(msg, 'DeprecationWarning', code, deprecated);
          codesWarned.add(code);
        }
      } else {
        process.emitWarning(msg, 'DeprecationWarning', deprecated);
      }
    }

    for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
      args[_key] = arguments[_key];
    }

    if (this instanceof deprecated ? this.constructor : void 0) {
      return ReflectConstruct(fn, args, this instanceof deprecated ? this.constructor : void 0);
    }

    return ReflectApply(fn, this, args);
  } // The wrapper will keep the same prototype as fn to maintain prototype chain


  ObjectSetPrototypeOf(deprecated, fn);

  if (fn.prototype) {
    // Setting this (rather than using Object.setPrototype, as above) ensures
    // that calling the unwrapped constructor gives an instanceof the wrapped
    // constructor.
    deprecated.prototype = fn.prototype;
  }

  return deprecated;
}

function decorateErrorStack(err) {
  if (!(isError(err) && err.stack) || getHiddenValue(err, kDecoratedPrivateSymbolIndex) === true) return;
  var arrow = getHiddenValue(err, kArrowMessagePrivateSymbolIndex);

  if (arrow) {
    err.stack = arrow + err.stack;
    setHiddenValue(err, kDecoratedPrivateSymbolIndex, true);
  }
}

function assertCrypto() {
  if (noCrypto) throw new ERR_NO_CRYPTO();
} // Return undefined if there is no match.
// Move the "slow cases" to a separate function to make sure this function gets
// inlined properly. That prioritizes the common case.


function normalizeEncoding(enc) {
  if (enc == null || enc === 'utf8' || enc === 'utf-8') return 'utf8';
  return slowCases(enc);
}

function slowCases(enc) {
  switch (enc.length) {
    case 4:
      if (enc === 'UTF8') return 'utf8';
      if (enc === 'ucs2' || enc === 'UCS2') return 'utf16le';
      enc = "".concat(enc).toLowerCase();
      if (enc === 'utf8') return 'utf8';
      if (enc === 'ucs2') return 'utf16le';
      break;

    case 3:
      if (enc === 'hex' || enc === 'HEX' || "".concat(enc).toLowerCase() === 'hex') return 'hex';
      break;

    case 5:
      if (enc === 'ascii') return 'ascii';
      if (enc === 'ucs-2') return 'utf16le';
      if (enc === 'UTF-8') return 'utf8';
      if (enc === 'ASCII') return 'ascii';
      if (enc === 'UCS-2') return 'utf16le';
      enc = "".concat(enc).toLowerCase();
      if (enc === 'utf-8') return 'utf8';
      if (enc === 'ascii') return 'ascii';
      if (enc === 'ucs-2') return 'utf16le';
      break;

    case 6:
      if (enc === 'base64') return 'base64';
      if (enc === 'latin1' || enc === 'binary') return 'latin1';
      if (enc === 'BASE64') return 'base64';
      if (enc === 'LATIN1' || enc === 'BINARY') return 'latin1';
      enc = "".concat(enc).toLowerCase();
      if (enc === 'base64') return 'base64';
      if (enc === 'latin1' || enc === 'binary') return 'latin1';
      break;

    case 7:
      if (enc === 'utf16le' || enc === 'UTF16LE' || "".concat(enc).toLowerCase() === 'utf16le') return 'utf16le';
      break;

    case 8:
      if (enc === 'utf-16le' || enc === 'UTF-16LE' || "".concat(enc).toLowerCase() === 'utf-16le') return 'utf16le';
      break;

    case 9:
      if (enc === 'base64url' || enc === 'BASE64URL' || "".concat(enc).toLowerCase() === 'base64url') return 'base64url';
      break;

    default:
      if (enc === '') return 'utf8';
  }
}

function emitExperimentalWarning(feature) {
  if (experimentalWarnings.has(feature)) return;
  var msg = "".concat(feature, " is an experimental feature. This feature could ") + 'change at any time';
  experimentalWarnings.add(feature);
  process.emitWarning(msg, 'ExperimentalWarning');
}

function filterDuplicateStrings(items, low) {
  var map = new SafeMap();

  for (var i = 0; i < items.length; i++) {
    var item = items[i];
    var key = StringPrototypeToLowerCase(item);

    if (low) {
      map.set(key, key);
    } else {
      map.set(key, item);
    }
  }

  return ArrayPrototypeSort(ArrayFrom(map.values()));
}

function cachedResult(fn) {
  var result;
  return function () {
    if (result === undefined) result = fn();
    return ArrayPrototypeSlice(result);
  };
} // Useful for Wrapping an ES6 Class with a constructor Function that
// does not require the new keyword. For instance:
//   class A { constructor(x) {this.x = x;}}
//   const B = createClassWrapper(A);
//   B() instanceof A // true
//   B() instanceof B // true


function createClassWrapper(type) {
  function fn() {
    for (var _len2 = arguments.length, args = new Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
      args[_key2] = arguments[_key2];
    }

    return ReflectConstruct(type, args, (this instanceof fn ? this.constructor : void 0) || type);
  } // Mask the wrapper function name and length values


  ObjectDefineProperties(fn, {
    name: {
      value: type.name
    },
    length: {
      value: type.length
    }
  });
  ObjectSetPrototypeOf(fn, type);
  fn.prototype = type.prototype;
  return fn;
}

var signalsToNamesMapping;

function getSignalsToNamesMapping() {
  if (signalsToNamesMapping !== undefined) return signalsToNamesMapping;
  signalsToNamesMapping = ObjectCreate(null);

  for (var key in signals) {
    signalsToNamesMapping[signals[key]] = key;
  }

  return signalsToNamesMapping;
}

function convertToValidSignal(signal) {
  if (typeof signal === 'number' && getSignalsToNamesMapping()[signal]) return signal;

  if (typeof signal === 'string') {
    var signalName = signals[StringPrototypeToUpperCase(signal)];
    if (signalName) return signalName;
  }

  throw new ERR_UNKNOWN_SIGNAL(signal);
}

function getConstructorOf(obj) {
  while (obj) {
    var descriptor = ObjectGetOwnPropertyDescriptor(obj, 'constructor');

    if (descriptor !== undefined && typeof descriptor.value === 'function' && descriptor.value.name !== '') {
      return descriptor.value;
    }

    obj = ObjectGetPrototypeOf(obj);
  }

  return null;
}

function getSystemErrorName(err) {
  var entry = uvErrmapGet(err);
  return entry ? entry[0] : "Unknown system error ".concat(err);
}

function getSystemErrorMap() {
  return lazyUv().getErrorMap();
}

var kCustomPromisifiedSymbol = SymbolFor('nodejs.util.promisify.custom');

var kCustomPromisifyArgsSymbol = _Symbol('customPromisifyArgs');

function promisify(original) {
  if (typeof original !== 'function') throw new ERR_INVALID_ARG_TYPE('original', 'Function', original);

  if (original[kCustomPromisifiedSymbol]) {
    var _fn = original[kCustomPromisifiedSymbol];

    if (typeof _fn !== 'function') {
      throw new ERR_INVALID_ARG_TYPE('util.promisify.custom', 'Function', _fn);
    }

    return ObjectDefineProperty(_fn, kCustomPromisifiedSymbol, {
      value: _fn,
      enumerable: false,
      writable: false,
      configurable: true
    });
  } // Names to create an object from in case the callback receives multiple
  // arguments, e.g. ['bytesRead', 'buffer'] for fs.read.


  var argumentNames = original[kCustomPromisifyArgsSymbol];

  function fn() {
    var _this = this;

    for (var _len3 = arguments.length, args = new Array(_len3), _key3 = 0; _key3 < _len3; _key3++) {
      args[_key3] = arguments[_key3];
    }

    return new Promise(function (resolve, reject) {
      ArrayPrototypePush(args, function (err) {
        if (err) {
          return reject(err);
        }

        if (argumentNames !== undefined && (arguments.length <= 1 ? 0 : arguments.length - 1) > 1) {
          var obj = {};

          for (var i = 0; i < argumentNames.length; i++) {
            obj[argumentNames[i]] = i + 1 < 1 || arguments.length <= i + 1 ? undefined : arguments[i + 1];
          }

          resolve(obj);
        } else {
          resolve(arguments.length <= 1 ? undefined : arguments[1]);
        }
      });
      ReflectApply(original, _this, args);
    });
  }

  ObjectSetPrototypeOf(fn, ObjectGetPrototypeOf(original));
  ObjectDefineProperty(fn, kCustomPromisifiedSymbol, {
    value: fn,
    enumerable: false,
    writable: false,
    configurable: true
  });
  return ObjectDefineProperties(fn, ObjectGetOwnPropertyDescriptors(original));
}

promisify.custom = kCustomPromisifiedSymbol; // The built-in Array#join is slower in v8 6.0

function join(output, separator) {
  var str = '';

  if (output.length !== 0) {
    var lastIndex = output.length - 1;

    for (var i = 0; i < lastIndex; i++) {
      // It is faster not to use a template string here
      str += output[i];
      str += separator;
    }

    str += output[lastIndex];
  }

  return str;
} // As of V8 6.6, depending on the size of the array, this is anywhere
// between 1.5-10x faster than the two-arg version of Array#splice()


function spliceOne(list, index) {
  for (; index + 1 < list.length; index++) {
    list[index] = list[index + 1];
  }

  list.pop();
}

var kNodeModulesRE = /^(.*)[\\/]node_modules[\\/]/;
var getStructuredStack;

function isInsideNodeModules() {
  if (getStructuredStack === undefined) {
    // Lazy-load to avoid a circular dependency.
    var _require2 = require('vm'),
        runInNewContext = _require2.runInNewContext; // Use `runInNewContext()` to get something tamper-proof and
    // side-effect-free. Since this is currently only used for a deprecated API,
    // the perf implications should be okay.


    getStructuredStack = runInNewContext("(function() {\n      try { Error.stackTraceLimit = Infinity; } catch {}\n      return function structuredStack() {\n        const e = new Error();\n        overrideStackTrace.set(e, (err, trace) => trace);\n        return e.stack;\n      };\n    })()", {
      overrideStackTrace: overrideStackTrace
    }, {
      filename: 'structured-stack'
    });
  }

  var stack = getStructuredStack(); // Iterate over all stack frames and look for the first one not coming
  // from inside Node.js itself:

  if (ArrayIsArray(stack)) {
    var _iterator = _createForOfIteratorHelper(stack),
        _step;

    try {
      for (_iterator.s(); !(_step = _iterator.n()).done;) {
        var frame = _step.value;
        var filename = frame.getFileName(); // If a filename does not start with / or contain \,
        // it's likely from Node.js core.

        if (!RegExpPrototypeTest(/^\/|\\/, filename)) continue;
        return RegExpPrototypeTest(kNodeModulesRE, filename);
      }
    } catch (err) {
      _iterator.e(err);
    } finally {
      _iterator.f();
    }
  }

  return false;
}

function once(callback) {
  var called = false;
  return function () {
    if (called) return;
    called = true;

    for (var _len4 = arguments.length, args = new Array(_len4), _key4 = 0; _key4 < _len4; _key4++) {
      args[_key4] = arguments[_key4];
    }

    ReflectApply(callback, this, args);
  };
}

var validateUint32;

function sleep(msec) {
  // Lazy-load to avoid a circular dependency.
  if (validateUint32 === undefined) {
    var _require3 = require('internal/validators');

    validateUint32 = _require3.validateUint32;
  }

  validateUint32(msec, 'msec');

  _sleep(msec);
}

function createDeferredPromise() {
  var resolve;
  var reject;
  var promise = new Promise(function (res, rej) {
    resolve = res;
    reject = rej;
  });
  return {
    promise: promise,
    resolve: resolve,
    reject: reject
  };
}

module.exports = {
  assertCrypto: assertCrypto,
  cachedResult: cachedResult,
  convertToValidSignal: convertToValidSignal,
  createClassWrapper: createClassWrapper,
  createDeferredPromise: createDeferredPromise,
  decorateErrorStack: decorateErrorStack,
  deprecate: deprecate,
  emitExperimentalWarning: emitExperimentalWarning,
  filterDuplicateStrings: filterDuplicateStrings,
  getConstructorOf: getConstructorOf,
  getSystemErrorMap: getSystemErrorMap,
  getSystemErrorName: getSystemErrorName,
  isError: isError,
  isInsideNodeModules: isInsideNodeModules,
  join: join,
  normalizeEncoding: normalizeEncoding,
  once: once,
  promisify: promisify,
  sleep: sleep,
  spliceOne: spliceOne,
  removeColors: removeColors,
  // Symbol used to customize promisify conversion
  customPromisifyArgs: kCustomPromisifyArgsSymbol,
  // Symbol used to provide a custom inspect function for an object as an
  // alternative to using 'inspect'
  customInspectSymbol: SymbolFor('nodejs.util.inspect.custom'),
  // Used by the buffer module to capture an internal reference to the
  // default isEncoding implementation, just in case userland overrides it.
  kIsEncodingSymbol: _Symbol('kIsEncodingSymbol'),
  kVmBreakFirstLineSymbol: _Symbol('kVmBreakFirstLineSymbol')
};
