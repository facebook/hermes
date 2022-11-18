// @nolint
'use strict';

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = _getPrototypeOf(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

var _primordials = primordials,
    BigInt = _primordials.BigInt,
    Float32Array = _primordials.Float32Array,
    Float64Array = _primordials.Float64Array,
    MathFloor = _primordials.MathFloor,
    Number = _primordials.Number,
    Uint8Array = _primordials.Uint8Array;

var _require$codes = require('internal/errors').codes,
    ERR_BUFFER_OUT_OF_BOUNDS = _require$codes.ERR_BUFFER_OUT_OF_BOUNDS,
    ERR_INVALID_ARG_TYPE = _require$codes.ERR_INVALID_ARG_TYPE,
    ERR_OUT_OF_RANGE = _require$codes.ERR_OUT_OF_RANGE;

// var _require = require('internal/validators'),
//     validateNumber = _require.validateNumber;

var _internalBinding = internalBinding('buffer'),
    asciiSlice = _internalBinding.asciiSlice,
    base64Slice = _internalBinding.base64Slice,
    base64urlSlice = _internalBinding.base64urlSlice,
    latin1Slice = _internalBinding.latin1Slice,
    hexSlice = _internalBinding.hexSlice,
    ucs2Slice = _internalBinding.ucs2Slice,
    utf8Slice = _internalBinding.utf8Slice,
    asciiWrite = _internalBinding.asciiWrite,
    base64Write = _internalBinding.base64Write,
    base64urlWrite = _internalBinding.base64urlWrite,
    latin1Write = _internalBinding.latin1Write,
    hexWrite = _internalBinding.hexWrite,
    ucs2Write = _internalBinding.ucs2Write,
    utf8Write = _internalBinding.utf8Write,
    getZeroFillToggle = _internalBinding.getZeroFillToggle;

// var _internalBinding2 = internalBinding('util'),
//     untransferable_object_private_symbol = _internalBinding2.untransferable_object_private_symbol,
    // setHiddenValue = _internalBinding2.setHiddenValue; // Temporary buffers to convert numbers.


var float32Array = new Float32Array(1);
var uInt8Float32Array = new Uint8Array(float32Array.buffer);
var float64Array = new Float64Array(1);
var uInt8Float64Array = new Uint8Array(float64Array.buffer); // Check endianness.

float32Array[0] = -1; // 0xBF800000
// Either it is [0, 0, 128, 191] or [191, 128, 0, 0]. It is not possible to
// check this with `os.endianness()` because that is determined at compile time.

var bigEndian = uInt8Float32Array[3] === 0;

function checkBounds(buf, offset, byteLength) {
  validateNumber(offset, 'offset');
  if (buf[offset] === undefined || buf[offset + byteLength] === undefined) boundsError(offset, buf.length - (byteLength + 1));
}

function checkInt(value, min, max, buf, offset, byteLength) {
  if (value > max || value < min) {
    var n = typeof min === 'bigint' ? 'n' : '';
    var range;

    if (byteLength > 3) {
      if (min === 0) { //Removed bigint from 0 -> simplified if statement
        range = ">= 0".concat(n, " and < 2").concat(n, " ** ").concat((byteLength + 1) * 8).concat(n);
      } else {
        range = ">= -(2".concat(n, " ** ").concat((byteLength + 1) * 8 - 1).concat(n, ") and ") + "< 2".concat(n, " ** ").concat((byteLength + 1) * 8 - 1).concat(n);
      }
    } else {
      range = ">= ".concat(min).concat(n, " and <= ").concat(max).concat(n);
    }

    throw new ERR_OUT_OF_RANGE('value', range, value);
  }

  checkBounds(buf, offset, byteLength);
}

function boundsError(value, length, type) {
  if (MathFloor(value) !== value) {
    validateNumber(value, type);
    throw new ERR_OUT_OF_RANGE(type || 'offset', 'an integer', value);
  }

  if (length < 0) throw new ERR_BUFFER_OUT_OF_BOUNDS();
  throw new ERR_OUT_OF_RANGE(type || 'offset', ">= ".concat(type ? 1 : 0, " and <= ").concat(length), value);
} // Read integers.


function readBigUInt64LE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 7];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 8);
  var lo = first + this[++offset] * Math.pow(2, 8) + this[++offset] * Math.pow(2, 16) + this[++offset] * Math.pow(2, 24);
  var hi = this[++offset] + this[++offset] * Math.pow(2, 8) + this[++offset] * Math.pow(2, 16) + last * Math.pow(2, 24);
  return BigInt(lo) + (BigInt(hi) << 32); //Removed bigint from 32
}

function readBigUInt64BE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 7];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 8);
  var hi = first * Math.pow(2, 24) + this[++offset] * Math.pow(2, 16) + this[++offset] * Math.pow(2, 8) + this[++offset];
  var lo = this[++offset] * Math.pow(2, 24) + this[++offset] * Math.pow(2, 16) + this[++offset] * Math.pow(2, 8) + last;
  return (BigInt(hi) << 32) + BigInt(lo); //Removed bigint from 32
}

function readBigInt64LE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 7];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 8);
  var val = this[offset + 4] + this[offset + 5] * Math.pow(2, 8) + this[offset + 6] * Math.pow(2, 16) + (last << 24); // Overflow

  return (BigInt(val) << 32) + BigInt(first + this[++offset] * Math.pow(2, 8) + this[++offset] * Math.pow(2, 16) + this[++offset] * Math.pow(2, 24)); //Removed bigint from 32
}

function readBigInt64BE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 7];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 8);
  var val = (first << 24) + // Overflow
  this[++offset] * Math.pow(2, 16) + this[++offset] * Math.pow(2, 8) + this[++offset];
  return (BigInt(val) << 32) + BigInt(this[++offset] * Math.pow(2, 24) + this[++offset] * Math.pow(2, 16) + this[++offset] * Math.pow(2, 8) + last); //Removed bigint from 32
}

function readUIntLE(offset, byteLength) {
  if (offset === undefined) throw new ERR_INVALID_ARG_TYPE('offset', 'number', offset);
  if (byteLength === 6) return readUInt48LE(this, offset);
  if (byteLength === 5) return readUInt40LE(this, offset);
  if (byteLength === 3) return readUInt24LE(this, offset);
  if (byteLength === 4) return this.readUInt32LE(offset);
  if (byteLength === 2) return this.readUInt16LE(offset);
  if (byteLength === 1) return this.readUInt8(offset);
  boundsError(byteLength, 6, 'byteLength');
}

function readUInt48LE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 5];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 6);
  return first + buf[++offset] * Math.pow(2, 8) + buf[++offset] * Math.pow(2, 16) + buf[++offset] * Math.pow(2, 24) + (buf[++offset] + last * Math.pow(2, 8)) * Math.pow(2, 32);
}

function readUInt40LE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 4];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 5);
  return first + buf[++offset] * Math.pow(2, 8) + buf[++offset] * Math.pow(2, 16) + buf[++offset] * Math.pow(2, 24) + last * Math.pow(2, 32);
}

function readUInt32LE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 3];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 4);
  return first + this[++offset] * Math.pow(2, 8) + this[++offset] * Math.pow(2, 16) + last * Math.pow(2, 24);
}

function readUInt24LE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 2];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 3);
  return first + buf[++offset] * Math.pow(2, 8) + last * Math.pow(2, 16);
}

function readUInt16LE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 1];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 2);
  return first + last * Math.pow(2, 8);
}

function readUInt8() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var val = this[offset];
  if (val === undefined) boundsError(offset, this.length - 1);
  return val;
}

function readUIntBE(offset, byteLength) {
  if (offset === undefined) throw new ERR_INVALID_ARG_TYPE('offset', 'number', offset);
  if (byteLength === 6) return readUInt48BE(this, offset);
  if (byteLength === 5) return readUInt40BE(this, offset);
  if (byteLength === 3) return readUInt24BE(this, offset);
  if (byteLength === 4) return this.readUInt32BE(offset);
  if (byteLength === 2) return this.readUInt16BE(offset);
  if (byteLength === 1) return this.readUInt8(offset);
  boundsError(byteLength, 6, 'byteLength');
}

function readUInt48BE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 5];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 6);
  return (first * Math.pow(2, 8) + buf[++offset]) * Math.pow(2, 32) + buf[++offset] * Math.pow(2, 24) + buf[++offset] * Math.pow(2, 16) + buf[++offset] * Math.pow(2, 8) + last;
}

function readUInt40BE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 4];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 5);
  return first * Math.pow(2, 32) + buf[++offset] * Math.pow(2, 24) + buf[++offset] * Math.pow(2, 16) + buf[++offset] * Math.pow(2, 8) + last;
}

function readUInt32BE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 3];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 4);
  return first * Math.pow(2, 24) + this[++offset] * Math.pow(2, 16) + this[++offset] * Math.pow(2, 8) + last;
}

function readUInt24BE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 2];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 3);
  return first * Math.pow(2, 16) + buf[++offset] * Math.pow(2, 8) + last;
}

function readUInt16BE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 1];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 2);
  return first * Math.pow(2, 8) + last;
}

function readIntLE(offset, byteLength) {
  if (offset === undefined) throw new ERR_INVALID_ARG_TYPE('offset', 'number', offset);
  if (byteLength === 6) return readInt48LE(this, offset);
  if (byteLength === 5) return readInt40LE(this, offset);
  if (byteLength === 3) return readInt24LE(this, offset);
  if (byteLength === 4) return this.readInt32LE(offset);
  if (byteLength === 2) return this.readInt16LE(offset);
  if (byteLength === 1) return this.readInt8(offset);
  boundsError(byteLength, 6, 'byteLength');
}

function readInt48LE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 5];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 6);
  var val = buf[offset + 4] + last * Math.pow(2, 8);
  return (val | (val & Math.pow(2, 15)) * 0x1fffe) * Math.pow(2, 32) + first + buf[++offset] * Math.pow(2, 8) + buf[++offset] * Math.pow(2, 16) + buf[++offset] * Math.pow(2, 24);
}

function readInt40LE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 4];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 5);
  return (last | (last & Math.pow(2, 7)) * 0x1fffffe) * Math.pow(2, 32) + first + buf[++offset] * Math.pow(2, 8) + buf[++offset] * Math.pow(2, 16) + buf[++offset] * Math.pow(2, 24);
}

function readInt32LE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 3];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 4);
  return first + this[++offset] * Math.pow(2, 8) + this[++offset] * Math.pow(2, 16) + (last << 24); // Overflow
}

function readInt24LE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 2];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 3);
  var val = first + buf[++offset] * Math.pow(2, 8) + last * Math.pow(2, 16);
  return val | (val & Math.pow(2, 23)) * 0x1fe;
}

function readInt16LE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 1];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 2);
  var val = first + last * Math.pow(2, 8);
  return val | (val & Math.pow(2, 15)) * 0x1fffe;
}

function readInt8() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var val = this[offset];
  if (val === undefined) boundsError(offset, this.length - 1);
  return val | (val & Math.pow(2, 7)) * 0x1fffffe;
}

function readIntBE(offset, byteLength) {
  if (offset === undefined) throw new ERR_INVALID_ARG_TYPE('offset', 'number', offset);
  if (byteLength === 6) return readInt48BE(this, offset);
  if (byteLength === 5) return readInt40BE(this, offset);
  if (byteLength === 3) return readInt24BE(this, offset);
  if (byteLength === 4) return this.readInt32BE(offset);
  if (byteLength === 2) return this.readInt16BE(offset);
  if (byteLength === 1) return this.readInt8(offset);
  boundsError(byteLength, 6, 'byteLength');
}

function readInt48BE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 5];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 6);
  var val = buf[++offset] + first * Math.pow(2, 8);
  return (val | (val & Math.pow(2, 15)) * 0x1fffe) * Math.pow(2, 32) + buf[++offset] * Math.pow(2, 24) + buf[++offset] * Math.pow(2, 16) + buf[++offset] * Math.pow(2, 8) + last;
}

function readInt40BE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 4];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 5);
  return (first | (first & Math.pow(2, 7)) * 0x1fffffe) * Math.pow(2, 32) + buf[++offset] * Math.pow(2, 24) + buf[++offset] * Math.pow(2, 16) + buf[++offset] * Math.pow(2, 8) + last;
}

function readInt32BE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 3];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 4);
  return (first << 24) + // Overflow
  this[++offset] * Math.pow(2, 16) + this[++offset] * Math.pow(2, 8) + last;
}

function readInt24BE(buf) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  validateNumber(offset, 'offset');
  var first = buf[offset];
  var last = buf[offset + 2];
  if (first === undefined || last === undefined) boundsError(offset, buf.length - 3);
  var val = first * Math.pow(2, 16) + buf[++offset] * Math.pow(2, 8) + last;
  return val | (val & Math.pow(2, 23)) * 0x1fe;
}

function readInt16BE() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 1];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 2);
  var val = first * Math.pow(2, 8) + last;
  return val | (val & Math.pow(2, 15)) * 0x1fffe;
} // Read floats


function readFloatBackwards() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 3];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 4);
  uInt8Float32Array[3] = first;
  uInt8Float32Array[2] = this[++offset];
  uInt8Float32Array[1] = this[++offset];
  uInt8Float32Array[0] = last;
  return float32Array[0];
}

function readFloatForwards() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 3];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 4);
  uInt8Float32Array[0] = first;
  uInt8Float32Array[1] = this[++offset];
  uInt8Float32Array[2] = this[++offset];
  uInt8Float32Array[3] = last;
  return float32Array[0];
}

function readDoubleBackwards() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 7];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 8);
  uInt8Float64Array[7] = first;
  uInt8Float64Array[6] = this[++offset];
  uInt8Float64Array[5] = this[++offset];
  uInt8Float64Array[4] = this[++offset];
  uInt8Float64Array[3] = this[++offset];
  uInt8Float64Array[2] = this[++offset];
  uInt8Float64Array[1] = this[++offset];
  uInt8Float64Array[0] = last;
  return float64Array[0];
}

function readDoubleForwards() {
  var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
  validateNumber(offset, 'offset');
  var first = this[offset];
  var last = this[offset + 7];
  if (first === undefined || last === undefined) boundsError(offset, this.length - 8);
  uInt8Float64Array[0] = first;
  uInt8Float64Array[1] = this[++offset];
  uInt8Float64Array[2] = this[++offset];
  uInt8Float64Array[3] = this[++offset];
  uInt8Float64Array[4] = this[++offset];
  uInt8Float64Array[5] = this[++offset];
  uInt8Float64Array[6] = this[++offset];
  uInt8Float64Array[7] = last;
  return float64Array[0];
} // Write integers.


function writeBigU_Int64LE(buf, value, offset, min, max) {
  checkInt(value, min, max, buf, offset, 7);
  var lo = Number(value & 0xffffffff); //Removed bigint from mask
  buf[offset++] = lo;
  lo = lo >> 8;
  buf[offset++] = lo;
  lo = lo >> 8;
  buf[offset++] = lo;
  lo = lo >> 8;
  buf[offset++] = lo;
  var hi = Number(value >> 32 & 0xffffffff); //Removed bigint from 32 and mask
  buf[offset++] = hi;
  hi = hi >> 8;
  buf[offset++] = hi;
  hi = hi >> 8;
  buf[offset++] = hi;
  hi = hi >> 8;
  buf[offset++] = hi;
  return offset;
}

function writeBigUInt64LE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeBigU_Int64LE(this, value, offset, 0, 0xffffffffffffffff); //Removed bigint from mask and 0
}

function writeBigU_Int64BE(buf, value, offset, min, max) {
  checkInt(value, min, max, buf, offset, 7);
  var lo = Number(value & 0xffffffff); //Removed bigint from mask
  buf[offset + 7] = lo;
  lo = lo >> 8;
  buf[offset + 6] = lo;
  lo = lo >> 8;
  buf[offset + 5] = lo;
  lo = lo >> 8;
  buf[offset + 4] = lo;
  var hi = Number(value >> 32 & 0xffffffff); //Removed bigint from 32 and mask
  buf[offset + 3] = hi;
  hi = hi >> 8;
  buf[offset + 2] = hi;
  hi = hi >> 8;
  buf[offset + 1] = hi;
  hi = hi >> 8;
  buf[offset] = hi;
  return offset + 8;
}

function writeBigUInt64BE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeBigU_Int64BE(this, value, offset, 0, 0xffffffffffffffff); //Removed bigint from mask
}

function writeBigInt64LE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeBigU_Int64LE(this, value, offset, -0x8000000000000000, 0x7fffffffffffffff); //Removed bigint from mask
}

function writeBigInt64BE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeBigU_Int64BE(this, value, offset, -0x8000000000000000, 0x7fffffffffffffff); //Removed bigint from mask
}

function writeUIntLE(value, offset, byteLength) {
  if (byteLength === 6) return writeU_Int48LE(this, value, offset, 0, 0xffffffffffff);
  if (byteLength === 5) return writeU_Int40LE(this, value, offset, 0, 0xffffffffff);
  if (byteLength === 3) return writeU_Int24LE(this, value, offset, 0, 0xffffff);
  if (byteLength === 4) return writeU_Int32LE(this, value, offset, 0, 0xffffffff);
  if (byteLength === 2) return writeU_Int16LE(this, value, offset, 0, 0xffff);
  if (byteLength === 1) return writeU_Int8(this, value, offset, 0, 0xff);
  boundsError(byteLength, 6, 'byteLength');
}

function writeU_Int48LE(buf, value, offset, min, max) {
  value = +value;
  checkInt(value, min, max, buf, offset, 5);
  var newVal = MathFloor(value * Math.pow(2, -32));
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  buf[offset++] = newVal;
  buf[offset++] = newVal >>> 8;
  return offset;
}

function writeU_Int40LE(buf, value, offset, min, max) {
  value = +value;
  checkInt(value, min, max, buf, offset, 4);
  var newVal = value;
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  buf[offset++] = MathFloor(newVal * Math.pow(2, -32));
  return offset;
}

function writeU_Int32LE(buf, value, offset, min, max) {
  value = +value;
  checkInt(value, min, max, buf, offset, 3);
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  return offset;
}

function writeUInt32LE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeU_Int32LE(this, value, offset, 0, 0xffffffff);
}

function writeU_Int24LE(buf, value, offset, min, max) {
  value = +value;
  checkInt(value, min, max, buf, offset, 2);
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  value = value >>> 8;
  buf[offset++] = value;
  return offset;
}

function writeU_Int16LE(buf, value, offset, min, max) {
  value = +value;
  checkInt(value, min, max, buf, offset, 1);
  buf[offset++] = value;
  buf[offset++] = value >>> 8;
  return offset;
}

function writeUInt16LE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeU_Int16LE(this, value, offset, 0, 0xffff);
}

function writeU_Int8(buf, value, offset, min, max) {
  value = +value; // `checkInt()` can not be used here because it checks two entries.

  validateNumber(offset, 'offset');

  if (value > max || value < min) {
    throw new ERR_OUT_OF_RANGE('value', ">= ".concat(min, " and <= ").concat(max), value);
  }

  if (buf[offset] === undefined) boundsError(offset, buf.length - 1);
  buf[offset] = value;
  return offset + 1;
}

function writeUInt8(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeU_Int8(this, value, offset, 0, 0xff);
}

function writeUIntBE(value, offset, byteLength) {
  if (byteLength === 6) return writeU_Int48BE(this, value, offset, 0, 0xffffffffffff);
  if (byteLength === 5) return writeU_Int40BE(this, value, offset, 0, 0xffffffffff);
  if (byteLength === 3) return writeU_Int24BE(this, value, offset, 0, 0xffffff);
  if (byteLength === 4) return writeU_Int32BE(this, value, offset, 0, 0xffffffff);
  if (byteLength === 2) return writeU_Int16BE(this, value, offset, 0, 0xffff);
  if (byteLength === 1) return writeU_Int8(this, value, offset, 0, 0xff);
  boundsError(byteLength, 6, 'byteLength');
}

function writeU_Int48BE(buf, value, offset, min, max) {
  value = +value;
  checkInt(value, min, max, buf, offset, 5);
  var newVal = MathFloor(value * Math.pow(2, -32));
  buf[offset++] = newVal >>> 8;
  buf[offset++] = newVal;
  buf[offset + 3] = value;
  value = value >>> 8;
  buf[offset + 2] = value;
  value = value >>> 8;
  buf[offset + 1] = value;
  value = value >>> 8;
  buf[offset] = value;
  return offset + 4;
}

function writeU_Int40BE(buf, value, offset, min, max) {
  value = +value;
  checkInt(value, min, max, buf, offset, 4);
  buf[offset++] = MathFloor(value * Math.pow(2, -32));
  buf[offset + 3] = value;
  value = value >>> 8;
  buf[offset + 2] = value;
  value = value >>> 8;
  buf[offset + 1] = value;
  value = value >>> 8;
  buf[offset] = value;
  return offset + 4;
}

function writeU_Int32BE(buf, value, offset, min, max) {
  value = +value;
  checkInt(value, min, max, buf, offset, 3);
  buf[offset + 3] = value;
  value = value >>> 8;
  buf[offset + 2] = value;
  value = value >>> 8;
  buf[offset + 1] = value;
  value = value >>> 8;
  buf[offset] = value;
  return offset + 4;
}

function writeUInt32BE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeU_Int32BE(this, value, offset, 0, 0xffffffff);
}

function writeU_Int24BE(buf, value, offset, min, max) {
  value = +value;
  checkInt(value, min, max, buf, offset, 2);
  buf[offset + 2] = value;
  value = value >>> 8;
  buf[offset + 1] = value;
  value = value >>> 8;
  buf[offset] = value;
  return offset + 3;
}

function writeU_Int16BE(buf, value, offset, min, max) {
  value = +value;
  checkInt(value, min, max, buf, offset, 1);
  buf[offset++] = value >>> 8;
  buf[offset++] = value;
  return offset;
}

function writeUInt16BE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeU_Int16BE(this, value, offset, 0, 0xffff);
}

function writeIntLE(value, offset, byteLength) {
  if (byteLength === 6) return writeU_Int48LE(this, value, offset, -0x800000000000, 0x7fffffffffff);
  if (byteLength === 5) return writeU_Int40LE(this, value, offset, -0x8000000000, 0x7fffffffff);
  if (byteLength === 3) return writeU_Int24LE(this, value, offset, -0x800000, 0x7fffff);
  if (byteLength === 4) return writeU_Int32LE(this, value, offset, -0x80000000, 0x7fffffff);
  if (byteLength === 2) return writeU_Int16LE(this, value, offset, -0x8000, 0x7fff);
  if (byteLength === 1) return writeU_Int8(this, value, offset, -0x80, 0x7f);
  boundsError(byteLength, 6, 'byteLength');
}

function writeInt32LE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeU_Int32LE(this, value, offset, -0x80000000, 0x7fffffff);
}

function writeInt16LE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeU_Int16LE(this, value, offset, -0x8000, 0x7fff);
}

function writeInt8(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeU_Int8(this, value, offset, -0x80, 0x7f);
}

function writeIntBE(value, offset, byteLength) {
  if (byteLength === 6) return writeU_Int48BE(this, value, offset, -0x800000000000, 0x7fffffffffff);
  if (byteLength === 5) return writeU_Int40BE(this, value, offset, -0x8000000000, 0x7fffffffff);
  if (byteLength === 3) return writeU_Int24BE(this, value, offset, -0x800000, 0x7fffff);
  if (byteLength === 4) return writeU_Int32BE(this, value, offset, -0x80000000, 0x7fffffff);
  if (byteLength === 2) return writeU_Int16BE(this, value, offset, -0x8000, 0x7fff);
  if (byteLength === 1) return writeU_Int8(this, value, offset, -0x80, 0x7f);
  boundsError(byteLength, 6, 'byteLength');
}

function writeInt32BE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeU_Int32BE(this, value, offset, -0x80000000, 0x7fffffff);
}

function writeInt16BE(value) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  return writeU_Int16BE(this, value, offset, -0x8000, 0x7fff);
} // Write floats.


function writeDoubleForwards(val) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  val = +val;
  checkBounds(this, offset, 7);
  float64Array[0] = val;
  this[offset++] = uInt8Float64Array[0];
  this[offset++] = uInt8Float64Array[1];
  this[offset++] = uInt8Float64Array[2];
  this[offset++] = uInt8Float64Array[3];
  this[offset++] = uInt8Float64Array[4];
  this[offset++] = uInt8Float64Array[5];
  this[offset++] = uInt8Float64Array[6];
  this[offset++] = uInt8Float64Array[7];
  return offset;
}

function writeDoubleBackwards(val) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  val = +val;
  checkBounds(this, offset, 7);
  float64Array[0] = val;
  this[offset++] = uInt8Float64Array[7];
  this[offset++] = uInt8Float64Array[6];
  this[offset++] = uInt8Float64Array[5];
  this[offset++] = uInt8Float64Array[4];
  this[offset++] = uInt8Float64Array[3];
  this[offset++] = uInt8Float64Array[2];
  this[offset++] = uInt8Float64Array[1];
  this[offset++] = uInt8Float64Array[0];
  return offset;
}

function writeFloatForwards(val) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  val = +val;
  checkBounds(this, offset, 3);
  float32Array[0] = val;
  this[offset++] = uInt8Float32Array[0];
  this[offset++] = uInt8Float32Array[1];
  this[offset++] = uInt8Float32Array[2];
  this[offset++] = uInt8Float32Array[3];
  return offset;
}

function writeFloatBackwards(val) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  val = +val;
  checkBounds(this, offset, 3);
  float32Array[0] = val;
  this[offset++] = uInt8Float32Array[3];
  this[offset++] = uInt8Float32Array[2];
  this[offset++] = uInt8Float32Array[1];
  this[offset++] = uInt8Float32Array[0];
  return offset;
}

var FastBuffer = /*#__PURE__*/function (_Uint8Array) {
  _inherits(FastBuffer, _Uint8Array);

  var _super = _createSuper(FastBuffer);

  // Using an explicit constructor here is necessary to avoid relying on
  // `Array.prototype[Symbol.iterator]`, which can be mutated by users.
  // eslint-disable-next-line no-useless-constructor
  function FastBuffer(bufferOrLength, byteOffset, length) {
    _classCallCheck(this, FastBuffer);

    return _super.call(this, bufferOrLength, byteOffset, length);
  }

  return FastBuffer;
}(Uint8Array);

function addBufferPrototypeMethods(proto) {
  proto.readBigUInt64LE = readBigUInt64LE;
  proto.readBigUInt64BE = readBigUInt64BE;
  proto.readBigUint64LE = readBigUInt64LE;
  proto.readBigUint64BE = readBigUInt64BE;
  proto.readBigInt64LE = readBigInt64LE;
  proto.readBigInt64BE = readBigInt64BE;
  proto.writeBigUInt64LE = writeBigUInt64LE;
  proto.writeBigUInt64BE = writeBigUInt64BE;
  proto.writeBigUint64LE = writeBigUInt64LE;
  proto.writeBigUint64BE = writeBigUInt64BE;
  proto.writeBigInt64LE = writeBigInt64LE;
  proto.writeBigInt64BE = writeBigInt64BE;
  proto.readUIntLE = readUIntLE;
  proto.readUInt32LE = readUInt32LE;
  proto.readUInt16LE = readUInt16LE;
  proto.readUInt8 = readUInt8;
  proto.readUIntBE = readUIntBE;
  proto.readUInt32BE = readUInt32BE;
  proto.readUInt16BE = readUInt16BE;
  proto.readUintLE = readUIntLE;
  proto.readUint32LE = readUInt32LE;
  proto.readUint16LE = readUInt16LE;
  proto.readUint8 = readUInt8;
  proto.readUintBE = readUIntBE;
  proto.readUint32BE = readUInt32BE;
  proto.readUint16BE = readUInt16BE;
  proto.readIntLE = readIntLE;
  proto.readInt32LE = readInt32LE;
  proto.readInt16LE = readInt16LE;
  proto.readInt8 = readInt8;
  proto.readIntBE = readIntBE;
  proto.readInt32BE = readInt32BE;
  proto.readInt16BE = readInt16BE;
  proto.writeUIntLE = writeUIntLE;
  proto.writeUInt32LE = writeUInt32LE;
  proto.writeUInt16LE = writeUInt16LE;
  proto.writeUInt8 = writeUInt8;
  proto.writeUIntBE = writeUIntBE;
  proto.writeUInt32BE = writeUInt32BE;
  proto.writeUInt16BE = writeUInt16BE;
  proto.writeUintLE = writeUIntLE;
  proto.writeUint32LE = writeUInt32LE;
  proto.writeUint16LE = writeUInt16LE;
  proto.writeUint8 = writeUInt8;
  proto.writeUintBE = writeUIntBE;
  proto.writeUint32BE = writeUInt32BE;
  proto.writeUint16BE = writeUInt16BE;
  proto.writeIntLE = writeIntLE;
  proto.writeInt32LE = writeInt32LE;
  proto.writeInt16LE = writeInt16LE;
  proto.writeInt8 = writeInt8;
  proto.writeIntBE = writeIntBE;
  proto.writeInt32BE = writeInt32BE;
  proto.writeInt16BE = writeInt16BE;
  proto.readFloatLE = bigEndian ? readFloatBackwards : readFloatForwards;
  proto.readFloatBE = bigEndian ? readFloatForwards : readFloatBackwards;
  proto.readDoubleLE = bigEndian ? readDoubleBackwards : readDoubleForwards;
  proto.readDoubleBE = bigEndian ? readDoubleForwards : readDoubleBackwards;
  proto.writeFloatLE = bigEndian ? writeFloatBackwards : writeFloatForwards;
  proto.writeFloatBE = bigEndian ? writeFloatForwards : writeFloatBackwards;
  proto.writeDoubleLE = bigEndian ? writeDoubleBackwards : writeDoubleForwards;
  proto.writeDoubleBE = bigEndian ? writeDoubleForwards : writeDoubleBackwards;
  proto.asciiSlice = asciiSlice;
  proto.base64Slice = base64Slice;
  proto.base64urlSlice = base64urlSlice;
  proto.latin1Slice = latin1Slice;
  proto.hexSlice = hexSlice;
  proto.ucs2Slice = ucs2Slice;
  proto.utf8Slice = utf8Slice;
  proto.asciiWrite = asciiWrite;
  proto.base64Write = base64Write;
  proto.base64urlWrite = base64urlWrite;
  proto.latin1Write = latin1Write;
  proto.hexWrite = hexWrite;
  proto.ucs2Write = ucs2Write;
  proto.utf8Write = utf8Write;
} // This would better be placed in internal/worker/io.js, but that doesn't work
// because Buffer needs this and that would introduce a cyclic dependency.


function markAsUntransferable(obj) {
  if (_typeof(obj) !== 'object' && typeof obj !== 'function' || obj === null) return; // This object is a primitive and therefore already untransferable.
  setHiddenValue(obj, untransferable_object_private_symbol, true);
} // A toggle used to access the zero fill setting of the array buffer allocator
// in C++.
// |zeroFill| can be undefined when running inside an isolate where we
// do not own the ArrayBuffer allocator.  Zero fill is always on in that case.

var zeroFill = getZeroFillToggle();

function createUnsafeBuffer(size) {
  zeroFill[0] = 0;

  try {
    return new FastBuffer(size);
  } finally {
    zeroFill[0] = 1;
  }
} // The connection between the JS land zero fill toggle and the
// C++ one in the NodeArrayBufferAllocator gets lost if the toggle
// is deserialized from the snapshot, because V8 owns the underlying
// memory of this toggle. This resets the connection.


function reconnectZeroFillToggle() {
  zeroFill = getZeroFillToggle();
}

module.exports = {
  FastBuffer: FastBuffer,
  addBufferPrototypeMethods: addBufferPrototypeMethods,
  markAsUntransferable: markAsUntransferable,
  createUnsafeBuffer: createUnsafeBuffer,
  readUInt16BE: readUInt16BE,
  readUInt32BE: readUInt32BE,
  reconnectZeroFillToggle: reconnectZeroFillToggle
};
