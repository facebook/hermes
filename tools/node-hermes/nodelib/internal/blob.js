// @nolint
'use strict';

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) { try { var info = gen[key](arg); var value = info.value; } catch (error) { reject(error); return; } if (info.done) { resolve(value); } else { Promise.resolve(value).then(_next, _throw); } }

function _asyncToGenerator(fn) { return function () { var self = this, args = arguments; return new Promise(function (resolve, reject) { var gen = fn.apply(self, args); function _next(value) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value); } function _throw(err) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err); } _next(undefined); }); }; }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = _getPrototypeOf(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

var _primordials = primordials,
    ArrayFrom = _primordials.ArrayFrom,
    MathMax = _primordials.MathMax,
    MathMin = _primordials.MathMin,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    PromiseResolve = _primordials.PromiseResolve,
    RegExpPrototypeTest = _primordials.RegExpPrototypeTest,
    StringPrototypeToLowerCase = _primordials.StringPrototypeToLowerCase,
    _Symbol = _primordials.Symbol,
    SymbolIterator = _primordials.SymbolIterator,
    SymbolToStringTag = _primordials.SymbolToStringTag,
    Uint8Array = _primordials.Uint8Array;

var _internalBinding = internalBinding('buffer'),
    createBlob = _internalBinding.createBlob,
    FixedSizeBlobCopyJob = _internalBinding.FixedSizeBlobCopyJob;

// var _require = require('internal/encoding'),
//     TextDecoder = _require.TextDecoder;

// var _require2 = require('internal/worker/js_transferable'),
//     JSTransferable = _require2.JSTransferable,
//     kClone = _require2.kClone,
//     kDeserialize = _require2.kDeserialize;

var _require3 = require('internal/util/types'),
    isAnyArrayBuffer = _require3.isAnyArrayBuffer,
    isArrayBufferView = _require3.isArrayBufferView;

var _require4 = require('internal/util'),
    createDeferredPromise = _require4.createDeferredPromise,
    kInspect = _require4.customInspectSymbol,
    emitExperimentalWarning = _require4.emitExperimentalWarning;

var _require5 = require('internal/util/inspect'),
    inspect = _require5.inspect;

var _require6 = require('internal/errors'),
    AbortError = _require6.AbortError,
    _require6$codes = _require6.codes,
    ERR_INVALID_ARG_TYPE = _require6$codes.ERR_INVALID_ARG_TYPE,
    ERR_BUFFER_TOO_LARGE = _require6$codes.ERR_BUFFER_TOO_LARGE;

var _require7 = require('internal/validators'),
    validateObject = _require7.validateObject,
    isUint32 = _require7.isUint32;

var kHandle = _Symbol('kHandle');

var kType = _Symbol('kType');

var kLength = _Symbol('kLength');

var disallowedTypeCharacters = /(?:(?![ -~])[\s\S])/;
var Buffer;

function lazyBuffer() {
  if (Buffer === undefined) Buffer = require('buffer').Buffer;
  return Buffer;
}

function isBlob(object) {
  return (object === null || object === void 0 ? void 0 : object[kHandle]) !== undefined;
}

function getSource(source, encoding) {
  if (isBlob(source)) return [source.size, source[kHandle]];

  if (isAnyArrayBuffer(source)) {
    source = new Uint8Array(source);
  } else if (!isArrayBufferView(source)) {
    source = lazyBuffer().from("".concat(source), encoding);
  } // We copy into a new Uint8Array because the underlying
  // BackingStores are going to be detached and owned by
  // the Blob. We also don't want to have to worry about
  // byte offsets.


  source = new Uint8Array(source);
  return [source.byteLength, source];
}

var InternalBlob = /*#__PURE__*/function (_JSTransferable) {
  _inherits(InternalBlob, _JSTransferable);

  var _super = _createSuper(InternalBlob);

  function InternalBlob(handle, length) {
    var _this;

    var type = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : '';

    _classCallCheck(this, InternalBlob);

    _this = _super.call(this);
    _this[kHandle] = handle;
    _this[kType] = type;
    _this[kLength] = length;
    return _this;
  }

  return InternalBlob;
}(JSTransferable);

var Blob = /*#__PURE__*/function (_JSTransferable2) {
  _inherits(Blob, _JSTransferable2);

  var _super2 = _createSuper(Blob);

  function Blob() {
    var _this2;

    var sources = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : [];
    var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

    _classCallCheck(this, Blob);

    emitExperimentalWarning('buffer.Blob');

    if (sources === null || typeof sources[SymbolIterator] !== 'function' || typeof sources === 'string') {
      throw new ERR_INVALID_ARG_TYPE('sources', 'Iterable', sources);
    }

    validateObject(options, 'options');
    var _options$encoding = options.encoding,
        encoding = _options$encoding === void 0 ? 'utf8' : _options$encoding;
    var _options$type = options.type,
        type = _options$type === void 0 ? '' : _options$type;
    var length = 0;
    var sources_ = ArrayFrom(sources, function (source) {
      var _getSource = getSource(source, encoding),
          len = _getSource[0],
          src = _getSource[1];

      length += len;
      return src;
    });
    if (!isUint32(length)) throw new ERR_BUFFER_TOO_LARGE(0xFFFFFFFF);
    _this2 = _super2.call(this);
    _this2[kHandle] = createBlob(sources_, length);
    _this2[kLength] = length;
    type = "".concat(type);
    _this2[kType] = RegExpPrototypeTest(disallowedTypeCharacters, type) ? '' : StringPrototypeToLowerCase(type);
    return _this2;
  }

  _createClass(Blob, [{
    key: kInspect,
    value: function value(depth, options) {
      if (depth < 0) return this;

      var opts = _objectSpread(_objectSpread({}, options), {}, {
        depth: options.depth == null ? null : options.depth - 1
      });

      return "Blob ".concat(inspect({
        size: this.size,
        type: this.type
      }, opts));
    }
  }, {
    key: kClone,
    value: function value() {
      var handle = this[kHandle];
      var type = this[kType];
      var length = this[kLength];
      return {
        data: {
          handle: handle,
          type: type,
          length: length
        },
        deserializeInfo: 'internal/blob:InternalBlob'
      };
    }
  }, {
    key: kDeserialize,
    value: function value(_ref) {
      var handle = _ref.handle,
          type = _ref.type,
          length = _ref.length;
      this[kHandle] = handle;
      this[kType] = type;
      this[kLength] = length;
    }
  }, {
    key: "type",
    get: function get() {
      return this[kType];
    }
  }, {
    key: "size",
    get: function get() {
      return this[kLength];
    }
  }, {
    key: "slice",
    value: function slice() {
      var start = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
      var end = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : this[kLength];
      var contentType = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : '';

      if (start < 0) {
        start = MathMax(this[kLength] + start, 0);
      } else {
        start = MathMin(start, this[kLength]);
      }

      start |= 0;

      if (end < 0) {
        end = MathMax(this[kLength] + end, 0);
      } else {
        end = MathMin(end, this[kLength]);
      }

      end |= 0;
      contentType = "".concat(contentType);

      if (RegExpPrototypeTest(disallowedTypeCharacters, contentType)) {
        contentType = '';
      } else {
        contentType = StringPrototypeToLowerCase(contentType);
      }

      var span = MathMax(end - start, 0);
      return new InternalBlob(this[kHandle].slice(start, start + span), span, contentType);
    }
  }, {
    key: "arrayBuffer",
    value: function () {
      var _arrayBuffer = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee() {
        var job, ret, _createDeferredPromis, promise, resolve, reject;

        return regeneratorRuntime.wrap(function _callee$(_context) {
          while (1) {
            switch (_context.prev = _context.next) {
              case 0:
                job = new FixedSizeBlobCopyJob(this[kHandle]);
                ret = job.run();

                if (!(ret !== undefined)) {
                  _context.next = 4;
                  break;
                }

                return _context.abrupt("return", PromiseResolve(ret));

              case 4:
                _createDeferredPromis = createDeferredPromise(), promise = _createDeferredPromis.promise, resolve = _createDeferredPromis.resolve, reject = _createDeferredPromis.reject;

                job.ondone = function (err, ab) {
                  if (err !== undefined) return reject(new AbortError());
                  resolve(ab);
                };

                return _context.abrupt("return", promise);

              case 7:
              case "end":
                return _context.stop();
            }
          }
        }, _callee, this);
      }));

      function arrayBuffer() {
        return _arrayBuffer.apply(this, arguments);
      }

      return arrayBuffer;
    }()
  }, {
    key: "text",
    value: function () {
      var _text = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee2() {
        var dec;
        return regeneratorRuntime.wrap(function _callee2$(_context2) {
          while (1) {
            switch (_context2.prev = _context2.next) {
              case 0:
                dec = new TextDecoder();
                _context2.t0 = dec;
                _context2.next = 4;
                return this.arrayBuffer();

              case 4:
                _context2.t1 = _context2.sent;
                return _context2.abrupt("return", _context2.t0.decode.call(_context2.t0, _context2.t1));

              case 6:
              case "end":
                return _context2.stop();
            }
          }
        }, _callee2, this);
      }));

      function text() {
        return _text.apply(this, arguments);
      }

      return text;
    }()
  }]);

  return Blob;
}(JSTransferable);

ObjectDefineProperty(Blob.prototype, SymbolToStringTag, {
  configurable: true,
  value: 'Blob'
});
InternalBlob.prototype.constructor = Blob;
ObjectSetPrototypeOf(InternalBlob.prototype, Blob.prototype);
module.exports = {
  Blob: Blob,
  InternalBlob: InternalBlob,
  isBlob: isBlob
};
