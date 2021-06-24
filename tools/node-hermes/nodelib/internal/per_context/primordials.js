// @nolint
'use strict';
/* eslint-disable node-core/prefer-primordials */
// This file subclasses and stores the JS builtins that come from the VM
// so that Node.js's builtin modules do not need to later look these up from
// the global proxy, which can be mutated by users.
// Use of primordials have sometimes a dramatic impact on performance, please
// benchmark all changes made in performance-sensitive areas of the codebase.
// See: https://github.com/nodejs/node/pull/38248

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = _getPrototypeOf(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

var ReflectDefineProperty = Reflect.defineProperty,
    ReflectGetOwnPropertyDescriptor = Reflect.getOwnPropertyDescriptor,
    ReflectOwnKeys = Reflect.ownKeys; // `uncurryThis` is equivalent to `func => Function.prototype.call.bind(func)`.
// It is using `bind.bind(call)` to avoid using `Function.prototype.bind`
// and `Function.prototype.call` after it may have been mutated by users.

var _Function$prototype = Function.prototype,
    apply = _Function$prototype.apply,
    bind = _Function$prototype.bind,
    call = _Function$prototype.call;
var uncurryThis = bind.bind(call);
primordials.uncurryThis = uncurryThis; // `applyBind` is equivalent to `func => Function.prototype.apply.bind(func)`.
// It is using `bind.bind(apply)` to avoid using `Function.prototype.bind`
// and `Function.prototype.apply` after it may have been mutated by users.

var applyBind = bind.bind(apply);
primordials.applyBind = applyBind; // Methods that accept a variable number of arguments, and thus it's useful to
// also create `${prefix}${key}Apply`, which uses `Function.prototype.apply`,
// instead of `Function.prototype.call`, and thus doesn't require iterator
// destructuring.

var varargsMethods = [// 'ArrayPrototypeConcat' is omitted, because it performs the spread
// on its own for arrays and array-likes with a truthy
// @@isConcatSpreadable symbol property.
'ArrayOf', 'ArrayPrototypePush', 'ArrayPrototypeUnshift', // 'FunctionPrototypeCall' is omitted, since there's 'ReflectApply'
// and 'FunctionPrototypeApply'.
'MathHypot', 'MathMax', 'MathMin', 'StringPrototypeConcat', 'TypedArrayOf'];

function getNewKey(key) {
  return _typeof(key) === 'symbol' ? "Symbol".concat(key.description[7].toUpperCase()).concat(key.description.slice(8)) : "".concat(key[0].toUpperCase()).concat(key.slice(1));
}

function copyAccessor(dest, prefix, key, _ref) {
  var enumerable = _ref.enumerable,
      get = _ref.get,
      set = _ref.set;
  ReflectDefineProperty(dest, "".concat(prefix, "Get").concat(key), {
    value: uncurryThis(get),
    enumerable: enumerable
  });

  if (set !== undefined) {
    ReflectDefineProperty(dest, "".concat(prefix, "Set").concat(key), {
      value: uncurryThis(set),
      enumerable: enumerable
    });
  }
}

function copyPropsRenamed(src, dest, prefix) {
  var _iterator = _createForOfIteratorHelper(ReflectOwnKeys(src)),
      _step;

  try {
    for (_iterator.s(); !(_step = _iterator.n()).done;) {
      var key = _step.value;
      var newKey = getNewKey(key);
      var desc = ReflectGetOwnPropertyDescriptor(src, key);

      if ('get' in desc) {
        copyAccessor(dest, prefix, newKey, desc);
      } else {
        var name = "".concat(prefix).concat(newKey);
        ReflectDefineProperty(dest, name, desc);

        if (varargsMethods.includes(name)) {
          ReflectDefineProperty(dest, "".concat(name, "Apply"), {
            // `src` is bound as the `this` so that the static `this` points
            // to the object it was defined on,
            // e.g.: `ArrayOfApply` gets a `this` of `Array`:
            value: applyBind(desc.value, src)
          });
        }
      }
    }
  } catch (err) {
    _iterator.e(err);
  } finally {
    _iterator.f();
  }
}

function copyPropsRenamedBound(src, dest, prefix) {
  var _iterator2 = _createForOfIteratorHelper(ReflectOwnKeys(src)),
      _step2;

  try {
    for (_iterator2.s(); !(_step2 = _iterator2.n()).done;) {
      var key = _step2.value;
      var newKey = getNewKey(key);
      var desc = ReflectGetOwnPropertyDescriptor(src, key);

      if ('get' in desc) {
        copyAccessor(dest, prefix, newKey, desc);
      } else {
        var value = desc.value;

        if (typeof value === 'function') {
          desc.value = value.bind(src);
        }

        var name = "".concat(prefix).concat(newKey);
        ReflectDefineProperty(dest, name, desc);

        if (varargsMethods.includes(name)) {
          ReflectDefineProperty(dest, "".concat(name, "Apply"), {
            value: applyBind(value, src)
          });
        }
      }
    }
  } catch (err) {
    _iterator2.e(err);
  } finally {
    _iterator2.f();
  }
}

function copyPrototype(src, dest, prefix) {
  var _iterator3 = _createForOfIteratorHelper(ReflectOwnKeys(src)),
      _step3;

  try {
    for (_iterator3.s(); !(_step3 = _iterator3.n()).done;) {
      var key = _step3.value;
      var newKey = getNewKey(key);
      var desc = ReflectGetOwnPropertyDescriptor(src, key);

      if ('get' in desc) {
        copyAccessor(dest, prefix, newKey, desc);
      } else {
        var value = desc.value;

        if (typeof value === 'function') {
          desc.value = uncurryThis(value);
        }

        var name = "".concat(prefix).concat(newKey);
        ReflectDefineProperty(dest, name, desc);

        if (varargsMethods.includes(name)) {
          ReflectDefineProperty(dest, "".concat(name, "Apply"), {
            value: applyBind(value)
          });
        }
      }
    }
  } catch (err) {
    _iterator3.e(err);
  } finally {
    _iterator3.f();
  }
} // Create copies of configurable value properties of the global object


['Proxy', 'globalThis'].forEach(function (name) {
  // eslint-disable-next-line no-restricted-globals
  primordials[name] = globalThis[name];
}); // Create copies of URI handling functions

[decodeURI, decodeURIComponent, encodeURI, encodeURIComponent].forEach(function (fn) {
  primordials[fn.name] = fn;
}); // Create copies of the namespace objects

['JSON', 'Math', 'Proxy', 'Reflect'].forEach(function (name) {
  // eslint-disable-next-line no-restricted-globals
  copyPropsRenamed(global[name], primordials, name);
}); // Create copies of intrinsic objects

['Array', 'ArrayBuffer', 'Boolean', 'DataView', 'Date', 'Error', 'EvalError', 'Float32Array', 'Float64Array', 'Function', 'Int16Array', 'Int32Array', 'Int8Array', 'Map', 'Number', 'Object', 'RangeError', 'ReferenceError', 'RegExp', 'Set', 'String', 'Symbol', 'SyntaxError', 'TypeError', 'URIError', 'Uint16Array', 'Uint32Array', 'Uint8Array', 'Uint8ClampedArray', 'WeakMap', 'WeakSet'].forEach(function (name) {
  // eslint-disable-next-line no-restricted-globals
  var original = global[name];
  primordials[name] = original;
  copyPropsRenamed(original, primordials, name);
  copyPrototype(original.prototype, primordials, "".concat(name, "Prototype"));
}); // Create copies of intrinsic objects that require a valid `this` to call
// static methods.
// Refs: https://www.ecma-international.org/ecma-262/#sec-promise.all

['Promise'].forEach(function (name) {
  // eslint-disable-next-line no-restricted-globals
  var original = global[name];
  primordials[name] = original;
  copyPropsRenamedBound(original, primordials, name);
  copyPrototype(original.prototype, primordials, "".concat(name, "Prototype"));
}); // Create copies of abstract intrinsic objects that are not directly exposed
// on the global object.
// Refs: https://tc39.es/ecma262/#sec-%typedarray%-intrinsic-object
[{
  name: 'TypedArray',
  original: Reflect.getPrototypeOf(Uint8Array)
}, {
  name: 'ArrayIterator',
  original: {
    prototype: Reflect.getPrototypeOf(Array.prototype[Symbol.iterator]())
  }
}, {
  name: 'StringIterator',
  original: {
    prototype: Reflect.getPrototypeOf(String.prototype[Symbol.iterator]())
  }
}].forEach(function (_ref2) {
  var name = _ref2.name,
      original = _ref2.original;
  primordials[name] = original; // The static %TypedArray% methods require a valid `this`, but can't be bound,
  // as they need a subclass constructor as the receiver:

  copyPrototype(original, primordials, name);
  copyPrototype(original.prototype, primordials, "".concat(name, "Prototype"));
});
/* eslint-enable node-core/prefer-primordials */

var _primordials = primordials,
    ArrayPrototypeForEach = _primordials.ArrayPrototypeForEach,
    FinalizationRegistry = _primordials.FinalizationRegistry,
    FunctionPrototypeCall = _primordials.FunctionPrototypeCall,
    Map = _primordials.Map,
    ObjectFreeze = _primordials.ObjectFreeze,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    Promise = _primordials.Promise,
    PromisePrototypeThen = _primordials.PromisePrototypeThen,
    Set = _primordials.Set,
    SymbolIterator = _primordials.SymbolIterator,
    WeakMap = _primordials.WeakMap,
    WeakRef = _primordials.WeakRef,
    WeakSet = _primordials.WeakSet; // Because these functions are used by `makeSafe`, which is exposed
// on the `primordials` object, it's important to use const references
// to the primordials that they use:

var createSafeIterator = function createSafeIterator(factory, _next) {
  var SafeIterator = /*#__PURE__*/function () {
    function SafeIterator(iterable) {
      _classCallCheck(this, SafeIterator);

      this._iterator = factory(iterable);
    }

    _createClass(SafeIterator, [{
      key: "next",
      value: function next() {
        return _next(this._iterator);
      }
    }, {
      key: SymbolIterator,
      value: function value() {
        return this;
      }
    }]);

    return SafeIterator;
  }();

  ObjectSetPrototypeOf(SafeIterator.prototype, null);
  ObjectFreeze(SafeIterator.prototype);
  ObjectFreeze(SafeIterator);
  return SafeIterator;
};

primordials.SafeArrayIterator = createSafeIterator(primordials.ArrayPrototypeSymbolIterator, primordials.ArrayIteratorPrototypeNext);
primordials.SafeStringIterator = createSafeIterator(primordials.StringPrototypeSymbolIterator, primordials.StringIteratorPrototypeNext);

var copyProps = function copyProps(src, dest) {
  ArrayPrototypeForEach(ReflectOwnKeys(src), function (key) {
    if (!ReflectGetOwnPropertyDescriptor(dest, key)) {
      ReflectDefineProperty(dest, key, ReflectGetOwnPropertyDescriptor(src, key));
    }
  });
};
/**
 * @type {typeof primordials.makeSafe}
 */


var makeSafe = function makeSafe(unsafe, safe) {
  if (SymbolIterator in unsafe.prototype) {
    var dummy = new unsafe();
    var next; // We can reuse the same `next` method.

    ArrayPrototypeForEach(ReflectOwnKeys(unsafe.prototype), function (key) {
      if (!ReflectGetOwnPropertyDescriptor(safe.prototype, key)) {
        var _FunctionPrototypeCal;

        var desc = ReflectGetOwnPropertyDescriptor(unsafe.prototype, key);

        if (typeof desc.value === 'function' && desc.value.length === 0 && SymbolIterator in ((_FunctionPrototypeCal = FunctionPrototypeCall(desc.value, dummy)) !== null && _FunctionPrototypeCal !== void 0 ? _FunctionPrototypeCal : {})) {
          var _next2;

          var createIterator = uncurryThis(desc.value);
          (_next2 = next) !== null && _next2 !== void 0 ? _next2 : next = uncurryThis(createIterator(dummy).next);
          var SafeIterator = createSafeIterator(createIterator, next);

          desc.value = function () {
            return new SafeIterator(this);
          };
        }

        ReflectDefineProperty(safe.prototype, key, desc);
      }
    });
  } else {
    copyProps(unsafe.prototype, safe.prototype);
  }

  copyProps(unsafe, safe);
  ObjectSetPrototypeOf(safe.prototype, null);
  ObjectFreeze(safe.prototype);
  ObjectFreeze(safe);
  return safe;
};

primordials.makeSafe = makeSafe; // Subclass the constructors because we need to use their prototype
// methods later.
// Defining the `constructor` is necessary here to avoid the default
// constructor which uses the user-mutable `%ArrayIteratorPrototype%.next`.

primordials.SafeMap = makeSafe(Map, /*#__PURE__*/function (_Map) {
  _inherits(SafeMap, _Map);

  var _super = _createSuper(SafeMap);

  function SafeMap(i) {
    _classCallCheck(this, SafeMap);

    return _super.call(this, i);
  } // eslint-disable-line no-useless-constructor


  return SafeMap;
}(Map));
primordials.SafeWeakMap = makeSafe(WeakMap, /*#__PURE__*/function (_WeakMap) {
  _inherits(SafeWeakMap, _WeakMap);

  var _super2 = _createSuper(SafeWeakMap);

  function SafeWeakMap(i) {
    _classCallCheck(this, SafeWeakMap);

    return _super2.call(this, i);
  } // eslint-disable-line no-useless-constructor


  return SafeWeakMap;
}(WeakMap));
primordials.SafeSet = makeSafe(Set, /*#__PURE__*/function (_Set) {
  _inherits(SafeSet, _Set);

  var _super3 = _createSuper(SafeSet);

  function SafeSet(i) {
    _classCallCheck(this, SafeSet);

    return _super3.call(this, i);
  } // eslint-disable-line no-useless-constructor


  return SafeSet;
}(Set));
primordials.SafeWeakSet = makeSafe(WeakSet, /*#__PURE__*/function (_WeakSet) {
  _inherits(SafeWeakSet, _WeakSet);

  var _super4 = _createSuper(SafeWeakSet);

  function SafeWeakSet(i) {
    _classCallCheck(this, SafeWeakSet);

    return _super4.call(this, i);
  } // eslint-disable-line no-useless-constructor


  return SafeWeakSet;
}(WeakSet));

var SafePromise = makeSafe(Promise, /*#__PURE__*/function (_Promise) {
  _inherits(SafePromise, _Promise);

  var _super7 = _createSuper(SafePromise);

  // eslint-disable-next-line no-useless-constructor
  function SafePromise(executor) {
    _classCallCheck(this, SafePromise);

    return _super7.call(this, executor);
  }

  return SafePromise;
}(Promise));

primordials.PromisePrototypeCatch = function (thisPromise, onRejected) {
  return PromisePrototypeThen(thisPromise, undefined, onRejected);
};
/**
 * Attaches a callback that is invoked when the Promise is settled (fulfilled or
 * rejected). The resolved value cannot be modified from the callback.
 * Prefer using async functions when possible.
 * @param {Promise<any>} thisPromise
 * @param {() => void) | undefined | null} onFinally The callback to execute
 *        when the Promise is settled (fulfilled or rejected).
 * @returns A Promise for the completion of the callback.
 */


primordials.SafePromisePrototypeFinally = function (thisPromise, onFinally) {
  return (// Wrapping on a new Promise is necessary to not expose the SafePromise
    // prototype to user-land.
    new Promise(function (a, b) {
      return new SafePromise(function (a, b) {
        return PromisePrototypeThen(thisPromise, a, b);
      })["finally"](onFinally).then(a, b);
    })
  );
};

ObjectSetPrototypeOf(primordials, null);
ObjectFreeze(primordials);
