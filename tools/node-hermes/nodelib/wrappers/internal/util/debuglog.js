'use strict';

function _toConsumableArray(arr) { return _arrayWithoutHoles(arr) || _iterableToArray(arr) || _unsupportedIterableToArray(arr) || _nonIterableSpread(); }

function _nonIterableSpread() { throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _iterableToArray(iter) { if (typeof Symbol !== "undefined" && iter[Symbol.iterator] != null || iter["@@iterator"] != null) return Array.from(iter); }

function _arrayWithoutHoles(arr) { if (Array.isArray(arr)) return _arrayLikeToArray(arr); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

var _primordials = primordials,
    ObjectCreate = _primordials.ObjectCreate,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    RegExp = _primordials.RegExp,
    RegExpPrototypeExec = _primordials.RegExpPrototypeExec,
    SafeArrayIterator = _primordials.SafeArrayIterator,
    StringPrototypeToLowerCase = _primordials.StringPrototypeToLowerCase,
    StringPrototypeToUpperCase = _primordials.StringPrototypeToUpperCase;

var _require = require('internal/util/inspect'),
    inspect = _require.inspect,
    format = _require.format,
    formatWithOptions = _require.formatWithOptions; // `debugImpls` and `testEnabled` are deliberately not initialized so any call
// to `debuglog()` before `initializeDebugEnv()` is called will throw.


var debugImpls;
var debugEnvRegex = /^$/;
var testEnabled; // `debugEnv` is initial value of process.env.NODE_DEBUG

function initializeDebugEnv(debugEnv) {
  debugImpls = ObjectCreate(null);

  if (debugEnv) {
    // This is run before any user code, it's OK not to use primordials.
    debugEnv = debugEnv.replace(/[|\\{}()[\]^$+?.]/g, '\\$&').replaceAll('*', '.*').replaceAll(',', '$|^');
    var debugEnvRegex = new RegExp("^".concat(debugEnv, "$"), 'i');

    testEnabled = function testEnabled(str) {
      return RegExpPrototypeExec(debugEnvRegex, str) !== null;
    };
  } else {
    testEnabled = function testEnabled() {
      return false;
    };
  }
} // Emits warning when user sets
// NODE_DEBUG=http or NODE_DEBUG=http2.


function emitWarningIfNeeded(set) {
  if ('HTTP' === set || 'HTTP2' === set) {
    process.emitWarning('Setting the NODE_DEBUG environment variable ' + 'to \'' + StringPrototypeToLowerCase(set) + '\' can expose sensitive ' + 'data (such as passwords, tokens and authentication headers) ' + 'in the resulting log.');
  }
}

var noop = function noop() {};

function debuglogImpl(enabled, set) {
  if (debugImpls[set] === undefined) {
    if (enabled) {
      var pid = process.pid;
      emitWarningIfNeeded(set);

      debugImpls[set] = function debug() {
        var colors = process.stderr.hasColors && process.stderr.hasColors();

        for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
          args[_key] = arguments[_key];
        }

        var msg = formatWithOptions.apply(void 0, [{
          colors: colors
        }].concat(args));
        var coloredPID = inspect(pid, {
          colors: colors
        });
        process.stderr.write(format('%s %s: %s\n', set, coloredPID, msg));
      };
    } else {
      debugImpls[set] = noop;
    }
  }

  return debugImpls[set];
} // debuglogImpl depends on process.pid and process.env.NODE_DEBUG,
// so it needs to be called lazily in top scopes of internal modules
// that may be loaded before these run time states are allowed to
// be accessed.


function debuglog(set, cb) {
  function init() {
    set = StringPrototypeToUpperCase(set);
    enabled = testEnabled(set);
  }

  var _debug = function debug() {
    init(); // Only invokes debuglogImpl() when the debug function is
    // called for the first time.

    _debug = debuglogImpl(enabled, set);
    if (typeof cb === 'function') cb(_debug);

    for (var _len2 = arguments.length, args = new Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
      args[_key2] = arguments[_key2];
    }

    switch (args.length) {
      case 0:
        return _debug();

      case 1:
        return _debug(args[0]);

      case 2:
        return _debug(args[0], args[1]);

      default:
        return _debug.apply(void 0, _toConsumableArray(new SafeArrayIterator(args)));
    }
  };

  var enabled;

  var _test = function test() {
    init();

    _test = function test() {
      return enabled;
    };

    return enabled;
  };

  var logger = function logger() {
    for (var _len3 = arguments.length, args = new Array(_len3), _key3 = 0; _key3 < _len3; _key3++) {
      args[_key3] = arguments[_key3];
    }

    switch (args.length) {
      case 0:
        return _debug();

      case 1:
        return _debug(args[0]);

      case 2:
        return _debug(args[0], args[1]);

      default:
        return _debug.apply(void 0, _toConsumableArray(new SafeArrayIterator(args)));
    }
  };

  ObjectDefineProperty(logger, 'enabled', {
    get: function get() {
      return _test();
    },
    configurable: true,
    enumerable: true
  });
  return logger;
}

module.exports = {
  debuglog: debuglog,
  initializeDebugEnv: initializeDebugEnv
};
