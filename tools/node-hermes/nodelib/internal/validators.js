// @nolint
'use strict';

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

var _primordials = primordials,
    ArrayIsArray = _primordials.ArrayIsArray,
    ArrayPrototypeIncludes = _primordials.ArrayPrototypeIncludes,
    ArrayPrototypeJoin = _primordials.ArrayPrototypeJoin,
    ArrayPrototypeMap = _primordials.ArrayPrototypeMap,
    NumberIsInteger = _primordials.NumberIsInteger,
    NumberMAX_SAFE_INTEGER = _primordials.NumberMAX_SAFE_INTEGER,
    NumberMIN_SAFE_INTEGER = _primordials.NumberMIN_SAFE_INTEGER,
    NumberParseInt = _primordials.NumberParseInt,
    RegExpPrototypeTest = _primordials.RegExpPrototypeTest,
    String = _primordials.String,
    StringPrototypeToUpperCase = _primordials.StringPrototypeToUpperCase,
    StringPrototypeTrim = _primordials.StringPrototypeTrim;

var _require = require('internal/errors'),
    hideStackFrames = _require.hideStackFrames,
    _require$codes = _require.codes,
    ERR_SOCKET_BAD_PORT = _require$codes.ERR_SOCKET_BAD_PORT,
    ERR_INVALID_ARG_TYPE = _require$codes.ERR_INVALID_ARG_TYPE,
    ERR_INVALID_ARG_VALUE = _require$codes.ERR_INVALID_ARG_VALUE,
    ERR_OUT_OF_RANGE = _require$codes.ERR_OUT_OF_RANGE,
    ERR_UNKNOWN_SIGNAL = _require$codes.ERR_UNKNOWN_SIGNAL,
    ERR_INVALID_CALLBACK = _require$codes.ERR_INVALID_CALLBACK;

var _require2 = require('internal/util'),
    normalizeEncoding = _require2.normalizeEncoding;

var _require3 = require('internal/util/types'),
    isArrayBufferView = _require3.isArrayBufferView;

var signals = internalBinding('constants').os.signals;

function isInt32(value) {
  return value === (value | 0);
}

function isUint32(value) {
  return value === value >>> 0;
}

var octalReg = /^[0-7]+$/;
var modeDesc = 'must be a 32-bit unsigned integer or an octal string';
/**
 * Parse and validate values that will be converted into mode_t (the S_*
 * constants). Only valid numbers and octal strings are allowed. They could be
 * converted to 32-bit unsigned integers or non-negative signed integers in the
 * C++ land, but any value higher than 0o777 will result in platform-specific
 * behaviors.
 *
 * @param {*} value Values to be validated
 * @param {string} name Name of the argument
 * @param {number} [def] If specified, will be returned for invalid values
 * @returns {number}
 */

function parseFileMode(value, name, def) {
  var _value;

  (_value = value) !== null && _value !== void 0 ? _value : value = def;

  if (typeof value === 'string') {
    if (!RegExpPrototypeTest(octalReg, value)) {
      throw new ERR_INVALID_ARG_VALUE(name, value, modeDesc);
    }

    value = NumberParseInt(value, 8);
  }

  validateInt32(value, name, 0, Math.pow(2, 32) - 1);
  return value;
}

var validateInteger = hideStackFrames(function (value, name) {
  var min = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : NumberMIN_SAFE_INTEGER;
  var max = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : NumberMAX_SAFE_INTEGER;
  if (typeof value !== 'number') throw new ERR_INVALID_ARG_TYPE(name, 'number', value);
  if (!NumberIsInteger(value)) throw new ERR_OUT_OF_RANGE(name, 'an integer', value);
  if (value < min || value > max) throw new ERR_OUT_OF_RANGE(name, ">= ".concat(min, " && <= ").concat(max), value);
});
var validateInt32 = hideStackFrames(function (value, name) {
  var min = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : -2147483648;
  var max = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : 2147483647;

  // The defaults for min and max correspond to the limits of 32-bit integers.
  if (!isInt32(value)) {
    if (typeof value !== 'number') {
      throw new ERR_INVALID_ARG_TYPE(name, 'number', value);
    }

    if (!NumberIsInteger(value)) {
      throw new ERR_OUT_OF_RANGE(name, 'an integer', value);
    }

    throw new ERR_OUT_OF_RANGE(name, ">= ".concat(min, " && <= ").concat(max), value);
  }

  if (value < min || value > max) {
    throw new ERR_OUT_OF_RANGE(name, ">= ".concat(min, " && <= ").concat(max), value);
  }
});
var validateUint32 = hideStackFrames(function (value, name, positive) {
  if (!isUint32(value)) {
    if (typeof value !== 'number') {
      throw new ERR_INVALID_ARG_TYPE(name, 'number', value);
    }

    if (!NumberIsInteger(value)) {
      throw new ERR_OUT_OF_RANGE(name, 'an integer', value);
    }

    var min = positive ? 1 : 0; // 2 ** 32 === 4294967296

    throw new ERR_OUT_OF_RANGE(name, ">= ".concat(min, " && < 4294967296"), value);
  }

  if (positive && value === 0) {
    throw new ERR_OUT_OF_RANGE(name, '>= 1 && < 4294967296', value);
  }
});

function validateString(value, name) {
  if (typeof value !== 'string') throw new ERR_INVALID_ARG_TYPE(name, 'string', value);
}

function validateNumber(value, name) {
  if (typeof value !== 'number') throw new ERR_INVALID_ARG_TYPE(name, 'number', value);
}

var validateOneOf = hideStackFrames(function (value, name, oneOf) {
  if (!ArrayPrototypeIncludes(oneOf, value)) {
    var allowed = ArrayPrototypeJoin(ArrayPrototypeMap(oneOf, function (v) {
      return typeof v === 'string' ? "'".concat(v, "'") : String(v);
    }), ', ');
    var reason = 'must be one of: ' + allowed;
    throw new ERR_INVALID_ARG_VALUE(name, value, reason);
  }
});

function validateBoolean(value, name) {
  if (typeof value !== 'boolean') throw new ERR_INVALID_ARG_TYPE(name, 'boolean', value);
}

var validateObject = hideStackFrames(function (value, name) {
  var _ref = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {},
      _ref$nullable = _ref.nullable,
      nullable = _ref$nullable === void 0 ? false : _ref$nullable,
      _ref$allowArray = _ref.allowArray,
      allowArray = _ref$allowArray === void 0 ? false : _ref$allowArray,
      _ref$allowFunction = _ref.allowFunction,
      allowFunction = _ref$allowFunction === void 0 ? false : _ref$allowFunction;

  if (!nullable && value === null || !allowArray && ArrayIsArray(value) || _typeof(value) !== 'object' && (!allowFunction || typeof value !== 'function')) {
    throw new ERR_INVALID_ARG_TYPE(name, 'Object', value);
  }
});
var validateArray = hideStackFrames(function (value, name) {
  var _ref2 = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {},
      _ref2$minLength = _ref2.minLength,
      minLength = _ref2$minLength === void 0 ? 0 : _ref2$minLength;

  if (!ArrayIsArray(value)) {
    throw new ERR_INVALID_ARG_TYPE(name, 'Array', value);
  }

  if (value.length < minLength) {
    var reason = "must be longer than ".concat(minLength);
    throw new ERR_INVALID_ARG_VALUE(name, value, reason);
  }
});

function validateSignalName(signal) {
  var name = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'signal';
  if (typeof signal !== 'string') throw new ERR_INVALID_ARG_TYPE(name, 'string', signal);

  if (signals[signal] === undefined) {
    if (signals[StringPrototypeToUpperCase(signal)] !== undefined) {
      throw new ERR_UNKNOWN_SIGNAL(signal + ' (signals must use all capital letters)');
    }

    throw new ERR_UNKNOWN_SIGNAL(signal);
  }
}

var validateBuffer = hideStackFrames(function (buffer) {
  var name = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'buffer';

  if (!isArrayBufferView(buffer)) {
    throw new ERR_INVALID_ARG_TYPE(name, ['Buffer', 'TypedArray', 'DataView'], buffer);
  }
});

function validateEncoding(data, encoding) {
  var normalizedEncoding = normalizeEncoding(encoding);
  var length = data.length;

  if (normalizedEncoding === 'hex' && length % 2 !== 0) {
    throw new ERR_INVALID_ARG_VALUE('encoding', encoding, "is invalid for data of length ".concat(length));
  }
} // Check that the port number is not NaN when coerced to a number,
// is an integer and that it falls within the legal range of port numbers.


function validatePort(port) {
  var name = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'Port';

  var _ref3 = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {},
      _ref3$allowZero = _ref3.allowZero,
      allowZero = _ref3$allowZero === void 0 ? true : _ref3$allowZero;

  if (typeof port !== 'number' && typeof port !== 'string' || typeof port === 'string' && StringPrototypeTrim(port).length === 0 || +port !== +port >>> 0 || port > 0xFFFF || port === 0 && !allowZero) {
    throw new ERR_SOCKET_BAD_PORT(name, port, allowZero);
  }

  return port | 0;
}

var validateCallback = hideStackFrames(function (callback) {
  if (typeof callback !== 'function') throw new ERR_INVALID_CALLBACK(callback);
});
var validateAbortSignal = hideStackFrames(function (signal, name) {
  if (signal !== undefined && (signal === null || _typeof(signal) !== 'object' || !('aborted' in signal))) {
    throw new ERR_INVALID_ARG_TYPE(name, 'AbortSignal', signal);
  }
});
var validateFunction = hideStackFrames(function (value, name) {
  if (typeof value !== 'function') throw new ERR_INVALID_ARG_TYPE(name, 'Function', value);
});
module.exports = {
  isInt32: isInt32,
  isUint32: isUint32,
  parseFileMode: parseFileMode,
  validateArray: validateArray,
  validateBoolean: validateBoolean,
  validateBuffer: validateBuffer,
  validateEncoding: validateEncoding,
  validateFunction: validateFunction,
  validateInt32: validateInt32,
  validateInteger: validateInteger,
  validateNumber: validateNumber,
  validateObject: validateObject,
  validateOneOf: validateOneOf,
  validatePort: validatePort,
  validateSignalName: validateSignalName,
  validateString: validateString,
  validateUint32: validateUint32,
  validateCallback: validateCallback,
  validateAbortSignal: validateAbortSignal
};
