// @nolint
'use strict';

var _ObjectDefineProperti, _ObjectDefineProperti2;

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var _primordials = primordials,
    _Array = _primordials.Array,
    ArrayPrototypeJoin = _primordials.ArrayPrototypeJoin,
    ArrayPrototypeMap = _primordials.ArrayPrototypeMap,
    ArrayPrototypePush = _primordials.ArrayPrototypePush,
    ArrayPrototypeReduce = _primordials.ArrayPrototypeReduce,
    ArrayPrototypeSlice = _primordials.ArrayPrototypeSlice,
    FunctionPrototypeBind = _primordials.FunctionPrototypeBind,
    Int8Array = _primordials.Int8Array,
    Number = _primordials.Number,
    ObjectCreate = _primordials.ObjectCreate,
    ObjectDefineProperties = _primordials.ObjectDefineProperties,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    ObjectGetOwnPropertySymbols = _primordials.ObjectGetOwnPropertySymbols,
    ObjectGetPrototypeOf = _primordials.ObjectGetPrototypeOf,
    ObjectKeys = _primordials.ObjectKeys,
    ReflectApply = _primordials.ReflectApply,
    ReflectGetOwnPropertyDescriptor = _primordials.ReflectGetOwnPropertyDescriptor,
    ReflectOwnKeys = _primordials.ReflectOwnKeys,
    RegExpPrototypeExec = _primordials.RegExpPrototypeExec,
    String = _primordials.String,
    StringPrototypeCharCodeAt = _primordials.StringPrototypeCharCodeAt,
    StringPrototypeIncludes = _primordials.StringPrototypeIncludes,
    StringPrototypeReplace = _primordials.StringPrototypeReplace,
    StringPrototypeSlice = _primordials.StringPrototypeSlice,
    StringPrototypeSplit = _primordials.StringPrototypeSplit,
    StringPrototypeStartsWith = _primordials.StringPrototypeStartsWith,
    _Symbol = _primordials.Symbol,
    SymbolIterator = _primordials.SymbolIterator,
    SymbolToStringTag = _primordials.SymbolToStringTag,
    decodeURIComponent = _primordials.decodeURIComponent;

var _require = require('internal/util/inspect'),
    inspect = _require.inspect;

// var _require2 = require('internal/querystring'),
//     encodeStr = _require2.encodeStr,
//     hexTable = _require2.hexTable,
//     isHexTable = _require2.isHexTable;

var _require3 = require('internal/util'),
    getConstructorOf = _require3.getConstructorOf,
    removeColors = _require3.removeColors;

var _require$codes = require('internal/errors').codes,
    ERR_ARG_NOT_ITERABLE = _require$codes.ERR_ARG_NOT_ITERABLE,
    ERR_INVALID_ARG_TYPE = _require$codes.ERR_INVALID_ARG_TYPE,
    ERR_INVALID_ARG_VALUE = _require$codes.ERR_INVALID_ARG_VALUE,
    ERR_INVALID_FILE_URL_HOST = _require$codes.ERR_INVALID_FILE_URL_HOST,
    ERR_INVALID_FILE_URL_PATH = _require$codes.ERR_INVALID_FILE_URL_PATH,
    ERR_INVALID_THIS = _require$codes.ERR_INVALID_THIS,
    ERR_INVALID_TUPLE = _require$codes.ERR_INVALID_TUPLE,
    ERR_INVALID_URL = _require$codes.ERR_INVALID_URL,
    ERR_INVALID_URL_SCHEME = _require$codes.ERR_INVALID_URL_SCHEME,
    ERR_MISSING_ARGS = _require$codes.ERR_MISSING_ARGS;

var _require4 = require('internal/constants'),
    CHAR_AMPERSAND = _require4.CHAR_AMPERSAND,
    CHAR_BACKWARD_SLASH = _require4.CHAR_BACKWARD_SLASH,
    CHAR_EQUAL = _require4.CHAR_EQUAL,
    CHAR_FORWARD_SLASH = _require4.CHAR_FORWARD_SLASH,
    CHAR_LOWERCASE_A = _require4.CHAR_LOWERCASE_A,
    CHAR_LOWERCASE_Z = _require4.CHAR_LOWERCASE_Z,
    CHAR_PERCENT = _require4.CHAR_PERCENT,
    CHAR_PLUS = _require4.CHAR_PLUS;

var path = require('path');

var _require5 = require('internal/validators'),
    validateCallback = _require5.validateCallback,
    validateObject = _require5.validateObject;

// var querystring = require('querystring');

// var _process = process,
//     platform = _process.platform;
// var isWindows = platform === 'win32';

// var _internalBinding = internalBinding('url'),
//     _domainToASCII = _internalBinding.domainToASCII,
//     _domainToUnicode = _internalBinding.domainToUnicode,
//     encodeAuth = _internalBinding.encodeAuth,
//     _toUSVString = _internalBinding.toUSVString,
//     parse = _internalBinding.parse,
//     setURLConstructor = _internalBinding.setURLConstructor,
//     URL_FLAGS_CANNOT_BE_BASE = _internalBinding.URL_FLAGS_CANNOT_BE_BASE,
//     URL_FLAGS_HAS_FRAGMENT = _internalBinding.URL_FLAGS_HAS_FRAGMENT,
//     URL_FLAGS_HAS_HOST = _internalBinding.URL_FLAGS_HAS_HOST,
//     URL_FLAGS_HAS_PASSWORD = _internalBinding.URL_FLAGS_HAS_PASSWORD,
//     URL_FLAGS_HAS_PATH = _internalBinding.URL_FLAGS_HAS_PATH,
//     URL_FLAGS_HAS_QUERY = _internalBinding.URL_FLAGS_HAS_QUERY,
//     URL_FLAGS_HAS_USERNAME = _internalBinding.URL_FLAGS_HAS_USERNAME,
//     URL_FLAGS_IS_DEFAULT_SCHEME_PORT = _internalBinding.URL_FLAGS_IS_DEFAULT_SCHEME_PORT,
//     URL_FLAGS_SPECIAL = _internalBinding.URL_FLAGS_SPECIAL,
//     kFragment = _internalBinding.kFragment,
//     kHost = _internalBinding.kHost,
//     kHostname = _internalBinding.kHostname,
//     kPathStart = _internalBinding.kPathStart,
//     kPort = _internalBinding.kPort,
//     kQuery = _internalBinding.kQuery,
//     kSchemeStart = _internalBinding.kSchemeStart;

var context = _Symbol('context');

var cannotBeBase = _Symbol('cannot-be-base');

var cannotHaveUsernamePasswordPort = _Symbol('cannot-have-username-password-port');

var special = _Symbol('special');

var searchParams = _Symbol('query');

var kFormat = _Symbol('format'); // https://tc39.github.io/ecma262/#sec-%iteratorprototype%-object


var IteratorPrototype = ObjectGetPrototypeOf(ObjectGetPrototypeOf([][SymbolIterator]()));
var unpairedSurrogateRe = /(?:[^\uD800-\uDBFF]|^)[\uDC00-\uDFFF]|[\uD800-\uDBFF](?![\uDC00-\uDFFF])/;

function toUSVString(val) {
  var str = "".concat(val); // As of V8 5.5, `str.search()` (and `unpairedSurrogateRe[@@search]()`) are
  // slower than `unpairedSurrogateRe.exec()`.

  var match = RegExpPrototypeExec(unpairedSurrogateRe, str);
  if (!match) return str;
  return _toUSVString(str, match.index);
} // Refs: https://html.spec.whatwg.org/multipage/browsers.html#concept-origin-opaque


var kOpaqueOrigin = 'null'; // Refs: https://html.spec.whatwg.org/multipage/browsers.html#ascii-serialisation-of-an-origin

function serializeTupleOrigin(scheme, host, port) {
  return "".concat(scheme, "//").concat(host).concat(port === null ? '' : ":".concat(port));
} // This class provides the internal state of a URL object. An instance of this
// class is stored in every URL object and is accessed internally by setters
// and getters. It roughly corresponds to the concept of a URL record in the
// URL Standard, with a few differences. It is also the object transported to
// the C++ binding.
// Refs: https://url.spec.whatwg.org/#concept-url


var URLContext = function URLContext() {
  _classCallCheck(this, URLContext);

  this.flags = 0;
  this.scheme = ':';
  this.username = '';
  this.password = '';
  this.host = null;
  this.port = null;
  this.path = [];
  this.query = null;
  this.fragment = null;
};

var URLSearchParams = /*#__PURE__*/function () {
  // URL Standard says the default value is '', but as undefined and '' have
  // the same result, undefined is used to prevent unnecessary parsing.
  // Default parameter is necessary to keep URLSearchParams.length === 0 in
  // accordance with Web IDL spec.
  function URLSearchParams() {
    var init = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : undefined;

    _classCallCheck(this, URLSearchParams);

    if (init === null || init === undefined) {
      this[searchParams] = [];
    } else if (_typeof(init) === 'object' || typeof init === 'function') {
      var method = init[SymbolIterator];

      if (method === this[SymbolIterator]) {
        // While the spec does not have this branch, we can use it as a
        // shortcut to avoid having to go through the costly generic iterator.
        var childParams = init[searchParams];
        this[searchParams] = childParams.slice();
      } else if (method !== null && method !== undefined) {
        if (typeof method !== 'function') {
          throw new ERR_ARG_NOT_ITERABLE('Query pairs');
        } // Sequence<sequence<USVString>>
        // Note: per spec we have to first exhaust the lists then process them


        var pairs = [];

        var _iterator = _createForOfIteratorHelper(init),
            _step;

        try {
          for (_iterator.s(); !(_step = _iterator.n()).done;) {
            var _pair = _step.value;

            if (_typeof(_pair) !== 'object' && typeof _pair !== 'function' || _pair === null || typeof _pair[SymbolIterator] !== 'function') {
              throw new ERR_INVALID_TUPLE('Each query pair', '[name, value]');
            }

            var convertedPair = [];

            var _iterator2 = _createForOfIteratorHelper(_pair),
                _step2;

            try {
              for (_iterator2.s(); !(_step2 = _iterator2.n()).done;) {
                var element = _step2.value;
                ArrayPrototypePush(convertedPair, toUSVString(element));
              }
            } catch (err) {
              _iterator2.e(err);
            } finally {
              _iterator2.f();
            }

            ArrayPrototypePush(pairs, convertedPair);
          }
        } catch (err) {
          _iterator.e(err);
        } finally {
          _iterator.f();
        }

        this[searchParams] = [];

        for (var _i = 0, _pairs = pairs; _i < _pairs.length; _i++) {
          var pair = _pairs[_i];

          if (pair.length !== 2) {
            throw new ERR_INVALID_TUPLE('Each query pair', '[name, value]');
          }

          ArrayPrototypePush(this[searchParams], pair[0], pair[1]);
        }
      } else {
        // Record<USVString, USVString>
        // Need to use reflection APIs for full spec compliance.
        this[searchParams] = [];
        var keys = ReflectOwnKeys(init);

        for (var i = 0; i < keys.length; i++) {
          var key = keys[i];
          var desc = ReflectGetOwnPropertyDescriptor(init, key);

          if (desc !== undefined && desc.enumerable) {
            var typedKey = toUSVString(key);
            var typedValue = toUSVString(init[key]);
            this[searchParams].push(typedKey, typedValue);
          }
        }
      }
    } else {
      // USVString
      init = toUSVString(init);
      if (init[0] === '?') init = init.slice(1);
      initSearchParams(this, init);
    } // "associated url object"


    this[context] = null;
  }

  _createClass(URLSearchParams, [{
    key: inspect.custom,
    value: function value(recurseTimes, ctx) {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      if (typeof recurseTimes === 'number' && recurseTimes < 0) return ctx.stylize('[Object]', 'special');
      var separator = ', ';

      var innerOpts = _objectSpread({}, ctx);

      if (recurseTimes !== null) {
        innerOpts.depth = recurseTimes - 1;
      }

      var innerInspect = function innerInspect(v) {
        return inspect(v, innerOpts);
      };

      var list = this[searchParams];
      var output = [];

      for (var i = 0; i < list.length; i += 2) {
        ArrayPrototypePush(output, "".concat(innerInspect(list[i]), " => ").concat(innerInspect(list[i + 1])));
      }

      var length = ArrayPrototypeReduce(output, function (prev, cur) {
        return prev + removeColors(cur).length + separator.length;
      }, -separator.length);

      if (length > ctx.breakLength) {
        return "".concat(this.constructor.name, " {\n") + "  ".concat(ArrayPrototypeJoin(output, ',\n  '), " }");
      } else if (output.length) {
        return "".concat(this.constructor.name, " { ") + "".concat(ArrayPrototypeJoin(output, separator), " }");
      }

      return "".concat(this.constructor.name, " {}");
    }
  }, {
    key: "append",
    value: function append(name, value) {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      if (arguments.length < 2) {
        throw new ERR_MISSING_ARGS('name', 'value');
      }

      name = toUSVString(name);
      value = toUSVString(value);
      ArrayPrototypePush(this[searchParams], name, value);
      update(this[context], this);
    }
  }, {
    key: "delete",
    value: function _delete(name) {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      if (arguments.length < 1) {
        throw new ERR_MISSING_ARGS('name');
      }

      var list = this[searchParams];
      name = toUSVString(name);

      for (var i = 0; i < list.length;) {
        var cur = list[i];

        if (cur === name) {
          list.splice(i, 2);
        } else {
          i += 2;
        }
      }

      update(this[context], this);
    }
  }, {
    key: "get",
    value: function get(name) {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      if (arguments.length < 1) {
        throw new ERR_MISSING_ARGS('name');
      }

      var list = this[searchParams];
      name = toUSVString(name);

      for (var i = 0; i < list.length; i += 2) {
        if (list[i] === name) {
          return list[i + 1];
        }
      }

      return null;
    }
  }, {
    key: "getAll",
    value: function getAll(name) {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      if (arguments.length < 1) {
        throw new ERR_MISSING_ARGS('name');
      }

      var list = this[searchParams];
      var values = [];
      name = toUSVString(name);

      for (var i = 0; i < list.length; i += 2) {
        if (list[i] === name) {
          values.push(list[i + 1]);
        }
      }

      return values;
    }
  }, {
    key: "has",
    value: function has(name) {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      if (arguments.length < 1) {
        throw new ERR_MISSING_ARGS('name');
      }

      var list = this[searchParams];
      name = toUSVString(name);

      for (var i = 0; i < list.length; i += 2) {
        if (list[i] === name) {
          return true;
        }
      }

      return false;
    }
  }, {
    key: "set",
    value: function set(name, value) {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      if (arguments.length < 2) {
        throw new ERR_MISSING_ARGS('name', 'value');
      }

      var list = this[searchParams];
      name = toUSVString(name);
      value = toUSVString(value); // If there are any name-value pairs whose name is `name`, in `list`, set
      // the value of the first such name-value pair to `value` and remove the
      // others.

      var found = false;

      for (var i = 0; i < list.length;) {
        var cur = list[i];

        if (cur === name) {
          if (!found) {
            list[i + 1] = value;
            found = true;
            i += 2;
          } else {
            list.splice(i, 2);
          }
        } else {
          i += 2;
        }
      } // Otherwise, append a new name-value pair whose name is `name` and value
      // is `value`, to `list`.


      if (!found) {
        ArrayPrototypePush(list, name, value);
      }

      update(this[context], this);
    }
  }, {
    key: "sort",
    value: function sort() {
      var a = this[searchParams];
      var len = a.length;

      if (len <= 2) {// Nothing needs to be done.
      } else if (len < 100) {
        // 100 is found through testing.
        // Simple stable in-place insertion sort
        // Derived from v8/src/js/array.js
        for (var i = 2; i < len; i += 2) {
          var curKey = a[i];
          var curVal = a[i + 1];
          var j = void 0;

          for (j = i - 2; j >= 0; j -= 2) {
            if (a[j] > curKey) {
              a[j + 2] = a[j];
              a[j + 3] = a[j + 1];
            } else {
              break;
            }
          }

          a[j + 2] = curKey;
          a[j + 3] = curVal;
        }
      } else {
        // Bottom-up iterative stable merge sort
        var lBuffer = new _Array(len);
        var rBuffer = new _Array(len);

        for (var step = 2; step < len; step *= 2) {
          for (var start = 0; start < len - 2; start += 2 * step) {
            var mid = start + step;
            var end = mid + step;
            end = end < len ? end : len;
            if (mid > end) continue;
            merge(a, start, mid, end, lBuffer, rBuffer);
          }
        }
      }

      update(this[context], this);
    } // https://heycam.github.io/webidl/#es-iterators
    // Define entries here rather than [Symbol.iterator] as the function name
    // must be set to `entries`.

  }, {
    key: "entries",
    value: function entries() {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      return createSearchParamsIterator(this, 'key+value');
    }
  }, {
    key: "forEach",
    value: function forEach(callback) {
      var thisArg = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : undefined;

      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      validateCallback(callback);
      var list = this[searchParams];
      var i = 0;

      while (i < list.length) {
        var key = list[i];
        var value = list[i + 1];
        callback.call(thisArg, value, key, this); // In case the URL object's `search` is updated

        list = this[searchParams];
        i += 2;
      }
    } // https://heycam.github.io/webidl/#es-iterable

  }, {
    key: "keys",
    value: function keys() {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      return createSearchParamsIterator(this, 'key');
    }
  }, {
    key: "values",
    value: function values() {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      return createSearchParamsIterator(this, 'value');
    } // https://heycam.github.io/webidl/#es-stringifier
    // https://url.spec.whatwg.org/#urlsearchparams-stringification-behavior

  }, {
    key: "toString",
    value: function toString() {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new ERR_INVALID_THIS('URLSearchParams');
      }

      return serializeParams(this[searchParams]);
    }
  }]);

  return URLSearchParams;
}();

ObjectDefineProperties(URLSearchParams.prototype, (_ObjectDefineProperti = {
  append: {
    enumerable: true
  },
  "delete": {
    enumerable: true
  },
  get: {
    enumerable: true
  },
  getAll: {
    enumerable: true
  },
  has: {
    enumerable: true
  },
  set: {
    enumerable: true
  },
  sort: {
    enumerable: true
  },
  entries: {
    enumerable: true
  },
  forEach: {
    enumerable: true
  },
  keys: {
    enumerable: true
  },
  values: {
    enumerable: true
  },
  toString: {
    enumerable: true
  }
}, _defineProperty(_ObjectDefineProperti, SymbolToStringTag, {
  configurable: true,
  value: 'URLSearchParams'
}), _defineProperty(_ObjectDefineProperti, SymbolIterator, {
  configurable: true,
  writable: true,
  value: URLSearchParams.prototype.entries
}), _ObjectDefineProperti));

function onParseComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = this[context];
  ctx.flags = flags;
  ctx.scheme = protocol;
  ctx.username = (flags & URL_FLAGS_HAS_USERNAME) !== 0 ? username : '';
  ctx.password = (flags & URL_FLAGS_HAS_PASSWORD) !== 0 ? password : '';
  ctx.port = port;
  ctx.path = (flags & URL_FLAGS_HAS_PATH) !== 0 ? path : [];
  ctx.query = query;
  ctx.fragment = fragment;
  ctx.host = host;

  if (!this[searchParams]) {
    // Invoked from URL constructor
    this[searchParams] = new URLSearchParams();
    this[searchParams][context] = this;
  }

  initSearchParams(this[searchParams], query);
}

function onParseError(flags, input) {
  throw new ERR_INVALID_URL(input);
}

function onParseProtocolComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = this[context];

  if ((flags & URL_FLAGS_SPECIAL) !== 0) {
    ctx.flags |= URL_FLAGS_SPECIAL;
  } else {
    ctx.flags &= ~URL_FLAGS_SPECIAL;
  }

  ctx.scheme = protocol;
  ctx.port = port;
}

function onParseHostnameComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = this[context];

  if ((flags & URL_FLAGS_HAS_HOST) !== 0) {
    ctx.host = host;
    ctx.flags |= URL_FLAGS_HAS_HOST;
  } else {
    ctx.host = null;
    ctx.flags &= ~URL_FLAGS_HAS_HOST;
  }
}

function onParsePortComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  this[context].port = port;
}

function onParseHostComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  ReflectApply(onParseHostnameComplete, this, arguments);
  if (port !== null || (flags & URL_FLAGS_IS_DEFAULT_SCHEME_PORT) !== 0) ReflectApply(onParsePortComplete, this, arguments);
}

function onParsePathComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = this[context];

  if ((flags & URL_FLAGS_HAS_PATH) !== 0) {
    ctx.path = path;
    ctx.flags |= URL_FLAGS_HAS_PATH;
  } else {
    ctx.path = [];
    ctx.flags &= ~URL_FLAGS_HAS_PATH;
  } // The C++ binding may set host to empty string.


  if ((flags & URL_FLAGS_HAS_HOST) !== 0) {
    ctx.host = host;
    ctx.flags |= URL_FLAGS_HAS_HOST;
  }
}

function onParseSearchComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  this[context].query = query;
}

function onParseHashComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  this[context].fragment = fragment;
}

var URL = /*#__PURE__*/function () {
  function URL(input, base) {
    _classCallCheck(this, URL);

    // toUSVString is not needed.
    input = "".concat(input);
    var base_context;

    if (base !== undefined) {
      base_context = new URL(base)[context];
    }

    this[context] = new URLContext();
    parse(input, -1, base_context, undefined, FunctionPrototypeBind(onParseComplete, this), onParseError);
  }

  _createClass(URL, [{
    key: special,
    get: function get() {
      return (this[context].flags & URL_FLAGS_SPECIAL) !== 0;
    }
  }, {
    key: cannotBeBase,
    get: function get() {
      return (this[context].flags & URL_FLAGS_CANNOT_BE_BASE) !== 0;
    } // https://url.spec.whatwg.org/#cannot-have-a-username-password-port

  }, {
    key: cannotHaveUsernamePasswordPort,
    get: function get() {
      var _this$context = this[context],
          host = _this$context.host,
          scheme = _this$context.scheme;
      return host == null || host === '' || this[cannotBeBase] || scheme === 'file:';
    }
  }, {
    key: inspect.custom,
    value: function value(depth, opts) {
      if (this == null || ObjectGetPrototypeOf(this[context]) !== URLContext.prototype) {
        throw new ERR_INVALID_THIS('URL');
      }

      if (typeof depth === 'number' && depth < 0) return this;
      var constructor = getConstructorOf(this) || URL;
      var obj = ObjectCreate({
        constructor: constructor
      });
      obj.href = this.href;
      obj.origin = this.origin;
      obj.protocol = this.protocol;
      obj.username = this.username;
      obj.password = this.password;
      obj.host = this.host;
      obj.hostname = this.hostname;
      obj.port = this.port;
      obj.pathname = this.pathname;
      obj.search = this.search;
      obj.searchParams = this.searchParams;
      obj.hash = this.hash;

      if (opts.showHidden) {
        obj.cannotBeBase = this[cannotBeBase];
        obj.special = this[special];
        obj[context] = this[context];
      }

      return "".concat(constructor.name, " ").concat(inspect(obj, opts));
    }
  }, {
    key: kFormat,
    value: function value(options) {
      if (options) validateObject(options, 'options');
      options = _objectSpread({
        fragment: true,
        unicode: false,
        search: true,
        auth: true
      }, options);
      var ctx = this[context]; // https://url.spec.whatwg.org/#url-serializing

      var ret = ctx.scheme;

      if (ctx.host !== null) {
        ret += '//';
        var has_username = ctx.username !== '';
        var has_password = ctx.password !== '';

        if (options.auth && (has_username || has_password)) {
          if (has_username) ret += ctx.username;
          if (has_password) ret += ":".concat(ctx.password);
          ret += '@';
        }

        ret += options.unicode ? domainToUnicode(ctx.host) : ctx.host;
        if (ctx.port !== null) ret += ":".concat(ctx.port);
      }

      if (this[cannotBeBase]) {
        ret += ctx.path[0];
      } else {
        if (ctx.host === null && ctx.path.length > 1 && ctx.path[0] === '') {
          ret += '/.';
        }

        if (ctx.path.length) {
          ret += '/' + ArrayPrototypeJoin(ctx.path, '/');
        }
      }

      if (options.search && ctx.query !== null) ret += "?".concat(ctx.query);
      if (options.fragment && ctx.fragment !== null) ret += "#".concat(ctx.fragment);
      return ret;
    } // https://heycam.github.io/webidl/#es-stringifier

  }, {
    key: "toString",
    value: function toString() {
      return this[kFormat]({});
    }
  }, {
    key: "href",
    get: function get() {
      return this[kFormat]({});
    }
  }, {
    key: "href",
    set: function set(input) {
      // toUSVString is not needed.
      input = "".concat(input);
      parse(input, -1, undefined, undefined, FunctionPrototypeBind(onParseComplete, this), onParseError);
    } // readonly

  }, {
    key: "origin",
    get: function get() {
      // Refs: https://url.spec.whatwg.org/#concept-url-origin
      var ctx = this[context];

      switch (ctx.scheme) {
        case 'blob:':
          if (ctx.path.length > 0) {
            try {
              return new URL(ctx.path[0]).origin;
            } catch (_unused) {// Fall through... do nothing
            }
          }

          return kOpaqueOrigin;

        case 'ftp:':
        case 'http:':
        case 'https:':
        case 'ws:':
        case 'wss:':
          return serializeTupleOrigin(ctx.scheme, ctx.host, ctx.port);
      }

      return kOpaqueOrigin;
    }
  }, {
    key: "protocol",
    get: function get() {
      return this[context].scheme;
    }
  }, {
    key: "protocol",
    set: function set(scheme) {
      // toUSVString is not needed.
      scheme = "".concat(scheme);
      if (scheme.length === 0) return;
      var ctx = this[context];
      parse(scheme, kSchemeStart, null, ctx, FunctionPrototypeBind(onParseProtocolComplete, this));
    }
  }, {
    key: "username",
    get: function get() {
      return this[context].username;
    }
  }, {
    key: "username",
    set: function set(username) {
      // toUSVString is not needed.
      username = "".concat(username);
      if (this[cannotHaveUsernamePasswordPort]) return;
      var ctx = this[context];

      if (username === '') {
        ctx.username = '';
        ctx.flags &= ~URL_FLAGS_HAS_USERNAME;
        return;
      }

      ctx.username = encodeAuth(username);
      ctx.flags |= URL_FLAGS_HAS_USERNAME;
    }
  }, {
    key: "password",
    get: function get() {
      return this[context].password;
    }
  }, {
    key: "password",
    set: function set(password) {
      // toUSVString is not needed.
      password = "".concat(password);
      if (this[cannotHaveUsernamePasswordPort]) return;
      var ctx = this[context];

      if (password === '') {
        ctx.password = '';
        ctx.flags &= ~URL_FLAGS_HAS_PASSWORD;
        return;
      }

      ctx.password = encodeAuth(password);
      ctx.flags |= URL_FLAGS_HAS_PASSWORD;
    }
  }, {
    key: "host",
    get: function get() {
      var ctx = this[context];
      var ret = ctx.host || '';
      if (ctx.port !== null) ret += ":".concat(ctx.port);
      return ret;
    }
  }, {
    key: "host",
    set: function set(host) {
      var ctx = this[context]; // toUSVString is not needed.

      host = "".concat(host);

      if (this[cannotBeBase]) {
        // Cannot set the host if cannot-be-base is set
        return;
      }

      parse(host, kHost, null, ctx, FunctionPrototypeBind(onParseHostComplete, this));
    }
  }, {
    key: "hostname",
    get: function get() {
      return this[context].host || '';
    }
  }, {
    key: "hostname",
    set: function set(host) {
      var ctx = this[context]; // toUSVString is not needed.

      host = "".concat(host);

      if (this[cannotBeBase]) {
        // Cannot set the host if cannot-be-base is set
        return;
      }

      parse(host, kHostname, null, ctx, onParseHostnameComplete.bind(this));
    }
  }, {
    key: "port",
    get: function get() {
      var port = this[context].port;
      return port === null ? '' : String(port);
    }
  }, {
    key: "port",
    set: function set(port) {
      // toUSVString is not needed.
      port = "".concat(port);
      if (this[cannotHaveUsernamePasswordPort]) return;
      var ctx = this[context];

      if (port === '') {
        ctx.port = null;
        return;
      }

      parse(port, kPort, null, ctx, FunctionPrototypeBind(onParsePortComplete, this));
    }
  }, {
    key: "pathname",
    get: function get() {
      var ctx = this[context];
      if (this[cannotBeBase]) return ctx.path[0];
      if (ctx.path.length === 0) return '';
      return "/".concat(ArrayPrototypeJoin(ctx.path, '/'));
    }
  }, {
    key: "pathname",
    set: function set(path) {
      // toUSVString is not needed.
      path = "".concat(path);
      if (this[cannotBeBase]) return;
      parse(path, kPathStart, null, this[context], onParsePathComplete.bind(this));
    }
  }, {
    key: "search",
    get: function get() {
      var query = this[context].query;
      if (query === null || query === '') return '';
      return "?".concat(query);
    }
  }, {
    key: "search",
    set: function set(search) {
      var ctx = this[context];
      search = toUSVString(search);

      if (search === '') {
        ctx.query = null;
        ctx.flags &= ~URL_FLAGS_HAS_QUERY;
      } else {
        if (search[0] === '?') search = StringPrototypeSlice(search, 1);
        ctx.query = '';
        ctx.flags |= URL_FLAGS_HAS_QUERY;

        if (search) {
          parse(search, kQuery, null, ctx, FunctionPrototypeBind(onParseSearchComplete, this));
        }
      }

      initSearchParams(this[searchParams], search);
    } // readonly

  }, {
    key: "searchParams",
    get: function get() {
      return this[searchParams];
    }
  }, {
    key: "hash",
    get: function get() {
      var fragment = this[context].fragment;
      if (fragment === null || fragment === '') return '';
      return "#".concat(fragment);
    }
  }, {
    key: "hash",
    set: function set(hash) {
      var ctx = this[context]; // toUSVString is not needed.

      hash = "".concat(hash);

      if (!hash) {
        ctx.fragment = null;
        ctx.flags &= ~URL_FLAGS_HAS_FRAGMENT;
        return;
      }

      if (hash[0] === '#') hash = StringPrototypeSlice(hash, 1);
      ctx.fragment = '';
      ctx.flags |= URL_FLAGS_HAS_FRAGMENT;
      parse(hash, kFragment, null, ctx, FunctionPrototypeBind(onParseHashComplete, this));
    }
  }, {
    key: "toJSON",
    value: function toJSON() {
      return this[kFormat]({});
    }
  }]);

  return URL;
}();

ObjectDefineProperties(URL.prototype, (_ObjectDefineProperti2 = {}, _defineProperty(_ObjectDefineProperti2, kFormat, {
  configurable: false,
  writable: false
}), _defineProperty(_ObjectDefineProperti2, SymbolToStringTag, {
  configurable: true,
  value: 'URL'
}), _defineProperty(_ObjectDefineProperti2, "toString", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "href", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "origin", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "protocol", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "username", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "password", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "host", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "hostname", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "port", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "pathname", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "search", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "searchParams", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "hash", {
  enumerable: true
}), _defineProperty(_ObjectDefineProperti2, "toJSON", {
  enumerable: true
}), _ObjectDefineProperti2));

function update(url, params) {
  if (!url) return;
  var ctx = url[context];
  var serializedParams = params.toString();

  if (serializedParams) {
    ctx.query = serializedParams;
    ctx.flags |= URL_FLAGS_HAS_QUERY;
  } else {
    ctx.query = null;
    ctx.flags &= ~URL_FLAGS_HAS_QUERY;
  }
}

function initSearchParams(url, init) {
  if (!init) {
    url[searchParams] = [];
    return;
  }

  url[searchParams] = parseParams(init);
} // application/x-www-form-urlencoded parser
// Ref: https://url.spec.whatwg.org/#concept-urlencoded-parser


function parseParams(qs) {
  var out = [];
  var pairStart = 0;
  var lastPos = 0;
  var seenSep = false;
  var buf = '';
  var encoded = false;
  var encodeCheck = 0;
  var i;

  for (i = 0; i < qs.length; ++i) {
    var code = StringPrototypeCharCodeAt(qs, i); // Try matching key/value pair separator

    if (code === CHAR_AMPERSAND) {
      if (pairStart === i) {
        // We saw an empty substring between pair separators
        lastPos = pairStart = i + 1;
        continue;
      }

      if (lastPos < i) buf += qs.slice(lastPos, i);
      if (encoded) buf = querystring.unescape(buf);
      out.push(buf); // If `buf` is the key, add an empty value.

      if (!seenSep) out.push('');
      seenSep = false;
      buf = '';
      encoded = false;
      encodeCheck = 0;
      lastPos = pairStart = i + 1;
      continue;
    } // Try matching key/value separator (e.g. '=') if we haven't already


    if (!seenSep && code === CHAR_EQUAL) {
      // Key/value separator match!
      if (lastPos < i) buf += qs.slice(lastPos, i);
      if (encoded) buf = querystring.unescape(buf);
      out.push(buf);
      seenSep = true;
      buf = '';
      encoded = false;
      encodeCheck = 0;
      lastPos = i + 1;
      continue;
    } // Handle + and percent decoding.


    if (code === CHAR_PLUS) {
      if (lastPos < i) buf += StringPrototypeSlice(qs, lastPos, i);
      buf += ' ';
      lastPos = i + 1;
    } else if (!encoded) {
      // Try to match an (valid) encoded byte (once) to minimize unnecessary
      // calls to string decoding functions
      if (code === CHAR_PERCENT) {
        encodeCheck = 1;
      } else if (encodeCheck > 0) {
        if (isHexTable[code] === 1) {
          if (++encodeCheck === 3) {
            encoded = true;
          }
        } else {
          encodeCheck = 0;
        }
      }
    }
  } // Deal with any leftover key or value data
  // There is a trailing &. No more processing is needed.


  if (pairStart === i) return out;
  if (lastPos < i) buf += StringPrototypeSlice(qs, lastPos, i);
  if (encoded) buf = querystring.unescape(buf);
  ArrayPrototypePush(out, buf); // If `buf` is the key, add an empty value.

  if (!seenSep) ArrayPrototypePush(out, '');
  return out;
} // Adapted from querystring's implementation.
// Ref: https://url.spec.whatwg.org/#concept-urlencoded-byte-serializer


var noEscape = new Int8Array([
/*
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
*/
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, // 0x20 - 0x2F
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 0x30 - 0x3F
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // 0x50 - 0x5F
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 // 0x70 - 0x7F
]); // Special version of hexTable that uses `+` for U+0020 SPACE.

// var paramHexTable = hexTable.slice();
// paramHexTable[0x20] = '+'; // application/x-www-form-urlencoded serializer
// Ref: https://url.spec.whatwg.org/#concept-urlencoded-serializer

function serializeParams(array) {
  var len = array.length;
  if (len === 0) return '';
  var firstEncodedParam = encodeStr(array[0], noEscape, paramHexTable);
  var firstEncodedValue = encodeStr(array[1], noEscape, paramHexTable);
  var output = "".concat(firstEncodedParam, "=").concat(firstEncodedValue);

  for (var i = 2; i < len; i += 2) {
    var encodedParam = encodeStr(array[i], noEscape, paramHexTable);
    var encodedValue = encodeStr(array[i + 1], noEscape, paramHexTable);
    output += "&".concat(encodedParam, "=").concat(encodedValue);
  }

  return output;
} // Mainly to mitigate func-name-matching ESLint rule


function defineIDLClass(proto, classStr, obj) {
  // https://heycam.github.io/webidl/#dfn-class-string
  ObjectDefineProperty(proto, SymbolToStringTag, {
    writable: false,
    enumerable: false,
    configurable: true,
    value: classStr
  }); // https://heycam.github.io/webidl/#es-operations

  var _iterator3 = _createForOfIteratorHelper(ObjectKeys(obj)),
      _step3;

  try {
    for (_iterator3.s(); !(_step3 = _iterator3.n()).done;) {
      var key = _step3.value;
      ObjectDefineProperty(proto, key, {
        writable: true,
        enumerable: true,
        configurable: true,
        value: obj[key]
      });
    }
  } catch (err) {
    _iterator3.e(err);
  } finally {
    _iterator3.f();
  }

  var _iterator4 = _createForOfIteratorHelper(ObjectGetOwnPropertySymbols(obj)),
      _step4;

  try {
    for (_iterator4.s(); !(_step4 = _iterator4.n()).done;) {
      var _key = _step4.value;
      ObjectDefineProperty(proto, _key, {
        writable: true,
        enumerable: false,
        configurable: true,
        value: obj[_key]
      });
    }
  } catch (err) {
    _iterator4.e(err);
  } finally {
    _iterator4.f();
  }
} // for merge sort


function merge(out, start, mid, end, lBuffer, rBuffer) {
  var sizeLeft = mid - start;
  var sizeRight = end - mid;
  var l, r, o;

  for (l = 0; l < sizeLeft; l++) {
    lBuffer[l] = out[start + l];
  }

  for (r = 0; r < sizeRight; r++) {
    rBuffer[r] = out[mid + r];
  }

  l = 0;
  r = 0;
  o = start;

  while (l < sizeLeft && r < sizeRight) {
    if (lBuffer[l] <= rBuffer[r]) {
      out[o++] = lBuffer[l++];
      out[o++] = lBuffer[l++];
    } else {
      out[o++] = rBuffer[r++];
      out[o++] = rBuffer[r++];
    }
  }

  while (l < sizeLeft) {
    out[o++] = lBuffer[l++];
  }

  while (r < sizeRight) {
    out[o++] = rBuffer[r++];
  }
} // https://heycam.github.io/webidl/#dfn-default-iterator-object


function createSearchParamsIterator(target, kind) {
  var iterator = ObjectCreate(URLSearchParamsIteratorPrototype);
  iterator[context] = {
    target: target,
    kind: kind,
    index: 0
  };
  return iterator;
} // https://heycam.github.io/webidl/#dfn-iterator-prototype-object


var URLSearchParamsIteratorPrototype = ObjectCreate(IteratorPrototype);
defineIDLClass(URLSearchParamsIteratorPrototype, 'URLSearchParams Iterator', _defineProperty({
  next: function next() {
    if (!this || ObjectGetPrototypeOf(this) !== URLSearchParamsIteratorPrototype) {
      throw new ERR_INVALID_THIS('URLSearchParamsIterator');
    }

    var _this$context2 = this[context],
        target = _this$context2.target,
        kind = _this$context2.kind,
        index = _this$context2.index;
    var values = target[searchParams];
    var len = values.length;

    if (index >= len) {
      return {
        value: undefined,
        done: true
      };
    }

    var name = values[index];
    var value = values[index + 1];
    this[context].index = index + 2;
    var result;

    if (kind === 'key') {
      result = name;
    } else if (kind === 'value') {
      result = value;
    } else {
      result = [name, value];
    }

    return {
      value: result,
      done: false
    };
  }
}, inspect.custom, function (recurseTimes, ctx) {
  if (this == null || this[context] == null || this[context].target == null) throw new ERR_INVALID_THIS('URLSearchParamsIterator');
  if (typeof recurseTimes === 'number' && recurseTimes < 0) return ctx.stylize('[Object]', 'special');

  var innerOpts = _objectSpread({}, ctx);

  if (recurseTimes !== null) {
    innerOpts.depth = recurseTimes - 1;
  }

  var _this$context3 = this[context],
      target = _this$context3.target,
      kind = _this$context3.kind,
      index = _this$context3.index;
  var output = ArrayPrototypeReduce(ArrayPrototypeSlice(target[searchParams], index), function (prev, cur, i) {
    var key = i % 2 === 0;

    if (kind === 'key' && key) {
      ArrayPrototypePush(prev, cur);
    } else if (kind === 'value' && !key) {
      ArrayPrototypePush(prev, cur);
    } else if (kind === 'key+value' && !key) {
      ArrayPrototypePush(prev, [target[searchParams][index + i - 1], cur]);
    }

    return prev;
  }, []);
  var breakLn = inspect(output, innerOpts).includes('\n');
  var outputStrs = ArrayPrototypeMap(output, function (p) {
    return inspect(p, innerOpts);
  });
  var outputStr;

  if (breakLn) {
    outputStr = "\n  ".concat(ArrayPrototypeJoin(outputStrs, ',\n  '));
  } else {
    outputStr = " ".concat(ArrayPrototypeJoin(outputStrs, ', '));
  }

  return "".concat(this[SymbolToStringTag], " {").concat(outputStr, " }");
}));

function domainToASCII(domain) {
  if (arguments.length < 1) throw new ERR_MISSING_ARGS('domain'); // toUSVString is not needed.

  return _domainToASCII("".concat(domain));
}

function domainToUnicode(domain) {
  if (arguments.length < 1) throw new ERR_MISSING_ARGS('domain'); // toUSVString is not needed.

  return _domainToUnicode("".concat(domain));
} // Utility function that converts a URL object into an ordinary
// options object as expected by the http.request and https.request
// APIs.


function urlToHttpOptions(url) {
  var options = {
    protocol: url.protocol,
    hostname: typeof url.hostname === 'string' && StringPrototypeStartsWith(url.hostname, '[') ? StringPrototypeSlice(url.hostname, 1, -1) : url.hostname,
    hash: url.hash,
    search: url.search,
    pathname: url.pathname,
    path: "".concat(url.pathname || '').concat(url.search || ''),
    href: url.href
  };

  if (url.port !== '') {
    options.port = Number(url.port);
  }

  if (url.username || url.password) {
    options.auth = "".concat(url.username, ":").concat(url.password);
  }

  return options;
}

var forwardSlashRegEx = /\//g;

function getPathFromURLWin32(url) {
  var hostname = url.hostname;
  var pathname = url.pathname;

  for (var n = 0; n < pathname.length; n++) {
    if (pathname[n] === '%') {
      var third = pathname.codePointAt(n + 2) | 0x20;

      if (pathname[n + 1] === '2' && third === 102 || // 2f 2F /
      pathname[n + 1] === '5' && third === 99) {
        // 5c 5C \
        throw new ERR_INVALID_FILE_URL_PATH('must not include encoded \\ or / characters');
      }
    }
  }

  pathname = pathname.replace(forwardSlashRegEx, '\\');
  pathname = decodeURIComponent(pathname);

  if (hostname !== '') {
    // If hostname is set, then we have a UNC path
    // Pass the hostname through domainToUnicode just in case
    // it is an IDN using punycode encoding. We do not need to worry
    // about percent encoding because the URL parser will have
    // already taken care of that for us. Note that this only
    // causes IDNs with an appropriate `xn--` prefix to be decoded.
    return "\\\\".concat(domainToUnicode(hostname)).concat(pathname);
  } // Otherwise, it's a local path that requires a drive letter


  var letter = pathname.codePointAt(1) | 0x20;
  var sep = pathname[2];

  if (letter < CHAR_LOWERCASE_A || letter > CHAR_LOWERCASE_Z || // a..z A..Z
  sep !== ':') {
    throw new ERR_INVALID_FILE_URL_PATH('must be absolute');
  }

  return pathname.slice(1);
}

function getPathFromURLPosix(url) {
  if (url.hostname !== '') {
    throw new ERR_INVALID_FILE_URL_HOST(platform);
  }

  var pathname = url.pathname;

  for (var n = 0; n < pathname.length; n++) {
    if (pathname[n] === '%') {
      var third = pathname.codePointAt(n + 2) | 0x20;

      if (pathname[n + 1] === '2' && third === 102) {
        throw new ERR_INVALID_FILE_URL_PATH('must not include encoded / characters');
      }
    }
  }

  return decodeURIComponent(pathname);
}

function fileURLToPath(path) {
  if (typeof path === 'string') path = new URL(path);else if (!isURLInstance(path)) throw new ERR_INVALID_ARG_TYPE('path', ['string', 'URL'], path);
  if (path.protocol !== 'file:') throw new ERR_INVALID_URL_SCHEME('file');
  return isWindows ? getPathFromURLWin32(path) : getPathFromURLPosix(path);
} // The following characters are percent-encoded when converting from file path
// to URL:
// - %: The percent character is the only character not encoded by the
//        `pathname` setter.
// - \: Backslash is encoded on non-windows platforms since it's a valid
//      character but the `pathname` setters replaces it by a forward slash.
// - LF: The newline character is stripped out by the `pathname` setter.
//       (See whatwg/url#419)
// - CR: The carriage return character is also stripped out by the `pathname`
//       setter.
// - TAB: The tab character is also stripped out by the `pathname` setter.


var percentRegEx = /%/g;
var backslashRegEx = /\\/g;
var newlineRegEx = /\n/g;
var carriageReturnRegEx = /\r/g;
var tabRegEx = /\t/g;

function encodePathChars(filepath) {
  if (StringPrototypeIncludes(filepath, '%')) filepath = StringPrototypeReplace(filepath, percentRegEx, '%25'); // In posix, backslash is a valid character in paths:

  if (!isWindows && StringPrototypeIncludes(filepath, '\\')) filepath = StringPrototypeReplace(filepath, backslashRegEx, '%5C');
  if (StringPrototypeIncludes(filepath, '\n')) filepath = StringPrototypeReplace(filepath, newlineRegEx, '%0A');
  if (StringPrototypeIncludes(filepath, '\r')) filepath = StringPrototypeReplace(filepath, carriageReturnRegEx, '%0D');
  if (StringPrototypeIncludes(filepath, '\t')) filepath = StringPrototypeReplace(filepath, tabRegEx, '%09');
  return filepath;
}

function pathToFileURL(filepath) {
  var outURL = new URL('file://');

  if (isWindows && StringPrototypeStartsWith(filepath, '\\\\')) {
    // UNC path format: \\server\share\resource
    var paths = StringPrototypeSplit(filepath, '\\');

    if (paths.length <= 3) {
      throw new ERR_INVALID_ARG_VALUE('filepath', filepath, 'Missing UNC resource path');
    }

    var hostname = paths[2];

    if (hostname.length === 0) {
      throw new ERR_INVALID_ARG_VALUE('filepath', filepath, 'Empty UNC servername');
    }

    outURL.hostname = domainToASCII(hostname);
    outURL.pathname = encodePathChars(ArrayPrototypeJoin(ArrayPrototypeSlice(paths, 3), '/'));
  } else {
    var resolved = path.resolve(filepath); // path.resolve strips trailing slashes so we must add them back

    var filePathLast = StringPrototypeCharCodeAt(filepath, filepath.length - 1);
    if ((filePathLast === CHAR_FORWARD_SLASH || isWindows && filePathLast === CHAR_BACKWARD_SLASH) && resolved[resolved.length - 1] !== path.sep) resolved += '/';
    outURL.pathname = encodePathChars(resolved);
  }

  return outURL;
}

function isURLInstance(fileURLOrPath) {
  return fileURLOrPath != null && fileURLOrPath.href && fileURLOrPath.origin;
}

function toPathIfFileURL(fileURLOrPath) {
  if (!isURLInstance(fileURLOrPath)) return fileURLOrPath;
  return fileURLToPath(fileURLOrPath);
}

function constructUrl(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = new URLContext();
  ctx.flags = flags;
  ctx.scheme = protocol;
  ctx.username = (flags & URL_FLAGS_HAS_USERNAME) !== 0 ? username : '';
  ctx.password = (flags & URL_FLAGS_HAS_PASSWORD) !== 0 ? password : '';
  ctx.port = port;
  ctx.path = (flags & URL_FLAGS_HAS_PATH) !== 0 ? path : [];
  ctx.query = query;
  ctx.fragment = fragment;
  ctx.host = host;
  var url = ObjectCreate(URL.prototype);
  url[context] = ctx;
  var params = new URLSearchParams();
  url[searchParams] = params;
  params[context] = url;
  initSearchParams(params, query);
  return url;
}

// setURLConstructor(constructUrl);
module.exports = {
  toUSVString: toUSVString,
  fileURLToPath: fileURLToPath,
  pathToFileURL: pathToFileURL,
  toPathIfFileURL: toPathIfFileURL,
  isURLInstance: isURLInstance,
  URL: URL,
  URLSearchParams: URLSearchParams,
  domainToASCII: domainToASCII,
  domainToUnicode: domainToUnicode,
  urlToHttpOptions: urlToHttpOptions,
  formatSymbol: kFormat,
  searchParamsSymbol: searchParams
  // encodeStr: encodeStr
};
