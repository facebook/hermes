// @nolint
'use strict';

function _toConsumableArray(arr) { return _arrayWithoutHoles(arr) || _iterableToArray(arr) || _unsupportedIterableToArray(arr) || _nonIterableSpread(); }

function _nonIterableSpread() { throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }

function _iterableToArray(iter) { if (typeof Symbol !== "undefined" && iter[Symbol.iterator] != null || iter["@@iterator"] != null) return Array.from(iter); }

function _arrayWithoutHoles(arr) { if (Array.isArray(arr)) return _arrayLikeToArray(arr); }

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

var _primordials = primordials,
    _Array = _primordials.Array,
    ArrayIsArray = _primordials.ArrayIsArray,
    ArrayPrototypeFilter = _primordials.ArrayPrototypeFilter,
    ArrayPrototypeForEach = _primordials.ArrayPrototypeForEach,
    ArrayPrototypePop = _primordials.ArrayPrototypePop,
    ArrayPrototypePush = _primordials.ArrayPrototypePush,
    ArrayPrototypePushApply = _primordials.ArrayPrototypePushApply,
    ArrayPrototypeSort = _primordials.ArrayPrototypeSort,
    ArrayPrototypeUnshift = _primordials.ArrayPrototypeUnshift,
    BigIntPrototypeValueOf = _primordials.BigIntPrototypeValueOf,
    BooleanPrototypeValueOf = _primordials.BooleanPrototypeValueOf,
    DatePrototypeGetTime = _primordials.DatePrototypeGetTime,
    DatePrototypeToISOString = _primordials.DatePrototypeToISOString,
    DatePrototypeToString = _primordials.DatePrototypeToString,
    ErrorPrototypeToString = _primordials.ErrorPrototypeToString,
    FunctionPrototypeCall = _primordials.FunctionPrototypeCall,
    FunctionPrototypeToString = _primordials.FunctionPrototypeToString,
    JSONStringify = _primordials.JSONStringify,
    MapPrototypeGetSize = _primordials.MapPrototypeGetSize,
    MapPrototypeEntries = _primordials.MapPrototypeEntries,
    MathFloor = _primordials.MathFloor,
    MathMax = _primordials.MathMax,
    MathMin = _primordials.MathMin,
    MathRound = _primordials.MathRound,
    MathSqrt = _primordials.MathSqrt,
    Number = _primordials.Number,
    NumberIsNaN = _primordials.NumberIsNaN,
    NumberParseFloat = _primordials.NumberParseFloat,
    NumberParseInt = _primordials.NumberParseInt,
    NumberPrototypeValueOf = _primordials.NumberPrototypeValueOf,
    _Object = _primordials.Object,
    ObjectAssign = _primordials.ObjectAssign,
    ObjectCreate = _primordials.ObjectCreate,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    ObjectGetOwnPropertyDescriptor = _primordials.ObjectGetOwnPropertyDescriptor,
    ObjectGetOwnPropertyNames = _primordials.ObjectGetOwnPropertyNames,
    ObjectGetOwnPropertySymbols = _primordials.ObjectGetOwnPropertySymbols,
    ObjectGetPrototypeOf = _primordials.ObjectGetPrototypeOf,
    ObjectIs = _primordials.ObjectIs,
    ObjectKeys = _primordials.ObjectKeys,
    ObjectPrototypeHasOwnProperty = _primordials.ObjectPrototypeHasOwnProperty,
    ObjectPrototypePropertyIsEnumerable = _primordials.ObjectPrototypePropertyIsEnumerable,
    ObjectSeal = _primordials.ObjectSeal,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    ReflectOwnKeys = _primordials.ReflectOwnKeys,
    RegExp = _primordials.RegExp,
    RegExpPrototypeTest = _primordials.RegExpPrototypeTest,
    RegExpPrototypeToString = _primordials.RegExpPrototypeToString,
    SafeStringIterator = _primordials.SafeStringIterator,
    SafeMap = _primordials.SafeMap,
    SafeSet = _primordials.SafeSet,
    SetPrototypeGetSize = _primordials.SetPrototypeGetSize,
    SetPrototypeValues = _primordials.SetPrototypeValues,
    String = _primordials.String,
    StringPrototypeCharCodeAt = _primordials.StringPrototypeCharCodeAt,
    StringPrototypeCodePointAt = _primordials.StringPrototypeCodePointAt,
    StringPrototypeIncludes = _primordials.StringPrototypeIncludes,
    StringPrototypeNormalize = _primordials.StringPrototypeNormalize,
    StringPrototypePadEnd = _primordials.StringPrototypePadEnd,
    StringPrototypePadStart = _primordials.StringPrototypePadStart,
    StringPrototypeRepeat = _primordials.StringPrototypeRepeat,
    StringPrototypeReplace = _primordials.StringPrototypeReplace,
    StringPrototypeSlice = _primordials.StringPrototypeSlice,
    StringPrototypeSplit = _primordials.StringPrototypeSplit,
    StringPrototypeToLowerCase = _primordials.StringPrototypeToLowerCase,
    StringPrototypeTrim = _primordials.StringPrototypeTrim,
    StringPrototypeValueOf = _primordials.StringPrototypeValueOf,
    SymbolPrototypeToString = _primordials.SymbolPrototypeToString,
    SymbolPrototypeValueOf = _primordials.SymbolPrototypeValueOf,
    SymbolIterator = _primordials.SymbolIterator,
    SymbolToStringTag = _primordials.SymbolToStringTag,
    TypedArrayPrototypeGetLength = _primordials.TypedArrayPrototypeGetLength,
    TypedArrayPrototypeGetSymbolToStringTag = _primordials.TypedArrayPrototypeGetSymbolToStringTag,
    Uint8Array = _primordials.Uint8Array,
    globalThis = _primordials.globalThis,
    uncurryThis = _primordials.uncurryThis;

// var _internalBinding = internalBinding('util'),
//     getOwnNonIndexProperties = _internalBinding.getOwnNonIndexProperties,
//     getPromiseDetails = _internalBinding.getPromiseDetails,
//     getProxyDetails = _internalBinding.getProxyDetails,
//     kPending = _internalBinding.kPending,
//     kRejected = _internalBinding.kRejected,
//     previewEntries = _internalBinding.previewEntries,
//     internalGetConstructorName = _internalBinding.getConstructorName,
//     getExternalValue = _internalBinding.getExternalValue,
//     _internalBinding$prop = _internalBinding.propertyFilter,
//     ALL_PROPERTIES = _internalBinding$prop.ALL_PROPERTIES,
//     ONLY_ENUMERABLE = _internalBinding$prop.ONLY_ENUMERABLE;

var _require = require('internal/util'),
    customInspectSymbol = _require.customInspectSymbol,
    isError = _require.isError,
    join = _require.join,
    removeColors = _require.removeColors;

var _require2 = require('internal/errors'),
    ERR_INVALID_ARG_TYPE = _require2.codes.ERR_INVALID_ARG_TYPE,
    isStackOverflowError = _require2.isStackOverflowError;

var _require3 = require('internal/util/types'),
    isAsyncFunction = _require3.isAsyncFunction,
    isGeneratorFunction = _require3.isGeneratorFunction,
    isAnyArrayBuffer = _require3.isAnyArrayBuffer,
    isArrayBuffer = _require3.isArrayBuffer,
    isArgumentsObject = _require3.isArgumentsObject,
    isBoxedPrimitive = _require3.isBoxedPrimitive,
    isDataView = _require3.isDataView,
    isExternal = _require3.isExternal,
    isMap = _require3.isMap,
    isMapIterator = _require3.isMapIterator,
    isModuleNamespaceObject = _require3.isModuleNamespaceObject,
    isNativeError = _require3.isNativeError,
    isPromise = _require3.isPromise,
    isSet = _require3.isSet,
    isSetIterator = _require3.isSetIterator,
    isWeakMap = _require3.isWeakMap,
    isWeakSet = _require3.isWeakSet,
    isRegExp = _require3.isRegExp,
    isDate = _require3.isDate,
    isTypedArray = _require3.isTypedArray,
    isStringObject = _require3.isStringObject,
    isNumberObject = _require3.isNumberObject,
    isBooleanObject = _require3.isBooleanObject,
    isBigIntObject = _require3.isBigIntObject;

var assert = require('internal/assert');

// var _require4 = require('internal/bootstrap/loaders'),
//     NativeModule = _require4.NativeModule;

var _require5 = require('internal/validators'),
    validateObject = _require5.validateObject;

var hexSlice;
var builtInObjects = new SafeSet(ArrayPrototypeFilter(ObjectGetOwnPropertyNames(globalThis), function (e) {
  return RegExpPrototypeTest(/^[A-Z][a-zA-Z0-9]+$/, e);
})); // https://tc39.es/ecma262/#sec-IsHTMLDDA-internal-slot

var isUndetectableObject = function isUndetectableObject(v) {
  return typeof v === 'undefined' && v !== undefined;
}; // These options must stay in sync with `getUserOptions`. So if any option will
// be added or removed, `getUserOptions` must also be updated accordingly.


var inspectDefaultOptions = ObjectSeal({
  showHidden: false,
  depth: 2,
  colors: false,
  customInspect: true,
  showProxy: false,
  maxArrayLength: 100,
  maxStringLength: 10000,
  breakLength: 80,
  compact: 3,
  sorted: false,
  getters: false
});
var kObjectType = 0;
var kArrayType = 1;
var kArrayExtrasType = 2;
/* eslint-disable no-control-regex */

var strEscapeSequencesRegExp = /[\x00-\x1f\x27\x5c\x7f-\x9f]/;
var strEscapeSequencesReplacer = /[\x00-\x1f\x27\x5c\x7f-\x9f]/g;
var strEscapeSequencesRegExpSingle = /[\x00-\x1f\x5c\x7f-\x9f]/;
var strEscapeSequencesReplacerSingle = /[\x00-\x1f\x5c\x7f-\x9f]/g;
/* eslint-enable no-control-regex */

var keyStrRegExp = /^[a-zA-Z_][a-zA-Z_0-9]*$/;
var numberRegExp = /^(0|[1-9][0-9]*)$/;
var coreModuleRegExp = /^    at (?:[^/\\(]+ \(|)node:(.+):\d+:\d+\)?$/;
var nodeModulesRegExp = /[/\\]node_modules[/\\](.+?)(?=[/\\])/g;
var classRegExp = /^(\s+[^(]*?)\s*{/; // eslint-disable-next-line node-core/no-unescaped-regexp-dot

var stripCommentsRegExp = /(\/\/.*?\n)|(\/\*(.|\n)*?\*\/)/g;
var kMinLineLength = 16; // Constants to map the iterator state.

var kWeak = 0;
var kIterator = 1;
var kMapEntries = 2; // Escaped control characters (plus the single quote and the backslash). Use
// empty strings to fill up unused entries.

var meta = ['\\x00', '\\x01', '\\x02', '\\x03', '\\x04', '\\x05', '\\x06', '\\x07', // x07
'\\b', '\\t', '\\n', '\\x0B', '\\f', '\\r', '\\x0E', '\\x0F', // x0F
'\\x10', '\\x11', '\\x12', '\\x13', '\\x14', '\\x15', '\\x16', '\\x17', // x17
'\\x18', '\\x19', '\\x1A', '\\x1B', '\\x1C', '\\x1D', '\\x1E', '\\x1F', // x1F
'', '', '', '', '', '', '', "\\'", '', '', '', '', '', '', '', '', // x2F
'', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', // x3F
'', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', // x4F
'', '', '', '', '', '', '', '', '', '', '', '', '\\\\', '', '', '', // x5F
'', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', // x6F
'', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '\\x7F', // x7F
'\\x80', '\\x81', '\\x82', '\\x83', '\\x84', '\\x85', '\\x86', '\\x87', // x87
'\\x88', '\\x89', '\\x8A', '\\x8B', '\\x8C', '\\x8D', '\\x8E', '\\x8F', // x8F
'\\x90', '\\x91', '\\x92', '\\x93', '\\x94', '\\x95', '\\x96', '\\x97', // x97
'\\x98', '\\x99', '\\x9A', '\\x9B', '\\x9C', '\\x9D', '\\x9E', '\\x9F' // x9F
]; // Regex used for ansi escape code splitting
// Adopted from https://github.com/chalk/ansi-regex/blob/HEAD/index.js
// License: MIT, authors: @sindresorhus, Qix-, arjunmehta and LitoMore
// Matches all ansi escape code sequences in a string

var ansiPattern = "[\\u001B\\u009B][[\\]()#;?]*" + "(?:(?:(?:[a-zA-Z\\d]*(?:;[-a-zA-Z\\d\\/#&.:=?%@~_]*)*)?\\u0007)" + '|(?:(?:\\d{1,4}(?:;\\d{0,4})*)?[\\dA-PR-TZcf-ntqry=><~]))';
var ansi = new RegExp(ansiPattern, 'g');
var getStringWidth;

function getUserOptions(ctx, isCrossContext) {
  var ret = _objectSpread({
    stylize: ctx.stylize,
    showHidden: ctx.showHidden,
    depth: ctx.depth,
    colors: ctx.colors,
    customInspect: ctx.customInspect,
    showProxy: ctx.showProxy,
    maxArrayLength: ctx.maxArrayLength,
    maxStringLength: ctx.maxStringLength,
    breakLength: ctx.breakLength,
    compact: ctx.compact,
    sorted: ctx.sorted,
    getters: ctx.getters
  }, ctx.userOptions); // Typically, the target value will be an instance of `Object`. If that is
  // *not* the case, the object may come from another vm.Context, and we want
  // to avoid passing it objects from this Context in that case, so we remove
  // the prototype from the returned object itself + the `stylize()` function,
  // and remove all other non-primitives, including non-primitive user options.


  if (isCrossContext) {
    ObjectSetPrototypeOf(ret, null);

    var _iterator = _createForOfIteratorHelper(ObjectKeys(ret)),
        _step;

    try {
      for (_iterator.s(); !(_step = _iterator.n()).done;) {
        var key = _step.value;

        if ((_typeof(ret[key]) === 'object' || typeof ret[key] === 'function') && ret[key] !== null) {
          delete ret[key];
        }
      }
    } catch (err) {
      _iterator.e(err);
    } finally {
      _iterator.f();
    }

    ret.stylize = ObjectSetPrototypeOf(function (value, flavour) {
      var stylized;

      try {
        stylized = "".concat(ctx.stylize(value, flavour));
      } catch (_unused) {}

      if (typeof stylized !== 'string') return value; // `stylized` is a string as it should be, which is safe to pass along.

      return stylized;
    }, null);
  }

  return ret;
}
/**
 * Echos the value of any input. Tries to print the value out
 * in the best way possible given the different types.
 *
 * @param {any} value The value to print out.
 * @param {Object} opts Optional options object that alters the output.
 */

/* Legacy: value, showHidden, depth, colors */


function inspect(value, opts) {
  // Default options
  var ctx = {
    budget: {},
    indentationLvl: 0,
    seen: [],
    currentDepth: 0,
    stylize: stylizeNoColor,
    showHidden: inspectDefaultOptions.showHidden,
    depth: inspectDefaultOptions.depth,
    colors: inspectDefaultOptions.colors,
    customInspect: inspectDefaultOptions.customInspect,
    showProxy: inspectDefaultOptions.showProxy,
    maxArrayLength: inspectDefaultOptions.maxArrayLength,
    maxStringLength: inspectDefaultOptions.maxStringLength,
    breakLength: inspectDefaultOptions.breakLength,
    compact: inspectDefaultOptions.compact,
    sorted: inspectDefaultOptions.sorted,
    getters: inspectDefaultOptions.getters
  };

  if (arguments.length > 1) {
    // Legacy...
    if (arguments.length > 2) {
      if (arguments[2] !== undefined) {
        ctx.depth = arguments[2];
      }

      if (arguments.length > 3 && arguments[3] !== undefined) {
        ctx.colors = arguments[3];
      }
    } // Set user-specified options


    if (typeof opts === 'boolean') {
      ctx.showHidden = opts;
    } else if (opts) {
      var optKeys = ObjectKeys(opts);

      for (var i = 0; i < optKeys.length; ++i) {
        var key = optKeys[i]; // TODO(BridgeAR): Find a solution what to do about stylize. Either make
        // this function public or add a new API with a similar or better
        // functionality.

        if (ObjectPrototypeHasOwnProperty(inspectDefaultOptions, key) || key === 'stylize') {
          ctx[key] = opts[key];
        } else if (ctx.userOptions === undefined) {
          // This is required to pass through the actual user input.
          ctx.userOptions = opts;
        }
      }
    }
  }

  if (ctx.colors) ctx.stylize = stylizeWithColor;
  if (ctx.maxArrayLength === null) ctx.maxArrayLength = Infinity;
  if (ctx.maxStringLength === null) ctx.maxStringLength = Infinity;
  return formatValue(ctx, value, 0);
}

inspect.custom = customInspectSymbol;
ObjectDefineProperty(inspect, 'defaultOptions', {
  get: function get() {
    return inspectDefaultOptions;
  },
  set: function set(options) {
    validateObject(options, 'options');
    return ObjectAssign(inspectDefaultOptions, options);
  }
}); // Set Graphics Rendition https://en.wikipedia.org/wiki/ANSI_escape_code#graphics
// Each color consists of an array with the color code as first entry and the
// reset code as second entry.

var defaultFG = 39;
var defaultBG = 49;
inspect.colors = ObjectAssign(ObjectCreate(null), {
  reset: [0, 0],
  bold: [1, 22],
  dim: [2, 22],
  // Alias: faint
  italic: [3, 23],
  underline: [4, 24],
  blink: [5, 25],
  // Swap foreground and background colors
  inverse: [7, 27],
  // Alias: swapcolors, swapColors
  hidden: [8, 28],
  // Alias: conceal
  strikethrough: [9, 29],
  // Alias: strikeThrough, crossedout, crossedOut
  doubleunderline: [21, 24],
  // Alias: doubleUnderline
  black: [30, defaultFG],
  red: [31, defaultFG],
  green: [32, defaultFG],
  yellow: [33, defaultFG],
  blue: [34, defaultFG],
  magenta: [35, defaultFG],
  cyan: [36, defaultFG],
  white: [37, defaultFG],
  bgBlack: [40, defaultBG],
  bgRed: [41, defaultBG],
  bgGreen: [42, defaultBG],
  bgYellow: [43, defaultBG],
  bgBlue: [44, defaultBG],
  bgMagenta: [45, defaultBG],
  bgCyan: [46, defaultBG],
  bgWhite: [47, defaultBG],
  framed: [51, 54],
  overlined: [53, 55],
  gray: [90, defaultFG],
  // Alias: grey, blackBright
  redBright: [91, defaultFG],
  greenBright: [92, defaultFG],
  yellowBright: [93, defaultFG],
  blueBright: [94, defaultFG],
  magentaBright: [95, defaultFG],
  cyanBright: [96, defaultFG],
  whiteBright: [97, defaultFG],
  bgGray: [100, defaultBG],
  // Alias: bgGrey, bgBlackBright
  bgRedBright: [101, defaultBG],
  bgGreenBright: [102, defaultBG],
  bgYellowBright: [103, defaultBG],
  bgBlueBright: [104, defaultBG],
  bgMagentaBright: [105, defaultBG],
  bgCyanBright: [106, defaultBG],
  bgWhiteBright: [107, defaultBG]
});

function defineColorAlias(target, alias) {
  ObjectDefineProperty(inspect.colors, alias, {
    get: function get() {
      return this[target];
    },
    set: function set(value) {
      this[target] = value;
    },
    configurable: true,
    enumerable: false
  });
}

defineColorAlias('gray', 'grey');
defineColorAlias('gray', 'blackBright');
defineColorAlias('bgGray', 'bgGrey');
defineColorAlias('bgGray', 'bgBlackBright');
defineColorAlias('dim', 'faint');
defineColorAlias('strikethrough', 'crossedout');
defineColorAlias('strikethrough', 'strikeThrough');
defineColorAlias('strikethrough', 'crossedOut');
defineColorAlias('hidden', 'conceal');
defineColorAlias('inverse', 'swapColors');
defineColorAlias('inverse', 'swapcolors');
defineColorAlias('doubleunderline', 'doubleUnderline'); // TODO(BridgeAR): Add function style support for more complex styles.
// Don't use 'blue' not visible on cmd.exe

inspect.styles = ObjectAssign(ObjectCreate(null), {
  special: 'cyan',
  number: 'yellow',
  bigint: 'yellow',
  "boolean": 'yellow',
  undefined: 'grey',
  "null": 'bold',
  string: 'green',
  symbol: 'green',
  date: 'magenta',
  // "name": intentionally not styling
  // TODO(BridgeAR): Highlight regular expressions properly.
  regexp: 'red',
  module: 'underline'
});

function addQuotes(str, quotes) {
  if (quotes === -1) {
    return "\"".concat(str, "\"");
  }

  if (quotes === -2) {
    return "`".concat(str, "`");
  }

  return "'".concat(str, "'");
}

var escapeFn = function escapeFn(str) {
  return meta[StringPrototypeCharCodeAt(str)];
}; // Escape control characters, single quotes and the backslash.
// This is similar to JSON stringify escaping.


function strEscape(str) {
  var escapeTest = strEscapeSequencesRegExp;
  var escapeReplace = strEscapeSequencesReplacer;
  var singleQuote = 39; // Check for double quotes. If not present, do not escape single quotes and
  // instead wrap the text in double quotes. If double quotes exist, check for
  // backticks. If they do not exist, use those as fallback instead of the
  // double quotes.

  if (StringPrototypeIncludes(str, "'")) {
    // This invalidates the charCode and therefore can not be matched for
    // anymore.
    if (!StringPrototypeIncludes(str, '"')) {
      singleQuote = -1;
    } else if (!StringPrototypeIncludes(str, '`') && !StringPrototypeIncludes(str, '${')) {
      singleQuote = -2;
    }

    if (singleQuote !== 39) {
      escapeTest = strEscapeSequencesRegExpSingle;
      escapeReplace = strEscapeSequencesReplacerSingle;
    }
  } // Some magic numbers that worked out fine while benchmarking with v8 6.0


  if (str.length < 5000 && !RegExpPrototypeTest(escapeTest, str)) return addQuotes(str, singleQuote);

  if (str.length > 100) {
    str = StringPrototypeReplace(str, escapeReplace, escapeFn);
    return addQuotes(str, singleQuote);
  }

  var result = '';
  var last = 0;
  var lastIndex = str.length;

  for (var i = 0; i < lastIndex; i++) {
    var point = StringPrototypeCharCodeAt(str, i);

    if (point === singleQuote || point === 92 || point < 32 || point > 126 && point < 160) {
      if (last === i) {
        result += meta[point];
      } else {
        result += "".concat(StringPrototypeSlice(str, last, i)).concat(meta[point]);
      }

      last = i + 1;
    }
  }

  if (last !== lastIndex) {
    result += StringPrototypeSlice(str, last);
  }

  return addQuotes(result, singleQuote);
}

function stylizeWithColor(str, styleType) {
  var style = inspect.styles[styleType];

  if (style !== undefined) {
    var color = inspect.colors[style];
    if (color !== undefined) return "\x1B[".concat(color[0], "m").concat(str, "\x1B[").concat(color[1], "m");
  }

  return str;
}

function stylizeNoColor(str) {
  return str;
} // Return a new empty array to push in the results of the default formatter.


function getEmptyFormatArray() {
  return [];
}

function isInstanceof(object, proto) {
  try {
    return object instanceof proto;
  } catch (_unused2) {
    return false;
  }
}

function getConstructorName(obj, ctx, recurseTimes, protoProps) {
  var firstProto;
  var tmp = obj;

  while (obj || isUndetectableObject(obj)) {
    var descriptor = ObjectGetOwnPropertyDescriptor(obj, 'constructor');

    if (descriptor !== undefined && typeof descriptor.value === 'function' && descriptor.value.name !== '' && isInstanceof(tmp, descriptor.value)) {
      if (protoProps !== undefined && (firstProto !== obj || !builtInObjects.has(descriptor.value.name))) {
        addPrototypeProperties(ctx, tmp, firstProto || tmp, recurseTimes, protoProps);
      }

      return descriptor.value.name;
    }

    obj = ObjectGetPrototypeOf(obj);

    if (firstProto === undefined) {
      firstProto = obj;
    }
  }

  if (firstProto === null) {
    return null;
  }

  var res = internalGetConstructorName(tmp);

  if (recurseTimes > ctx.depth && ctx.depth !== null) {
    return "".concat(res, " <Complex prototype>");
  }

  var protoConstr = getConstructorName(firstProto, ctx, recurseTimes + 1, protoProps);

  if (protoConstr === null) {
    return "".concat(res, " <").concat(inspect(firstProto, _objectSpread(_objectSpread({}, ctx), {}, {
      customInspect: false,
      depth: -1
    })), ">");
  }

  return "".concat(res, " <").concat(protoConstr, ">");
} // This function has the side effect of adding prototype properties to the
// `output` argument (which is an array). This is intended to highlight user
// defined prototype properties.


function addPrototypeProperties(ctx, main, obj, recurseTimes, output) {
  var depth = 0;
  var keys;
  var keySet;

  do {
    if (depth !== 0 || main === obj) {
      obj = ObjectGetPrototypeOf(obj); // Stop as soon as a null prototype is encountered.

      if (obj === null) {
        return;
      } // Stop as soon as a built-in object type is detected.


      var descriptor = ObjectGetOwnPropertyDescriptor(obj, 'constructor');

      if (descriptor !== undefined && typeof descriptor.value === 'function' && builtInObjects.has(descriptor.value.name)) {
        return;
      }
    }

    if (depth === 0) {
      keySet = new SafeSet();
    } else {
      ArrayPrototypeForEach(keys, function (key) {
        return keySet.add(key);
      });
    } // Get all own property names and symbols.


    keys = ReflectOwnKeys(obj);
    ArrayPrototypePush(ctx.seen, main);

    var _iterator2 = _createForOfIteratorHelper(keys),
        _step2;

    try {
      for (_iterator2.s(); !(_step2 = _iterator2.n()).done;) {
        var key = _step2.value;

        // Ignore the `constructor` property and keys that exist on layers above.
        if (key === 'constructor' || ObjectPrototypeHasOwnProperty(main, key) || depth !== 0 && keySet.has(key)) {
          continue;
        }

        var desc = ObjectGetOwnPropertyDescriptor(obj, key);

        if (typeof desc.value === 'function') {
          continue;
        }

        var value = formatProperty(ctx, obj, recurseTimes, key, kObjectType, desc, main);

        if (ctx.colors) {
          // Faint!
          ArrayPrototypePush(output, "\x1B[2m".concat(value, "\x1B[22m"));
        } else {
          ArrayPrototypePush(output, value);
        }
      }
    } catch (err) {
      _iterator2.e(err);
    } finally {
      _iterator2.f();
    }

    ArrayPrototypePop(ctx.seen); // Limit the inspection to up to three prototype layers. Using `recurseTimes`
    // is not a good choice here, because it's as if the properties are declared
    // on the current object from the users perspective.
  } while (++depth !== 3);
}

function getPrefix(constructor, tag, fallback) {
  var size = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : '';

  if (constructor === null) {
    if (tag !== '' && fallback !== tag) {
      return "[".concat(fallback).concat(size, ": null prototype] [").concat(tag, "] ");
    }

    return "[".concat(fallback).concat(size, ": null prototype] ");
  }

  if (tag !== '' && constructor !== tag) {
    return "".concat(constructor).concat(size, " [").concat(tag, "] ");
  }

  return "".concat(constructor).concat(size, " ");
} // Look up the keys of the object.


function getKeys(value, showHidden) {
  var keys;
  var symbols = ObjectGetOwnPropertySymbols(value);

  if (showHidden) {
    keys = ObjectGetOwnPropertyNames(value);
    if (symbols.length !== 0) ArrayPrototypePushApply(keys, symbols);
  } else {
    // This might throw if `value` is a Module Namespace Object from an
    // unevaluated module, but we don't want to perform the actual type
    // check because it's expensive.
    // TODO(devsnek): track https://github.com/tc39/ecma262/issues/1209
    // and modify this logic as needed.
    try {
      keys = ObjectKeys(value);
    } catch (err) {
      assert(isNativeError(err) && err.name === 'ReferenceError' && isModuleNamespaceObject(value));
      keys = ObjectGetOwnPropertyNames(value);
    }

    if (symbols.length !== 0) {
      var filter = function filter(key) {
        return ObjectPrototypePropertyIsEnumerable(value, key);
      };

      ArrayPrototypePushApply(keys, ArrayPrototypeFilter(symbols, filter));
    }
  }

  return keys;
}

function getCtxStyle(value, constructor, tag) {
  var fallback = '';

  if (constructor === null) {
    fallback = internalGetConstructorName(value);

    if (fallback === tag) {
      fallback = 'Object';
    }
  }

  return getPrefix(constructor, tag, fallback);
}

function formatProxy(ctx, proxy, recurseTimes) {
  if (recurseTimes > ctx.depth && ctx.depth !== null) {
    return ctx.stylize('Proxy [Array]', 'special');
  }

  recurseTimes += 1;
  ctx.indentationLvl += 2;
  var res = [formatValue(ctx, proxy[0], recurseTimes), formatValue(ctx, proxy[1], recurseTimes)];
  ctx.indentationLvl -= 2;
  return reduceToSingleString(ctx, res, '', ['Proxy [', ']'], kArrayExtrasType, recurseTimes);
} // Note: using `formatValue` directly requires the indentation level to be
// corrected by setting `ctx.indentationLvL += diff` and then to decrease the
// value afterwards again.


function formatValue(ctx, value, recurseTimes, typedArray) {
  // Primitive types cannot have properties.
  if (_typeof(value) !== 'object' && typeof value !== 'function' && !isUndetectableObject(value)) {
    return formatPrimitive(ctx.stylize, value, ctx);
  }

  if (value === null) {
    return ctx.stylize('null', 'null');
  } // Memorize the context for custom inspection on proxies.


  var context = value; // Always check for proxies to prevent side effects and to prevent triggering
  // any proxy handlers.

  var proxy = getProxyDetails(value, !!ctx.showProxy);

  if (proxy !== undefined) {
    if (ctx.showProxy) {
      return formatProxy(ctx, proxy, recurseTimes);
    }

    value = proxy;
  } // Provide a hook for user-specified inspect functions.
  // Check that value is an object with an inspect function on it.


  if (ctx.customInspect) {
    var maybeCustom = value[customInspectSymbol];

    if (typeof maybeCustom === 'function' && // Filter out the util module, its inspect function is special.
    maybeCustom !== inspect && // Also filter out any prototype objects using the circular check.
    !(value.constructor && value.constructor.prototype === value)) {
      // This makes sure the recurseTimes are reported as before while using
      // a counter internally.
      var depth = ctx.depth === null ? null : ctx.depth - recurseTimes;
      var isCrossContext = proxy !== undefined || !(context instanceof _Object);
      var ret = FunctionPrototypeCall(maybeCustom, context, depth, getUserOptions(ctx, isCrossContext)); // If the custom inspection method returned `this`, don't go into
      // infinite recursion.

      if (ret !== context) {
        if (typeof ret !== 'string') {
          return formatValue(ctx, ret, recurseTimes);
        }

        return ret.replace(/\n/g, "\n".concat(' '.repeat(ctx.indentationLvl)));
      }
    }
  } // Using an array here is actually better for the average case than using
  // a Set. `seen` will only check for the depth and will never grow too large.


  if (ctx.seen.includes(value)) {
    var index = 1;

    if (ctx.circular === undefined) {
      ctx.circular = new SafeMap();
      ctx.circular.set(value, index);
    } else {
      index = ctx.circular.get(value);

      if (index === undefined) {
        index = ctx.circular.size + 1;
        ctx.circular.set(value, index);
      }
    }

    return ctx.stylize("[Circular *".concat(index, "]"), 'special');
  }

  return formatRaw(ctx, value, recurseTimes, typedArray);
}

function formatRaw(ctx, value, recurseTimes, typedArray) {
  var keys;
  var protoProps;

  if (ctx.showHidden && (recurseTimes <= ctx.depth || ctx.depth === null)) {
    protoProps = [];
  }

  var constructor = getConstructorName(value, ctx, recurseTimes, protoProps); // Reset the variable to check for this later on.

  if (protoProps !== undefined && protoProps.length === 0) {
    protoProps = undefined;
  }

  var tag = value[SymbolToStringTag]; // Only list the tag in case it's non-enumerable / not an own property.
  // Otherwise we'd print this twice.

  if (typeof tag !== 'string' || tag !== '' && (ctx.showHidden ? ObjectPrototypeHasOwnProperty : ObjectPrototypePropertyIsEnumerable)(value, SymbolToStringTag)) {
    tag = '';
  }

  var base = '';
  var formatter = getEmptyFormatArray;
  var braces;
  var noIterator = true;
  var i = 0;
  var filter = ctx.showHidden ? ALL_PROPERTIES : ONLY_ENUMERABLE;
  var extrasType = kObjectType; // Iterators and the rest are split to reduce checks.
  // We have to check all values in case the constructor is set to null.
  // Otherwise it would not possible to identify all types properly.

  if (value[SymbolIterator] || constructor === null) {
    noIterator = false;

    if (ArrayIsArray(value)) {
      // Only set the constructor for non ordinary ("Array [...]") arrays.
      var prefix = constructor !== 'Array' || tag !== '' ? getPrefix(constructor, tag, 'Array', "(".concat(value.length, ")")) : '';
      keys = getOwnNonIndexProperties(value, filter);
      braces = ["".concat(prefix, "["), ']'];
      if (value.length === 0 && keys.length === 0 && protoProps === undefined) return "".concat(braces[0], "]");
      extrasType = kArrayExtrasType;
      formatter = formatArray;
    } else if (isSet(value)) {
      var size = SetPrototypeGetSize(value);

      var _prefix = getPrefix(constructor, tag, 'Set', "(".concat(size, ")"));

      keys = getKeys(value, ctx.showHidden);
      formatter = constructor !== null ? formatSet.bind(null, value) : formatSet.bind(null, SetPrototypeValues(value));
      if (size === 0 && keys.length === 0 && protoProps === undefined) return "".concat(_prefix, "{}");
      braces = ["".concat(_prefix, "{"), '}'];
    } else if (isMap(value)) {
      var _size = MapPrototypeGetSize(value);

      var _prefix2 = getPrefix(constructor, tag, 'Map', "(".concat(_size, ")"));

      keys = getKeys(value, ctx.showHidden);
      formatter = constructor !== null ? formatMap.bind(null, value) : formatMap.bind(null, MapPrototypeEntries(value));
      if (_size === 0 && keys.length === 0 && protoProps === undefined) return "".concat(_prefix2, "{}");
      braces = ["".concat(_prefix2, "{"), '}'];
    } else if (isTypedArray(value)) {
      keys = getOwnNonIndexProperties(value, filter);
      var bound = value;
      var fallback = '';

      if (constructor === null) {
        fallback = TypedArrayPrototypeGetSymbolToStringTag(value); // Reconstruct the array information.

        bound = new primordials[fallback](value);
      }

      var _size2 = TypedArrayPrototypeGetLength(value);

      var _prefix3 = getPrefix(constructor, tag, fallback, "(".concat(_size2, ")"));

      braces = ["".concat(_prefix3, "["), ']'];
      if (value.length === 0 && keys.length === 0 && !ctx.showHidden) return "".concat(braces[0], "]"); // Special handle the value. The original value is required below. The
      // bound function is required to reconstruct missing information.

      formatter = formatTypedArray.bind(null, bound, _size2);
      extrasType = kArrayExtrasType;
    } else if (isMapIterator(value)) {
      keys = getKeys(value, ctx.showHidden);
      braces = getIteratorBraces('Map', tag); // Add braces to the formatter parameters.

      formatter = formatIterator.bind(null, braces);
    } else if (isSetIterator(value)) {
      keys = getKeys(value, ctx.showHidden);
      braces = getIteratorBraces('Set', tag); // Add braces to the formatter parameters.

      formatter = formatIterator.bind(null, braces);
    } else {
      noIterator = true;
    }
  }

  if (noIterator) {
    keys = getKeys(value, ctx.showHidden);
    braces = ['{', '}'];

    if (constructor === 'Object') {
      if (isArgumentsObject(value)) {
        braces[0] = '[Arguments] {';
      } else if (tag !== '') {
        braces[0] = "".concat(getPrefix(constructor, tag, 'Object'), "{");
      }

      if (keys.length === 0 && protoProps === undefined) {
        return "".concat(braces[0], "}");
      }
    } else if (typeof value === 'function') {
      base = getFunctionBase(value, constructor, tag);
      if (keys.length === 0 && protoProps === undefined) return ctx.stylize(base, 'special');
    } else if (isRegExp(value)) {
      // Make RegExps say that they are RegExps
      base = RegExpPrototypeToString(constructor !== null ? value : new RegExp(value));

      var _prefix4 = getPrefix(constructor, tag, 'RegExp');

      if (_prefix4 !== 'RegExp ') base = "".concat(_prefix4).concat(base);

      if (keys.length === 0 && protoProps === undefined || recurseTimes > ctx.depth && ctx.depth !== null) {
        return ctx.stylize(base, 'regexp');
      }
    } else if (isDate(value)) {
      // Make dates with properties first say the date
      base = NumberIsNaN(DatePrototypeGetTime(value)) ? DatePrototypeToString(value) : DatePrototypeToISOString(value);

      var _prefix5 = getPrefix(constructor, tag, 'Date');

      if (_prefix5 !== 'Date ') base = "".concat(_prefix5).concat(base);

      if (keys.length === 0 && protoProps === undefined) {
        return ctx.stylize(base, 'date');
      }
    } else if (isError(value)) {
      base = formatError(value, constructor, tag, ctx, keys);
      if (keys.length === 0 && protoProps === undefined) return base;
    } else if (isAnyArrayBuffer(value)) {
      // Fast path for ArrayBuffer and SharedArrayBuffer.
      // Can't do the same for DataView because it has a non-primitive
      // .buffer property that we need to recurse for.
      var arrayType = isArrayBuffer(value) ? 'ArrayBuffer' : 'SharedArrayBuffer';

      var _prefix6 = getPrefix(constructor, tag, arrayType);

      if (typedArray === undefined) {
        formatter = formatArrayBuffer;
      } else if (keys.length === 0 && protoProps === undefined) {
        return _prefix6 + "{ byteLength: ".concat(formatNumber(ctx.stylize, value.byteLength), " }");
      }

      braces[0] = "".concat(_prefix6, "{");
      ArrayPrototypeUnshift(keys, 'byteLength');
    } else if (isDataView(value)) {
      braces[0] = "".concat(getPrefix(constructor, tag, 'DataView'), "{"); // .buffer goes last, it's not a primitive like the others.

      ArrayPrototypeUnshift(keys, 'byteLength', 'byteOffset', 'buffer');
    } else if (isPromise(value)) {
      braces[0] = "".concat(getPrefix(constructor, tag, 'Promise'), "{");
      formatter = formatPromise;
    } else if (isWeakSet(value)) {
      braces[0] = "".concat(getPrefix(constructor, tag, 'WeakSet'), "{");
      formatter = ctx.showHidden ? formatWeakSet : formatWeakCollection;
    } else if (isWeakMap(value)) {
      braces[0] = "".concat(getPrefix(constructor, tag, 'WeakMap'), "{");
      formatter = ctx.showHidden ? formatWeakMap : formatWeakCollection;
    } else if (isModuleNamespaceObject(value)) {
      braces[0] = "".concat(getPrefix(constructor, tag, 'Module'), "{"); // Special handle keys for namespace objects.

      formatter = formatNamespaceObject.bind(null, keys);
    } else if (isBoxedPrimitive(value)) {
      base = getBoxedBase(value, ctx, keys, constructor, tag);

      if (keys.length === 0 && protoProps === undefined) {
        return base;
      }
    } else {
      if (keys.length === 0 && protoProps === undefined) {
        if (isExternal(value)) {
          var address = getExternalValue(value).toString(16);
          return ctx.stylize("[External: ".concat(address, "]"), 'special');
        }

        return "".concat(getCtxStyle(value, constructor, tag), "{}");
      }

      braces[0] = "".concat(getCtxStyle(value, constructor, tag), "{");
    }
  }

  if (recurseTimes > ctx.depth && ctx.depth !== null) {
    var constructorName = getCtxStyle(value, constructor, tag).slice(0, -1);
    if (constructor !== null) constructorName = "[".concat(constructorName, "]");
    return ctx.stylize(constructorName, 'special');
  }

  recurseTimes += 1;
  ctx.seen.push(value);
  ctx.currentDepth = recurseTimes;
  var output;
  var indentationLvl = ctx.indentationLvl;

  try {
    output = formatter(ctx, value, recurseTimes);

    for (i = 0; i < keys.length; i++) {
      output.push(formatProperty(ctx, value, recurseTimes, keys[i], extrasType));
    }

    if (protoProps !== undefined) {
      var _output;

      (_output = output).push.apply(_output, _toConsumableArray(protoProps));
    }
  } catch (err) {
    var _constructorName = getCtxStyle(value, constructor, tag).slice(0, -1);

    return handleMaxCallStackSize(ctx, err, _constructorName, indentationLvl);
  }

  if (ctx.circular !== undefined) {
    var index = ctx.circular.get(value);

    if (index !== undefined) {
      var reference = ctx.stylize("<ref *".concat(index, ">"), 'special'); // Add reference always to the very beginning of the output.

      if (ctx.compact !== true) {
        base = base === '' ? reference : "".concat(reference, " ").concat(base);
      } else {
        braces[0] = "".concat(reference, " ").concat(braces[0]);
      }
    }
  }

  ctx.seen.pop();

  if (ctx.sorted) {
    var comparator = ctx.sorted === true ? undefined : ctx.sorted;

    if (extrasType === kObjectType) {
      output = output.sort(comparator);
    } else if (keys.length > 1) {
      var _output2;

      var sorted = output.slice(output.length - keys.length).sort(comparator);

      (_output2 = output).splice.apply(_output2, [output.length - keys.length, keys.length].concat(_toConsumableArray(sorted)));
    }
  }

  var res = reduceToSingleString(ctx, output, base, braces, extrasType, recurseTimes, value);
  var budget = ctx.budget[ctx.indentationLvl] || 0;
  var newLength = budget + res.length;
  ctx.budget[ctx.indentationLvl] = newLength; // If any indentationLvl exceeds this limit, limit further inspecting to the
  // minimum. Otherwise the recursive algorithm might continue inspecting the
  // object even though the maximum string size (~2 ** 28 on 32 bit systems and
  // ~2 ** 30 on 64 bit systems) exceeded. The actual output is not limited at
  // exactly 2 ** 27 but a bit higher. This depends on the object shape.
  // This limit also makes sure that huge objects don't block the event loop
  // significantly.

  if (newLength > Math.pow(2, 27)) {
    ctx.depth = -1;
  }

  return res;
}

function getIteratorBraces(type, tag) {
  if (tag !== "".concat(type, " Iterator")) {
    if (tag !== '') tag += '] [';
    tag += "".concat(type, " Iterator");
  }

  return ["[".concat(tag, "] {"), '}'];
}

function getBoxedBase(value, ctx, keys, constructor, tag) {
  var fn;
  var type;

  if (isNumberObject(value)) {
    fn = NumberPrototypeValueOf;
    type = 'Number';
  } else if (isStringObject(value)) {
    fn = StringPrototypeValueOf;
    type = 'String'; // For boxed Strings, we have to remove the 0-n indexed entries,
    // since they just noisy up the output and are redundant
    // Make boxed primitive Strings look like such

    keys.splice(0, value.length);
  } else if (isBooleanObject(value)) {
    fn = BooleanPrototypeValueOf;
    type = 'Boolean';
  } else if (isBigIntObject(value)) {
    fn = BigIntPrototypeValueOf;
    type = 'BigInt';
  } else {
    fn = SymbolPrototypeValueOf;
    type = 'Symbol';
  }

  var base = "[".concat(type);

  if (type !== constructor) {
    if (constructor === null) {
      base += ' (null prototype)';
    } else {
      base += " (".concat(constructor, ")");
    }
  }

  base += ": ".concat(formatPrimitive(stylizeNoColor, fn(value), ctx), "]");

  if (tag !== '' && tag !== constructor) {
    base += " [".concat(tag, "]");
  }

  if (keys.length !== 0 || ctx.stylize === stylizeNoColor) return base;
  return ctx.stylize(base, StringPrototypeToLowerCase(type));
}

function getClassBase(value, constructor, tag) {
  var hasName = ObjectPrototypeHasOwnProperty(value, 'name');
  var name = hasName && value.name || '(anonymous)';
  var base = "class ".concat(name);

  if (constructor !== 'Function' && constructor !== null) {
    base += " [".concat(constructor, "]");
  }

  if (tag !== '' && constructor !== tag) {
    base += " [".concat(tag, "]");
  }

  if (constructor !== null) {
    var superName = ObjectGetPrototypeOf(value).name;

    if (superName) {
      base += " extends ".concat(superName);
    }
  } else {
    base += ' extends [null prototype]';
  }

  return "[".concat(base, "]");
}

function getFunctionBase(value, constructor, tag) {
  var stringified = FunctionPrototypeToString(value);

  if (stringified.slice(0, 5) === 'class' && stringified.endsWith('}')) {
    var slice = stringified.slice(5, -1);
    var bracketIndex = slice.indexOf('{');

    if (bracketIndex !== -1 && (!slice.slice(0, bracketIndex).includes('(') || // Slow path to guarantee that it's indeed a class.
    classRegExp.test(slice.replace(stripCommentsRegExp)))) {
      return getClassBase(value, constructor, tag);
    }
  }

  var type = 'Function';

  if (isGeneratorFunction(value)) {
    type = "Generator".concat(type);
  }

  if (isAsyncFunction(value)) {
    type = "Async".concat(type);
  }

  var base = "[".concat(type);

  if (constructor === null) {
    base += ' (null prototype)';
  }

  if (value.name === '') {
    base += ' (anonymous)';
  } else {
    base += ": ".concat(value.name);
  }

  base += ']';

  if (constructor !== type && constructor !== null) {
    base += " ".concat(constructor);
  }

  if (tag !== '' && constructor !== tag) {
    base += " [".concat(tag, "]");
  }

  return base;
}

function formatError(err, constructor, tag, ctx, keys) {
  var name = err.name != null ? String(err.name) : 'Error';
  var len = name.length;
  var stack = err.stack ? String(err.stack) : ErrorPrototypeToString(err); // Do not "duplicate" error properties that are already included in the output
  // otherwise.

  if (!ctx.showHidden && keys.length !== 0) {
    for (var _i = 0, _arr = ['name', 'message', 'stack']; _i < _arr.length; _i++) {
      var _name = _arr[_i];
      var index = keys.indexOf(_name); // Only hide the property in case it's part of the original stack

      if (index !== -1 && stack.includes(err[_name])) {
        keys.splice(index, 1);
      }
    }
  } // A stack trace may contain arbitrary data. Only manipulate the output
  // for "regular errors" (errors that "look normal") for now.


  if (constructor === null || name.endsWith('Error') && stack.startsWith(name) && (stack.length === len || stack[len] === ':' || stack[len] === '\n')) {
    var fallback = 'Error';

    if (constructor === null) {
      var start = stack.match(/^([A-Z][a-z_ A-Z0-9[\]()-]+)(?::|\n {4}at)/) || stack.match(/^([a-z_A-Z0-9-]*Error)$/);
      fallback = start && start[1] || '';
      len = fallback.length;
      fallback = fallback || 'Error';
    }

    var prefix = getPrefix(constructor, tag, fallback).slice(0, -1);

    if (name !== prefix) {
      if (prefix.includes(name)) {
        if (len === 0) {
          stack = "".concat(prefix, ": ").concat(stack);
        } else {
          stack = "".concat(prefix).concat(stack.slice(len));
        }
      } else {
        stack = "".concat(prefix, " [").concat(name, "]").concat(stack.slice(len));
      }
    }
  } // Ignore the error message if it's contained in the stack.


  var pos = err.message && stack.indexOf(err.message) || -1;
  if (pos !== -1) pos += err.message.length; // Wrap the error in brackets in case it has no stack trace.

  var stackStart = stack.indexOf('\n    at', pos);

  if (stackStart === -1) {
    stack = "[".concat(stack, "]");
  } else if (ctx.colors) {
    // Highlight userland code and node modules.
    var newStack = stack.slice(0, stackStart);
    var lines = stack.slice(stackStart + 1).split('\n');

    var _iterator3 = _createForOfIteratorHelper(lines),
        _step3;

    try {
      for (_iterator3.s(); !(_step3 = _iterator3.n()).done;) {
        var line = _step3.value;
        var core = line.match(coreModuleRegExp);

        if (core !== null && NativeModule.exists(core[1])) {
          newStack += "\n".concat(ctx.stylize(line, 'undefined'));
        } else {
          // This adds underscores to all node_modules to quickly identify them.
          var nodeModule = void 0;
          newStack += '\n';
          var _pos = 0;

          while (nodeModule = nodeModulesRegExp.exec(line)) {
            // '/node_modules/'.length === 14
            newStack += line.slice(_pos, nodeModule.index + 14);
            newStack += ctx.stylize(nodeModule[1], 'module');
            _pos = nodeModule.index + nodeModule[0].length;
          }

          newStack += _pos === 0 ? line : line.slice(_pos);
        }
      }
    } catch (err) {
      _iterator3.e(err);
    } finally {
      _iterator3.f();
    }

    stack = newStack;
  } // The message and the stack have to be indented as well!


  if (ctx.indentationLvl !== 0) {
    var indentation = ' '.repeat(ctx.indentationLvl);
    stack = stack.replace(/\n/g, "\n".concat(indentation));
  }

  return stack;
}

function groupArrayElements(ctx, output, value) {
  var totalLength = 0;
  var maxLength = 0;
  var i = 0;
  var outputLength = output.length;

  if (ctx.maxArrayLength < output.length) {
    // This makes sure the "... n more items" part is not taken into account.
    outputLength--;
  }

  var separatorSpace = 2; // Add 1 for the space and 1 for the separator.

  var dataLen = new _Array(outputLength); // Calculate the total length of all output entries and the individual max
  // entries length of all output entries. We have to remove colors first,
  // otherwise the length would not be calculated properly.

  for (; i < outputLength; i++) {
    var len = getStringWidth(output[i], ctx.colors);
    dataLen[i] = len;
    totalLength += len + separatorSpace;
    if (maxLength < len) maxLength = len;
  } // Add two to `maxLength` as we add a single whitespace character plus a comma
  // in-between two entries.


  var actualMax = maxLength + separatorSpace; // Check if at least three entries fit next to each other and prevent grouping
  // of arrays that contains entries of very different length (i.e., if a single
  // entry is longer than 1/5 of all other entries combined). Otherwise the
  // space in-between small entries would be enormous.

  if (actualMax * 3 + ctx.indentationLvl < ctx.breakLength && (totalLength / actualMax > 5 || maxLength <= 6)) {
    var approxCharHeights = 2.5;
    var averageBias = MathSqrt(actualMax - totalLength / output.length);
    var biasedMax = MathMax(actualMax - 3 - averageBias, 1); // Dynamically check how many columns seem possible.

    var columns = MathMin( // Ideally a square should be drawn. We expect a character to be about 2.5
    // times as high as wide. This is the area formula to calculate a square
    // which contains n rectangles of size `actualMax * approxCharHeights`.
    // Divide that by `actualMax` to receive the correct number of columns.
    // The added bias increases the columns for short entries.
    MathRound(MathSqrt(approxCharHeights * biasedMax * outputLength) / biasedMax), // Do not exceed the breakLength.
    MathFloor((ctx.breakLength - ctx.indentationLvl) / actualMax), // Limit array grouping for small `compact` modes as the user requested
    // minimal grouping.
    ctx.compact * 4, // Limit the columns to a maximum of fifteen.
    15); // Return with the original output if no grouping should happen.

    if (columns <= 1) {
      return output;
    }

    var tmp = [];
    var maxLineLength = [];

    for (var _i2 = 0; _i2 < columns; _i2++) {
      var lineMaxLength = 0;

      for (var j = _i2; j < output.length; j += columns) {
        if (dataLen[j] > lineMaxLength) lineMaxLength = dataLen[j];
      }

      lineMaxLength += separatorSpace;
      maxLineLength[_i2] = lineMaxLength;
    }

    var order = StringPrototypePadStart;

    if (value !== undefined) {
      for (var _i3 = 0; _i3 < output.length; _i3++) {
        if (typeof value[_i3] !== 'number' && typeof value[_i3] !== 'bigint') {
          order = StringPrototypePadEnd;
          break;
        }
      }
    } // Each iteration creates a single line of grouped entries.


    for (var _i4 = 0; _i4 < outputLength; _i4 += columns) {
      // The last lines may contain less entries than columns.
      var max = MathMin(_i4 + columns, outputLength);
      var str = '';
      var _j = _i4;

      for (; _j < max - 1; _j++) {
        // Calculate extra color padding in case it's active. This has to be
        // done line by line as some lines might contain more colors than
        // others.
        var padding = maxLineLength[_j - _i4] + output[_j].length - dataLen[_j];
        str += order("".concat(output[_j], ", "), padding, ' ');
      }

      if (order === StringPrototypePadStart) {
        var _padding = maxLineLength[_j - _i4] + output[_j].length - dataLen[_j] - separatorSpace;

        str += StringPrototypePadStart(output[_j], _padding, ' ');
      } else {
        str += output[_j];
      }

      ArrayPrototypePush(tmp, str);
    }

    if (ctx.maxArrayLength < output.length) {
      ArrayPrototypePush(tmp, output[outputLength]);
    }

    output = tmp;
  }

  return output;
}

function handleMaxCallStackSize(ctx, err, constructorName, indentationLvl) {
  if (isStackOverflowError(err)) {
    ctx.seen.pop();
    ctx.indentationLvl = indentationLvl;
    return ctx.stylize("[".concat(constructorName, ": Inspection interrupted ") + 'prematurely. Maximum call stack size exceeded.]', 'special');
  }
  /* c8 ignore next */


  assert.fail(err.stack);
}

function formatNumber(fn, value) {
  // Format -0 as '-0'. Checking `value === -0` won't distinguish 0 from -0.
  return fn(ObjectIs(value, -0) ? '-0' : "".concat(value), 'number');
}

function formatBigInt(fn, value) {
  return fn("".concat(value, "n"), 'bigint');
}

function formatPrimitive(fn, value, ctx) {
  if (typeof value === 'string') {
    var trailer = '';

    if (value.length > ctx.maxStringLength) {
      var remaining = value.length - ctx.maxStringLength;
      value = value.slice(0, ctx.maxStringLength);
      trailer = "... ".concat(remaining, " more character").concat(remaining > 1 ? 's' : '');
    }

    if (ctx.compact !== true && // TODO(BridgeAR): Add unicode support. Use the readline getStringWidth
    // function.
    value.length > kMinLineLength && value.length > ctx.breakLength - ctx.indentationLvl - 4) {
      return value.split(/(?<=\n)/).map(function (line) {
        return fn(strEscape(line), 'string');
      }).join(" +\n".concat(' '.repeat(ctx.indentationLvl + 2))) + trailer;
    }

    return fn(strEscape(value), 'string') + trailer;
  }

  if (typeof value === 'number') return formatNumber(fn, value);
  if (typeof value === 'bigint') return formatBigInt(fn, value);
  if (typeof value === 'boolean') return fn("".concat(value), 'boolean');
  if (typeof value === 'undefined') return fn('undefined', 'undefined'); // es6 symbol primitive

  return fn(SymbolPrototypeToString(value), 'symbol');
}

function formatNamespaceObject(keys, ctx, value, recurseTimes) {
  var output = new _Array(keys.length);

  for (var i = 0; i < keys.length; i++) {
    try {
      output[i] = formatProperty(ctx, value, recurseTimes, keys[i], kObjectType);
    } catch (err) {
      assert(isNativeError(err) && err.name === 'ReferenceError'); // Use the existing functionality. This makes sure the indentation and
      // line breaks are always correct. Otherwise it is very difficult to keep
      // this aligned, even though this is a hacky way of dealing with this.

      var tmp = _defineProperty({}, keys[i], '');

      output[i] = formatProperty(ctx, tmp, recurseTimes, keys[i], kObjectType);
      var pos = output[i].lastIndexOf(' '); // We have to find the last whitespace and have to replace that value as
      // it will be visualized as a regular string.

      output[i] = output[i].slice(0, pos + 1) + ctx.stylize('<uninitialized>', 'special');
    }
  } // Reset the keys to an empty array. This prevents duplicated inspection.


  keys.length = 0;
  return output;
} // The array is sparse and/or has extra keys


function formatSpecialArray(ctx, value, recurseTimes, maxLength, output, i) {
  var keys = ObjectKeys(value);
  var index = i;

  for (; i < keys.length && output.length < maxLength; i++) {
    var key = keys[i];
    var tmp = +key; // Arrays can only have up to 2^32 - 1 entries

    if (tmp > Math.pow(2, 32) - 2) {
      break;
    }

    if ("".concat(index) !== key) {
      if (!numberRegExp.test(key)) {
        break;
      }

      var emptyItems = tmp - index;
      var ending = emptyItems > 1 ? 's' : '';
      var message = "<".concat(emptyItems, " empty item").concat(ending, ">");
      output.push(ctx.stylize(message, 'undefined'));
      index = tmp;

      if (output.length === maxLength) {
        break;
      }
    }

    output.push(formatProperty(ctx, value, recurseTimes, key, kArrayType));
    index++;
  }

  var remaining = value.length - index;

  if (output.length !== maxLength) {
    if (remaining > 0) {
      var _ending = remaining > 1 ? 's' : '';

      var _message = "<".concat(remaining, " empty item").concat(_ending, ">");

      output.push(ctx.stylize(_message, 'undefined'));
    }
  } else if (remaining > 0) {
    output.push("... ".concat(remaining, " more item").concat(remaining > 1 ? 's' : ''));
  }

  return output;
}

function formatArrayBuffer(ctx, value) {
  var buffer;

  try {
    buffer = new Uint8Array(value);
  } catch (_unused3) {
    return [ctx.stylize('(detached)', 'special')];
  }

  if (hexSlice === undefined) hexSlice = uncurryThis(require('buffer').Buffer.prototype.hexSlice);
  var str = StringPrototypeTrim(StringPrototypeReplace(hexSlice(buffer, 0, MathMin(ctx.maxArrayLength, buffer.length)), /(.{2})/g, '$1 '));
  var remaining = buffer.length - ctx.maxArrayLength;
  if (remaining > 0) str += " ... ".concat(remaining, " more byte").concat(remaining > 1 ? 's' : '');
  return ["".concat(ctx.stylize('[Uint8Contents]', 'special'), ": <").concat(str, ">")];
}

function formatArray(ctx, value, recurseTimes) {
  var valLen = value.length;
  var len = MathMin(MathMax(0, ctx.maxArrayLength), valLen);
  var remaining = valLen - len;
  var output = [];

  for (var i = 0; i < len; i++) {
    // Special handle sparse arrays.
    if (!ObjectPrototypeHasOwnProperty(value, i)) {
      return formatSpecialArray(ctx, value, recurseTimes, len, output, i);
    }

    output.push(formatProperty(ctx, value, recurseTimes, i, kArrayType));
  }

  if (remaining > 0) output.push("... ".concat(remaining, " more item").concat(remaining > 1 ? 's' : ''));
  return output;
}

function formatTypedArray(value, length, ctx, ignored, recurseTimes) {
  var maxLength = MathMin(MathMax(0, ctx.maxArrayLength), length);
  var remaining = value.length - maxLength;
  var output = new _Array(maxLength);
  var elementFormatter = value.length > 0 && typeof value[0] === 'number' ? formatNumber : formatBigInt;

  for (var i = 0; i < maxLength; ++i) {
    output[i] = elementFormatter(ctx.stylize, value[i]);
  }

  if (remaining > 0) {
    output[maxLength] = "... ".concat(remaining, " more item").concat(remaining > 1 ? 's' : '');
  }

  if (ctx.showHidden) {
    // .buffer goes last, it's not a primitive like the others.
    // All besides `BYTES_PER_ELEMENT` are actually getters.
    ctx.indentationLvl += 2;

    for (var _i5 = 0, _arr2 = ['BYTES_PER_ELEMENT', 'length', 'byteLength', 'byteOffset', 'buffer']; _i5 < _arr2.length; _i5++) {
      var key = _arr2[_i5];
      var str = formatValue(ctx, value[key], recurseTimes, true);
      ArrayPrototypePush(output, "[".concat(key, "]: ").concat(str));
    }

    ctx.indentationLvl -= 2;
  }

  return output;
}

function formatSet(value, ctx, ignored, recurseTimes) {
  var output = [];
  ctx.indentationLvl += 2;

  var _iterator4 = _createForOfIteratorHelper(value),
      _step4;

  try {
    for (_iterator4.s(); !(_step4 = _iterator4.n()).done;) {
      var v = _step4.value;
      ArrayPrototypePush(output, formatValue(ctx, v, recurseTimes));
    }
  } catch (err) {
    _iterator4.e(err);
  } finally {
    _iterator4.f();
  }

  ctx.indentationLvl -= 2;
  return output;
}

function formatMap(value, ctx, ignored, recurseTimes) {
  var output = [];
  ctx.indentationLvl += 2;

  var _iterator5 = _createForOfIteratorHelper(value),
      _step5;

  try {
    for (_iterator5.s(); !(_step5 = _iterator5.n()).done;) {
      var _step5$value = _step5.value,
          k = _step5$value[0],
          v = _step5$value[1];
      output.push("".concat(formatValue(ctx, k, recurseTimes), " => ").concat(formatValue(ctx, v, recurseTimes)));
    }
  } catch (err) {
    _iterator5.e(err);
  } finally {
    _iterator5.f();
  }

  ctx.indentationLvl -= 2;
  return output;
}

function formatSetIterInner(ctx, recurseTimes, entries, state) {
  var maxArrayLength = MathMax(ctx.maxArrayLength, 0);
  var maxLength = MathMin(maxArrayLength, entries.length);
  var output = new _Array(maxLength);
  ctx.indentationLvl += 2;

  for (var i = 0; i < maxLength; i++) {
    output[i] = formatValue(ctx, entries[i], recurseTimes);
  }

  ctx.indentationLvl -= 2;

  if (state === kWeak && !ctx.sorted) {
    // Sort all entries to have a halfway reliable output (if more entries than
    // retrieved ones exist, we can not reliably return the same output) if the
    // output is not sorted anyway.
    ArrayPrototypeSort(output);
  }

  var remaining = entries.length - maxLength;

  if (remaining > 0) {
    ArrayPrototypePush(output, "... ".concat(remaining, " more item").concat(remaining > 1 ? 's' : ''));
  }

  return output;
}

function formatMapIterInner(ctx, recurseTimes, entries, state) {
  var maxArrayLength = MathMax(ctx.maxArrayLength, 0); // Entries exist as [key1, val1, key2, val2, ...]

  var len = entries.length / 2;
  var remaining = len - maxArrayLength;
  var maxLength = MathMin(maxArrayLength, len);
  var output = new _Array(maxLength);
  var i = 0;
  ctx.indentationLvl += 2;

  if (state === kWeak) {
    for (; i < maxLength; i++) {
      var pos = i * 2;
      output[i] = "".concat(formatValue(ctx, entries[pos], recurseTimes), " => ").concat(formatValue(ctx, entries[pos + 1], recurseTimes));
    } // Sort all entries to have a halfway reliable output (if more entries than
    // retrieved ones exist, we can not reliably return the same output) if the
    // output is not sorted anyway.


    if (!ctx.sorted) output = output.sort();
  } else {
    for (; i < maxLength; i++) {
      var _pos2 = i * 2;

      var res = [formatValue(ctx, entries[_pos2], recurseTimes), formatValue(ctx, entries[_pos2 + 1], recurseTimes)];
      output[i] = reduceToSingleString(ctx, res, '', ['[', ']'], kArrayExtrasType, recurseTimes);
    }
  }

  ctx.indentationLvl -= 2;

  if (remaining > 0) {
    output.push("... ".concat(remaining, " more item").concat(remaining > 1 ? 's' : ''));
  }

  return output;
}

function formatWeakCollection(ctx) {
  return [ctx.stylize('<items unknown>', 'special')];
}

function formatWeakSet(ctx, value, recurseTimes) {
  var entries = previewEntries(value);
  return formatSetIterInner(ctx, recurseTimes, entries, kWeak);
}

function formatWeakMap(ctx, value, recurseTimes) {
  var entries = previewEntries(value);
  return formatMapIterInner(ctx, recurseTimes, entries, kWeak);
}

function formatIterator(braces, ctx, value, recurseTimes) {
  var _previewEntries = previewEntries(value, true),
      entries = _previewEntries[0],
      isKeyValue = _previewEntries[1];

  if (isKeyValue) {
    // Mark entry iterators as such.
    braces[0] = braces[0].replace(/ Iterator] {$/, ' Entries] {');
    return formatMapIterInner(ctx, recurseTimes, entries, kMapEntries);
  }

  return formatSetIterInner(ctx, recurseTimes, entries, kIterator);
}

function formatPromise(ctx, value, recurseTimes) {
  var output;

  var _getPromiseDetails = getPromiseDetails(value),
      state = _getPromiseDetails[0],
      result = _getPromiseDetails[1];

  if (state === kPending) {
    output = [ctx.stylize('<pending>', 'special')];
  } else {
    ctx.indentationLvl += 2;
    var str = formatValue(ctx, result, recurseTimes);
    ctx.indentationLvl -= 2;
    output = [state === kRejected ? "".concat(ctx.stylize('<rejected>', 'special'), " ").concat(str) : str];
  }

  return output;
}

function formatProperty(ctx, value, recurseTimes, key, type, desc) {
  var original = arguments.length > 6 && arguments[6] !== undefined ? arguments[6] : value;
  var name, str;
  var extra = ' ';
  desc = desc || ObjectGetOwnPropertyDescriptor(value, key) || {
    value: value[key],
    enumerable: true
  };

  if (desc.value !== undefined) {
    var diff = ctx.compact !== true || type !== kObjectType ? 2 : 3;
    ctx.indentationLvl += diff;
    str = formatValue(ctx, desc.value, recurseTimes);

    if (diff === 3 && ctx.breakLength < getStringWidth(str, ctx.colors)) {
      extra = "\n".concat(' '.repeat(ctx.indentationLvl));
    }

    ctx.indentationLvl -= diff;
  } else if (desc.get !== undefined) {
    var label = desc.set !== undefined ? 'Getter/Setter' : 'Getter';
    var s = ctx.stylize;
    var sp = 'special';

    if (ctx.getters && (ctx.getters === true || ctx.getters === 'get' && desc.set === undefined || ctx.getters === 'set' && desc.set !== undefined)) {
      try {
        var tmp = FunctionPrototypeCall(desc.get, original);
        ctx.indentationLvl += 2;

        if (tmp === null) {
          str = "".concat(s("[".concat(label, ":"), sp), " ").concat(s('null', 'null')).concat(s(']', sp));
        } else if (_typeof(tmp) === 'object') {
          str = "".concat(s("[".concat(label, "]"), sp), " ").concat(formatValue(ctx, tmp, recurseTimes));
        } else {
          var primitive = formatPrimitive(s, tmp, ctx);
          str = "".concat(s("[".concat(label, ":"), sp), " ").concat(primitive).concat(s(']', sp));
        }

        ctx.indentationLvl -= 2;
      } catch (err) {
        var message = "<Inspection threw (".concat(err.message, ")>");
        str = "".concat(s("[".concat(label, ":"), sp), " ").concat(message).concat(s(']', sp));
      }
    } else {
      str = ctx.stylize("[".concat(label, "]"), sp);
    }
  } else if (desc.set !== undefined) {
    str = ctx.stylize('[Setter]', 'special');
  } else {
    str = ctx.stylize('undefined', 'undefined');
  }

  if (type === kArrayType) {
    return str;
  }

  if (_typeof(key) === 'symbol') {
    var _tmp2 = StringPrototypeReplace(SymbolPrototypeToString(key), strEscapeSequencesReplacer, escapeFn);

    name = "[".concat(ctx.stylize(_tmp2, 'symbol'), "]");
  } else if (key === '__proto__') {
    name = "['__proto__']";
  } else if (desc.enumerable === false) {
    var _tmp3 = StringPrototypeReplace(key, strEscapeSequencesReplacer, escapeFn);

    name = "[".concat(_tmp3, "]");
  } else if (RegExpPrototypeTest(keyStrRegExp, key)) {
    name = ctx.stylize(key, 'name');
  } else {
    name = ctx.stylize(strEscape(key), 'string');
  }

  return "".concat(name, ":").concat(extra).concat(str);
}

function isBelowBreakLength(ctx, output, start, base) {
  // Each entry is separated by at least a comma. Thus, we start with a total
  // length of at least `output.length`. In addition, some cases have a
  // whitespace in-between each other that is added to the total as well.
  // TODO(BridgeAR): Add unicode support. Use the readline getStringWidth
  // function. Check the performance overhead and make it an opt-in in case it's
  // significant.
  var totalLength = output.length + start;
  if (totalLength + output.length > ctx.breakLength) return false;

  for (var i = 0; i < output.length; i++) {
    if (ctx.colors) {
      totalLength += removeColors(output[i]).length;
    } else {
      totalLength += output[i].length;
    }

    if (totalLength > ctx.breakLength) {
      return false;
    }
  } // Do not line up properties on the same line if `base` contains line breaks.


  return base === '' || !StringPrototypeIncludes(base, '\n');
}

function reduceToSingleString(ctx, output, base, braces, extrasType, recurseTimes, value) {
  if (ctx.compact !== true) {
    if (typeof ctx.compact === 'number' && ctx.compact >= 1) {
      // Memorize the original output length. In case the output is grouped,
      // prevent lining up the entries on a single line.
      var entries = output.length; // Group array elements together if the array contains at least six
      // separate entries.

      if (extrasType === kArrayExtrasType && entries > 6) {
        output = groupArrayElements(ctx, output, value);
      } // `ctx.currentDepth` is set to the most inner depth of the currently
      // inspected object part while `recurseTimes` is the actual current depth
      // that is inspected.
      //
      // Example:
      //
      // const a = { first: [ 1, 2, 3 ], second: { inner: [ 1, 2, 3 ] } }
      //
      // The deepest depth of `a` is 2 (a.second.inner) and `a.first` has a max
      // depth of 1.
      //
      // Consolidate all entries of the local most inner depth up to
      // `ctx.compact`, as long as the properties are smaller than
      // `ctx.breakLength`.


      if (ctx.currentDepth - recurseTimes < ctx.compact && entries === output.length) {
        // Line up all entries on a single line in case the entries do not
        // exceed `breakLength`. Add 10 as constant to start next to all other
        // factors that may reduce `breakLength`.
        var start = output.length + ctx.indentationLvl + braces[0].length + base.length + 10;

        if (isBelowBreakLength(ctx, output, start, base)) {
          return "".concat(base ? "".concat(base, " ") : '').concat(braces[0], " ").concat(join(output, ', ')) + " ".concat(braces[1]);
        }
      }
    } // Line up each entry on an individual line.


    var _indentation = "\n".concat(StringPrototypeRepeat(' ', ctx.indentationLvl));

    return "".concat(base ? "".concat(base, " ") : '').concat(braces[0]).concat(_indentation, "  ") + "".concat(join(output, ",".concat(_indentation, "  "))).concat(_indentation).concat(braces[1]);
  } // Line up all entries on a single line in case the entries do not exceed
  // `breakLength`.


  if (isBelowBreakLength(ctx, output, 0, base)) {
    return "".concat(braces[0]).concat(base ? " ".concat(base) : '', " ").concat(join(output, ', '), " ") + braces[1];
  }

  var indentation = StringPrototypeRepeat(' ', ctx.indentationLvl); // If the opening "brace" is too large, like in the case of "Set {",
  // we need to force the first item to be on the next line or the
  // items will not line up correctly.

  var ln = base === '' && braces[0].length === 1 ? ' ' : "".concat(base ? " ".concat(base) : '', "\n").concat(indentation, "  "); // Line up each entry on an individual line.

  return "".concat(braces[0]).concat(ln).concat(join(output, ",\n".concat(indentation, "  ")), " ").concat(braces[1]);
}

function hasBuiltInToString(value) {
  // Prevent triggering proxy traps.
  var getFullProxy = false;
  var proxyTarget = getProxyDetails(value, getFullProxy);

  if (proxyTarget !== undefined) {
    value = proxyTarget;
  } // Count objects that have no `toString` function as built-in.


  if (typeof value.toString !== 'function') {
    return true;
  } // The object has a own `toString` property. Thus it's not not a built-in one.


  if (ObjectPrototypeHasOwnProperty(value, 'toString')) {
    return false;
  } // Find the object that has the `toString` property as own property in the
  // prototype chain.


  var pointer = value;

  do {
    pointer = ObjectGetPrototypeOf(pointer);
  } while (!ObjectPrototypeHasOwnProperty(pointer, 'toString')); // Check closer if the object is a built-in.


  var descriptor = ObjectGetOwnPropertyDescriptor(pointer, 'constructor');
  return descriptor !== undefined && typeof descriptor.value === 'function' && builtInObjects.has(descriptor.value.name);
}

var firstErrorLine = function firstErrorLine(error) {
  return StringPrototypeSplit(error.message, '\n', 1)[0];
};

var CIRCULAR_ERROR_MESSAGE;

function tryStringify(arg) {
  try {
    return JSONStringify(arg);
  } catch (err) {
    // Populate the circular error message lazily
    if (!CIRCULAR_ERROR_MESSAGE) {
      try {
        var a = {};
        a.a = a;
        JSONStringify(a);
      } catch (err) {
        CIRCULAR_ERROR_MESSAGE = firstErrorLine(err);
      }
    }

    if (err.name === 'TypeError' && firstErrorLine(err) === CIRCULAR_ERROR_MESSAGE) {
      return '[Circular]';
    }

    throw err;
  }
}

function format() {
  for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
    args[_key] = arguments[_key];
  }

  return formatWithOptionsInternal(undefined, args);
}

function formatWithOptions(inspectOptions) {
  if (_typeof(inspectOptions) !== 'object' || inspectOptions === null) {
    throw new ERR_INVALID_ARG_TYPE('inspectOptions', 'object', inspectOptions);
  }

  for (var _len2 = arguments.length, args = new Array(_len2 > 1 ? _len2 - 1 : 0), _key2 = 1; _key2 < _len2; _key2++) {
    args[_key2 - 1] = arguments[_key2];
  }

  return formatWithOptionsInternal(inspectOptions, args);
}

function formatWithOptionsInternal(inspectOptions, args) {
  var first = args[0];
  var a = 0;
  var str = '';
  var join = '';

  if (typeof first === 'string') {
    if (args.length === 1) {
      return first;
    }

    var tempStr;
    var lastPos = 0;

    for (var i = 0; i < first.length - 1; i++) {
      if (StringPrototypeCharCodeAt(first, i) === 37) {
        // '%'
        var nextChar = StringPrototypeCharCodeAt(first, ++i);

        if (a + 1 !== args.length) {
          switch (nextChar) {
            case 115:
              // 's'
              var tempArg = args[++a];

              if (typeof tempArg === 'number') {
                tempStr = formatNumber(stylizeNoColor, tempArg);
              } else if (typeof tempArg === 'bigint') {
                tempStr = "".concat(tempArg, "n");
              } else if (_typeof(tempArg) !== 'object' || tempArg === null || !hasBuiltInToString(tempArg)) {
                tempStr = String(tempArg);
              } else {
                tempStr = inspect(tempArg, _objectSpread(_objectSpread({}, inspectOptions), {}, {
                  compact: 3,
                  colors: false,
                  depth: 0
                }));
              }

              break;

            case 106:
              // 'j'
              tempStr = tryStringify(args[++a]);
              break;

            case 100:
              // 'd'
              var tempNum = args[++a];

              if (typeof tempNum === 'bigint') {
                tempStr = "".concat(tempNum, "n");
              } else if (_typeof(tempNum) === 'symbol') {
                tempStr = 'NaN';
              } else {
                tempStr = formatNumber(stylizeNoColor, Number(tempNum));
              }

              break;

            case 79:
              // 'O'
              tempStr = inspect(args[++a], inspectOptions);
              break;

            case 111:
              // 'o'
              tempStr = inspect(args[++a], _objectSpread(_objectSpread({}, inspectOptions), {}, {
                showHidden: true,
                showProxy: true,
                depth: 4
              }));
              break;

            case 105:
              // 'i'
              var tempInteger = args[++a];

              if (typeof tempInteger === 'bigint') {
                tempStr = "".concat(tempInteger, "n");
              } else if (_typeof(tempInteger) === 'symbol') {
                tempStr = 'NaN';
              } else {
                tempStr = formatNumber(stylizeNoColor, NumberParseInt(tempInteger));
              }

              break;

            case 102:
              // 'f'
              var tempFloat = args[++a];

              if (_typeof(tempFloat) === 'symbol') {
                tempStr = 'NaN';
              } else {
                tempStr = formatNumber(stylizeNoColor, NumberParseFloat(tempFloat));
              }

              break;

            case 99:
              // 'c'
              a += 1;
              tempStr = '';
              break;

            case 37:
              // '%'
              str += StringPrototypeSlice(first, lastPos, i);
              lastPos = i + 1;
              continue;

            default:
              // Any other character is not a correct placeholder
              continue;
          }

          if (lastPos !== i - 1) {
            str += StringPrototypeSlice(first, lastPos, i - 1);
          }

          str += tempStr;
          lastPos = i + 1;
        } else if (nextChar === 37) {
          str += StringPrototypeSlice(first, lastPos, i);
          lastPos = i + 1;
        }
      }
    }

    if (lastPos !== 0) {
      a++;
      join = ' ';

      if (lastPos < first.length) {
        str += StringPrototypeSlice(first, lastPos);
      }
    }
  }

  while (a < args.length) {
    var value = args[a];
    str += join;
    str += typeof value !== 'string' ? inspect(value, inspectOptions) : value;
    join = ' ';
    a++;
  }

  return str;
}

if (false) {//(internalBinding('config').hasIntl) {
  var icu = internalBinding('icu'); // icu.getStringWidth(string, ambiguousAsFullWidth, expandEmojiSequence)
  // Defaults: ambiguousAsFullWidth = false; expandEmojiSequence = true;
  // TODO(BridgeAR): Expose the options to the user. That is probably the
  // best thing possible at the moment, since it's difficult to know what
  // the receiving end supports.

  getStringWidth = function getStringWidth(str) {
    var removeControlChars = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : true;
    var width = 0;
    if (removeControlChars) str = stripVTControlCharacters(str);

    for (var i = 0; i < str.length; i++) {
      // Try to avoid calling into C++ by first handling the ASCII portion of
      // the string. If it is fully ASCII, we skip the C++ part.
      var code = str.charCodeAt(i);

      if (code >= 127) {
        width += icu.getStringWidth(str.slice(i).normalize('NFC'));
        break;
      }

      width += code >= 32 ? 1 : 0;
    }

    return width;
  };
} else {
  /**
   * Returns the number of columns required to display the given string.
   */
  getStringWidth = function getStringWidth(str) {
    var removeControlChars = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : true;
    var width = 0;
    if (removeControlChars) str = stripVTControlCharacters(str);
    str = StringPrototypeNormalize(str, 'NFC');

    var _iterator6 = _createForOfIteratorHelper(new SafeStringIterator(str)),
        _step6;

    try {
      for (_iterator6.s(); !(_step6 = _iterator6.n()).done;) {
        var _char = _step6.value;
        var code = StringPrototypeCodePointAt(_char, 0);

        if (isFullWidthCodePoint(code)) {
          width += 2;
        } else if (!isZeroWidthCodePoint(code)) {
          width++;
        }
      }
    } catch (err) {
      _iterator6.e(err);
    } finally {
      _iterator6.f();
    }

    return width;
  };
  /**
   * Returns true if the character represented by a given
   * Unicode code point is full-width. Otherwise returns false.
   */


  var isFullWidthCodePoint = function isFullWidthCodePoint(code) {
    // Code points are partially derived from:
    // https://www.unicode.org/Public/UNIDATA/EastAsianWidth.txt
    return code >= 0x1100 && (code <= 0x115f || // Hangul Jamo
    code === 0x2329 || // LEFT-POINTING ANGLE BRACKET
    code === 0x232a || // RIGHT-POINTING ANGLE BRACKET
    // CJK Radicals Supplement .. Enclosed CJK Letters and Months
    code >= 0x2e80 && code <= 0x3247 && code !== 0x303f || // Enclosed CJK Letters and Months .. CJK Unified Ideographs Extension A
    code >= 0x3250 && code <= 0x4dbf || // CJK Unified Ideographs .. Yi Radicals
    code >= 0x4e00 && code <= 0xa4c6 || // Hangul Jamo Extended-A
    code >= 0xa960 && code <= 0xa97c || // Hangul Syllables
    code >= 0xac00 && code <= 0xd7a3 || // CJK Compatibility Ideographs
    code >= 0xf900 && code <= 0xfaff || // Vertical Forms
    code >= 0xfe10 && code <= 0xfe19 || // CJK Compatibility Forms .. Small Form Variants
    code >= 0xfe30 && code <= 0xfe6b || // Halfwidth and Fullwidth Forms
    code >= 0xff01 && code <= 0xff60 || code >= 0xffe0 && code <= 0xffe6 || // Kana Supplement
    code >= 0x1b000 && code <= 0x1b001 || // Enclosed Ideographic Supplement
    code >= 0x1f200 && code <= 0x1f251 || // Miscellaneous Symbols and Pictographs 0x1f300 - 0x1f5ff
    // Emoticons 0x1f600 - 0x1f64f
    code >= 0x1f300 && code <= 0x1f64f || // CJK Unified Ideographs Extension B .. Tertiary Ideographic Plane
    code >= 0x20000 && code <= 0x3fffd);
  };

  var isZeroWidthCodePoint = function isZeroWidthCodePoint(code) {
    return code <= 0x1F || // C0 control codes
    code >= 0x7F && code <= 0x9F || // C1 control codes
    code >= 0x300 && code <= 0x36F || // Combining Diacritical Marks
    code >= 0x200B && code <= 0x200F || // Modifying Invisible Characters
    // Combining Diacritical Marks for Symbols
    code >= 0x20D0 && code <= 0x20FF || code >= 0xFE00 && code <= 0xFE0F || // Variation Selectors
    code >= 0xFE20 && code <= 0xFE2F || // Combining Half Marks
    code >= 0xE0100 && code <= 0xE01EF; // Variation Selectors
  };
}
/**
 * Remove all VT control characters. Use to estimate displayed string width.
 */


function stripVTControlCharacters(str) {
  return str.replace(ansi, '');
}

module.exports = {
  inspect: inspect,
  format: format,
  formatWithOptions: formatWithOptions,
  getStringWidth: getStringWidth,
  inspectDefaultOptions: inspectDefaultOptions,
  stripVTControlCharacters: stripVTControlCharacters
};
