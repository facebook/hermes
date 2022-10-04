// @nolint
'use strict'; // The whole point behind this internal module is to allow Node.js to no
// longer be forced to treat every error message change as a semver-major
// change. The NodeError classes here all expose a `code` property whose
// value statically and permanently identifies the error. While the error
// message may change, the code should not.

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = _getPrototypeOf(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

var _primordials = primordials,
    AggregateError = _primordials.AggregateError,
    ArrayFrom = _primordials.ArrayFrom,
    ArrayIsArray = _primordials.ArrayIsArray,
    ArrayPrototypeIncludes = _primordials.ArrayPrototypeIncludes,
    ArrayPrototypeIndexOf = _primordials.ArrayPrototypeIndexOf,
    ArrayPrototypeJoin = _primordials.ArrayPrototypeJoin,
    ArrayPrototypeMap = _primordials.ArrayPrototypeMap,
    ArrayPrototypePop = _primordials.ArrayPrototypePop,
    ArrayPrototypePush = _primordials.ArrayPrototypePush,
    ArrayPrototypeSlice = _primordials.ArrayPrototypeSlice,
    ArrayPrototypeSplice = _primordials.ArrayPrototypeSplice,
    ArrayPrototypeUnshift = _primordials.ArrayPrototypeUnshift,
    Error = _primordials.Error,
    ErrorCaptureStackTrace = _primordials.ErrorCaptureStackTrace,
    ErrorPrototypeToString = _primordials.ErrorPrototypeToString,
    JSONStringify = _primordials.JSONStringify,
    MapPrototypeGet = _primordials.MapPrototypeGet,
    MathAbs = _primordials.MathAbs,
    MathMax = _primordials.MathMax,
    Number = _primordials.Number,
    NumberIsInteger = _primordials.NumberIsInteger,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    ObjectIsExtensible = _primordials.ObjectIsExtensible,
    ObjectGetOwnPropertyDescriptor = _primordials.ObjectGetOwnPropertyDescriptor,
    ObjectKeys = _primordials.ObjectKeys,
    ObjectPrototypeHasOwnProperty = _primordials.ObjectPrototypeHasOwnProperty,
    RangeError = _primordials.RangeError,
    ReflectApply = _primordials.ReflectApply,
    RegExpPrototypeTest = _primordials.RegExpPrototypeTest,
    SafeArrayIterator = _primordials.SafeArrayIterator,
    SafeMap = _primordials.SafeMap,
    SafeWeakMap = _primordials.SafeWeakMap,
    String = _primordials.String,
    StringPrototypeEndsWith = _primordials.StringPrototypeEndsWith,
    StringPrototypeIncludes = _primordials.StringPrototypeIncludes,
    StringPrototypeMatch = _primordials.StringPrototypeMatch,
    StringPrototypeSlice = _primordials.StringPrototypeSlice,
    StringPrototypeSplit = _primordials.StringPrototypeSplit,
    StringPrototypeStartsWith = _primordials.StringPrototypeStartsWith,
    StringPrototypeToLowerCase = _primordials.StringPrototypeToLowerCase,
    _Symbol = _primordials.Symbol,
    SymbolFor = _primordials.SymbolFor,
    SyntaxError = _primordials.SyntaxError,
    _TypeError = _primordials.TypeError,
    URIError = _primordials.URIError;
var isWindows = process.platform === 'win32';
var messages = new SafeMap();
var codes = {};
var classRegExp = /^([A-Z][a-z0-9]*)+$/; // Sorted by a rough estimate on most frequently used entries.

var kTypes = ['string', 'function', 'number', 'object', // Accept 'Function' and 'Object' as alternative to the lower cased version.
'Function', 'Object', 'boolean', 'bigint', 'symbol'];
var MainContextError = Error;
var overrideStackTrace = new SafeWeakMap();

var kNoOverride = _Symbol('kNoOverride');

var userStackTraceLimit;
var nodeInternalPrefix = '__node_internal_';

var prepareStackTrace = function prepareStackTrace(globalThis, error, trace) {
  var _trace$;

  // API for node internals to override error stack formatting
  // without interfering with userland code.
  if (overrideStackTrace.has(error)) {
    var f = overrideStackTrace.get(error);
    overrideStackTrace["delete"](error);
    return f(error, trace);
  }

  var firstFrame = (_trace$ = trace[0]) === null || _trace$ === void 0 ? void 0 : _trace$.getFunctionName();

  if (firstFrame && StringPrototypeStartsWith(firstFrame, nodeInternalPrefix)) {
    for (var l = trace.length - 1; l >= 0; l--) {
      var _trace$l;

      var fn = (_trace$l = trace[l]) === null || _trace$l === void 0 ? void 0 : _trace$l.getFunctionName();

      if (fn && StringPrototypeStartsWith(fn, nodeInternalPrefix)) {
        ArrayPrototypeSplice(trace, 0, l + 1);
        break;
      }
    } // `userStackTraceLimit` is the user value for `Error.stackTraceLimit`,
    // it is updated at every new exception in `captureLargerStackTrace`.


    if (trace.length > userStackTraceLimit) ArrayPrototypeSplice(trace, userStackTraceLimit);
  }

  var globalOverride = maybeOverridePrepareStackTrace(globalThis, error, trace);
  if (globalOverride !== kNoOverride) return globalOverride; // Normal error formatting:
  //
  // Error: Message
  //     at function (file)
  //     at file

  var errorString = ErrorPrototypeToString(error);

  if (trace.length === 0) {
    return errorString;
  }

  return "".concat(errorString, "\n    at ").concat(ArrayPrototypeJoin(trace, '\n    at '));
};

var maybeOverridePrepareStackTrace = function maybeOverridePrepareStackTrace(globalThis, error, trace) {
  var _globalThis$Error;

  // Polyfill of V8's Error.prepareStackTrace API.
  // https://crbug.com/v8/7848
  // `globalThis` is the global that contains the constructor which
  // created `error`.
  if (typeof ((_globalThis$Error = globalThis.Error) === null || _globalThis$Error === void 0 ? void 0 : _globalThis$Error.prepareStackTrace) === 'function') {
    return globalThis.Error.prepareStackTrace(error, trace);
  } // We still have legacy usage that depends on the main context's `Error`
  // being used, even when the error is from a different context.
  // TODO(devsnek): evaluate if this can be eventually deprecated/removed.


  if (typeof MainContextError.prepareStackTrace === 'function') {
    return MainContextError.prepareStackTrace(error, trace);
  }

  return kNoOverride;
};

var aggregateTwoErrors = hideStackFrames(function (innerError, outerError) {
  if (innerError && outerError) {
    if (ArrayIsArray(outerError.errors)) {
      // If `outerError` is already an `AggregateError`.
      ArrayPrototypePush(outerError.errors, innerError);
      return outerError;
    } // eslint-disable-next-line no-restricted-syntax


    var err = new AggregateError(new SafeArrayIterator([outerError, innerError]), outerError.message);
    err.code = outerError.code;
    return err;
  }

  return innerError || outerError;
}); // Lazily loaded

var util;
var assert;
var internalUtil = null;

function lazyInternalUtil() {
  if (!internalUtil) {
    internalUtil = require('internal/util');
  }

  return internalUtil;
}

var internalUtilInspect = null;

function lazyInternalUtilInspect() {
  if (!internalUtilInspect) {
    internalUtilInspect = require('internal/util/inspect');
  }

  return internalUtilInspect;
}

var buffer;

function lazyBuffer() {
  if (buffer === undefined) buffer = require('buffer').Buffer;
  return buffer;
}

var addCodeToName = hideStackFrames(function addCodeToName(err, name, code) {
  // Set the stack
  err = captureLargerStackTrace(err); // Add the error code to the name to include it in the stack trace.

  err.name = "".concat(name, " [").concat(code, "]"); // Access the stack to generate the error message including the error code
  // from the name.

  err.stack; // eslint-disable-line no-unused-expressions
  // Reset the name to the actual name.

  if (name === 'SystemError') {
    ObjectDefineProperty(err, 'name', {
      value: name,
      enumerable: false,
      writable: true,
      configurable: true
    });
  } else {
    delete err.name;
  }
});

function isErrorStackTraceLimitWritable() {
  var desc = ObjectGetOwnPropertyDescriptor(Error, 'stackTraceLimit');

  if (desc === undefined) {
    return ObjectIsExtensible(Error);
  }

  return ObjectPrototypeHasOwnProperty(desc, 'writable') ? desc.writable : desc.set !== undefined;
} // A specialized Error that includes an additional info property with
// additional information about the error condition.
// It has the properties present in a UVException but with a custom error
// message followed by the uv error code and uv error message.
// It also has its own error code with the original uv error context put into
// `err.info`.
// The context passed into this error must have .code, .syscall and .message,
// and may have .path and .dest.


var SystemError = /*#__PURE__*/function (_Error) {
  _inherits(SystemError, _Error);

  var _super = _createSuper(SystemError);

  function SystemError(key, context) {
    var _this;

    _classCallCheck(this, SystemError);

    var limit = Error.stackTraceLimit;
    if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = 0;
    _this = _super.call(this); // Reset the limit and setting the name property.

    if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = limit;
    var prefix = getMessage(key, [], _assertThisInitialized(_this));
    var message = "".concat(prefix, ": ").concat(context.syscall, " returned ") + "".concat(context.code, " (").concat(context.message, ")");
    if (context.path !== undefined) message += " ".concat(context.path);
    if (context.dest !== undefined) message += " => ".concat(context.dest);
    ObjectDefineProperty(_assertThisInitialized(_this), 'message', {
      value: message,
      enumerable: false,
      writable: true,
      configurable: true
    });
    addCodeToName(_assertThisInitialized(_this), 'SystemError', key);
    _this.code = key;
    ObjectDefineProperty(_assertThisInitialized(_this), 'info', {
      value: context,
      enumerable: true,
      configurable: true,
      writable: false
    });
    ObjectDefineProperty(_assertThisInitialized(_this), 'errno', {
      get: function get() {
        return context.errno;
      },
      set: function set(value) {
        context.errno = value;
      },
      enumerable: true,
      configurable: true
    });
    ObjectDefineProperty(_assertThisInitialized(_this), 'syscall', {
      get: function get() {
        return context.syscall;
      },
      set: function set(value) {
        context.syscall = value;
      },
      enumerable: true,
      configurable: true
    });

    if (context.path !== undefined) {
      // TODO(BridgeAR): Investigate why and when the `.toString()` was
      // introduced. The `path` and `dest` properties in the context seem to
      // always be of type string. We should probably just remove the
      // `.toString()` and `Buffer.from()` operations and set the value on the
      // context as the user did.
      ObjectDefineProperty(_assertThisInitialized(_this), 'path', {
        get: function get() {
          return context.path != null ? context.path.toString() : context.path;
        },
        set: function set(value) {
          context.path = value ? lazyBuffer().from(value.toString()) : undefined;
        },
        enumerable: true,
        configurable: true
      });
    }

    if (context.dest !== undefined) {
      ObjectDefineProperty(_assertThisInitialized(_this), 'dest', {
        get: function get() {
          return context.dest != null ? context.dest.toString() : context.dest;
        },
        set: function set(value) {
          context.dest = value ? lazyBuffer().from(value.toString()) : undefined;
        },
        enumerable: true,
        configurable: true
      });
    }

    return _this;
  }

  _createClass(SystemError, [{
    key: "toString",
    value: function toString() {
      return "".concat(this.name, " [").concat(this.code, "]: ").concat(this.message);
    }
  }, {
    key: SymbolFor('nodejs.util.inspect.custom'),
    value: function value(recurseTimes, ctx) {
      return lazyInternalUtilInspect().inspect(this, _objectSpread(_objectSpread({}, ctx), {}, {
        getters: true,
        customInspect: false
      }));
    }
  }]);

  return SystemError;
}(Error);

function makeSystemErrorWithCode(key) {
  return /*#__PURE__*/function (_SystemError) {
    _inherits(NodeError, _SystemError);

    var _super2 = _createSuper(NodeError);

    function NodeError(ctx) {
      _classCallCheck(this, NodeError);

      return _super2.call(this, key, ctx);
    }

    return NodeError;
  }(SystemError);
}

function makeNodeErrorWithCode(Base, key) {
  return function NodeError() {
    var limit = Error.stackTraceLimit;
    if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = 0;
    var error = new Base(); // Reset the limit and setting the name property.

    if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = limit;

    for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
      args[_key] = arguments[_key];
    }

    var message = getMessage(key, args, error);
    ObjectDefineProperty(error, 'message', {
      value: message,
      enumerable: false,
      writable: true,
      configurable: true
    });
    ObjectDefineProperty(error, 'toString', {
      value: function value() {
        return "".concat(this.name, " [").concat(key, "]: ").concat(this.message);
      },
      enumerable: false,
      writable: true,
      configurable: true
    });
    addCodeToName(error, Base.name, key);
    error.code = key;
    return error;
  };
}
/**
 * This function removes unnecessary frames from Node.js core errors.
 * @template {(...args: any[]) => any} T
 * @type {(fn: T) => T}
 */


function hideStackFrames(fn) {
  // We rename the functions that will be hidden to cut off the stacktrace
  // at the outermost one
  var hidden = nodeInternalPrefix + fn.name;
  ObjectDefineProperty(fn, 'name', {
    value: hidden
  });
  return fn;
} // Utility function for registering the error codes. Only used here. Exported
// *only* to allow for testing.


function E(sym, val, def) {
  // Special case for SystemError that formats the error message differently
  // The SystemErrors only have SystemError as their base classes.
  messages.set(sym, val);

  if (def === SystemError) {
    def = makeSystemErrorWithCode(sym);
  } else {
    def = makeNodeErrorWithCode(def, sym);
  }

  for (var _len2 = arguments.length, otherClasses = new Array(_len2 > 3 ? _len2 - 3 : 0), _key2 = 3; _key2 < _len2; _key2++) {
    otherClasses[_key2 - 3] = arguments[_key2];
  }

  if (otherClasses.length !== 0) {
    otherClasses.forEach(function (clazz) {
      def[clazz.name] = makeNodeErrorWithCode(clazz, sym);
    });
  }

  codes[sym] = def;
}

function getMessage(key, args, self) {
  var msg = messages.get(key);
  if (assert === undefined) assert = require('internal/assert');

  if (typeof msg === 'function') {
    assert(msg.length <= args.length, // Default options do not count.
    "Code: ".concat(key, "; The provided arguments length (").concat(args.length, ") does not ") + "match the required ones (".concat(msg.length, ")."));
    return ReflectApply(msg, self, args);
  }

  var expectedLength = (StringPrototypeMatch(msg, /%[dfijoOs]/g) || []).length;
  assert(expectedLength === args.length, "Code: ".concat(key, "; The provided arguments length (").concat(args.length, ") does not ") + "match the required ones (".concat(expectedLength, ")."));
  if (args.length === 0) return msg;
  ArrayPrototypeUnshift(args, msg);
  return ReflectApply(lazyInternalUtilInspect().format, null, args);
}

var uvBinding;

function lazyUv() {
  if (!uvBinding) {
    uvBinding = internalBinding('uv');
  }

  return uvBinding;
}

var uvUnmappedError = ['UNKNOWN', 'unknown error'];

function uvErrmapGet(name) {
  uvBinding = lazyUv();

  if (!uvBinding.errmap) {
    uvBinding.errmap = uvBinding.getErrorMap();
  }

  return MapPrototypeGet(uvBinding.errmap, name);
}

var captureLargerStackTrace = hideStackFrames(function captureLargerStackTrace(err) {
  var stackTraceLimitIsWritable = isErrorStackTraceLimitWritable();
  if (stackTraceLimitIsWritable) {
    userStackTraceLimit = Error.stackTraceLimit;
    Error.stackTraceLimit = Infinity;
  }
  // ErrorCaptureStackTrace(err); // Reset the limit
  if (stackTraceLimitIsWritable) Error.stackTraceLimit = userStackTraceLimit;
  return err;
});
/**
 * This creates an error compatible with errors produced in the C++
 * function UVException using a context object with data assembled in C++.
 * The goal is to migrate them to ERR_* errors later when compatibility is
 * not a concern.
 *
 * @param {Object} ctx
 * @returns {Error}
 */

var uvException = hideStackFrames(function uvException(ctx) {
  var _ref = uvErrmapGet(ctx.errno) || uvUnmappedError,
      code = _ref[0],
      uvmsg = _ref[1];

  var message = "".concat(code, ": ").concat(ctx.message || uvmsg, ", ").concat(ctx.syscall);
  var path;
  var dest;

  if (ctx.path) {
    path = ctx.path.toString();
    message += " '".concat(path, "'");
  }

  if (ctx.dest) {
    dest = ctx.dest.toString();
    message += " -> '".concat(dest, "'");
  } // Reducing the limit improves the performance significantly. We do not lose
  // the stack frames due to the `captureStackTrace()` function that is called
  // later.


  var tmpLimit = Error.stackTraceLimit;
  if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = 0; // Pass the message to the constructor instead of setting it on the object
  // to make sure it is the same as the one created in C++
  // eslint-disable-next-line no-restricted-syntax

  var err = new Error(message);
  if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = tmpLimit;

  var _iterator = _createForOfIteratorHelper(ObjectKeys(ctx)),
      _step;

  try {
    for (_iterator.s(); !(_step = _iterator.n()).done;) {
      var prop = _step.value;

      if (prop === 'message' || prop === 'path' || prop === 'dest') {
        continue;
      }

      err[prop] = ctx[prop];
    }
  } catch (err) {
    _iterator.e(err);
  } finally {
    _iterator.f();
  }

  err.code = code;

  if (path) {
    err.path = path;
  }

  if (dest) {
    err.dest = dest;
  }

  return captureLargerStackTrace(err);
});
/**
 * This creates an error compatible with errors produced in the C++
 * This function should replace the deprecated
 * `exceptionWithHostPort()` function.
 *
 * @param {number} err - A libuv error number
 * @param {string} syscall
 * @param {string} address
 * @param {number} [port]
 * @returns {Error}
 */

var uvExceptionWithHostPort = hideStackFrames(function uvExceptionWithHostPort(err, syscall, address, port) {
  var _ref2 = uvErrmapGet(err) || uvUnmappedError,
      code = _ref2[0],
      uvmsg = _ref2[1];

  var message = "".concat(syscall, " ").concat(code, ": ").concat(uvmsg);
  var details = '';

  if (port && port > 0) {
    details = " ".concat(address, ":").concat(port);
  } else if (address) {
    details = " ".concat(address);
  } // Reducing the limit improves the performance significantly. We do not
  // lose the stack frames due to the `captureStackTrace()` function that
  // is called later.


  var tmpLimit = Error.stackTraceLimit;
  if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = 0; // eslint-disable-next-line no-restricted-syntax

  var ex = new Error("".concat(message).concat(details));
  if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = tmpLimit;
  ex.code = code;
  ex.errno = err;
  ex.syscall = syscall;
  ex.address = address;

  if (port) {
    ex.port = port;
  }

  return captureLargerStackTrace(ex);
});
/**
 * This used to be util._errnoException().
 *
 * @param {number} err - A libuv error number
 * @param {string} syscall
 * @param {string} [original]
 * @returns {Error}
 */

var errnoException = hideStackFrames(function errnoException(err, syscall, original) {
  // TODO(joyeecheung): We have to use the type-checked
  // getSystemErrorName(err) to guard against invalid arguments from users.
  // This can be replaced with [ code ] = errmap.get(err) when this method
  // is no longer exposed to user land.
  if (util === undefined) util = require('util');
  var code = util.getSystemErrorName(err);
  var message = original ? "".concat(syscall, " ").concat(code, " ").concat(original) : "".concat(syscall, " ").concat(code);
  var tmpLimit = Error.stackTraceLimit;
  if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = 0; // eslint-disable-next-line no-restricted-syntax

  var ex = new Error(message);
  if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = tmpLimit;
  ex.errno = err;
  ex.code = code;
  ex.syscall = syscall;
  return captureLargerStackTrace(ex);
});
/**
 * Deprecated, new function is `uvExceptionWithHostPort()`
 * New function added the error description directly
 * from C++. this method for backwards compatibility
 * @param {number} err - A libuv error number
 * @param {string} syscall
 * @param {string} address
 * @param {number} [port]
 * @param {string} [additional]
 * @returns {Error}
 */

var exceptionWithHostPort = hideStackFrames(function exceptionWithHostPort(err, syscall, address, port, additional) {
  // TODO(joyeecheung): We have to use the type-checked
  // getSystemErrorName(err) to guard against invalid arguments from users.
  // This can be replaced with [ code ] = errmap.get(err) when this method
  // is no longer exposed to user land.
  if (util === undefined) util = require('util');
  var code = util.getSystemErrorName(err);
  var details = '';

  if (port && port > 0) {
    details = " ".concat(address, ":").concat(port);
  } else if (address) {
    details = " ".concat(address);
  }

  if (additional) {
    details += " - Local (".concat(additional, ")");
  } // Reducing the limit improves the performance significantly. We do not
  // lose the stack frames due to the `captureStackTrace()` function that
  // is called later.


  var tmpLimit = Error.stackTraceLimit;
  if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = 0; // eslint-disable-next-line no-restricted-syntax

  var ex = new Error("".concat(syscall, " ").concat(code).concat(details));
  if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = tmpLimit;
  ex.errno = err;
  ex.code = code;
  ex.syscall = syscall;
  ex.address = address;

  if (port) {
    ex.port = port;
  }

  return captureLargerStackTrace(ex);
});
/**
 * @param {number|string} code - A libuv error number or a c-ares error code
 * @param {string} syscall
 * @param {string} [hostname]
 * @returns {Error}
 */

var dnsException = hideStackFrames(function (code, syscall, hostname) {
  var errno; // If `code` is of type number, it is a libuv error number, else it is a
  // c-ares error code.
  // TODO(joyeecheung): translate c-ares error codes into numeric ones and
  // make them available in a property that's not error.errno (since they
  // can be in conflict with libuv error codes). Also make sure
  // util.getSystemErrorName() can understand them when an being informed that
  // the number is a c-ares error code.

  if (typeof code === 'number') {
    errno = code; // ENOTFOUND is not a proper POSIX error, but this error has been in place
    // long enough that it's not practical to remove it.

    if (code === lazyUv().UV_EAI_NODATA || code === lazyUv().UV_EAI_NONAME) {
      code = 'ENOTFOUND'; // Fabricated error name.
    } else {
      code = lazyInternalUtil().getSystemErrorName(code);
    }
  }

  var message = "".concat(syscall, " ").concat(code).concat(hostname ? " ".concat(hostname) : ''); // Reducing the limit improves the performance significantly. We do not lose
  // the stack frames due to the `captureStackTrace()` function that is called
  // later.

  var tmpLimit = Error.stackTraceLimit;
  if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = 0; // eslint-disable-next-line no-restricted-syntax

  var ex = new Error(message);
  if (isErrorStackTraceLimitWritable()) Error.stackTraceLimit = tmpLimit;
  ex.errno = errno;
  ex.code = code;
  ex.syscall = syscall;

  if (hostname) {
    ex.hostname = hostname;
  }

  return captureLargerStackTrace(ex);
});

function connResetException(msg) {
  // eslint-disable-next-line no-restricted-syntax
  var ex = new Error(msg);
  ex.code = 'ECONNRESET';
  return ex;
}

var maxStack_ErrorName;
var maxStack_ErrorMessage;
/**
 * Returns true if `err.name` and `err.message` are equal to engine-specific
 * values indicating max call stack size has been exceeded.
 * "Maximum call stack size exceeded" in V8.
 *
 * @param {Error} err
 * @returns {boolean}
 */

function isStackOverflowError(err) {
  if (maxStack_ErrorMessage === undefined) {
    try {
      var overflowStack = function overflowStack() {
        overflowStack();
      };

      overflowStack();
    } catch (err) {
      maxStack_ErrorMessage = err.message;
      maxStack_ErrorName = err.name;
    }
  }

  return err && err.name === maxStack_ErrorName && err.message === maxStack_ErrorMessage;
} // Only use this for integers! Decimal numbers do not work with this function.


function addNumericalSeparator(val) {
  var res = '';
  var i = val.length;
  var start = val[0] === '-' ? 1 : 0;

  for (; i >= start + 4; i -= 3) {
    res = "_".concat(StringPrototypeSlice(val, i - 3, i)).concat(res);
  }

  return "".concat(StringPrototypeSlice(val, 0, i)).concat(res);
} // Used to enhance the stack that will be picked up by the inspector


var kEnhanceStackBeforeInspector = _Symbol('kEnhanceStackBeforeInspector'); // These are supposed to be called only on fatal exceptions before
// the process exits.


var fatalExceptionStackEnhancers = {
  beforeInspector: function beforeInspector(error) {
    if (typeof error[kEnhanceStackBeforeInspector] !== 'function') {
      return error.stack;
    }

    try {
      // Set the error.stack here so it gets picked up by the
      // inspector.
      error.stack = error[kEnhanceStackBeforeInspector]();
    } catch (_unused) {// We are just enhancing the error. If it fails, ignore it.
    }

    return error.stack;
  },
  afterInspector: function afterInspector(error) {
    var originalStack = error.stack;
    var useColors = true; // Some consoles do not convert ANSI escape sequences to colors,
    // rather display them directly to the stdout. On those consoles,
    // libuv emulates colors by intercepting stdout stream and calling
    // corresponding Windows API functions for setting console colors.
    // However, fatal error are handled differently and we cannot easily
    // highlight them. On Windows, detecting whether a console supports
    // ANSI escape sequences is not reliable.

    if (process.platform === 'win32') {
      var info = internalBinding('os').getOSInformation();
      var ver = ArrayPrototypeMap(StringPrototypeSplit(info[2], '.'), Number);

      if (ver[0] !== 10 || ver[2] < 14393) {
        useColors = false;
      }
    }

    var _lazyInternalUtilInsp = lazyInternalUtilInspect(),
        inspect = _lazyInternalUtilInsp.inspect,
        defaultColors = _lazyInternalUtilInsp.inspectDefaultOptions.colors;

    var colors = useColors && (internalBinding('util').guessHandleType(2) === 'TTY' && require('internal/tty').hasColors() || defaultColors);

    try {
      return inspect(error, {
        colors: colors,
        customInspect: false,
        depth: MathMax(inspect.defaultOptions.depth, 5)
      });
    } catch (_unused2) {
      return originalStack;
    }
  }
}; // Node uses an AbortError that isn't exactly the same as the DOMException
// to make usage of the error in userland and readable-stream easier.
// It is a regular error with `.code` and `.name`.

var AbortError = /*#__PURE__*/function (_Error2) {
  _inherits(AbortError, _Error2);

  var _super3 = _createSuper(AbortError);

  function AbortError() {
    var _this2;

    _classCallCheck(this, AbortError);

    _this2 = _super3.call(this, 'The operation was aborted');
    _this2.code = 'ABORT_ERR';
    _this2.name = 'AbortError';
    return _this2;
  }

  return AbortError;
}(Error);

module.exports = {
  addCodeToName: addCodeToName,
  // Exported for NghttpError
  aggregateTwoErrors: aggregateTwoErrors,
  codes: codes,
  dnsException: dnsException,
  errnoException: errnoException,
  exceptionWithHostPort: exceptionWithHostPort,
  getMessage: getMessage,
  hideStackFrames: hideStackFrames,
  isErrorStackTraceLimitWritable: isErrorStackTraceLimitWritable,
  isStackOverflowError: isStackOverflowError,
  connResetException: connResetException,
  uvErrmapGet: uvErrmapGet,
  uvException: uvException,
  uvExceptionWithHostPort: uvExceptionWithHostPort,
  SystemError: SystemError,
  AbortError: AbortError,
  // This is exported only to facilitate testing.
  E: E,
  kNoOverride: kNoOverride,
  prepareStackTrace: prepareStackTrace,
  maybeOverridePrepareStackTrace: maybeOverridePrepareStackTrace,
  overrideStackTrace: overrideStackTrace,
  kEnhanceStackBeforeInspector: kEnhanceStackBeforeInspector,
  fatalExceptionStackEnhancers: fatalExceptionStackEnhancers
}; // To declare an error message, use the E(sym, val, def) function above. The sym
// must be an upper case string. The val can be either a function or a string.
// The def must be an error class.
// The return value of the function must be a string.
// Examples:
// E('EXAMPLE_KEY1', 'This is the error value', Error);
// E('EXAMPLE_KEY2', (a, b) => return `${a} ${b}`, RangeError);
//
// Once an error code has been assigned, the code itself MUST NOT change and
// any given error code must never be reused to identify a different error.
//
// Any error code added here should also be added to the documentation
//
// Note: Please try to keep these in alphabetical order
//
// Note: Node.js specific errors must begin with the prefix ERR_

E('ERR_AMBIGUOUS_ARGUMENT', 'The "%s" argument is ambiguous. %s', _TypeError);
E('ERR_ARG_NOT_ITERABLE', '%s must be iterable', _TypeError);
E('ERR_ASSERTION', '%s', Error);
E('ERR_ASYNC_CALLBACK', '%s must be a function', _TypeError);
E('ERR_ASYNC_TYPE', 'Invalid name for async "type": %s', _TypeError);
E('ERR_BROTLI_INVALID_PARAM', '%s is not a valid Brotli parameter', RangeError);
E('ERR_BUFFER_OUT_OF_BOUNDS', // Using a default argument here is important so the argument is not counted
// towards `Function#length`.
function () {
  var name = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : undefined;

  if (name) {
    return "\"".concat(name, "\" is outside of buffer bounds");
  }

  return 'Attempt to access memory outside buffer bounds';
}, RangeError);
E('ERR_BUFFER_TOO_LARGE', 'Cannot create a Buffer larger than %s bytes', RangeError);
E('ERR_CANNOT_WATCH_SIGINT', 'Cannot watch for SIGINT signals', Error);
E('ERR_CHILD_CLOSED_BEFORE_REPLY', 'Child closed before reply received', Error);
E('ERR_CHILD_PROCESS_IPC_REQUIRED', "Forked processes must have an IPC channel, missing value 'ipc' in %s", Error);
E('ERR_CHILD_PROCESS_STDIO_MAXBUFFER', '%s maxBuffer length exceeded', RangeError);
E('ERR_CONSOLE_WRITABLE_STREAM', 'Console expects a writable stream instance for %s', _TypeError);
E('ERR_CONTEXT_NOT_INITIALIZED', 'context used is not initialized', Error);
E('ERR_CRYPTO_CUSTOM_ENGINE_NOT_SUPPORTED', 'Custom engines not supported by this OpenSSL', Error);
E('ERR_CRYPTO_ECDH_INVALID_FORMAT', 'Invalid ECDH format: %s', _TypeError);
E('ERR_CRYPTO_ECDH_INVALID_PUBLIC_KEY', 'Public key is not valid for specified curve', Error);
E('ERR_CRYPTO_ENGINE_UNKNOWN', 'Engine "%s" was not found', Error);
E('ERR_CRYPTO_FIPS_FORCED', 'Cannot set FIPS mode, it was forced with --force-fips at startup.', Error);
E('ERR_CRYPTO_FIPS_UNAVAILABLE', 'Cannot set FIPS mode in a non-FIPS build.', Error);
E('ERR_CRYPTO_HASH_FINALIZED', 'Digest already called', Error);
E('ERR_CRYPTO_HASH_UPDATE_FAILED', 'Hash update failed', Error);
E('ERR_CRYPTO_INCOMPATIBLE_KEY', 'Incompatible %s: %s', Error);
E('ERR_CRYPTO_INCOMPATIBLE_KEY_OPTIONS', 'The selected key encoding %s %s.', Error);
E('ERR_CRYPTO_INVALID_DIGEST', 'Invalid digest: %s', _TypeError);
E('ERR_CRYPTO_INVALID_JWK', 'Invalid JWK data', _TypeError);
E('ERR_CRYPTO_INVALID_KEY_OBJECT_TYPE', 'Invalid key object type %s, expected %s.', _TypeError);
E('ERR_CRYPTO_INVALID_STATE', 'Invalid state for operation %s', Error);
E('ERR_CRYPTO_JWK_UNSUPPORTED_CURVE', 'Unsupported JWK EC curve: %s.', Error);
E('ERR_CRYPTO_JWK_UNSUPPORTED_KEY_TYPE', 'Unsupported JWK Key Type.', Error);
E('ERR_CRYPTO_PBKDF2_ERROR', 'PBKDF2 error', Error);
E('ERR_CRYPTO_SCRYPT_INVALID_PARAMETER', 'Invalid scrypt parameter', Error);
E('ERR_CRYPTO_SCRYPT_NOT_SUPPORTED', 'Scrypt algorithm not supported', Error); // Switch to TypeError. The current implementation does not seem right.

E('ERR_CRYPTO_SIGN_KEY_REQUIRED', 'No key provided to sign', Error);
E('ERR_DEBUGGER_ERROR', '%s', Error);
E('ERR_DEBUGGER_STARTUP_ERROR', '%s', Error);
E('ERR_DIR_CLOSED', 'Directory handle was closed', Error);
E('ERR_DIR_CONCURRENT_OPERATION', 'Cannot do synchronous work on directory handle with concurrent ' + 'asynchronous operations', Error);
E('ERR_DNS_SET_SERVERS_FAILED', 'c-ares failed to set servers: "%s" [%s]', Error);
E('ERR_DOMAIN_CALLBACK_NOT_AVAILABLE', 'A callback was registered through ' + 'process.setUncaughtExceptionCaptureCallback(), which is mutually ' + 'exclusive with using the `domain` module', Error);
E('ERR_DOMAIN_CANNOT_SET_UNCAUGHT_EXCEPTION_CAPTURE', 'The `domain` module is in use, which is mutually exclusive with calling ' + 'process.setUncaughtExceptionCaptureCallback()', Error);
E('ERR_ENCODING_INVALID_ENCODED_DATA', function (encoding, ret) {
  this.errno = ret;
  return "The encoded data was not valid for encoding ".concat(encoding);
}, _TypeError);
E('ERR_ENCODING_NOT_SUPPORTED', 'The "%s" encoding is not supported', RangeError);
E('ERR_EVAL_ESM_CANNOT_PRINT', '--print cannot be used with ESM input', Error);
E('ERR_EVENT_RECURSION', 'The event "%s" is already being dispatched', Error);
E('ERR_FALSY_VALUE_REJECTION', function (reason) {
  this.reason = reason;
  return 'Promise was rejected with falsy value';
}, Error);
E('ERR_FEATURE_UNAVAILABLE_ON_PLATFORM', 'The feature %s is unavailable on the current platform' + ', which is being used to run Node.js', _TypeError);
E('ERR_FS_EISDIR', 'Path is a directory', SystemError);
E('ERR_FS_FILE_TOO_LARGE', 'File size (%s) is greater than 2 GB', RangeError);
E('ERR_FS_INVALID_SYMLINK_TYPE', 'Symlink type must be one of "dir", "file", or "junction". Received "%s"', Error); // Switch to TypeError. The current implementation does not seem right

E('ERR_HTTP2_ALTSVC_INVALID_ORIGIN', 'HTTP/2 ALTSVC frames require a valid origin', _TypeError);
E('ERR_HTTP2_ALTSVC_LENGTH', 'HTTP/2 ALTSVC frames are limited to 16382 bytes', _TypeError);
E('ERR_HTTP2_CONNECT_AUTHORITY', ':authority header is required for CONNECT requests', Error);
E('ERR_HTTP2_CONNECT_PATH', 'The :path header is forbidden for CONNECT requests', Error);
E('ERR_HTTP2_CONNECT_SCHEME', 'The :scheme header is forbidden for CONNECT requests', Error);
E('ERR_HTTP2_GOAWAY_SESSION', 'New streams cannot be created after receiving a GOAWAY', Error);
E('ERR_HTTP2_HEADERS_AFTER_RESPOND', 'Cannot specify additional headers after response initiated', Error);
E('ERR_HTTP2_HEADERS_SENT', 'Response has already been initiated.', Error);
E('ERR_HTTP2_HEADER_SINGLE_VALUE', 'Header field "%s" must only have a single value', _TypeError);
E('ERR_HTTP2_INFO_STATUS_NOT_ALLOWED', 'Informational status codes cannot be used', RangeError);
E('ERR_HTTP2_INVALID_CONNECTION_HEADERS', 'HTTP/1 Connection specific headers are forbidden: "%s"', _TypeError);
E('ERR_HTTP2_INVALID_HEADER_VALUE', 'Invalid value "%s" for header "%s"', _TypeError);
E('ERR_HTTP2_INVALID_INFO_STATUS', 'Invalid informational status code: %s', RangeError);
E('ERR_HTTP2_INVALID_ORIGIN', 'HTTP/2 ORIGIN frames require a valid origin', _TypeError);
E('ERR_HTTP2_INVALID_PACKED_SETTINGS_LENGTH', 'Packed settings length must be a multiple of six', RangeError);
E('ERR_HTTP2_INVALID_PSEUDOHEADER', '"%s" is an invalid pseudoheader or is used incorrectly', _TypeError);
E('ERR_HTTP2_INVALID_SESSION', 'The session has been destroyed', Error);
E('ERR_HTTP2_INVALID_SETTING_VALUE', // Using default arguments here is important so the arguments are not counted
// towards `Function#length`.
function (name, actual) {
  var min = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : undefined;
  var max = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : undefined;
  this.actual = actual;

  if (min !== undefined) {
    this.min = min;
    this.max = max;
  }

  return "Invalid value for setting \"".concat(name, "\": ").concat(actual);
}, _TypeError, RangeError);
E('ERR_HTTP2_INVALID_STREAM', 'The stream has been destroyed', Error);
E('ERR_HTTP2_MAX_PENDING_SETTINGS_ACK', 'Maximum number of pending settings acknowledgements', Error);
E('ERR_HTTP2_NESTED_PUSH', 'A push stream cannot initiate another push stream.', Error);
E('ERR_HTTP2_NO_MEM', 'Out of memory', Error);
E('ERR_HTTP2_NO_SOCKET_MANIPULATION', 'HTTP/2 sockets should not be directly manipulated (e.g. read and written)', Error);
E('ERR_HTTP2_ORIGIN_LENGTH', 'HTTP/2 ORIGIN frames are limited to 16382 bytes', _TypeError);
E('ERR_HTTP2_OUT_OF_STREAMS', 'No stream ID is available because maximum stream ID has been reached', Error);
E('ERR_HTTP2_PAYLOAD_FORBIDDEN', 'Responses with %s status must not have a payload', Error);
E('ERR_HTTP2_PING_CANCEL', 'HTTP2 ping cancelled', Error);
E('ERR_HTTP2_PING_LENGTH', 'HTTP2 ping payload must be 8 bytes', RangeError);
E('ERR_HTTP2_PSEUDOHEADER_NOT_ALLOWED', 'Cannot set HTTP/2 pseudo-headers', _TypeError);
E('ERR_HTTP2_PUSH_DISABLED', 'HTTP/2 client has disabled push streams', Error);
E('ERR_HTTP2_SEND_FILE', 'Directories cannot be sent', Error);
E('ERR_HTTP2_SEND_FILE_NOSEEK', 'Offset or length can only be specified for regular files', Error);
E('ERR_HTTP2_SESSION_ERROR', 'Session closed with error code %s', Error);
E('ERR_HTTP2_SETTINGS_CANCEL', 'HTTP2 session settings canceled', Error);
E('ERR_HTTP2_SOCKET_BOUND', 'The socket is already bound to an Http2Session', Error);
E('ERR_HTTP2_SOCKET_UNBOUND', 'The socket has been disconnected from the Http2Session', Error);
E('ERR_HTTP2_STATUS_101', 'HTTP status code 101 (Switching Protocols) is forbidden in HTTP/2', Error);
E('ERR_HTTP2_STATUS_INVALID', 'Invalid status code: %s', RangeError);
E('ERR_HTTP2_STREAM_CANCEL', function (error) {
  var msg = 'The pending stream has been canceled';

  if (error) {
    this.cause = error;
    if (typeof error.message === 'string') msg += " (caused by: ".concat(error.message, ")");
  }

  return msg;
}, Error);
E('ERR_HTTP2_STREAM_ERROR', 'Stream closed with error code %s', Error);
E('ERR_HTTP2_STREAM_SELF_DEPENDENCY', 'A stream cannot depend on itself', Error);
E('ERR_HTTP2_TOO_MANY_INVALID_FRAMES', 'Too many invalid HTTP/2 frames', Error);
E('ERR_HTTP2_TRAILERS_ALREADY_SENT', 'Trailing headers have already been sent', Error);
E('ERR_HTTP2_TRAILERS_NOT_READY', 'Trailing headers cannot be sent until after the wantTrailers event is ' + 'emitted', Error);
E('ERR_HTTP2_UNSUPPORTED_PROTOCOL', 'protocol "%s" is unsupported.', Error);
E('ERR_HTTP_HEADERS_SENT', 'Cannot %s headers after they are sent to the client', Error);
E('ERR_HTTP_INVALID_HEADER_VALUE', 'Invalid value "%s" for header "%s"', _TypeError);
E('ERR_HTTP_INVALID_STATUS_CODE', 'Invalid status code: %s', RangeError);
E('ERR_HTTP_REQUEST_TIMEOUT', 'Request timeout', Error);
E('ERR_HTTP_SOCKET_ENCODING', 'Changing the socket encoding is not allowed per RFC7230 Section 3.', Error);
E('ERR_HTTP_TRAILER_INVALID', 'Trailers are invalid with this transfer encoding', Error);
E('ERR_INCOMPATIBLE_OPTION_PAIR', 'Option "%s" cannot be used in combination with option "%s"', _TypeError);
E('ERR_INPUT_TYPE_NOT_ALLOWED', '--input-type can only be used with string ' + 'input via --eval, --print, or STDIN', Error);
E('ERR_INSPECTOR_ALREADY_ACTIVATED', 'Inspector is already activated. Close it with inspector.close() ' + 'before activating it again.', Error);
E('ERR_INSPECTOR_ALREADY_CONNECTED', '%s is already connected', Error);
E('ERR_INSPECTOR_CLOSED', 'Session was closed', Error);
E('ERR_INSPECTOR_COMMAND', 'Inspector error %d: %s', Error);
E('ERR_INSPECTOR_NOT_ACTIVE', 'Inspector is not active', Error);
E('ERR_INSPECTOR_NOT_AVAILABLE', 'Inspector is not available', Error);
E('ERR_INSPECTOR_NOT_CONNECTED', 'Session is not connected', Error);
E('ERR_INSPECTOR_NOT_WORKER', 'Current thread is not a worker', Error);
E('ERR_INTERNAL_ASSERTION', function (message) {
  var suffix = 'This is caused by either a bug in Node.js ' + 'or incorrect usage of Node.js internals.\n' + 'Please open an issue with this stack trace at ' + 'https://github.com/nodejs/node/issues\n';
  return message === undefined ? suffix : "".concat(message, "\n").concat(suffix);
}, Error);
E('ERR_INVALID_ADDRESS_FAMILY', function (addressType, host, port) {
  this.host = host;
  this.port = port;
  return "Invalid address family: ".concat(addressType, " ").concat(host, ":").concat(port);
}, RangeError);
E('ERR_INVALID_ARG_TYPE', function (name, expected, actual) {
  assert(typeof name === 'string', "'name' must be a string");

  if (!ArrayIsArray(expected)) {
    expected = [expected];
  }

  var msg = 'The ';

  if (StringPrototypeEndsWith(name, ' argument')) {
    // For cases like 'first argument'
    msg += "".concat(name, " ");
  } else {
    var type = StringPrototypeIncludes(name, '.') ? 'property' : 'argument';
    msg += "\"".concat(name, "\" ").concat(type, " ");
  }

  msg += 'must be ';
  var types = [];
  var instances = [];
  var other = [];

  var _iterator2 = _createForOfIteratorHelper(expected),
      _step2;

  try {
    for (_iterator2.s(); !(_step2 = _iterator2.n()).done;) {
      var value = _step2.value;
      assert(typeof value === 'string', 'All expected entries have to be of type string');

      if (ArrayPrototypeIncludes(kTypes, value)) {
        ArrayPrototypePush(types, StringPrototypeToLowerCase(value));
      } else if (RegExpPrototypeTest(classRegExp, value)) {
        ArrayPrototypePush(instances, value);
      } else {
        assert(value !== 'object', 'The value "object" should be written as "Object"');
        ArrayPrototypePush(other, value);
      }
    } // Special handle `object` in case other instances are allowed to outline
    // the differences between each other.

  } catch (err) {
    _iterator2.e(err);
  } finally {
    _iterator2.f();
  }

  if (instances.length > 0) {
    var pos = ArrayPrototypeIndexOf(types, 'object');

    if (pos !== -1) {
      ArrayPrototypeSplice(types, pos, 1);
      ArrayPrototypePush(instances, 'Object');
    }
  }

  if (types.length > 0) {
    if (types.length > 2) {
      var last = ArrayPrototypePop(types);
      msg += "one of type ".concat(ArrayPrototypeJoin(types, ', '), ", or ").concat(last);
    } else if (types.length === 2) {
      msg += "one of type ".concat(types[0], " or ").concat(types[1]);
    } else {
      msg += "of type ".concat(types[0]);
    }

    if (instances.length > 0 || other.length > 0) msg += ' or ';
  }

  if (instances.length > 0) {
    if (instances.length > 2) {
      var _last = ArrayPrototypePop(instances);

      msg += "an instance of ".concat(ArrayPrototypeJoin(instances, ', '), ", or ").concat(_last);
    } else {
      msg += "an instance of ".concat(instances[0]);

      if (instances.length === 2) {
        msg += " or ".concat(instances[1]);
      }
    }

    if (other.length > 0) msg += ' or ';
  }

  if (other.length > 0) {
    if (other.length > 2) {
      var _last2 = ArrayPrototypePop(other);

      msg += "one of ".concat(ArrayPrototypeJoin(other, ', '), ", or ").concat(_last2);
    } else if (other.length === 2) {
      msg += "one of ".concat(other[0], " or ").concat(other[1]);
    } else {
      if (StringPrototypeToLowerCase(other[0]) !== other[0]) msg += 'an ';
      msg += "".concat(other[0]);
    }
  }

  if (actual == null) {
    msg += ". Received ".concat(actual);
  } else if (typeof actual === 'function' && actual.name) {
    msg += ". Received function ".concat(actual.name);
  } else if (_typeof(actual) === 'object') {
    if (actual.constructor && actual.constructor.name) {
      msg += ". Received an instance of ".concat(actual.constructor.name);
    } else {
      var inspected = lazyInternalUtilInspect().inspect(actual, {
        depth: -1
      });
      msg += ". Received ".concat(inspected);
    }
  } else {
    var _inspected = lazyInternalUtilInspect().inspect(actual, {
      colors: false
    });

    if (_inspected.length > 25) _inspected = "".concat(StringPrototypeSlice(_inspected, 0, 25), "...");
    msg += ". Received type ".concat(_typeof(actual), " (").concat(_inspected, ")");
  }

  return msg;
}, _TypeError);
E('ERR_INVALID_ARG_VALUE', function (name, value) {
  var reason = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : 'is invalid';
  var inspected = lazyInternalUtilInspect().inspect(value);
  if (inspected.length > 128) {
    inspected = "".concat(StringPrototypeSlice(inspected, 0, 128), "...");
  }
  var type = StringPrototypeIncludes(name, '.') ? 'property' : 'argument';
  return "The ".concat(type, " '").concat(name, "' ").concat(reason, ". Received ").concat(inspected);
}, _TypeError, RangeError);
E('ERR_INVALID_ASYNC_ID', 'Invalid %s value: %s', RangeError);
E('ERR_INVALID_BUFFER_SIZE', 'Buffer size must be a multiple of %s', RangeError);
E('ERR_INVALID_CALLBACK', 'Callback must be a function. Received %O', _TypeError);
E('ERR_INVALID_CHAR', // Using a default argument here is important so the argument is not counted
// towards `Function#length`.
function (name) {
  var field = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : undefined;
  var msg = "Invalid character in ".concat(name);

  if (field !== undefined) {
    msg += " [\"".concat(field, "\"]");
  }

  return msg;
}, _TypeError);
E('ERR_INVALID_CURSOR_POS', 'Cannot set cursor row without setting its column', _TypeError);
E('ERR_INVALID_FD', '"fd" must be a positive integer: %s', RangeError);
E('ERR_INVALID_FD_TYPE', 'Unsupported fd type: %s', _TypeError);
E('ERR_INVALID_FILE_URL_HOST', 'File URL host must be "localhost" or empty on %s', _TypeError);
E('ERR_INVALID_FILE_URL_PATH', 'File URL path %s', _TypeError);
E('ERR_INVALID_HANDLE_TYPE', 'This handle type cannot be sent', _TypeError);
E('ERR_INVALID_HTTP_TOKEN', '%s must be a valid HTTP token ["%s"]', _TypeError);
E('ERR_INVALID_IP_ADDRESS', 'Invalid IP address: %s', _TypeError);
E('ERR_INVALID_MODULE_SPECIFIER', function (request, reason) {
  var base = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : undefined;
  return "Invalid module \"".concat(request, "\" ").concat(reason).concat(base ? " imported from ".concat(base) : '');
}, _TypeError);
E('ERR_INVALID_PACKAGE_CONFIG', function (path, base, message) {
  return "Invalid package config ".concat(path).concat(base ? " while importing ".concat(base) : '').concat(message ? ". ".concat(message) : '');
}, Error);
E('ERR_INVALID_PACKAGE_TARGET', function (pkgPath, key, target) {
  var isImport = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : false;
  var base = arguments.length > 4 && arguments[4] !== undefined ? arguments[4] : undefined;
  var relError = typeof target === 'string' && !isImport && target.length && !StringPrototypeStartsWith(target, './');

  if (key === '.') {
    assert(isImport === false);
    return "Invalid \"exports\" main target ".concat(JSONStringify(target), " defined ") + "in the package config ".concat(pkgPath, "package.json").concat(base ? " imported from ".concat(base) : '').concat(relError ? '; targets must start with "./"' : '');
  }

  return "Invalid \"".concat(isImport ? 'imports' : 'exports', "\" target ").concat(JSONStringify(target), " defined for '").concat(key, "' in the package config ").concat(pkgPath, "package.json").concat(base ? " imported from ".concat(base) : '').concat(relError ? '; targets must start with "./"' : '');
}, Error);
E('ERR_INVALID_PERFORMANCE_MARK', 'The "%s" performance mark has not been set', Error);
E('ERR_INVALID_PROTOCOL', 'Protocol "%s" not supported. Expected "%s"', _TypeError);
E('ERR_INVALID_REPL_EVAL_CONFIG', 'Cannot specify both "breakEvalOnSigint" and "eval" for REPL', _TypeError);
E('ERR_INVALID_REPL_INPUT', '%s', _TypeError);
E('ERR_INVALID_RETURN_PROPERTY', function (input, name, prop, value) {
  return "Expected a valid ".concat(input, " to be returned for the \"").concat(prop, "\" from the") + " \"".concat(name, "\" function but got ").concat(value, ".");
}, _TypeError);
E('ERR_INVALID_RETURN_PROPERTY_VALUE', function (input, name, prop, value) {
  var type;

  if (value && value.constructor && value.constructor.name) {
    type = "instance of ".concat(value.constructor.name);
  } else {
    type = "type ".concat(_typeof(value));
  }

  return "Expected ".concat(input, " to be returned for the \"").concat(prop, "\" from the") + " \"".concat(name, "\" function but got ").concat(type, ".");
}, _TypeError);
E('ERR_INVALID_RETURN_VALUE', function (input, name, value) {
  var type;

  if (value && value.constructor && value.constructor.name) {
    type = "instance of ".concat(value.constructor.name);
  } else {
    type = "type ".concat(_typeof(value));
  }

  return "Expected ".concat(input, " to be returned from the \"").concat(name, "\"") + " function but got ".concat(type, ".");
}, _TypeError);
E('ERR_INVALID_STATE', 'Invalid state: %s', Error);
E('ERR_INVALID_SYNC_FORK_INPUT', 'Asynchronous forks do not support ' + 'Buffer, TypedArray, DataView or string input: %s', _TypeError);
E('ERR_INVALID_THIS', 'Value of "this" must be of type %s', _TypeError);
E('ERR_INVALID_TUPLE', '%s must be an iterable %s tuple', _TypeError);
E('ERR_INVALID_URI', 'URI malformed', URIError);
E('ERR_INVALID_URL', function (input) {
  this.input = input; // Don't include URL in message.
  // (See https://github.com/nodejs/node/pull/38614)

  return 'Invalid URL';
}, _TypeError);
E('ERR_INVALID_URL_SCHEME', function (expected) {
  if (typeof expected === 'string') expected = [expected];
  assert(expected.length <= 2);
  var res = expected.length === 2 ? "one of scheme ".concat(expected[0], " or ").concat(expected[1]) : "of scheme ".concat(expected[0]);
  return "The URL must be ".concat(res);
}, _TypeError);
E('ERR_IPC_CHANNEL_CLOSED', 'Channel closed', Error);
E('ERR_IPC_DISCONNECTED', 'IPC channel is already disconnected', Error);
E('ERR_IPC_ONE_PIPE', 'Child process can have only one IPC pipe', Error);
E('ERR_IPC_SYNC_FORK', 'IPC cannot be used with synchronous forks', Error);
E('ERR_MANIFEST_ASSERT_INTEGRITY', function (moduleURL, realIntegrities) {
  var msg = "The content of \"".concat(moduleURL, "\" does not match the expected integrity.");

  if (realIntegrities.size) {
    var sri = ArrayPrototypeJoin(ArrayFrom(realIntegrities.entries(), function (_ref3) {
      var alg = _ref3[0],
          dgs = _ref3[1];
      return "".concat(alg, "-").concat(dgs);
    }), ' ');
    msg += " Integrities found are: ".concat(sri);
  } else {
    msg += ' The resource was not found in the policy.';
  }

  return msg;
}, Error);
E('ERR_MANIFEST_DEPENDENCY_MISSING', 'Manifest resource %s does not list %s as a dependency specifier for ' + 'conditions: %s', Error);
E('ERR_MANIFEST_INTEGRITY_MISMATCH', 'Manifest resource %s has multiple entries but integrity lists do not match', SyntaxError);
E('ERR_MANIFEST_INVALID_RESOURCE_FIELD', 'Manifest resource %s has invalid property value for %s', _TypeError);
E('ERR_MANIFEST_TDZ', 'Manifest initialization has not yet run', Error);
E('ERR_MANIFEST_UNKNOWN_ONERROR', 'Manifest specified unknown error behavior "%s".', SyntaxError);
E('ERR_METHOD_NOT_IMPLEMENTED', 'The %s method is not implemented', Error);
E('ERR_MISSING_ARGS', function () {
  for (var _len3 = arguments.length, args = new Array(_len3), _key3 = 0; _key3 < _len3; _key3++) {
    args[_key3] = arguments[_key3];
  }

  assert(args.length > 0, 'At least one arg needs to be specified');
  var msg = 'The ';
  var len = args.length;

  var wrap = function wrap(a) {
    return "\"".concat(a, "\"");
  };

  args = ArrayPrototypeMap(args, function (a) {
    return ArrayIsArray(a) ? ArrayPrototypeJoin(ArrayPrototypeMap(a, wrap), ' or ') : wrap(a);
  });

  switch (len) {
    case 1:
      msg += "".concat(args[0], " argument");
      break;

    case 2:
      msg += "".concat(args[0], " and ").concat(args[1], " arguments");
      break;

    default:
      msg += ArrayPrototypeJoin(ArrayPrototypeSlice(args, 0, len - 1), ', ');
      msg += ", and ".concat(args[len - 1], " arguments");
      break;
  }

  return "".concat(msg, " must be specified");
}, _TypeError);
E('ERR_MISSING_OPTION', '%s is required', _TypeError);
E('ERR_MODULE_NOT_FOUND', function (path, base) {
  var type = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : 'package';
  return "Cannot find ".concat(type, " '").concat(path, "' imported from ").concat(base);
}, Error);
E('ERR_MULTIPLE_CALLBACK', 'Callback called multiple times', Error);
E('ERR_NAPI_CONS_FUNCTION', 'Constructor must be a function', _TypeError);
E('ERR_NAPI_INVALID_DATAVIEW_ARGS', 'byte_offset + byte_length should be less than or equal to the size in ' + 'bytes of the array passed in', RangeError);
E('ERR_NAPI_INVALID_TYPEDARRAY_ALIGNMENT', 'start offset of %s should be a multiple of %s', RangeError);
E('ERR_NAPI_INVALID_TYPEDARRAY_LENGTH', 'Invalid typed array length', RangeError);
E('ERR_NO_CRYPTO', 'Node.js is not compiled with OpenSSL crypto support', Error);
E('ERR_NO_ICU', '%s is not supported on Node.js compiled without ICU', _TypeError);
E('ERR_OPERATION_FAILED', 'Operation failed: %s', Error);
E('ERR_OUT_OF_RANGE', function (str, range, input) {
  var replaceDefaultBoolean = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : false;
  assert(range, 'Missing "range" argument');
  var msg = replaceDefaultBoolean ? str : "The value of \"".concat(str, "\" is out of range.");
  var received;

  if (NumberIsInteger(input) && MathAbs(input) > Math.pow(2, 32)) {
    received = addNumericalSeparator(String(input));
  } else if (typeof input === 'bigint') {
    received = String(input);

    if (input > Math.pow(2, 32) || input < -Math.pow(2, 32)) { //Removed the bigint on 2 and 32 twice
      received = addNumericalSeparator(received);
    }

    received += 'n';
  } else {
    received = lazyInternalUtilInspect().inspect(input);
  }

  msg += " It must be ".concat(range, ". Received ").concat(received);
  return msg;
}, RangeError);
E('ERR_PACKAGE_IMPORT_NOT_DEFINED', function (specifier, packagePath, base) {
  return "Package import specifier \"".concat(specifier, "\" is not defined").concat(packagePath ? " in package ".concat(packagePath, "package.json") : '', " imported from ").concat(base);
}, _TypeError);
E('ERR_PACKAGE_PATH_NOT_EXPORTED', function (pkgPath, subpath) {
  var base = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : undefined;
  if (subpath === '.') return "No \"exports\" main defined in ".concat(pkgPath, "package.json").concat(base ? " imported from ".concat(base) : '');
  return "Package subpath '".concat(subpath, "' is not defined by \"exports\" in ").concat(pkgPath, "package.json").concat(base ? " imported from ".concat(base) : '');
}, Error);
E('ERR_PERFORMANCE_INVALID_TIMESTAMP', '%d is not a valid timestamp', _TypeError);
E('ERR_PERFORMANCE_MEASURE_INVALID_OPTIONS', '%s', _TypeError);
E('ERR_REQUIRE_ESM', function (filename) {
  var parentPath = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : null;
  var packageJsonPath = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : null;
  var msg = "Must use import to load ES Module: ".concat(filename);

  if (parentPath && packageJsonPath) {
    var path = require('path');

    var basename = path.basename(filename) === path.basename(parentPath) ? filename : path.basename(filename);
    msg += '\nrequire() of ES modules is not supported.\nrequire() of ' + "".concat(filename, " from ").concat(parentPath, " ") + 'is an ES module file as it is a .js file whose nearest parent ' + 'package.json contains "type": "module" which defines all .js ' + 'files in that package scope as ES modules.\nInstead rename ' + "".concat(basename, " to end in .cjs, change the requiring code to use ") + 'import(), or remove "type": "module" from ' + "".concat(packageJsonPath, ".\n");
    return msg;
  }

  return msg;
}, Error);
E('ERR_SCRIPT_EXECUTION_INTERRUPTED', 'Script execution was interrupted by `SIGINT`', Error);
E('ERR_SERVER_ALREADY_LISTEN', 'Listen method has been called more than once without closing.', Error);
E('ERR_SERVER_NOT_RUNNING', 'Server is not running.', Error);
E('ERR_SOCKET_ALREADY_BOUND', 'Socket is already bound', Error);
E('ERR_SOCKET_BAD_BUFFER_SIZE', 'Buffer size must be a positive integer', _TypeError);
E('ERR_SOCKET_BAD_PORT', function (name, port) {
  var allowZero = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : true;
  assert(typeof allowZero === 'boolean', "The 'allowZero' argument must be of type boolean.");
  var operator = allowZero ? '>=' : '>';
  return "".concat(name, " should be ").concat(operator, " 0 and < 65536. Received ").concat(port, ".");
}, RangeError);
E('ERR_SOCKET_BAD_TYPE', 'Bad socket type specified. Valid types are: udp4, udp6', _TypeError);
E('ERR_SOCKET_BUFFER_SIZE', 'Could not get or set buffer size', SystemError);
E('ERR_SOCKET_CLOSED', 'Socket is closed', Error);
E('ERR_SOCKET_DGRAM_IS_CONNECTED', 'Already connected', Error);
E('ERR_SOCKET_DGRAM_NOT_CONNECTED', 'Not connected', Error);
E('ERR_SOCKET_DGRAM_NOT_RUNNING', 'Not running', Error);
E('ERR_SRI_PARSE', 'Subresource Integrity string %j had an unexpected %j at position %d', SyntaxError);
E('ERR_STREAM_ALREADY_FINISHED', 'Cannot call %s after a stream was finished', Error);
E('ERR_STREAM_CANNOT_PIPE', 'Cannot pipe, not readable', Error);
E('ERR_STREAM_DESTROYED', 'Cannot call %s after a stream was destroyed', Error);
E('ERR_STREAM_NULL_VALUES', 'May not write null values to stream', _TypeError);
E('ERR_STREAM_PREMATURE_CLOSE', 'Premature close', Error);
E('ERR_STREAM_PUSH_AFTER_EOF', 'stream.push() after EOF', Error);
E('ERR_STREAM_UNSHIFT_AFTER_END_EVENT', 'stream.unshift() after end event', Error);
E('ERR_STREAM_WRAP', 'Stream has StringDecoder set or is in objectMode', Error);
E('ERR_STREAM_WRITE_AFTER_END', 'write after end', Error);
E('ERR_SYNTHETIC', 'JavaScript Callstack', Error);
E('ERR_SYSTEM_ERROR', 'A system error occurred', SystemError);
E('ERR_TLS_CERT_ALTNAME_INVALID', function (reason, host, cert) {
  this.reason = reason;
  this.host = host;
  this.cert = cert;
  return "Hostname/IP does not match certificate's altnames: ".concat(reason);
}, Error);
E('ERR_TLS_DH_PARAM_SIZE', 'DH parameter size %s is less than 2048', Error);
E('ERR_TLS_HANDSHAKE_TIMEOUT', 'TLS handshake timeout', Error);
E('ERR_TLS_INVALID_CONTEXT', '%s must be a SecureContext', _TypeError);
E('ERR_TLS_INVALID_PROTOCOL_VERSION', '%j is not a valid %s TLS protocol version', _TypeError);
E('ERR_TLS_INVALID_STATE', 'TLS socket connection must be securely established', Error);
E('ERR_TLS_PROTOCOL_VERSION_CONFLICT', 'TLS protocol version %j conflicts with secureProtocol %j', _TypeError);
E('ERR_TLS_RENEGOTIATION_DISABLED', 'TLS session renegotiation disabled for this socket', Error); // This should probably be a `TypeError`.

E('ERR_TLS_REQUIRED_SERVER_NAME', '"servername" is required parameter for Server.addContext', Error);
E('ERR_TLS_SESSION_ATTACK', 'TLS session renegotiation attack detected', Error);
E('ERR_TLS_SNI_FROM_SERVER', 'Cannot issue SNI from a TLS server-side socket', Error);
E('ERR_TRACE_EVENTS_CATEGORY_REQUIRED', 'At least one category is required', _TypeError);
E('ERR_TRACE_EVENTS_UNAVAILABLE', 'Trace events are unavailable', Error); // This should probably be a `RangeError`.

E('ERR_TTY_INIT_FAILED', 'TTY initialization failed', SystemError);
E('ERR_UNAVAILABLE_DURING_EXIT', 'Cannot call function in process exit ' + 'handler', Error);
E('ERR_UNCAUGHT_EXCEPTION_CAPTURE_ALREADY_SET', '`process.setupUncaughtExceptionCapture()` was called while a capture ' + 'callback was already active', Error);
E('ERR_UNESCAPED_CHARACTERS', '%s contains unescaped characters', _TypeError);
E('ERR_UNHANDLED_ERROR', // Using a default argument here is important so the argument is not counted
// towards `Function#length`.
function () {
  var err = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : undefined;
  var msg = 'Unhandled error.';
  if (err === undefined) return msg;
  return "".concat(msg, " (").concat(err, ")");
}, Error);
E('ERR_UNKNOWN_BUILTIN_MODULE', 'No such built-in module: %s', Error);
E('ERR_UNKNOWN_CREDENTIAL', '%s identifier does not exist: %s', Error);
E('ERR_UNKNOWN_ENCODING', 'Unknown encoding: %s', _TypeError);
E('ERR_UNKNOWN_FILE_EXTENSION', 'Unknown file extension "%s" for %s', _TypeError);
E('ERR_UNKNOWN_MODULE_FORMAT', 'Unknown module format: %s', RangeError);
E('ERR_UNKNOWN_SIGNAL', 'Unknown signal: %s', _TypeError);
E('ERR_UNSUPPORTED_DIR_IMPORT', "Directory import '%s' is not supported " + 'resolving ES modules imported from %s', Error);
E('ERR_UNSUPPORTED_ESM_URL_SCHEME', function (url) {
  var msg = 'Only file and data URLs are supported by the default ESM loader';

  if (isWindows && url.protocol.length === 2) {
    msg += '. On Windows, absolute paths must be valid file:// URLs';
  }

  msg += ". Received protocol '".concat(url.protocol, "'");
  return msg;
}, Error); // This should probably be a `TypeError`.

E('ERR_VALID_PERFORMANCE_ENTRY_TYPE', 'At least one valid performance entry type is required', Error);
E('ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING', 'A dynamic import callback was not specified.', _TypeError);
E('ERR_VM_MODULE_ALREADY_LINKED', 'Module has already been linked', Error);
E('ERR_VM_MODULE_CANNOT_CREATE_CACHED_DATA', 'Cached data cannot be created for a module which has been evaluated', Error);
E('ERR_VM_MODULE_DIFFERENT_CONTEXT', 'Linked modules must use the same context', Error);
E('ERR_VM_MODULE_LINKING_ERRORED', 'Linking has already failed for the provided module', Error);
E('ERR_VM_MODULE_NOT_MODULE', 'Provided module is not an instance of Module', Error);
E('ERR_VM_MODULE_STATUS', 'Module status %s', Error);
E('ERR_WASI_ALREADY_STARTED', 'WASI instance has already started', Error);
E('ERR_WORKER_INIT_FAILED', 'Worker initialization failure: %s', Error);
E('ERR_WORKER_INVALID_EXEC_ARGV', function (errors) {
  var msg = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'invalid execArgv flags';
  return "Initiated Worker with ".concat(msg, ": ").concat(ArrayPrototypeJoin(errors, ', '));
}, Error);
E('ERR_WORKER_NOT_RUNNING', 'Worker instance not running', Error);
E('ERR_WORKER_OUT_OF_MEMORY', 'Worker terminated due to reaching memory limit: %s', Error);
E('ERR_WORKER_PATH', function (filename) {
  return 'The worker script or module filename must be an absolute path or a ' + 'relative path starting with \'./\' or \'../\'.' + (StringPrototypeStartsWith(filename, 'file://') ? ' Wrap file:// URLs with `new URL`.' : '') + (StringPrototypeStartsWith(filename, 'data:text/javascript') ? ' Wrap data: URLs with `new URL`.' : '') + " Received \"".concat(filename, "\"");
}, _TypeError);
E('ERR_WORKER_UNSERIALIZABLE_ERROR', 'Serializing an uncaught exception failed', Error);
E('ERR_WORKER_UNSUPPORTED_EXTENSION', 'The worker script extension must be ".js", ".mjs", or ".cjs". Received "%s"', _TypeError);
E('ERR_WORKER_UNSUPPORTED_OPERATION', '%s is not supported in workers', _TypeError);
E('ERR_ZLIB_INITIALIZATION_FAILED', 'Initialization failed', Error);
