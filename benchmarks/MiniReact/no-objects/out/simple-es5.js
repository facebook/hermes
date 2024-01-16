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
 *   app/simple/index.js
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
/* file: packages/react/index.js */
function react_index$INTERNAL$fastArrayJoin(arr, sep) {
  var result = '';
  for (var i = 0, e = arr.length; i < e; ++i) {
    if (i !== 0) result += sep;
    result += arr[i];
  }
  return result;
}
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
      // return output.join('');
      return react_index$INTERNAL$fastArrayJoin(output, '\n');
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
              if (typeof propValue === 'function') {
                continue;
              }
              str += ` ${propName}=${(_JSON$stringify = JSON.stringify(propValue)) != null ? _JSON$stringify : 'undefined'}`;
            }
            str += '>';
            out.push(str);
            this.printChildren(fiber, out, level + 1);
            out.push(padStr + '</' + tag + '>');
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
          case 'text':
            {
              // Nothing to reconcile, these nodes are visited by the main doWork() loop
              break;
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
      if (typeof elementOrString === 'object') {
        var element = sh_CHECKED_CAST$default(elementOrString);
        if (typeof element.type === 'function') {
          var component = sh_CHECKED_CAST$default(element.type);
          // const type: FiberType = {
          //   kind: 'component',
          //   component,
          // };
          var type = new react_index$INTERNAL$FiberTypeComponent(component);
          fiber = new react_index$INTERNAL$Fiber(type, element.props, element.key);
        } else {
          react_invariant$default(typeof element.type === 'string', 'Expected a host component name such as "div" or "span", got ' + sh_CHECKED_CAST$default(element.type));
          // const type: FiberType = {
          //   kind: 'host',
          //   tag: element.type,
          // };
          var _type = new react_index$INTERNAL$FiberTypeHost(sh_CHECKED_CAST$default(element.type));
          react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
          // const {children, ...props} = element.props;
          var children = element.props.children;
          var _props = Object.assign({}, element.props);
          delete _props.children;
          fiber = new react_index$INTERNAL$Fiber(_type, _props, element.key);
          if (Array.isArray(children)) {
            var _prev = null;
            for (var childElement of sh_CHECKED_CAST$default(children)) {
              var child = this.mountFiber(sh_CHECKED_CAST$default(childElement), fiber);
              if (_prev !== null) {
                sh_CHECKED_CAST$default(_prev).sibling = child;
              } else {
                // set parent to point to first child
                fiber.child = child;
              }
              _prev = child;
            }
          } else if (typeof children === 'string') {
            var _child = new react_index$INTERNAL$Fiber({
              kind: 'text',
              text: children
            }, {}, null);
            _child.parent = fiber;
            fiber.child = _child;
          } else if (children != null) {
            var _child2 = this.mountFiber(children, fiber);
            fiber.child = _child2;
          }
        }
      } else {
        react_invariant$default(typeof elementOrString === 'string', 'Expected a string');
        // const type: FiberType = {
        //   kind: 'text',
        //   text: element,
        // };
        var _type2 = new react_index$INTERNAL$FiberTypeText(sh_CHECKED_CAST$default(elementOrString));
        fiber = new react_index$INTERNAL$Fiber(_type2, {}, null);
      }
      fiber.parent = parent;
      return fiber;
    }
  }, {
    key: "reconcileFiber",
    value: function reconcileFiber(parent, prevChild, element) {
      if (prevChild !== null && sh_CHECKED_CAST$default(prevChild).type === element.type) {
        var _prevChild = sh_CHECKED_CAST$default(_prevChild);
        // Only host nodes have to be reconciled: otherwise this is a function component
        // and its children will be reconciled when they are later emitted in a host
        // position (ie as a direct result of render)
        if (_prevChild.type.kind === 'host') {
          react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
          // const {children, ...props} = element.props;
          var children = element.props.children;
          var _props2 = Object.assign({}, element.props);
          delete _props2.children;
          _prevChild.props = _props2;
          this.reconcileChildren(_prevChild, children);
        } else if (_prevChild.type.kind === 'component') {
          react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
          _prevChild.props = element.props;
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
var react_index$INTERNAL$FiberTypeText = /*#__PURE__*/function (_react_index$INTERNAL3) {
  "use strict";

  _inherits(react_index$INTERNAL$FiberTypeText, _react_index$INTERNAL3);
  var _super3 = _createSuper(react_index$INTERNAL$FiberTypeText);
  function react_index$INTERNAL$FiberTypeText(text) {
    var _this4;
    _classCallCheck(this, react_index$INTERNAL$FiberTypeText);
    _this4 = _super3.call(this, 'text');
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
  // TODO: Get this to work.
  return props.children;
}
/* file: app/simple/App.js */
function App$INTERNAL$Button(props) {
  return react_index$jsx('button', {
    id: props.id,
    onClick: props.onClick,
    children: 'Click me'
  }, null);
}
function App$INTERNAL$Input(props) {
  return react_index$jsx('input', {
    id: props.id,
    type: "text",
    onChange: props.onChange,
    value: props.value
  }, null);
}
function App$INTERNAL$TextArea(props) {
  return react_index$jsx('textarea', {
    onChange: props.onChange,
    children: props.value
  }, null);
}
function App$INTERNAL$Select(props) {
  var children = [];
  for (var i = 0; i < props.options.length; i++) {
    var option = props.options[i];
    children.push(react_index$jsx('option', {
      value: option.value,
      children: option.label
    }, option.value));
  }
  return react_index$jsx('select', {
    onChange: props.onChange,
    children: children
  }, null);
}
function App$INTERNAL$Checkbox(props) {
  return react_index$jsx('input', {
    type: "checkbox",
    checked: props.checked,
    onChange: props.onChange
  }, null);
}
function App$INTERNAL$Radio(props) {
  return react_index$jsx('input', {
    type: "radio",
    checked: props.checked,
    onChange: props.onChange
  }, null);
}
function App$INTERNAL$Slider(props) {
  return react_index$jsx('input', {
    type: "range",
    min: props.min,
    max: props.max,
    step: props.step,
    value: props.value,
    onChange: props.onChange
  }, null);
}
function App$INTERNAL$ProgressBar(props) {
  return react_index$jsx('div', {
    style: {
      width: `${props.progress}%`
    }
  }, null);
}
function App$INTERNAL$Spinner(props) {
  return react_index$jsx('div', {
    className: "spinner",
    children: 'Loading...'
  }, null);
}
function App$INTERNAL$Modal(props) {
  if (!props.isOpen) {
    return react_index$jsx('div', {
      className: "modal closed"
    }, null);
  }
  return react_index$jsx('div', {
    className: "modal open",
    children: [react_index$jsx('div', {
      className: "overlay",
      onClick: props.onClose,
      children: 'X'
    }, null), react_index$jsx('div', {
      className: "content",
      children: props.children
    }, null)]
  }, null);
}
function App$INTERNAL$Tooltip(props) {
  if (!props.isOpen) {
    return react_index$jsx('div', {
      className: "tooltip closed"
    }, null);
  }
  return react_index$jsx('div', {
    className: "tooltip open",
    children: [react_index$jsx('div', {
      className: "arrow"
    }, null), react_index$jsx('div', {
      className: "content",
      children: props.children
    }, null)]
  }, null);
}
function App$default(props) {
  var _react_index$useState = react_index$useState(''),
    _react_index$useState2 = _slicedToArray(_react_index$useState, 2),
    text = _react_index$useState2[0],
    setText = _react_index$useState2[1];
  var _react_index$useState3 = react_index$useState(0),
    _react_index$useState4 = _slicedToArray(_react_index$useState3, 2),
    number = _react_index$useState4[0],
    setNumber = _react_index$useState4[1];
  var _react_index$useState5 = react_index$useState(false),
    _react_index$useState6 = _slicedToArray(_react_index$useState5, 2),
    isChecked = _react_index$useState6[0],
    setIsChecked = _react_index$useState6[1];
  var _react_index$useState7 = react_index$useState(false),
    _react_index$useState8 = _slicedToArray(_react_index$useState7, 2),
    isSelected = _react_index$useState8[0],
    setIsSelected = _react_index$useState8[1];
  var _react_index$useState9 = react_index$useState(false),
    _react_index$useState10 = _slicedToArray(_react_index$useState9, 2),
    isOpen = _react_index$useState10[0],
    setIsOpen = _react_index$useState10[1];
  var _react_index$useState11 = react_index$useState(true),
    _react_index$useState12 = _slicedToArray(_react_index$useState11, 2),
    isTooltipOpen = _react_index$useState12[0],
    setIsTooltipOpen = _react_index$useState12[1];
  return react_index$jsx('div', {
    children: [react_index$jsx('h1', {
      children: 'React Benchmark'
    }, null), react_index$jsx(App$INTERNAL$Button, {
      id: "toggle-modal",
      onClick: function onClick() {
        return setIsOpen(!isOpen);
      },
      children: 'Toggle Modal'
    }, null), react_index$jsx(App$INTERNAL$Modal, {
      isOpen: isOpen,
      onClose: function onClose() {
        return setIsOpen(false);
      },
      children: [react_index$jsx('h2', {
        children: 'Modal Content'
      }, null), react_index$jsx('p', {
        children: 'This is some modal content.'
      }, null), react_index$jsx(App$INTERNAL$Tooltip, {
        isOpen: isTooltipOpen,
        onClose: function onClose() {
          return setIsTooltipOpen(false);
        },
        children: [react_index$jsx('h3', {
          children: 'Tooltip Content'
        }, null), react_index$jsx('p', {
          children: 'This is some tooltip content.'
        }, null)]
      }, null)]
    }, null), react_index$jsx('div', {
      children: [react_index$jsx('h2', {
        children: 'Form Elements'
      }, null), react_index$jsx(App$INTERNAL$Input, {
        id: "update-text",
        value: text,
        onChange: function onChange(e) {
          return setText(e.target.value);
        }
      }, null), react_index$jsx(App$INTERNAL$TextArea, {
        value: text,
        onChange: function onChange(e) {
          return setText(e.target.value);
        }
      }, null), react_index$jsx(App$INTERNAL$Select, {
        options: [{
          label: 'Option 1',
          value: 1
        }, {
          label: 'Option 2',
          value: 2
        }, {
          label: 'Option 3',
          value: 3
        }],
        onChange: function onChange(e) {
          return setNumber(parseInt(e.target.value));
        }
      }, null), react_index$jsx(App$INTERNAL$Checkbox, {
        checked: isChecked,
        onChange: function onChange(e) {
          return setIsChecked(e.target.checked);
        }
      }, null), react_index$jsx(App$INTERNAL$Radio, {
        checked: isSelected,
        onChange: function onChange(e) {
          return setIsSelected(e.target.checked);
        }
      }, null), react_index$jsx(App$INTERNAL$Slider, {
        min: 0,
        max: 100,
        step: 1,
        value: number,
        onChange: function onChange(e) {
          return setNumber(parseInt(e.target.value));
        }
      }, null), react_index$jsx(App$INTERNAL$ProgressBar, {
        progress: number
      }, null), react_index$jsx(App$INTERNAL$Spinner, {}, null)]
    }, null)]
  }, null);
}
/* file: app/simple/index.js */
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
    var rootElement = react_index$jsx(App$default, {}, null);
    index$INTERNAL$printIf1(i, root.render(rootElement));
    react_index$callOnClickOrChange('toggle-modal', null);
    react_index$callOnClickOrChange('update-text', {
      target: {
        value: '!!!!! some text !!!!!'
      }
    });
    sh_microtask$drainMicrotaskQueue();
    index$INTERNAL$printIf1(i, root.render(rootElement));
  }
}
index$INTERNAL$run(1);
//# sourceMappingURL=simple-es5.js.map
