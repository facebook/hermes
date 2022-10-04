'use strict';

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var _primordials = primordials,
    StringPrototypeSlice = _primordials.StringPrototypeSlice,
    SymbolIterator = _primordials.SymbolIterator,
    TypedArrayPrototypeSet = _primordials.TypedArrayPrototypeSet,
    Uint8Array = _primordials.Uint8Array;

var _require = require('buffer'),
    Buffer = _require.Buffer;

var _require2 = require('internal/util/inspect'),
    inspect = _require2.inspect;

module.exports = /*#__PURE__*/function () {
  function BufferList() {
    _classCallCheck(this, BufferList);

    this.head = null;
    this.tail = null;
    this.length = 0;
  }

  _createClass(BufferList, [{
    key: "push",
    value: function push(v) {
      var entry = {
        data: v,
        next: null
      };
      if (this.length > 0) this.tail.next = entry;else this.head = entry;
      this.tail = entry;
      ++this.length;
    }
  }, {
    key: "unshift",
    value: function unshift(v) {
      var entry = {
        data: v,
        next: this.head
      };
      if (this.length === 0) this.tail = entry;
      this.head = entry;
      ++this.length;
    }
  }, {
    key: "shift",
    value: function shift() {
      if (this.length === 0) return;
      var ret = this.head.data;
      if (this.length === 1) this.head = this.tail = null;else this.head = this.head.next;
      --this.length;
      return ret;
    }
  }, {
    key: "clear",
    value: function clear() {
      this.head = this.tail = null;
      this.length = 0;
    }
  }, {
    key: "join",
    value: function join(s) {
      if (this.length === 0) return '';
      var p = this.head;
      var ret = '' + p.data;

      while (p = p.next) {
        ret += s + p.data;
      }

      return ret;
    }
  }, {
    key: "concat",
    value: function concat(n) {
      if (this.length === 0) return Buffer.alloc(0);
      var ret = Buffer.allocUnsafe(n >>> 0);
      var p = this.head;
      var i = 0;

      while (p) {
        TypedArrayPrototypeSet(ret, p.data, i);
        i += p.data.length;
        p = p.next;
      }

      return ret;
    } // Consumes a specified amount of bytes or characters from the buffered data.

  }, {
    key: "consume",
    value: function consume(n, hasStrings) {
      var data = this.head.data;

      if (n < data.length) {
        // `slice` is the same for buffers and strings.
        var slice = data.slice(0, n);
        this.head.data = data.slice(n);
        return slice;
      }

      if (n === data.length) {
        // First chunk is a perfect match.
        return this.shift();
      } // Result spans more than one buffer.


      return hasStrings ? this._getString(n) : this._getBuffer(n);
    }
  }, {
    key: "first",
    value: function first() {
      return this.head.data;
    }
  // }, {
  //   key: SymbolIterator,
  //   value: /*#__PURE__*/regeneratorRuntime.mark(function value() {
  //     var p;
  //     return regeneratorRuntime.wrap(function value$(_context) {
  //       while (1) {
  //         switch (_context.prev = _context.next) {
  //           case 0:
  //             p = this.head;

  //           case 1:
  //             if (!p) {
  //               _context.next = 7;
  //               break;
  //             }

  //             _context.next = 4;
  //             return p.data;

  //           case 4:
  //             p = p.next;
  //             _context.next = 1;
  //             break;

  //           case 7:
  //           case "end":
  //             return _context.stop();
  //         }
  //       }
  //     }, value, this);
  //   }) // Consumes a specified amount of characters from the buffered data.

  }, {
    key: "_getString",
    value: function _getString(n) {
      var ret = '';
      var p = this.head;
      var c = 0;

      do {
        var str = p.data;

        if (n > str.length) {
          ret += str;
          n -= str.length;
        } else {
          if (n === str.length) {
            ret += str;
            ++c;
            if (p.next) this.head = p.next;else this.head = this.tail = null;
          } else {
            ret += StringPrototypeSlice(str, 0, n);
            this.head = p;
            p.data = StringPrototypeSlice(str, n);
          }

          break;
        }

        ++c;
      } while (p = p.next);

      this.length -= c;
      return ret;
    } // Consumes a specified amount of bytes from the buffered data.

  }, {
    key: "_getBuffer",
    value: function _getBuffer(n) {
      var ret = Buffer.allocUnsafe(n);
      var retLen = n;
      var p = this.head;
      var c = 0;

      do {
        var buf = p.data;

        if (n > buf.length) {
          TypedArrayPrototypeSet(ret, buf, retLen - n);
          n -= buf.length;
        } else {
          if (n === buf.length) {
            TypedArrayPrototypeSet(ret, buf, retLen - n);
            ++c;
            if (p.next) this.head = p.next;else this.head = this.tail = null;
          } else {
            TypedArrayPrototypeSet(ret, new Uint8Array(buf.buffer, buf.byteOffset, n), retLen - n);
            this.head = p;
            p.data = buf.slice(n);
          }

          break;
        }

        ++c;
      } while (p = p.next);

      this.length -= c;
      return ret;
    } // Make sure the linked list only shows the minimal necessary information.

  }, {
    key: inspect.custom,
    value: function value(_, options) {
      return inspect(this, _objectSpread(_objectSpread({}, options), {}, {
        // Only inspect one level.
        depth: 0,
        // It should not recurse.
        customInspect: false
      }));
    }
  }]);

  return BufferList;
}();
