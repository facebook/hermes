var _excluded = ["color"],
  _excluded2 = ["className", "variant", "size", "asChild"],
  _excluded3 = ["asChild"],
  _excluded4 = ["decorative", "orientation"],
  _excluded5 = ["className", "orientation", "decorative"],
  _excluded6 = ["className"],
  _excluded7 = ["className"],
  _excluded8 = ["className"],
  _excluded9 = ["album", "aspectRatio", "width", "height", "className"];
function _defineProperty(obj, key, value) { key = _toPropertyKey(key); if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }
function _objectWithoutProperties(source, excluded) { if (source == null) return {}; var target = _objectWithoutPropertiesLoose(source, excluded); var key, i; if (Object.getOwnPropertySymbols) { var sourceSymbolKeys = Object.getOwnPropertySymbols(source); for (i = 0; i < sourceSymbolKeys.length; i++) { key = sourceSymbolKeys[i]; if (excluded.indexOf(key) >= 0) continue; if (!Object.prototype.propertyIsEnumerable.call(source, key)) continue; target[key] = source[key]; } } return target; }
function _objectWithoutPropertiesLoose(source, excluded) { if (source == null) return {}; var target = {}; var sourceKeys = Object.keys(source); var key, i; for (i = 0; i < sourceKeys.length; i++) { key = sourceKeys[i]; if (excluded.indexOf(key) >= 0) continue; target[key] = source[key]; } return target; }
function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); Object.defineProperty(subClass, "prototype", { writable: false }); if (superClass) _setPrototypeOf(subClass, superClass); }
function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf ? Object.setPrototypeOf.bind() : function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }
function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = _getPrototypeOf(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }
function _possibleConstructorReturn(self, call) { if (call && (typeof call === "object" || typeof call === "function")) { return call; } else if (call !== void 0) { throw new TypeError("Derived constructors may only return object or undefined"); } return _assertThisInitialized(self); }
function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }
function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }
function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf.bind() : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }
function _slicedToArray(arr, i) { return _arrayWithHoles(arr) || _iterableToArrayLimit(arr, i) || _unsupportedIterableToArray(arr, i) || _nonIterableRest(); }
function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }
function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }
function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) arr2[i] = arr[i]; return arr2; }
function _iterableToArrayLimit(r, l) { var t = null == r ? null : "undefined" != typeof Symbol && r[Symbol.iterator] || r["@@iterator"]; if (null != t) { var e, n, i, u, a = [], f = !0, o = !1; try { if (i = (t = t.call(r)).next, 0 === l) { if (Object(t) !== t) return; f = !1; } else for (; !(f = (e = i.call(t)).done) && (a.push(e.value), a.length !== l); f = !0); } catch (r) { o = !0, n = r; } finally { try { if (!f && null != t.return && (u = t.return(), Object(u) !== u)) return; } finally { if (o) throw n; } } return a; } }
function _arrayWithHoles(arr) { if (Array.isArray(arr)) return arr; }
function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, _toPropertyKey(descriptor.key), descriptor); } }
function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); Object.defineProperty(Constructor, "prototype", { writable: false }); return Constructor; }
function _toPropertyKey(arg) { var key = _toPrimitive(arg, "string"); return typeof key === "symbol" ? key : String(key); }
function _toPrimitive(input, hint) { if (typeof input !== "object" || input === null) return input; var prim = input[Symbol.toPrimitive]; if (prim !== undefined) { var res = prim.call(input, hint || "default"); if (typeof res !== "object") return res; throw new TypeError("@@toPrimitive must return a primitive value."); } return (hint === "string" ? String : Number)(input); }
function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @generated
 *
 * Entrypoints:
 *   app/music/index.js
 */
/* file: packages/react/invariant.js */
function react_invariant$default(condition, format) {
  'inline';

  if (!condition) {
    throw new Error(format);
  }
}
/* file: packages/sh/CHECKED_CAST.js */
function sh_CHECKED_CAST$default(value) {
  'inline';

  return value;
}
/* file: packages/sh/microtask.js */
var sh_microtask$INTERNAL$microtaskQueue = [];
function sh_microtask$drainMicrotaskQueue() {
  for (var i = 0; i < sh_microtask$INTERNAL$microtaskQueue.length; i++) {
    sh_microtask$INTERNAL$microtaskQueue[i]();
    sh_microtask$INTERNAL$microtaskQueue[i] = undefined;
  }
  sh_microtask$INTERNAL$microtaskQueue = [];
}
function sh_microtask$queueMicrotask(callback) {
  sh_microtask$INTERNAL$microtaskQueue.push(callback);
}
/* file: packages/sh/fastarray.js */
function sh_fastarray$join(arr, sep) {
  var result = '';
  for (var i = 0, e = arr.length; i < e; ++i) {
    if (i !== 0) result += sep;
    result += arr[i];
  }
  return result;
}
function sh_fastarray$reduce(arr, fn, initialValue) {
  var acc = initialValue;
  for (var i = 0, e = arr.length; i < e; ++i) {
    acc = fn(acc, arr[i], i);
  }
  return acc;
}
function sh_fastarray$map(arr, fn) {
  var output = [];
  for (var i = 0, e = arr.length; i < e; ++i) {
    output.push(fn(arr[i], i));
  }
  return output;
}
function sh_fastarray$includes(arr, searchElement) {
  for (var i = 0, e = arr.length; i < e; ++i) {
    if (arr[i] === searchElement) {
      return true;
    }
  }
  return false;
}
/* file: packages/react/index.js */
function react_index$INTERNAL$padString(str, len) {
  var result = '';
  for (var i = 0; i < len; i++) {
    result += str;
  }
  return result;
}

/**
 * The type of an element in React. A React element may be a:
 *
 * - String. These elements are intrinsics that depend on the React renderer
 *   implementation.
 * - React component. See `ComponentType` for more information about its
 *   different variants.
 */
/**
 * Type of a React element. React elements are commonly created using JSX
 * literals, which desugar to React.createElement calls (see below).
 */
// type React$Element<ElementType: React$ElementType> = {|
//   type: ElementType,
//   props: Props,
//   key: React$Key | null,
//   ref: any,
// |};
var react_index$INTERNAL$React$Element = /*#__PURE__*/_createClass(function react_index$INTERNAL$React$Element(type, props, key, ref) {
  "use strict";

  _classCallCheck(this, react_index$INTERNAL$React$Element);
  this.type = type;
  this.props = props;
  this.key = key;
  this.ref = ref;
});
/**
 * The type of the key that React uses to determine where items in a new list
 * have moved.
 */
var react_index$INTERNAL$REACT_FRAGMENT_TYPE = 1 /* Symbol.for('react.fragment') */;
/* eslint-disable lint/strictly-null, lint/react-state-props-mutation, lint/flow-react-element */

/**
 * The current root
 */
var react_index$INTERNAL$workInProgressRoot = null;
/**
 * The currently rendering fiber. Only set when a component is being rendered.
 */
var react_index$INTERNAL$workInProgressFiber = null;
/**
 * The previous state hook, or null if no state hook has been evaluated yet.
 */
var react_index$INTERNAL$workInProgressState = null;
/**
 * Queue of updates triggered *during* render.
 */
var react_index$INTERNAL$renderPhaseUpdateQueue = [];
/**
 * Public API to create a new "root", this is where React attaches rendering to a host element.
 * In our case we don't actually have a real host, and currently only "render" to strings.
 */
function react_index$createRoot() {
  return new react_index$INTERNAL$Root();
}
/**
 * Hook to create (on initial render) or access (on update) a state, using the index of the useState
 * call within the component as the identity. Thus conditionally calling this API can cause state to
 * be lost.
 */
function react_index$useState(
/**
 * Initial value of the state
 */
initial) {
  var root = sh_CHECKED_CAST$default(react_index$INTERNAL$workInProgressRoot);
  var fiber = sh_CHECKED_CAST$default(react_index$INTERNAL$workInProgressFiber);
  react_invariant$default(fiber !== null && root !== null, 'useState() called outside of render');
  var state;
  var _workInProgressState = react_index$INTERNAL$workInProgressState;
  if (_workInProgressState === null) {
    // Get or initialize the first state on the fiber
    var nextState = fiber.state;
    if (nextState === null) {
      nextState = new react_index$INTERNAL$State(initial);
      fiber.state = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = sh_CHECKED_CAST$default(nextState);
  } else {
    var _nextState = sh_CHECKED_CAST$default(_workInProgressState).next;
    if (_nextState === null) {
      _nextState = new react_index$INTERNAL$State(initial);
      sh_CHECKED_CAST$default(_workInProgressState).next = _nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = sh_CHECKED_CAST$default(_nextState);
  }
  // NOTE: this should just work because of subtying, State<T> should be subtype of State<mixed>
  react_index$INTERNAL$workInProgressState = sh_CHECKED_CAST$default(state);
  return [
  // Untyped check that the existing state value has the correct type,
  // This is safe if components follow the rules of hooks
  sh_CHECKED_CAST$default(state.value), function (updater) {
    var update = new react_index$INTERNAL$Update(fiber, sh_CHECKED_CAST$default(state), sh_CHECKED_CAST$default(updater));
    if (react_index$INTERNAL$workInProgressFiber !== null) {
      // called during render
      react_index$INTERNAL$renderPhaseUpdateQueue.push(update);
    } else {
      root.notify(update);
    }
  }];
}
var react_index$INTERNAL$callbacks = new Map();
function react_index$callOnClickOrChange(id, event) {
  var callback = react_index$INTERNAL$callbacks.get(id);
  if (callback == null) {
    throw new Error('No callback registered with id: ' + id);
  }
  callback(event);
}
/**
 * The type of value that may be passed to the setState function (second part of useState return value).
 * - T: the new value
 * - (prev: T) => T: a function to compute the new value from the old value
 */
// type Updater<T> = T | ((prev: T) => T);
/**
 * The type of the setState function (second element of the array returned by useState).
 */
// type SetState<T> = (value: Updater<T>) => void;
/**
 * A queued state update.
 */
var react_index$INTERNAL$Update = /*#__PURE__*/function () {
  "use strict";

  function react_index$INTERNAL$Update(fiber, state, updater) {
    _classCallCheck(this, react_index$INTERNAL$Update);
    this.fiber = fiber;
    this.state = state;
    this.updater = updater;
  }
  _createClass(react_index$INTERNAL$Update, [{
    key: "run",
    value: function run() {
      var state = this.state;
      var value = state.value;
      var updater = this.updater;
      if (typeof updater === 'function') {
        // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
        // thus the fact that updater is a function here menas its a function of T => T.
        var fn = sh_CHECKED_CAST$default(updater);
        value = fn(state.value);
      } else {
        // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
        // thus the fact that updater is *not* a function here means it is a T
        value = sh_CHECKED_CAST$default(updater);
      }
      var changed = !Object.is(state.value, value);
      state.value = value;
      return changed;
    }
  }]);
  return react_index$INTERNAL$Update;
}();
var react_index$INTERNAL$Root = /*#__PURE__*/function () {
  "use strict";

  function react_index$INTERNAL$Root() {
    _classCallCheck(this, react_index$INTERNAL$Root);
    this.root = null;
    this.element = null;
    this.updateQueue = [];
  }
  _createClass(react_index$INTERNAL$Root, [{
    key: "notify",
    value: function notify(update) {
      var _this = this;
      this.updateQueue.push(update);
      if (this.updateQueue.length === 1) {
        sh_microtask$queueMicrotask(function () {
          var element = _this.element;
          react_invariant$default(element !== null, 'Expected an element to be set after rendering');
          _this.doWork(sh_CHECKED_CAST$default(element));
        });
      }
    }
  }, {
    key: "render",
    value: function render(element) {
      react_invariant$default(react_index$INTERNAL$workInProgressFiber === null && react_index$INTERNAL$workInProgressState === null, 'Cannot render, an existing render is in progress');
      var hasChanges = element !== this.element;
      this.element = element;
      if (hasChanges) {
        this.doWork(element);
      }
      react_invariant$default(this.root !== null, 'Expected root to be rendered');
      var root = sh_CHECKED_CAST$default(this.root);
      var output = [];
      this.printFiber(root, output, 0);
      return sh_fastarray$join(output, '\n');
    }
  }, {
    key: "doWork",
    value: function doWork(element) {
      var mustRender = this.root === null;
      for (var update of this.updateQueue) {
        mustRender = update.run() || mustRender;
      }
      this.updateQueue = [];
      if (!mustRender) {
        return;
      }
      // Visit the tree in pre-order, rendering each node
      // and then processing its children
      // eslint-disable-next-line consistent-this
      react_index$INTERNAL$workInProgressRoot = this;
      var fiber = this.root;
      if (fiber === null) {
        fiber = this.mountFiber(element, null);
        this.root = fiber;
      }
      while (fiber !== null) {
        // Render the fiber, which creates child/sibling nodes
        var fiber2 = sh_CHECKED_CAST$default(fiber);
        this.renderFiber(fiber2);
        // advance to the next fiber
        if (fiber2.child !== null) {
          fiber = fiber2.child;
        } else if (fiber2.sibling !== null) {
          fiber = fiber2.sibling;
        } else {
          fiber = fiber2.parent;
          while (fiber !== null && sh_CHECKED_CAST$default(fiber).sibling === null) {
            fiber = sh_CHECKED_CAST$default(fiber).parent;
          }
          if (fiber !== null) {
            fiber = sh_CHECKED_CAST$default(fiber).sibling;
          }
        }
      }
      react_index$INTERNAL$workInProgressRoot = null;
    }
  }, {
    key: "printFiber",
    value: function printFiber(fiber, out, level) {
      switch (fiber.type.kind) {
        case 'host':
          {
            var tag = sh_CHECKED_CAST$default(fiber.type).tag;
            var padStr = react_index$INTERNAL$padString(' ', level);
            var str = padStr + '<' + tag;
            for (var _ref of Object.entries(fiber.props)) {
              var _JSON$stringify;
              var _ref2 = _slicedToArray(_ref, 2);
              var propName = _ref2[0];
              var propValue = _ref2[1];
              if (propValue == null || typeof propValue === 'function') {
                continue;
              }
              str += ` ${propName}=${(_JSON$stringify = JSON.stringify(propValue)) != null ? _JSON$stringify : 'undefined'}`;
            }
            if (fiber.child == null) {
              str += ' />';
              out.push(str);
            } else {
              str += '>';
              out.push(str);
              this.printChildren(fiber, out, level + 1);
              out.push(padStr + '</' + tag + '>');
            }
            break;
          }
        case 'text':
          {
            var text = sh_CHECKED_CAST$default(fiber.type).text;
            if (text !== '') {
              out.push(react_index$INTERNAL$padString(' ', level) + text);
            }
            break;
          }
        case 'fragment':
        case 'component':
          {
            this.printChildren(fiber, out, level);
            break;
          }
      }
    }
  }, {
    key: "printChildren",
    value: function printChildren(fiber, out, level) {
      var current = fiber.child;
      while (current !== null) {
        this.printFiber(sh_CHECKED_CAST$default(current), out, level);
        current = sh_CHECKED_CAST$default(current).sibling;
      }
    }
  }, {
    key: "renderFiber",
    value: function renderFiber(fiber) {
      try {
        react_index$INTERNAL$workInProgressFiber = fiber;
        react_index$INTERNAL$workInProgressState = null;
        switch (fiber.type.kind) {
          case 'component':
            {
              react_invariant$default(react_index$INTERNAL$renderPhaseUpdateQueue.length === 0, 'Expected no queued render updates');
              var render = sh_CHECKED_CAST$default(fiber.type).component;
              var element = render(fiber.props);
              var iterationCount = 0;
              while (react_index$INTERNAL$renderPhaseUpdateQueue.length !== 0) {
                iterationCount++;
                react_invariant$default(iterationCount < 1000, 'Possible infinite loop with setState during render');
                var hasChanges = false;
                for (var update of react_index$INTERNAL$renderPhaseUpdateQueue) {
                  react_invariant$default(update.fiber === fiber, 'setState() during render is currently only supported when updating the component ' + 'being rendered. Setting state from another component is not supported.');
                  hasChanges = update.run() || hasChanges;
                }
                react_index$INTERNAL$renderPhaseUpdateQueue.length = 0;
                if (!hasChanges) {
                  break;
                }
                element = render(fiber.props);
              }
              fiber.child = this.reconcileFiber(fiber, fiber.child, element);
              break;
            }
          case 'host':
            {
              var id = fiber.props.id;
              if (id != null) {
                var onClick = fiber.props.onClick;
                if (onClick != null) {
                  react_index$INTERNAL$callbacks.set(id, onClick);
                }
                var onChange = fiber.props.onChange;
                if (onChange != null) {
                  react_index$INTERNAL$callbacks.set(id, onChange);
                }
              }
              break;
            }
          case 'fragment':
          case 'text':
            {
              // Nothing to reconcile, these nodes are visited by the main doWork() loop
              break;
            }
          default:
            {
              throw new Error('Unexpected fiber kind: ' + fiber.type.kind);
            }
        }
      } finally {
        react_index$INTERNAL$workInProgressFiber = null;
        react_index$INTERNAL$workInProgressState = null;
      }
    }
  }, {
    key: "mountFiber",
    value: function mountFiber(elementOrString, parent) {
      var fiber;
      // TODO: Support Array of Node's being returned from a component.
      if (typeof elementOrString === 'object') {
        var element = sh_CHECKED_CAST$default(elementOrString);
        if (typeof element.type === 'function') {
          var component = sh_CHECKED_CAST$default(element.type);
          var type = new react_index$INTERNAL$FiberTypeComponent(component);
          fiber = new react_index$INTERNAL$Fiber(type, element.props, element.key);
        } else if (typeof element.type === 'string') {
          react_invariant$default(typeof element.type === 'string', 'Expected a host component name such as "div" or "span", got ' + typeof element.type);
          var _type = new react_index$INTERNAL$FiberTypeHost(sh_CHECKED_CAST$default(element.type));
          react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
          // const {children, ...props} = element.props;
          var children = element.props.children;
          var _props = Object.assign({}, element.props);
          delete _props.children;
          fiber = new react_index$INTERNAL$Fiber(_type, _props, element.key);
          this.mountChildren(children, fiber);
        } else {
          switch (element.type) {
            case react_index$INTERNAL$REACT_FRAGMENT_TYPE:
              {
                var _type2 = new react_index$INTERNAL$FiberTypeFragment();
                fiber = new react_index$INTERNAL$Fiber(_type2, element.props, element.key);
                this.mountChildren(element.props.children, fiber);
                break;
              }
            default:
              {
                throw new Error(`Unknown element type ${element.type}`);
              }
          }
        }
      } else if (typeof elementOrString === 'string') {
        var _type3 = new react_index$INTERNAL$FiberTypeText(sh_CHECKED_CAST$default(elementOrString));
        fiber = new react_index$INTERNAL$Fiber(_type3, {}, null);
      } else {
        throw new Error(`Unexpected element type of ${typeof elementOrString}`);
      }
      fiber.parent = parent;
      return fiber;
    }
  }, {
    key: "mountChildren",
    value: function mountChildren(children, parentFiber) {
      if (Array.isArray(children)) {
        var _prev = null;
        for (var childElement of sh_CHECKED_CAST$default(children)) {
          if (childElement == null) {
            continue;
          }
          var child = this.mountFiber(sh_CHECKED_CAST$default(childElement), parentFiber);
          if (_prev !== null) {
            sh_CHECKED_CAST$default(_prev).sibling = child;
          } else {
            // set parent to point to first child
            parentFiber.child = child;
          }
          _prev = child;
        }
      } else if (children != null) {
        var _child = this.mountFiber(children, parentFiber);
        parentFiber.child = _child;
      }
    }
  }, {
    key: "reconcileFiber",
    value: function reconcileFiber(parent, prevChild, element) {
      if (prevChild !== null && sh_CHECKED_CAST$default(prevChild).type === element.type) {
        var _prevChild = sh_CHECKED_CAST$default(_prevChild);
        // Only host and fragment nodes have to be reconciled: otherwise this is a
        // function component and its children will be reconciled when they are later
        // emitted in a host position (ie as a direct result of render)
        switch (_prevChild.type.kind) {
          case 'host':
            {
              react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
              // const {children, ...props} = element.props;
              var children = element.props.children;
              var _props2 = Object.assign({}, element.props);
              delete _props2.children;
              _prevChild.props = _props2;
              this.reconcileChildren(_prevChild, children);
              break;
            }
          case 'fragment':
            {
              react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
              var _children = element.props.children;
              this.reconcileChildren(_prevChild, _children);
              break;
            }
          case 'component':
            {
              react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
              _prevChild.props = element.props;
              break;
            }
          default:
            {
              throw new Error(`Unknown node kind ${_prevChild.type.kind}`);
            }
        }
        return _prevChild;
      } else {
        var child = this.mountFiber(element, parent);
        return child;
      }
    }
  }, {
    key: "reconcileChildren",
    value: function reconcileChildren(parent, children) {
      var prevChild = parent.child;
      if (Array.isArray(children)) {
        var childrenArray = sh_CHECKED_CAST$default(children);
        // Fast-path for empty and single-element arrays
        if (childrenArray.length === 0) {
          parent.child = null;
        } else if (childrenArray.length === 1) {
          parent.child = this.reconcileFiber(parent, prevChild, childrenArray[0]);
          sh_CHECKED_CAST$default(parent.child).sibling = null;
        } else {
          this.reconcileMultipleChildren(parent, childrenArray);
        }
      } else if (typeof children === 'string') {
        if (prevChild === null || sh_CHECKED_CAST$default(prevChild).type.kind !== 'text') {
          var child = new react_index$INTERNAL$Fiber({
            kind: 'text',
            text: children
          }, {}, null);
          parent.child = child;
        } else {
          sh_CHECKED_CAST$default(sh_CHECKED_CAST$default(prevChild).type).text = sh_CHECKED_CAST$default(children);
        }
      } else if (children != null) {
        parent.child = this.reconcileFiber(parent, prevChild, sh_CHECKED_CAST$default(children));
        sh_CHECKED_CAST$default(parent.child).sibling = null;
      } else {
        parent.child = null;
        if (prevChild !== null) {
          sh_CHECKED_CAST$default(prevChild).parent = null;
        }
      }
    }
  }, {
    key: "reconcileMultipleChildren",
    value: function reconcileMultipleChildren(parent, children) {
      react_invariant$default(children.length > 1, 'Expected children to have multiple elements');
      // map existing children by key to make subsequent lookup O(log n)
      var keyedChildren = new Map();
      var current = parent.child;
      while (current !== null) {
        if (sh_CHECKED_CAST$default(current).key !== null) {
          keyedChildren.set(sh_CHECKED_CAST$default(current).key, current);
        }
        current = sh_CHECKED_CAST$default(current).sibling;
      }
      var prev = null; // previous fiber at this key/index
      var prevByIndex = parent.child; // keep track of prev fiber at this index
      for (var childElement of children) {
        var _ref3;
        var prevFiber = (_ref3 = childElement.key != null ? keyedChildren.get(childElement.key) : null) != null ? _ref3 : prevByIndex;
        var child = void 0;
        if (prevFiber != null) {
          child = this.reconcileFiber(parent, prevFiber, childElement);
        } else {
          child = this.mountFiber(childElement, parent);
        }
        if (prev !== null) {
          sh_CHECKED_CAST$default(prev).sibling = child;
        } else {
          // set parent to point to first child
          parent.child = child;
        }
        prev = child;
        prevByIndex = prevByIndex !== null ? sh_CHECKED_CAST$default(prevByIndex).sibling : null;
      }
    }
  }]);
  return react_index$INTERNAL$Root;
}();
/**
 * Describes the `type` field of Fiber, which can hold different data depending on the fiber's kind:
 * - Component stores a function of props => element.
 * - Host stores the name of the host component, ie "div"
 * - Text stores the text itself.
 */
// type FiberType =
//   | {
//       kind: 'component',
//       component: Component,
//     }
//   | {
//       kind: 'host',
//       tag: string,
//     }
//   | {
//       kind: 'text',
//       text: string,
//     };
var react_index$INTERNAL$FiberType = /*#__PURE__*/_createClass(function react_index$INTERNAL$FiberType(kind) {
  "use strict";

  _classCallCheck(this, react_index$INTERNAL$FiberType);
  this.kind = kind;
});
var react_index$INTERNAL$FiberTypeComponent = /*#__PURE__*/function (_react_index$INTERNAL) {
  "use strict";

  _inherits(react_index$INTERNAL$FiberTypeComponent, _react_index$INTERNAL);
  var _super = _createSuper(react_index$INTERNAL$FiberTypeComponent);
  function react_index$INTERNAL$FiberTypeComponent(component) {
    var _this2;
    _classCallCheck(this, react_index$INTERNAL$FiberTypeComponent);
    _this2 = _super.call(this, 'component');
    _this2.component = component;
    return _this2;
  }
  return _createClass(react_index$INTERNAL$FiberTypeComponent);
}(react_index$INTERNAL$FiberType);
var react_index$INTERNAL$FiberTypeHost = /*#__PURE__*/function (_react_index$INTERNAL2) {
  "use strict";

  _inherits(react_index$INTERNAL$FiberTypeHost, _react_index$INTERNAL2);
  var _super2 = _createSuper(react_index$INTERNAL$FiberTypeHost);
  function react_index$INTERNAL$FiberTypeHost(tag) {
    var _this3;
    _classCallCheck(this, react_index$INTERNAL$FiberTypeHost);
    _this3 = _super2.call(this, 'host');
    _this3.tag = tag;
    return _this3;
  }
  return _createClass(react_index$INTERNAL$FiberTypeHost);
}(react_index$INTERNAL$FiberType);
var react_index$INTERNAL$FiberTypeFragment = /*#__PURE__*/function (_react_index$INTERNAL3) {
  "use strict";

  _inherits(react_index$INTERNAL$FiberTypeFragment, _react_index$INTERNAL3);
  var _super3 = _createSuper(react_index$INTERNAL$FiberTypeFragment);
  function react_index$INTERNAL$FiberTypeFragment() {
    _classCallCheck(this, react_index$INTERNAL$FiberTypeFragment);
    return _super3.call(this, 'fragment');
  }
  return _createClass(react_index$INTERNAL$FiberTypeFragment);
}(react_index$INTERNAL$FiberType);
var react_index$INTERNAL$FiberTypeText = /*#__PURE__*/function (_react_index$INTERNAL4) {
  "use strict";

  _inherits(react_index$INTERNAL$FiberTypeText, _react_index$INTERNAL4);
  var _super4 = _createSuper(react_index$INTERNAL$FiberTypeText);
  function react_index$INTERNAL$FiberTypeText(text) {
    var _this4;
    _classCallCheck(this, react_index$INTERNAL$FiberTypeText);
    _this4 = _super4.call(this, 'text');
    _this4.text = text;
    return _this4;
  }
  return _createClass(react_index$INTERNAL$FiberTypeText);
}(react_index$INTERNAL$FiberType);
/**
 * The type of component props as seen by the framework, because processing is heterogenous
 * the framework only looks at the identity of prop values and does not otherwise make any
 * assumptions about which props may exist and what their types are.
 */
/**
 * Data storage for the useState() hook
 */
var react_index$INTERNAL$State = /*#__PURE__*/_createClass(function react_index$INTERNAL$State(value) {
  "use strict";

  _classCallCheck(this, react_index$INTERNAL$State);
  this.value = value;
  this.next = null;
  this.prev = null;
});
/**
 * Represents a node in the UI tree, and may correspond to a user-defined function component,
 * a host node, or a text node.
 */
var react_index$INTERNAL$Fiber = /*#__PURE__*/_createClass(function react_index$INTERNAL$Fiber(type, props, key) {
  "use strict";

  _classCallCheck(this, react_index$INTERNAL$Fiber);
  this.type = type;
  this.props = props;
  this.key = key;
  this.parent = null;
  this.child = null;
  this.sibling = null;
  this.state = null;
});
function react_index$jsx(type, props, key) {
  'inline';

  return {
    type: type,
    props: props,
    key: key,
    ref: null
  };
}
function react_index$Fragment(props) {
  'inline';

  return {
    type: react_index$INTERNAL$REACT_FRAGMENT_TYPE,
    props: props,
    key: null,
    ref: null
  };
}
function react_index$forwardRef(comp) {
  return function (props) {
    return comp(props, null);
  };
}
/* file: app/music/data/albums.js */
// TODO: Need Object support
// export type Album = {
//   name: string
//   artist: string
//   cover: string
// };

var albums$listenNowAlbums = [{
  name: 'React Rendezvous',
  artist: 'Ethan Byte',
  cover: 'https://images.unsplash.com/photo-1611348586804-61bf6c080437?w=300&dpr=2&q=80'
}, {
  name: 'Async Awakenings',
  artist: 'Nina Netcode',
  cover: 'https://images.unsplash.com/photo-1468817814611-b7edf94b5d60?w=300&dpr=2&q=80'
}, {
  name: 'The Art of Reusability',
  artist: 'Lena Logic',
  cover: 'https://images.unsplash.com/photo-1528143358888-6d3c7f67bd5d?w=300&dpr=2&q=80'
}, {
  name: 'Stateful Symphony',
  artist: 'Beth Binary',
  cover: 'https://images.unsplash.com/photo-1490300472339-79e4adc6be4a?w=300&dpr=2&q=80'
}];
var albums$madeForYouAlbums = [{
  name: 'Thinking Components',
  artist: 'Lena Logic',
  cover: 'https://images.unsplash.com/photo-1615247001958-f4bc92fa6a4a?w=300&dpr=2&q=80'
}, {
  name: 'Functional Fury',
  artist: 'Beth Binary',
  cover: 'https://images.unsplash.com/photo-1513745405825-efaf9a49315f?w=300&dpr=2&q=80'
}, {
  name: 'React Rendezvous',
  artist: 'Ethan Byte',
  cover: 'https://images.unsplash.com/photo-1614113489855-66422ad300a4?w=300&dpr=2&q=80'
}, {
  name: 'Stateful Symphony',
  artist: 'Beth Binary',
  cover: 'https://images.unsplash.com/photo-1446185250204-f94591f7d702?w=300&dpr=2&q=80'
}, {
  name: 'Async Awakenings',
  artist: 'Nina Netcode',
  cover: 'https://images.unsplash.com/photo-1468817814611-b7edf94b5d60?w=300&dpr=2&q=80'
}, {
  name: 'The Art of Reusability',
  artist: 'Lena Logic',
  cover: 'https://images.unsplash.com/photo-1490300472339-79e4adc6be4a?w=300&dpr=2&q=80'
}];
/* file: packages/next/image.js */
function next_image$default(props) {
  return react_index$jsx('img', Object.assign({}, props), null);
}
/* file: packages/@radix-ui/react-icons/index.js */
var _radix_ui_index$PlusCircledIcon = react_index$forwardRef( /*<SVGSVGElement, IconProps>*/function (_ref4, forwardedRef) {
  var _ref4$color = _ref4.color,
    color = _ref4$color === void 0 ? 'currentColor' : _ref4$color,
    props = _objectWithoutProperties(_ref4, _excluded);
  return react_index$jsx('svg', Object.assign({
    width: "15",
    height: "15",
    viewBox: "0 0 15 15",
    fill: "none",
    xmlns: "http://www.w3.org/2000/svg"
  }, props, {
    ref: forwardedRef,
    children: react_index$jsx('path', {
      d: "M7.49991 0.876892C3.84222 0.876892 0.877075 3.84204 0.877075 7.49972C0.877075 11.1574 3.84222 14.1226 7.49991 14.1226C11.1576 14.1226 14.1227 11.1574 14.1227 7.49972C14.1227 3.84204 11.1576 0.876892 7.49991 0.876892ZM1.82707 7.49972C1.82707 4.36671 4.36689 1.82689 7.49991 1.82689C10.6329 1.82689 13.1727 4.36671 13.1727 7.49972C13.1727 10.6327 10.6329 13.1726 7.49991 13.1726C4.36689 13.1726 1.82707 10.6327 1.82707 7.49972ZM7.50003 4C7.77617 4 8.00003 4.22386 8.00003 4.5V7H10.5C10.7762 7 11 7.22386 11 7.5C11 7.77614 10.7762 8 10.5 8H8.00003V10.5C8.00003 10.7761 7.77617 11 7.50003 11C7.22389 11 7.00003 10.7761 7.00003 10.5V8H4.50003C4.22389 8 4.00003 7.77614 4.00003 7.5C4.00003 7.22386 4.22389 7 4.50003 7H7.00003V4.5C7.00003 4.22386 7.22389 4 7.50003 4Z",
      fill: color,
      fillRule: "evenodd",
      clipRule: "evenodd"
    }, null)
  }), null);
});
/* file: packages/class-variance-authority/index.js */
function class_variance_authority_index$cva(base, variants) {
  var baseString = typeof base === 'string' ? sh_CHECKED_CAST$default(base) : sh_fastarray$join(sh_CHECKED_CAST$default(base), ' ');
  return function (opts) {
    return baseString;
  };
}
/* file: lib/utils.js */
// TODO switch from legacy function when SH supports rest args.
function utils$cn() {
  for (var _len = arguments.length, rest = new Array(_len), _key = 0; _key < _len; _key++) {
    rest[_key] = arguments[_key];
  }
  return rest.join(' ');
}
/* file: registry/new-york/ui/button.js */
var button$buttonVariants = class_variance_authority_index$cva('inline-flex items-center justify-center whitespace-nowrap rounded-md text-sm font-medium transition-colors focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-ring disabled:pointer-events-none disabled:opacity-50', {
  variants: {
    variant: {
      default: 'bg-primary text-primary-foreground shadow hover:bg-primary/90',
      destructive: 'bg-destructive text-destructive-foreground shadow-sm hover:bg-destructive/90',
      outline: 'border border-input bg-background shadow-sm hover:bg-accent hover:text-accent-foreground',
      secondary: 'bg-secondary text-secondary-foreground shadow-sm hover:bg-secondary/80',
      ghost: 'hover:bg-accent hover:text-accent-foreground',
      link: 'text-primary underline-offset-4 hover:underline'
    },
    size: {
      default: 'h-9 px-4 py-2',
      sm: 'h-8 rounded-md px-3 text-xs',
      lg: 'h-10 rounded-md px-8',
      icon: 'h-9 w-9'
    }
  },
  defaultVariants: {
    variant: 'default',
    size: 'default'
  }
});
var button$Button = react_index$forwardRef(function (_ref5, ref) {
  var className = _ref5.className,
    variant = _ref5.variant,
    size = _ref5.size,
    _ref5$asChild = _ref5.asChild,
    asChild = _ref5$asChild === void 0 ? false : _ref5$asChild,
    props = _objectWithoutProperties(_ref5, _excluded2);
  return react_index$jsx('button', Object.assign({
    className: utils$cn(button$buttonVariants({
      variant: variant,
      size: size,
      className: className
    })),
    ref: ref
  }, props), null);
});
/* file: packages/@radix-ui/react-primitive/index.js */
// import * as ReactDOM from 'react-dom';
// import {Slot} from '@radix-ui/react-slot';
var _radix_ui_index$2$INTERNAL$NODES = ['a', 'button', 'div', 'form', 'h2', 'h3', 'img', 'input', 'label', 'li', 'nav', 'ol', 'p', 'span', 'svg', 'ul'];

// Temporary while we await merge of this fix:
// https://github.com/DefinitelyTyped/DefinitelyTyped/pull/55396
// prettier-ignore
// type PropsWithoutRef<P> = P extends any ? ('ref' extends keyof P ? Pick<P, Exclude<keyof P, 'ref'>> : P) : P;
// type ComponentPropsWithoutRef<T extends React.ElementType> = PropsWithoutRef<
//   React.ComponentProps<T>,
// >;

// type Primitives = {
//   [E in (typeof NODES)[number]]: PrimitiveForwardRefComponent<E>,
// };

// type PrimitivePropsWithRef<E extends React.ElementType> =
//   React.ComponentPropsWithRef<E> & {
//     asChild?: boolean,
//   };

// interface PrimitiveForwardRefComponent<E extends React.ElementType>
//   extends React.ForwardRefExoticComponent<PrimitivePropsWithRef<E>> {}

/* -------------------------------------------------------------------------------------------------
 * Primitive
 * -----------------------------------------------------------------------------------------------*/

var _radix_ui_index$2$Primitive = sh_fastarray$reduce(_radix_ui_index$2$INTERNAL$NODES, function (primitive, node, _i) {
  var Node = react_index$forwardRef(function (props /* PrimitivePropsWithRef<typeof node> */, forwardedRef) {
    var asChild = props.asChild,
      primitiveProps = _objectWithoutProperties(props, _excluded3);
    var Comp = asChild ? Slot : node;
    // TODO?
    // React.useEffect(() => {
    //   (window as any)[Symbol.for('radix-ui')] = true;
    // }, []);

    return react_index$jsx(Comp, Object.assign({}, primitiveProps, {
      ref: forwardedRef
    }), null);
  });
  // TODO: SH doesn't support setting properties on functions
  // Node.displayName = `Primitive.${node}`;

  return Object.assign({}, primitive, _defineProperty({}, node, Node));
}, {});
/* -------------------------------------------------------------------------------------------------
 * Utils
 * -----------------------------------------------------------------------------------------------*/

/**
 * Flush custom event dispatch
 * https://github.com/radix-ui/primitives/pull/1378
 *
 * React batches *all* event handlers since version 18, this introduces certain considerations when using custom event types.
 *
 * Internally, React prioritises events in the following order:
 *  - discrete
 *  - continuous
 *  - default
 *
 * https://github.com/facebook/react/blob/a8a4742f1c54493df00da648a3f9d26e3db9c8b5/packages/react-dom/src/events/ReactDOMEventListener.js#L294-L350
 *
 * `discrete` is an  important distinction as updates within these events are applied immediately.
 * React however, is not able to infer the priority of custom event types due to how they are detected internally.
 * Because of this, it's possible for updates from custom events to be unexpectedly batched when
 * dispatched by another `discrete` event.
 *
 * In order to ensure that updates from custom events are applied predictably, we need to manually flush the batch.
 * This utility should be used when dispatching a custom event from within another `discrete` event, this utility
 * is not nessesary when dispatching known event types, or if dispatching a custom type inside a non-discrete event.
 * For example:
 *
 * dispatching a known click 👎
 * target.dispatchEvent(new Event(‘click’))
 *
 * dispatching a custom type within a non-discrete event 👎
 * onScroll={(event) => event.target.dispatchEvent(new CustomEvent(‘customType’))}
 *
 * dispatching a custom type within a `discrete` event 👍
 * onPointerDown={(event) => dispatchDiscreteCustomEvent(event.target, new CustomEvent(‘customType’))}
 *
 * Note: though React classifies `focus`, `focusin` and `focusout` events as `discrete`, it's  not recommended to use
 * this utility with them. This is because it's possible for those handlers to be called implicitly during render
 * e.g. when focus is within a component as it is unmounted, or when managing focus on mount.
 */

// function dispatchDiscreteCustomEvent<E extends CustomEvent>(
//   target: E['target'],
//   event: E,
// ) {
//   if (target) ReactDOM.flushSync(() => target.dispatchEvent(event));
// }

/* -----------------------------------------------------------------------------------------------*/

var _radix_ui_index$2$Root = _radix_ui_index$2$Primitive;
/* file: packages/@radix-ui/react-separator/index.js */
/* -------------------------------------------------------------------------------------------------
 *  Separator
 * -----------------------------------------------------------------------------------------------*/
var _radix_ui_index$1$INTERNAL$DEFAULT_ORIENTATION = 'horizontal';
var _radix_ui_index$1$INTERNAL$ORIENTATIONS = ['horizontal', 'vertical'];
// type Orientation = (typeof ORIENTATIONS)[number];
// type SeparatorElement = React.ElementRef<typeof Primitive.div>;
// type PrimitiveDivProps = Radix.ComponentPropsWithoutRef<typeof Primitive.div>;
// interface SeparatorProps extends PrimitiveDivProps {
//   /**
//    * Either `vertical` or `horizontal`. Defaults to `horizontal`.
//    */
//   orientation?: Orientation;
//   /**
//    * Whether or not the component is purely decorative. When true, accessibility-related attributes
//    * are updated so that that the rendered element is removed from the accessibility tree.
//    */
//   decorative?: boolean;
// }

var _radix_ui_index$1$Separator = react_index$forwardRef( /*<SeparatorElement, SeparatorProps>*/function (props, forwardedRef) {
  var decorative = props.decorative,
    _props$orientation = props.orientation,
    orientationProp = _props$orientation === void 0 ? _radix_ui_index$1$INTERNAL$DEFAULT_ORIENTATION : _props$orientation,
    domProps = _objectWithoutProperties(props, _excluded4);
  var orientation = _radix_ui_index$1$INTERNAL$isValidOrientation(orientationProp) ? orientationProp : _radix_ui_index$1$INTERNAL$DEFAULT_ORIENTATION;
  // `aria-orientation` defaults to `horizontal` so we only need it if `orientation` is vertical
  var ariaOrientation = orientation === 'vertical' ? orientation : undefined;
  var semanticProps = decorative ? {
    role: 'none'
  } : {
    'aria-orientation': ariaOrientation,
    role: 'separator'
  };
  return react_index$jsx(_radix_ui_index$2$Primitive.div, Object.assign({
    'data-orientation': orientation
  }, semanticProps, domProps, {
    ref: forwardedRef
  }), null);
});
function _radix_ui_index$1$INTERNAL$isValidOrientation(orientation) {
  return sh_fastarray$includes(_radix_ui_index$1$INTERNAL$ORIENTATIONS, orientation);
}
var _radix_ui_index$1$Root = _radix_ui_index$1$Separator;
/* file: registry/new-york/ui/separator.js */
var separator$Separator = react_index$forwardRef(
/*<
React.ElementRef<typeof SeparatorPrimitive.Root>,
React.ComponentPropsWithoutRef<typeof SeparatorPrimitive.Root>,
>*/
function (_ref6, ref) {
  var className = _ref6.className,
    _ref6$orientation = _ref6.orientation,
    orientation = _ref6$orientation === void 0 ? 'horizontal' : _ref6$orientation,
    _ref6$decorative = _ref6.decorative,
    decorative = _ref6$decorative === void 0 ? true : _ref6$decorative,
    props = _objectWithoutProperties(_ref6, _excluded5);
  return react_index$jsx(_radix_ui_index$1$Root, Object.assign({
    ref: ref,
    decorative: decorative,
    orientation: orientation,
    className: utils$cn('shrink-0 bg-border', orientation === 'horizontal' ? 'h-[1px] w-full' : 'h-full w-[1px]', className)
  }, props), null);
});
/* file: packages/@radix-ui/react-tabs/index.js */
// TODO: https://github.com/radix-ui/primitives/blob/main/packages/react/tabs/src/Tabs.tsx
function _radix_ui_index$3$List(props) {
  return react_index$jsx('div', Object.assign({}, props), null);
}
function _radix_ui_index$3$Trigger(props) {
  return react_index$jsx('div', Object.assign({}, props), null);
}
function _radix_ui_index$3$Content(props) {
  return react_index$jsx('div', Object.assign({}, props), null);
}
function _radix_ui_index$3$Tabs(props) {
  return react_index$jsx('div', Object.assign({}, props), null);
}
var _radix_ui_index$3$Root = _radix_ui_index$3$Tabs;
/* file: registry/new-york/ui/tabs.js */
var tabs$Tabs = _radix_ui_index$3$Root;
var tabs$TabsList = react_index$forwardRef(
/*<
React.ElementRef<typeof TabsPrimitive.List>,
React.ComponentPropsWithoutRef<typeof TabsPrimitive.List>,
>*/
function (_ref7, ref) {
  var className = _ref7.className,
    props = _objectWithoutProperties(_ref7, _excluded6);
  return react_index$jsx(_radix_ui_index$3$List, Object.assign({
    ref: ref,
    className: utils$cn('inline-flex h-9 items-center justify-center rounded-lg bg-muted p-1 text-muted-foreground', className)
  }, props), null);
});
// TODO: SH does not support setting properties on functions
// TabsList.displayName = TabsPrimitive.List.displayName;

var tabs$TabsTrigger = react_index$forwardRef(
/*<
React.ElementRef<typeof TabsPrimitive.Trigger>,
React.ComponentPropsWithoutRef<typeof TabsPrimitive.Trigger>,
>*/
function (_ref8, ref) {
  var className = _ref8.className,
    props = _objectWithoutProperties(_ref8, _excluded7);
  return react_index$jsx(_radix_ui_index$3$Trigger, Object.assign({
    ref: ref,
    className: utils$cn('inline-flex items-center justify-center whitespace-nowrap rounded-md px-3 py-1 text-sm font-medium ring-offset-background transition-all focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2 disabled:pointer-events-none disabled:opacity-50 data-[state=active]:bg-background data-[state=active]:text-foreground data-[state=active]:shadow', className)
  }, props), null);
});
// TODO: SH does not support setting properties on functions
// TabsTrigger.displayName = TabsPrimitive.Trigger.displayName;

var tabs$TabsContent = react_index$forwardRef(
/*<
React.ElementRef<typeof TabsPrimitive.Content>,
React.ComponentPropsWithoutRef<typeof TabsPrimitive.Content>,
>*/
function (_ref9, ref) {
  var className = _ref9.className,
    props = _objectWithoutProperties(_ref9, _excluded8);
  return react_index$jsx(_radix_ui_index$3$Content, Object.assign({
    ref: ref,
    className: utils$cn('mt-2 ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2', className)
  }, props), null);
});
/* file: app/music/data/playlists.js */
var playlists$playlists = ['Recently Added', 'Recently Played', 'Top Songs', 'Top Albums', 'Top Artists', 'Logic Discography', 'Bedtime Beats', 'Feeling Happy', 'I miss Y2K Pop', 'Runtober', 'Mellow Days', 'Eminem Essentials'];
/* file: app/music/components/album-artwork.js */
// TODO: Support objects
// type AlbumArtworkProps = {
//   ...React.HTMLAttributes<HTMLDivElement>,
//   album: Album
//   aspectRatio?: "portrait" | "square"
//   width?: number
//   height?: number
// };

function album_artwork$AlbumArtwork(_ref10) {
  var album = _ref10.album,
    _ref10$aspectRatio = _ref10.aspectRatio,
    aspectRatio = _ref10$aspectRatio === void 0 ? 'portrait' : _ref10$aspectRatio,
    width = _ref10.width,
    height = _ref10.height,
    className = _ref10.className,
    props = _objectWithoutProperties(_ref10, _excluded9);
  return react_index$jsx('div', Object.assign({
    className: utils$cn('space-y-3', className)
  }, props, {
    children: [null, react_index$jsx('div', {
      className: "overflow-hidden rounded-md",
      children: react_index$jsx(next_image$default, {
        src: album.cover,
        alt: album.name,
        width: width,
        height: height,
        className: utils$cn('h-auto w-auto object-cover transition-all hover:scale-105', aspectRatio === 'portrait' ? 'aspect-[3/4]' : 'aspect-square')
      }, null)
    }, null), null, null, react_index$jsx('div', {
      className: "space-y-1 text-sm",
      children: [react_index$jsx('h3', {
        className: "font-medium leading-none",
        children: album.name
      }, null), react_index$jsx('p', {
        className: "text-xs text-muted-foreground",
        children: album.artist
      }, null)]
    }, null)]
  }), null);
}
/* file: app/music/page.js */
function page$default(props) {
  return react_index$jsx(react_index$Fragment, {
    children: [react_index$jsx('div', {
      className: "md:hidden",
      children: [react_index$jsx(next_image$default, {
        src: "/examples/music-light.png",
        width: 1280,
        height: 1114,
        alt: "Music",
        className: "block dark:hidden"
      }, null), react_index$jsx(next_image$default, {
        src: "/examples/music-dark.png",
        width: 1280,
        height: 1114,
        alt: "Music",
        className: "hidden dark:block"
      }, null)]
    }, null), react_index$jsx('div', {
      className: "hidden md:block",
      children: [null, react_index$jsx('div', {
        className: "border-t",
        children: react_index$jsx('div', {
          className: "bg-background",
          children: react_index$jsx('div', {
            className: "grid lg:grid-cols-5",
            children: [null, react_index$jsx('div', {
              className: "col-span-3 lg:col-span-4 lg:border-l",
              children: react_index$jsx('div', {
                className: "h-full px-4 py-6 lg:px-8",
                children: react_index$jsx(tabs$Tabs, {
                  defaultValue: "music",
                  className: "h-full space-y-6",
                  children: [react_index$jsx('div', {
                    className: "space-between flex items-center",
                    children: [react_index$jsx(tabs$TabsList, {
                      children: [react_index$jsx(tabs$TabsTrigger, {
                        value: "music",
                        className: "relative",
                        children: 'Music'
                      }, null), react_index$jsx(tabs$TabsTrigger, {
                        value: "podcasts",
                        children: 'Podcasts'
                      }, null), react_index$jsx(tabs$TabsTrigger, {
                        value: "live",
                        disabled: true,
                        children: 'Live'
                      }, null)]
                    }, null), react_index$jsx('div', {
                      className: "ml-auto mr-4",
                      children: react_index$jsx(button$Button, {
                        children: [react_index$jsx(_radix_ui_index$PlusCircledIcon, {
                          className: "mr-2 h-4 w-4"
                        }, null), 'Add music']
                      }, null)
                    }, null)]
                  }, null), react_index$jsx(tabs$TabsContent, {
                    value: "music",
                    className: "border-none p-0 outline-none",
                    children: [react_index$jsx('div', {
                      className: "flex items-center justify-between",
                      children: react_index$jsx('div', {
                        className: "space-y-1",
                        children: [react_index$jsx('h2', {
                          className: "text-2xl font-semibold tracking-tight",
                          children: 'Listen Now'
                        }, null), react_index$jsx('p', {
                          className: "text-sm text-muted-foreground",
                          children: 'Top picks for you. Updated daily.'
                        }, null)]
                      }, null)
                    }, null), react_index$jsx(separator$Separator, {
                      className: "my-4"
                    }, null), react_index$jsx('div', {
                      className: "relative",
                      children: [null, react_index$jsx('div', {
                        className: "flex space-x-4 pb-4",
                        children: sh_fastarray$map(albums$listenNowAlbums, function (album, _i) {
                          return react_index$jsx(album_artwork$AlbumArtwork, {
                            album: album,
                            className: "w-[250px]",
                            aspectRatio: "portrait",
                            width: 250,
                            height: 330
                          }, album.name);
                        })
                      }, null), null]
                    }, null), react_index$jsx('div', {
                      className: "mt-6 space-y-1",
                      children: [react_index$jsx('h2', {
                        className: "text-2xl font-semibold tracking-tight",
                        children: 'Made for You'
                      }, null), react_index$jsx('p', {
                        className: "text-sm text-muted-foreground",
                        children: 'Your personal playlists. Updated daily.'
                      }, null)]
                    }, null), react_index$jsx(separator$Separator, {
                      className: "my-4"
                    }, null), react_index$jsx('div', {
                      className: "relative",
                      children: [null, react_index$jsx('div', {
                        className: "flex space-x-4 pb-4",
                        children: sh_fastarray$map(albums$madeForYouAlbums, function (album, _i) {
                          return react_index$jsx(album_artwork$AlbumArtwork, {
                            album: album,
                            className: "w-[150px]",
                            aspectRatio: "square",
                            width: 150,
                            height: 150
                          }, album.name);
                        })
                      }, null), null]
                    }, null)]
                  }, null), react_index$jsx(tabs$TabsContent, {
                    value: "podcasts",
                    className: "h-full flex-col border-none p-0 data-[state=active]:flex",
                    children: [react_index$jsx('div', {
                      className: "flex items-center justify-between",
                      children: react_index$jsx('div', {
                        className: "space-y-1",
                        children: [react_index$jsx('h2', {
                          className: "text-2xl font-semibold tracking-tight",
                          children: 'New Episodes'
                        }, null), react_index$jsx('p', {
                          className: "text-sm text-muted-foreground",
                          children: 'Your favorite podcasts. Updated daily.'
                        }, null)]
                      }, null)
                    }, null), react_index$jsx(separator$Separator, {
                      className: "my-4"
                    }, null), null]
                  }, null)]
                }, null)
              }, null)
            }, null)]
          }, null)
        }, null)
      }, null)]
    }, null)]
  }, null);
}
/* file: app/music/index.js */
function index$INTERNAL$printIf1(i, str) {
  if (i === 1) {
    print('===============================');
    print(str);
    print('===============================');
  }
}
function index$INTERNAL$run(N) {
  for (var i = 1; i <= N; ++i) {
    var root = react_index$createRoot();
    var rootElement = react_index$jsx(page$default, {}, null);
    index$INTERNAL$printIf1(i, root.render(rootElement));

    // React.callOnClickOrChange('click-me', null);
    // drainMicrotaskQueue();
    // printIf1(i, root.render(rootElement));
  }
}
index$INTERNAL$run(1);
//# sourceMappingURL=music-es5.js.map
