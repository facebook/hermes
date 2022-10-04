'use strict'; // See https://console.spec.whatwg.org/#console-namespace
// > For historical web-compatibility reasons, the namespace object
// > for console must have as its [[Prototype]] an empty object,
// > created as if by ObjectCreate(%ObjectPrototype%),
// > instead of %ObjectPrototype%.
// Since in Node.js, the Console constructor has been exposed through
// require('console'), we need to keep the Console constructor but
// we cannot actually use `new Console` to construct the global console.
// Therefore, the console.Console.prototype is not
// in the global console prototype chain anymore.

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

var _primordials = primordials,
    FunctionPrototypeBind = _primordials.FunctionPrototypeBind,
    ObjectCreate = _primordials.ObjectCreate,
    ReflectDefineProperty = _primordials.ReflectDefineProperty,
    ReflectGetOwnPropertyDescriptor = _primordials.ReflectGetOwnPropertyDescriptor,
    ReflectOwnKeys = _primordials.ReflectOwnKeys;

var _require = require('internal/console/constructor'),
    Console = _require.Console,
    kBindStreamsLazy = _require.kBindStreamsLazy,
    kBindProperties = _require.kBindProperties;

var globalConsole = ObjectCreate({}); // Since Console is not on the prototype chain of the global console,
// the symbol properties on Console.prototype have to be looked up from
// the global console itself. In addition, we need to make the global
// console a namespace by binding the console methods directly onto
// the global console with the receiver fixed.

var _iterator = _createForOfIteratorHelper(ReflectOwnKeys(Console.prototype)),
    _step;

try {
  for (_iterator.s(); !(_step = _iterator.n()).done;) {
    var prop = _step.value;

    if (prop === 'constructor') {
      continue;
    }

    var desc = ReflectGetOwnPropertyDescriptor(Console.prototype, prop);

    if (typeof desc.value === 'function') {
      // fix the receiver
      var name = desc.value.name;
      desc.value = FunctionPrototypeBind(desc.value, globalConsole);
      ReflectDefineProperty(desc.value, 'name', {
        value: name
      });
    }

    ReflectDefineProperty(globalConsole, prop, desc);
  }
} catch (err) {
  _iterator.e(err);
} finally {
  _iterator.f();
}

globalConsole[kBindStreamsLazy](process);
globalConsole[kBindProperties](true, 'auto'); // This is a legacy feature - the Console constructor is exposed on
// the global console instance.

globalConsole.Console = Console;
module.exports = globalConsole;
