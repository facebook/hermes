var _excluded = ["className", "variant", "size", "asChild"];
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
function sh_fastarray$fastArrayJoin(arr, sep) {
  var result = '';
  for (var i = 0, e = arr.length; i < e; ++i) {
    if (i !== 0) result += sep;
    result += arr[i];
  }
  return result;
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
      return sh_fastarray$fastArrayJoin(output, '\n');
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
/* file: packages/class-variance-authority/index.js */
function class_variance_authority_index$cva(base, variants) {
  var baseString = typeof base === 'string' ? sh_CHECKED_CAST$default(base) : sh_fastarray$fastArrayJoin(sh_CHECKED_CAST$default(base), ' ');
  return function (opts) {
    return baseString;
  };
}
/* file: lib/utils.js */
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
var button$Button = react_index$forwardRef(function (_ref4, ref) {
  var className = _ref4.className,
    variant = _ref4.variant,
    size = _ref4.size,
    _ref4$asChild = _ref4.asChild,
    asChild = _ref4$asChild === void 0 ? false : _ref4$asChild,
    props = _objectWithoutProperties(_ref4, _excluded);
  return react_index$jsx('button', Object.assign({
    className: utils$cn(button$buttonVariants({
      variant: variant,
      size: size,
      className: className
    })),
    ref: ref
  }, props), null);
});
/* file: app/music/page.js */
function page$default(props) {
  var _react_index$useState = react_index$useState(true),
    _react_index$useState2 = _slicedToArray(_react_index$useState, 2),
    toggle = _react_index$useState2[0],
    setToggle = _react_index$useState2[1];
  return react_index$jsx(react_index$Fragment, {
    children: [react_index$jsx(button$Button, {
      id: "click-me",
      onClick: function onClick() {
        return setToggle(!toggle);
      },
      children: ['Click me: ', String(toggle)]
    }, null), react_index$jsx('span', {
      children: 'Other'
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
    react_index$callOnClickOrChange('click-me', null);
    sh_microtask$drainMicrotaskQueue();
    index$INTERNAL$printIf1(i, root.render(rootElement));
  }
}
index$INTERNAL$run(1);
//# sourceMappingURL=music-es5.js.map
