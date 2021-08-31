// @nolint
'use strict';

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

var _primordials = primordials,
    ArrayBufferIsView = _primordials.ArrayBufferIsView,
    ObjectDefineProperties = _primordials.ObjectDefineProperties,
    TypedArrayPrototypeGetSymbolToStringTag = _primordials.TypedArrayPrototypeGetSymbolToStringTag;

function isTypedArray(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) !== undefined;
}

function isUint8Array(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'Uint8Array';
}

function isUint8ClampedArray(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'Uint8ClampedArray';
}

function isUint16Array(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'Uint16Array';
}

function isUint32Array(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'Uint32Array';
}

function isInt8Array(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'Int8Array';
}

function isInt16Array(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'Int16Array';
}

function isInt32Array(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'Int32Array';
}

function isFloat32Array(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'Float32Array';
}

function isFloat64Array(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'Float64Array';
}

function isBigInt64Array(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'BigInt64Array';
}

function isBigUint64Array(value) {
  return TypedArrayPrototypeGetSymbolToStringTag(value) === 'BigUint64Array';
}

// New function: Rather than defining in internalBinding('types'), is defined here
function isMapIterator(value) {
  return value[Symbol.toStringTag] === "Map Iterator";
}

// New function: Rather than defining in internalBinding('types'), is defined here
function isMap(value) {
  return Object.getPrototypeOf(value) === Map.prototype;
}

function isSetIterator(value) {
  return value[Symbol.toStringTag] === "Set Iterator";
}

function isSet(value) {
  return Object.getPrototypeOf(value) === Set.prototype;
}

module.exports = { //_objectSpread(_objectSpread({}, internalBinding('types')), {}, {
  isArrayBufferView: ArrayBufferIsView,
  isTypedArray: isTypedArray,
  isUint8Array: isUint8Array,
  isUint8ClampedArray: isUint8ClampedArray,
  isUint16Array: isUint16Array,
  isUint32Array: isUint32Array,
  isInt8Array: isInt8Array,
  isInt16Array: isInt16Array,
  isInt32Array: isInt32Array,
  isFloat32Array: isFloat32Array,
  isFloat64Array: isFloat64Array,
  isBigInt64Array: isBigInt64Array,
  isBigUint64Array: isBigUint64Array,
  isMapIterator: isMapIterator,
  isMap: isMap,
  isSetIterator: isSetIterator,
  isSet: isSet,
}; //});
var isCryptoKey;
var isKeyObject;
ObjectDefineProperties(module.exports, {
  isKeyObject: {
    configurable: false,
    enumerable: true,
    value: function value(obj) {
      if (!process.versions.openssl) {
        return false;
      }

      if (!isKeyObject) {
        var _require = require('internal/crypto/keys');

        isKeyObject = _require.isKeyObject;
      }

      return isKeyObject(obj);
    }
  },
  isCryptoKey: {
    configurable: false,
    enumerable: true,
    value: function value(obj) {
      if (!process.versions.openssl) {
        return false;
      }

      if (!isCryptoKey) {
        var _require2 = require('internal/crypto/keys');

        isCryptoKey = _require2.isCryptoKey;
      }

      return isCryptoKey(obj);
    }
  }
});
