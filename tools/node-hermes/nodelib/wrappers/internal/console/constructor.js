'use strict'; // The Console constructor is not actually used to construct the global
// console. It's exported for backwards compatibility.

var _ObjectDefineProperti2;

function _toConsumableArray(arr) { return _arrayWithoutHoles(arr) || _iterableToArray(arr) || _unsupportedIterableToArray(arr) || _nonIterableSpread(); }

function _nonIterableSpread() { throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }

function _iterableToArray(iter) { if (typeof Symbol !== "undefined" && iter[Symbol.iterator] != null || iter["@@iterator"] != null) return Array.from(iter); }

function _arrayWithoutHoles(arr) { if (Array.isArray(arr)) return _arrayLikeToArray(arr); }

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

var _primordials = primordials,
    ArrayFrom = _primordials.ArrayFrom,
    ArrayIsArray = _primordials.ArrayIsArray,
    ArrayPrototypeForEach = _primordials.ArrayPrototypeForEach,
    ArrayPrototypePush = _primordials.ArrayPrototypePush,
    ArrayPrototypeUnshift = _primordials.ArrayPrototypeUnshift,
    Boolean = _primordials.Boolean,
    ErrorCaptureStackTrace = _primordials.ErrorCaptureStackTrace,
    FunctionPrototypeBind = _primordials.FunctionPrototypeBind,
    MathFloor = _primordials.MathFloor,
    Number = _primordials.Number,
    NumberPrototypeToFixed = _primordials.NumberPrototypeToFixed,
    ObjectDefineProperties = _primordials.ObjectDefineProperties,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    ObjectKeys = _primordials.ObjectKeys,
    ObjectPrototypeHasOwnProperty = _primordials.ObjectPrototypeHasOwnProperty,
    ObjectValues = _primordials.ObjectValues,
    ReflectApply = _primordials.ReflectApply,
    ReflectConstruct = _primordials.ReflectConstruct,
    ReflectOwnKeys = _primordials.ReflectOwnKeys,
    SafeArrayIterator = _primordials.SafeArrayIterator,
    SafeMap = _primordials.SafeMap,
    SafeWeakMap = _primordials.SafeWeakMap,
    StringPrototypeIncludes = _primordials.StringPrototypeIncludes,
    StringPrototypePadStart = _primordials.StringPrototypePadStart,
    StringPrototypeRepeat = _primordials.StringPrototypeRepeat,
    StringPrototypeReplace = _primordials.StringPrototypeReplace,
    StringPrototypeSlice = _primordials.StringPrototypeSlice,
    StringPrototypeSplit = _primordials.StringPrototypeSplit,
    _Symbol = _primordials.Symbol,
    SymbolHasInstance = _primordials.SymbolHasInstance,
    SymbolToStringTag = _primordials.SymbolToStringTag;

// var _internalBinding = internalBinding('trace_events'),
//     trace = _internalBinding.trace;

var _require = require('internal/errors'),
    isStackOverflowError = _require.isStackOverflowError,
    _require$codes = _require.codes,
    ERR_CONSOLE_WRITABLE_STREAM = _require$codes.ERR_CONSOLE_WRITABLE_STREAM,
    ERR_INVALID_ARG_TYPE = _require$codes.ERR_INVALID_ARG_TYPE,
    ERR_INVALID_ARG_VALUE = _require$codes.ERR_INVALID_ARG_VALUE,
    ERR_INCOMPATIBLE_OPTION_PAIR = _require$codes.ERR_INCOMPATIBLE_OPTION_PAIR;

var _require2 = require('internal/validators'),
    validateInteger = _require2.validateInteger;

// var _internalBinding2 = internalBinding('util'),
//     previewEntries = _internalBinding2.previewEntries;

var _require3 = require('buffer'),
    isBuffer = _require3.Buffer.isBuffer;

var _require4 = require('internal/util/inspect'),
    inspect = _require4.inspect,
    formatWithOptions = _require4.formatWithOptions;

var _require5 = require('internal/util/types'),
    isTypedArray = _require5.isTypedArray,
    isSet = _require5.isSet,
    isMap = _require5.isMap,
    isSetIterator = _require5.isSetIterator,
    isMapIterator = _require5.isMapIterator;

var _require6 = require('internal/constants'),
    CHAR_LOWERCASE_B = _require6.CHAR_LOWERCASE_B,
    CHAR_LOWERCASE_E = _require6.CHAR_LOWERCASE_E,
    CHAR_LOWERCASE_N = _require6.CHAR_LOWERCASE_N,
    CHAR_UPPERCASE_C = _require6.CHAR_UPPERCASE_C;

var kCounts = _Symbol('counts');

var kTraceConsoleCategory = 'node,node.console';
var kTraceCount = CHAR_UPPERCASE_C;
var kTraceBegin = CHAR_LOWERCASE_B;
var kTraceEnd = CHAR_LOWERCASE_E;
var kTraceInstant = CHAR_LOWERCASE_N;
var kSecond = 1000;
var kMinute = 60 * kSecond;
var kHour = 60 * kMinute;
var kMaxGroupIndentation = 1000; // Lazy loaded for startup performance.

var cliTable; // Track amount of indentation required via `console.group()`.

var kGroupIndent = _Symbol('kGroupIndent');

var kGroupIndentationWidth = _Symbol('kGroupIndentWidth');

var kFormatForStderr = _Symbol('kFormatForStderr');

var kFormatForStdout = _Symbol('kFormatForStdout');

var kGetInspectOptions = _Symbol('kGetInspectOptions');

var kColorMode = _Symbol('kColorMode');

var kIsConsole = _Symbol('kIsConsole');

var kWriteToConsole = _Symbol('kWriteToConsole');

var kBindProperties = _Symbol('kBindProperties');

var kBindStreamsEager = _Symbol('kBindStreamsEager');

var kBindStreamsLazy = _Symbol('kBindStreamsLazy');

var kUseStdout = _Symbol('kUseStdout');

var kUseStderr = _Symbol('kUseStderr');

var optionsMap = new SafeWeakMap();

function Console(options
/* or: stdout, stderr, ignoreErrors = true */
) {
  var _this = this;

  // We have to test new.target here to see if this function is called
  // with new, because we need to define a custom instanceof to accommodate
  // the global console.
  if (!(this instanceof Console ? this.constructor : void 0)) {
    return ReflectConstruct(Console, arguments);
  }

  if (!options || typeof options.write === 'function') {
    options = {
      stdout: options,
      stderr: arguments[1],
      ignoreErrors: arguments[2]
    };
  }

  var _options = options,
      stdout = _options.stdout,
      _options$stderr = _options.stderr,
      stderr = _options$stderr === void 0 ? stdout : _options$stderr,
      _options$ignoreErrors = _options.ignoreErrors,
      ignoreErrors = _options$ignoreErrors === void 0 ? true : _options$ignoreErrors,
      _options$colorMode = _options.colorMode,
      colorMode = _options$colorMode === void 0 ? 'auto' : _options$colorMode,
      inspectOptions = _options.inspectOptions,
      groupIndentation = _options.groupIndentation;

  if (!stdout || typeof stdout.write !== 'function') {
    throw new ERR_CONSOLE_WRITABLE_STREAM('stdout');
  }

  if (!stderr || typeof stderr.write !== 'function') {
    throw new ERR_CONSOLE_WRITABLE_STREAM('stderr');
  }

  if (typeof colorMode !== 'boolean' && colorMode !== 'auto') throw new ERR_INVALID_ARG_VALUE('colorMode', colorMode);

  if (groupIndentation !== undefined) {
    validateInteger(groupIndentation, 'groupIndentation', 0, kMaxGroupIndentation);
  }

  if (_typeof(inspectOptions) === 'object' && inspectOptions !== null) {
    if (inspectOptions.colors !== undefined && options.colorMode !== undefined) {
      throw new ERR_INCOMPATIBLE_OPTION_PAIR('options.inspectOptions.color', 'colorMode');
    }

    optionsMap.set(this, inspectOptions);
  } else if (inspectOptions !== undefined) {
    throw new ERR_INVALID_ARG_TYPE('options.inspectOptions', 'object', inspectOptions);
  } // Bind the prototype functions to this Console instance

  ArrayPrototypeForEach(ObjectKeys(Console.prototype), function (key) {
    // We have to bind the methods grabbed from the instance instead of from
    // the prototype so that users extending the Console can override them
    // from the prototype chain of the subclass.
    _this[key] = FunctionPrototypeBind(_this[key], _this);
    ObjectDefineProperty(_this[key], 'name', {
      value: key
    });
  });
  this[kBindStreamsEager](stdout, stderr);
  this[kBindProperties](ignoreErrors, colorMode, groupIndentation);
}


var consolePropAttributes = {
  writable: true,
  enumerable: false,
  configurable: true
}; // Fixup global.console instanceof global.console.Console

ObjectDefineProperty(Console, SymbolHasInstance, {
  value: function value(instance) {
    return instance[kIsConsole];
  }
});
var kColorInspectOptions = {
  colors: true
};
var kNoColorInspectOptions = {};
ObjectDefineProperties(Console.prototype, (_ObjectDefineProperti2 = {}, _defineProperty(_ObjectDefineProperti2, kBindStreamsEager, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
  // Eager version for the Console constructor
  value: function value(stdout, stderr) {
    ObjectDefineProperties(this, {
      '_stdout': _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
        value: stdout
      }),
      '_stderr': _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
        value: stderr
      })
    });
  }
})), _defineProperty(_ObjectDefineProperti2, kBindStreamsLazy, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
  // Lazily load the stdout and stderr from an object so we don't
  // create the stdio streams when they are not even accessed
  value: function value(object) {
    var stdout;
    var stderr;
    ObjectDefineProperties(this, {
      '_stdout': {
        enumerable: false,
        configurable: true,
        get: function get() {
          if (!stdout) stdout = object.stdout;
          return stdout;
        },
        set: function set(value) {
          stdout = value;
        }
      },
      '_stderr': {
        enumerable: false,
        configurable: true,
        get: function get() {
          if (!stderr) stderr = object.stderr;

          return stderr;
        },
        set: function set(value) {
          stderr = value;
        }
      }
    });
  }
})), _defineProperty(_ObjectDefineProperti2, kBindProperties, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
  value: function value(ignoreErrors, colorMode) {
    var _ObjectDefineProperti;

    var groupIndentation = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : 2;
    ObjectDefineProperties(this, (_ObjectDefineProperti = {
      '_stdoutErrorHandler': _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
        value: createWriteErrorHandler(this, kUseStdout)
      }),
      '_stderrErrorHandler': _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
        value: createWriteErrorHandler(this, kUseStderr)
      }),
      '_ignoreErrors': _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
        value: Boolean(ignoreErrors)
      }),
      '_times': _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
        value: new SafeMap()
      })
    }, _defineProperty(_ObjectDefineProperti, kCounts, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
      value: new SafeMap()
    })), _defineProperty(_ObjectDefineProperti, kColorMode, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
      value: colorMode
    })), _defineProperty(_ObjectDefineProperti, kIsConsole, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
      value: true
    })), _defineProperty(_ObjectDefineProperti, kGroupIndent, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
      value: ''
    })), _defineProperty(_ObjectDefineProperti, kGroupIndentationWidth, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
      value: groupIndentation
    })), _defineProperty(_ObjectDefineProperti, SymbolToStringTag, {
      writable: false,
      enumerable: false,
      configurable: true,
      value: 'console'
    }), _ObjectDefineProperti));
  }
})), _defineProperty(_ObjectDefineProperti2, kWriteToConsole, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
  value: function value(streamSymbol, string) {
    var ignoreErrors = this._ignoreErrors;
    var groupIndent = this[kGroupIndent];
    var useStdout = streamSymbol === kUseStdout;
    var stream = useStdout ? this._stdout : this._stderr;
    var errorHandler = useStdout ? this._stdoutErrorHandler : this._stderrErrorHandler;
    if (groupIndent.length !== 0) {
      if (StringPrototypeIncludes(string, '\n')) {
        string = StringPrototypeReplace(string, /\n/g, "\n".concat(groupIndent));
      }

      string = groupIndent + string;
    }

    string += '\n';
    if (ignoreErrors === false) return stream.write(string); // There may be an error occurring synchronously (e.g. for files or TTYs
    // on POSIX systems) or asynchronously (e.g. pipes on POSIX systems), so
    // handle both situations.

    try {
      // Add and later remove a noop error handler to catch synchronous
      // errors.
      if (stream.listenerCount('error') === 0) stream.once('error', noop);
      stream.write(string, errorHandler);
    } catch (e) {
      // Console is a debugging utility, so it swallowing errors is not
      // desirable even in edge cases such as low stack space.
      if (isStackOverflowError(e)) throw e; // Sorry, there's no proper way to pass along the error here.
    } finally {
      stream.removeListener('error', noop);
    }
  }
})), _defineProperty(_ObjectDefineProperti2, kGetInspectOptions, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
  value: function value(stream) {
    var color = this[kColorMode];

    if (color === 'auto') {
      color = stream.isTTY && (typeof stream.getColorDepth === 'function' ? stream.getColorDepth() > 2 : true);
    }

    var options = optionsMap.get(this);

    if (options) {
      if (options.colors === undefined) {
        options.colors = color;
      }

      return options;
    }

    return color ? kColorInspectOptions : kNoColorInspectOptions;
  }
})), _defineProperty(_ObjectDefineProperti2, kFormatForStdout, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
  value: function value(args) {
    var opts = this[kGetInspectOptions](this._stdout);
    ArrayPrototypeUnshift(args, opts);
    return ReflectApply(formatWithOptions, null, args);
  }
})), _defineProperty(_ObjectDefineProperti2, kFormatForStderr, _objectSpread(_objectSpread({}, consolePropAttributes), {}, {
  value: function value(args) {
    var opts = this[kGetInspectOptions](this._stderr);
    ArrayPrototypeUnshift(args, opts);
    return ReflectApply(formatWithOptions, null, args);
  }
})), _ObjectDefineProperti2)); // Make a function that can serve as the callback passed to `stream.write()`.

function createWriteErrorHandler(instance, streamSymbol) {
  return function (err) {
    // This conditional evaluates to true if and only if there was an error
    // that was not already emitted (which happens when the _write callback
    // is invoked asynchronously).
    var stream = streamSymbol === kUseStdout ? instance._stdout : instance._stderr;

    if (err !== null && !stream._writableState.errorEmitted) {
      // If there was an error, it will be emitted on `stream` as
      // an `error` event. Adding a `once` listener will keep that error
      // from becoming an uncaught exception, but since the handler is
      // removed after the event, non-console.* writes won't be affected.
      // we are only adding noop if there is no one else listening for 'error'
      if (stream.listenerCount('error') === 0) {
        stream.once('error', noop);
      }
    }
  };
}

var consoleMethods = {
  log: function log() {
    for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
      args[_key] = arguments[_key];
    }
    this[kWriteToConsole](kUseStdout, this[kFormatForStdout](args));
  },
  warn: function warn() {
    for (var _len2 = arguments.length, args = new Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
      args[_key2] = arguments[_key2];
    }

    this[kWriteToConsole](kUseStderr, this[kFormatForStderr](args));
  },
  dir: function dir(object, options) {
    this[kWriteToConsole](kUseStdout, inspect(object, _objectSpread(_objectSpread({
      customInspect: false
    }, this[kGetInspectOptions](this._stdout)), options)));
  },
  time: function time() {
    var label = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 'default';
    // Coerces everything other than Symbol to a string
    label = "".concat(label);

    if (this._times.has(label)) {
      process.emitWarning("Label '".concat(label, "' already exists for console.time()"));
      return;
    }

    // trace(kTraceBegin, kTraceConsoleCategory, "time::".concat(label), 0);

    this._times.set(label, process.hrtime());
  },
  timeEnd: function timeEnd() {
    var label = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 'default';
    // Coerces everything other than Symbol to a string
    label = "".concat(label);
    var found = timeLogImpl(this, 'timeEnd', label);
    // trace(kTraceEnd, kTraceConsoleCategory, "time::".concat(label), 0);

    if (found) {
      this._times["delete"](label);
    }
  },
  timeLog: function timeLog() {
    var label = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 'default';
    // Coerces everything other than Symbol to a string
    label = "".concat(label);

    for (var _len3 = arguments.length, data = new Array(_len3 > 1 ? _len3 - 1 : 0), _key3 = 1; _key3 < _len3; _key3++) {
      data[_key3 - 1] = arguments[_key3];
    }

    timeLogImpl(this, 'timeLog', label, data);
    // trace(kTraceInstant, kTraceConsoleCategory, "time::".concat(label), 0);
  },
  trace: function trace() {
    for (var _len4 = arguments.length, args = new Array(_len4), _key4 = 0; _key4 < _len4; _key4++) {
      args[_key4] = arguments[_key4];
    }

    var err = {
      name: 'Trace',
      message: this[kFormatForStderr](args)
    };
    ErrorCaptureStackTrace(err, trace);
    this.error(err.stack);
  },
  assert: function assert(expression) {
    if (!expression) {
      for (var _len5 = arguments.length, args = new Array(_len5 > 1 ? _len5 - 1 : 0), _key5 = 1; _key5 < _len5; _key5++) {
        args[_key5 - 1] = arguments[_key5];
      }

      args[0] = "Assertion failed".concat(args.length === 0 ? '' : ": ".concat(args[0])); // The arguments will be formatted in warn() again

      ReflectApply(this.warn, this, args);
    }
  },
  // Defined by: https://console.spec.whatwg.org/#clear
  clear: function clear() {
    // It only makes sense to clear if _stdout is a TTY.
    // Otherwise, do nothing.
    if (this._stdout.isTTY && process.env.TERM !== 'dumb') {
      // The require is here intentionally to avoid readline being
      // required too early when console is first loaded.
      var _require7 = require('readline'),
          cursorTo = _require7.cursorTo,
          clearScreenDown = _require7.clearScreenDown;

      cursorTo(this._stdout, 0, 0);
      clearScreenDown(this._stdout);
    }
  },
  // Defined by: https://console.spec.whatwg.org/#count
  count: function count() {
    var label = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 'default';
    // Ensures that label is a string, and only things that can be
    // coerced to strings. e.g. Symbol is not allowed
    label = "".concat(label);
    var counts = this[kCounts];
    var count = counts.get(label);
    if (count === undefined) count = 1;else count++;
    counts.set(label, count);
    // trace(kTraceCount, kTraceConsoleCategory, "count::".concat(label), 0, count);
    this.log("".concat(label, ": ").concat(count));
  },
  // Defined by: https://console.spec.whatwg.org/#countreset
  countReset: function countReset() {
    var label = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 'default';
    var counts = this[kCounts];

    if (!counts.has(label)) {
      process.emitWarning("Count for '".concat(label, "' does not exist"));
      return;
    }

    // trace(kTraceCount, kTraceConsoleCategory, "count::".concat(label), 0, 0);
    counts["delete"]("".concat(label));
  },
  group: function group() {
    for (var _len6 = arguments.length, data = new Array(_len6), _key6 = 0; _key6 < _len6; _key6++) {
      data[_key6] = arguments[_key6];
    }

    if (data.length > 0) {
      ReflectApply(this.log, this, data);
    }

    this[kGroupIndent] += StringPrototypeRepeat(' ', this[kGroupIndentationWidth]);
  },
  groupEnd: function groupEnd() {
    this[kGroupIndent] = StringPrototypeSlice(this[kGroupIndent], 0, this[kGroupIndent].length - this[kGroupIndentationWidth]);
  },
  // https://console.spec.whatwg.org/#table
  table: function table(tabularData, properties) {
    var _this2 = this;

    if (properties !== undefined && !ArrayIsArray(properties)) throw new ERR_INVALID_ARG_TYPE('properties', 'Array', properties);
    if (tabularData === null || _typeof(tabularData) !== 'object') return this.log(tabularData);
    if (cliTable === undefined) cliTable = require('internal/cli_table');

    var _final = function _final(k, v) {
      return _this2.log(cliTable(k, v));
    };

    var _inspect = function _inspect(v) {
      var depth = v !== null && _typeof(v) === 'object' && !isArray(v) && ObjectKeys(v).length > 2 ? -1 : 0;

      var opt = _objectSpread({
        depth: depth,
        maxArrayLength: 3,
        breakLength: Infinity
      }, _this2[kGetInspectOptions](_this2._stdout));

      return inspect(v, opt);
    };

    var getIndexArray = function getIndexArray(length) {
      return ArrayFrom({
        length: length
      }, function (_, i) {
        return _inspect(i);
      });
    };

    var mapIter = isMapIterator(tabularData);
    var isKeyValue = false;
    var i = 0;

    if (mapIter) {
      process.emitWarning("This data type is currently not supported for the console table functionality of node-hermes");
      return;
      // var res = previewEntries(tabularData, true);
      // tabularData = res[0];
      // isKeyValue = res[1];
    }

    if (isKeyValue || isMap(tabularData)) {
      process.emitWarning("This data type is currently not supported for the console table functionality of node-hermes");
      return;
      // var _keys = [];
      // var _values = [];
      // var length = 0;

      // if (mapIter) {
      //   for (; i < tabularData.length / 2; ++i) {
      //     ArrayPrototypePush(_keys, _inspect(tabularData[i * 2]));
      //     ArrayPrototypePush(_values, _inspect(tabularData[i * 2 + 1]));
      //     length++;
      //   }
      // } else {
      //   var _iterator = _createForOfIteratorHelper(tabularData),
      //       _step;

      //   try {
      //     for (_iterator.s(); !(_step = _iterator.n()).done;) {
      //       var _step$value = _step.value,
      //           k = _step$value[0],
      //           v = _step$value[1];
      //       ArrayPrototypePush(_keys, _inspect(k));
      //       ArrayPrototypePush(_values, _inspect(v));
      //       length++;
      //     }
      //   } catch (err) {
      //     _iterator.e(err);
      //   } finally {
      //     _iterator.f();
      //   }
      // }
      // return _final([iterKey, keyKey, valuesKey], [getIndexArray(length), _keys, _values]);
    }

    var setIter = isSetIterator(tabularData);
    // if (setIter) tabularData = previewEntries(tabularData);
    var setlike = setIter || mapIter || isSet(tabularData);

    if (setlike) {
      process.emitWarning("This data type is currently not supported for the console table functionality of node-hermes");
      return;
    //   var _values2 = [];
    //   var _length = 0;

    //   var _iterator2 = _createForOfIteratorHelper(tabularData),
    //       _step2;

    //   try {
    //     for (_iterator2.s(); !(_step2 = _iterator2.n()).done;) {
    //       var _v = _step2.value;
    //       ArrayPrototypePush(_values2, _inspect(_v));
    //       _length++;
    //     }
    //   } catch (err) {
    //     _iterator2.e(err);
    //   } finally {
    //     _iterator2.f();
    //   }

    //   return _final([iterKey, valuesKey], [getIndexArray(_length), _values2]);
    }

    var map = {};
    var hasPrimitives = false;
    var valuesKeyArray = [];
    var indexKeyArray = ObjectKeys(tabularData);

    for (; i < indexKeyArray.length; i++) {
      var item = tabularData[indexKeyArray[i]];
      var primitive = item === null || typeof item !== 'function' && _typeof(item) !== 'object';

      if (properties === undefined && primitive) {
        hasPrimitives = true;
        valuesKeyArray[i] = _inspect(item);
      } else {
        var _keys2 = properties || ObjectKeys(item);

        var _iterator3 = _createForOfIteratorHelper(_keys2),
            _step3;

        try {
          for (_iterator3.s(); !(_step3 = _iterator3.n()).done;) {
            var key = _step3.value;
            if (map[key] === undefined) map[key] = [];
            if (primitive && properties || !ObjectPrototypeHasOwnProperty(item, key)) map[key][i] = '';else map[key][i] = _inspect(item[key]);
          }
        } catch (err) {
          _iterator3.e(err);
        } finally {
          _iterator3.f();
        }
      }
    }

    var keys = ObjectKeys(map);
    var values = ObjectValues(map);

    if (hasPrimitives) {
      ArrayPrototypePush(keys, valuesKey);
      ArrayPrototypePush(values, valuesKeyArray);
    }

    ArrayPrototypeUnshift(keys, indexKey);
    ArrayPrototypeUnshift(values, indexKeyArray);
    return _final(keys, values);
  }
}; // Returns true if label was found

function timeLogImpl(self, name, label, data) {
  var time = self._times.get(label);

  if (time === undefined) {
    process.emitWarning("No such label '".concat(label, "' for console.").concat(name, "()"));
    return false;
  }
  var duration = process.hrtime(time);
  var ms = duration[0] * 1000 + duration[1] / 1e6;
  var formatted = formatTime(ms);

  if (data === undefined) {
    self.log('%s: %s', label, formatted);
  } else {
    self.log.apply(self, ['%s: %s', label, formatted].concat(_toConsumableArray(new SafeArrayIterator(data))));
  }

  return true;
}

function pad(value) {
  return StringPrototypePadStart("".concat(value), 2, '0');
}

function formatTime(ms) {
  var hours = 0;
  var minutes = 0;
  var seconds = 0;

  if (ms >= kSecond) {
    if (ms >= kMinute) {
      if (ms >= kHour) {
        hours = MathFloor(ms / kHour);
        ms = ms % kHour;
      }

      minutes = MathFloor(ms / kMinute);
      ms = ms % kMinute;
    }

    seconds = ms / kSecond;
  }

  if (hours !== 0 || minutes !== 0) {
    var _StringPrototypeSplit = StringPrototypeSplit(NumberPrototypeToFixed(seconds, 3), '.');

    seconds = _StringPrototypeSplit[0];
    ms = _StringPrototypeSplit[1];
    var res = hours !== 0 ? "".concat(hours, ":").concat(pad(minutes)) : minutes;
    return "".concat(res, ":").concat(pad(seconds), ".").concat(ms, " (").concat(hours !== 0 ? 'h:m' : '', "m:ss.mmm)");
  }

  if (seconds !== 0) {
    return "".concat(NumberPrototypeToFixed(seconds, 3), "s");
  }

  return "".concat(Number(NumberPrototypeToFixed(ms, 3)), "ms");
}

var keyKey = 'Key';
var valuesKey = 'Values';
var indexKey = '(index)';
var iterKey = '(iteration index)';

var isArray = function isArray(v) {
  return ArrayIsArray(v) || isTypedArray(v) || isBuffer(v);
};

function noop() {}

var _iterator4 = _createForOfIteratorHelper(ReflectOwnKeys(consoleMethods)),
    _step4;

try {
  for (_iterator4.s(); !(_step4 = _iterator4.n()).done;) {
    var method = _step4.value;
    Console.prototype[method] = consoleMethods[method];
  }
} catch (err) {
  _iterator4.e(err);
} finally {
  _iterator4.f();
}

Console.prototype.debug = Console.prototype.log;
Console.prototype.info = Console.prototype.log;
Console.prototype.dirxml = Console.prototype.log;
Console.prototype.error = Console.prototype.warn;
Console.prototype.groupCollapsed = Console.prototype.group;

module.exports = {
  Console: Console,
  kBindStreamsLazy: kBindStreamsLazy,
  kBindProperties: kBindProperties,
  formatTime: formatTime // exported for tests

};
