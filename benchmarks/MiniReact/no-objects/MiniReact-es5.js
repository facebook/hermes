function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); Object.defineProperty(subClass, "prototype", { writable: false }); if (superClass) _setPrototypeOf(subClass, superClass); }
function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf ? Object.setPrototypeOf.bind() : function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }
function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = _getPrototypeOf(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }
function _possibleConstructorReturn(self, call) { if (call && (typeof call === "object" || typeof call === "function")) { return call; } else if (call !== void 0) { throw new TypeError("Derived constructors may only return object or undefined"); } return _assertThisInitialized(self); }
function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }
function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }
function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf.bind() : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }
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
 *   index.js
 */
/* file: invariant.js */
function invariant$default(condition, format) {
  'inline';

  if (!condition) {
    throw new Error(format);
  }
}
/* file: CHECKED_CAST.js */
function CHECKED_CAST$default(value) {
  'inline';

  return value;
}
/* file: React.js */
function React$INTERNAL$queueMicrotask(callback) {
  HermesInternal.enqueueJob(callback);
}
function React$INTERNAL$fastArrayJoin(arr, sep) {
  var result = '';
  for (var i = 0, e = arr.length; i < e; ++i) {
    if (i !== 0) result += sep;
    result += arr[i];
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
var React$INTERNAL$React$Element = /*#__PURE__*/_createClass(function React$INTERNAL$React$Element(type, props, key, ref) {
  "use strict";

  _classCallCheck(this, React$INTERNAL$React$Element);
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
var React$INTERNAL$workInProgressRoot = null;
/**
 * The currently rendering fiber. Only set when a component is being rendered.
 */
var React$INTERNAL$workInProgressFiber = null;
/**
 * The previous state hook, or null if no state hook has been evaluated yet.
 */
var React$INTERNAL$workInProgressState = null;
/**
 * Queue of updates triggered *during* render.
 */
var React$INTERNAL$renderPhaseUpdateQueue = [];
/**
 * Public API to create a new "root", this is where React attaches rendering to a host element.
 * In our case we don't actually have a real host, and currently only "render" to strings.
 */
function React$createRoot() {
  return new React$INTERNAL$Root();
}
/**
 * Hook to create (on initial render) or access (on update) a state, using the index of the useState
 * call within the component as the identity. Thus conditionally calling this API can cause state to
 * be lost.
 */
function React$useState(
/**
 * Initial value of the state
 */
initial) {
  var root = React$INTERNAL$workInProgressRoot;
  var fiber = React$INTERNAL$workInProgressFiber;
  invariant$default(fiber !== null && root !== null, 'useState() called outside of render');
  var state;
  var _workInProgressState = React$INTERNAL$workInProgressState;
  if (_workInProgressState === null) {
    // Get or initialize the first state on the fiber
    var nextState = fiber.state;
    if (nextState === null) {
      nextState = new React$INTERNAL$State(initial);
      fiber.state = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = CHECKED_CAST$default(nextState);
  } else {
    var _nextState = _workInProgressState.next;
    if (_nextState === null) {
      _nextState = new React$INTERNAL$State(initial);
      _workInProgressState.next = _nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = CHECKED_CAST$default(_nextState);
  }
  // NOTE: this should just work because of subtying, State<T> should be subtype of State<mixed>
  React$INTERNAL$workInProgressState = CHECKED_CAST$default(state);
  return [
  // Untyped check that the existing state value has the correct type,
  // This is safe if components follow the rules of hooks
  CHECKED_CAST$default(state.value), function (updater) {
    var update = new React$INTERNAL$Update(fiber, CHECKED_CAST$default(state), CHECKED_CAST$default(updater));
    if (React$INTERNAL$workInProgressFiber !== null) {
      // called during render
      React$INTERNAL$renderPhaseUpdateQueue.push(update);
    } else {
      root.notify(update);
    }
  }];
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
var React$INTERNAL$Update = /*#__PURE__*/function () {
  "use strict";

  function React$INTERNAL$Update(fiber, state, updater) {
    _classCallCheck(this, React$INTERNAL$Update);
    this.fiber = fiber;
    this.state = state;
    this.updater = updater;
  }
  _createClass(React$INTERNAL$Update, [{
    key: "run",
    value: function run() {
      var state = this.state;
      var value = state.value;
      var updater = this.updater;
      if (typeof updater === 'function') {
        // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
        // thus the fact that updater is a function here menas its a function of T => T.
        var fn = CHECKED_CAST$default(updater);
        value = fn(state.value);
      } else {
        // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
        // thus the fact that updater is *not* a function here means it is a T
        value = CHECKED_CAST$default(updater);
      }
      var changed = !Object.is(state.value, value);
      state.value = value;
      return changed;
    }
  }]);
  return React$INTERNAL$Update;
}();
var React$INTERNAL$Root = /*#__PURE__*/function () {
  "use strict";

  function React$INTERNAL$Root() {
    _classCallCheck(this, React$INTERNAL$Root);
    this.root = null;
    this.element = null;
    this.updateQueue = [];
  }
  _createClass(React$INTERNAL$Root, [{
    key: "notify",
    value: function notify(update) {
      var _this = this;
      this.updateQueue.push(update);
      if (this.updateQueue.length === 1) {
        React$INTERNAL$queueMicrotask(function () {
          var element = _this.element;
          invariant$default(element !== null, 'Expected an element to be set after rendering');
          _this.doWork(CHECKED_CAST$default(element));
        });
      }
    }
  }, {
    key: "render",
    value: function render(element) {
      invariant$default(React$INTERNAL$workInProgressFiber === null && React$INTERNAL$workInProgressState === null, 'Cannot render, an existing render is in progress');
      var hasChanges = element !== this.element;
      this.element = element;
      if (hasChanges) {
        this.doWork(element);
      }
      invariant$default(this.root !== null, 'Expected root to be rendered');
      var root = CHECKED_CAST$default(this.root);
      var output = [];
      this.printFiber(root, output);
      // return output.join('');
      return React$INTERNAL$fastArrayJoin(output, '');
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
      React$INTERNAL$workInProgressRoot = this;
      var fiber = this.root;
      if (fiber === null) {
        fiber = this.mountFiber(element, null);
        this.root = fiber;
      }
      while (fiber !== null) {
        // Render the fiber, which creates child/sibling nodes
        var fiber2 = CHECKED_CAST$default(fiber);
        this.renderFiber(fiber2);
        // advance to the next fiber
        if (fiber2.child !== null) {
          fiber = fiber2.child;
        } else if (fiber2.sibling !== null) {
          fiber = fiber2.sibling;
        } else {
          fiber = fiber2.parent;
          while (fiber !== null && CHECKED_CAST$default(fiber).sibling === null) {
            fiber = CHECKED_CAST$default(fiber).parent;
          }
          if (fiber !== null) {
            fiber = CHECKED_CAST$default(fiber).sibling;
          }
        }
      }
      React$INTERNAL$workInProgressRoot = null;
    }
  }, {
    key: "printFiber",
    value: function printFiber(fiber, out) {
      switch (fiber.type.kind) {
        case 'host':
          {
            var tag = CHECKED_CAST$default(fiber.type).tag;
            out.push('<' + tag);
            for (var prop of Object.entries(fiber.props)) {
              var _JSON$stringify;
              out.push(` ${prop.prop}=${(_JSON$stringify = JSON.stringify(prop.value)) != null ? _JSON$stringify : 'undefined'}`);
            }
            out.push('>');
            this.printChildren(fiber, out);
            out.push('</' + tag + '>');
            break;
          }
        case 'text':
          {
            var text = CHECKED_CAST$default(fiber.type).text;
            out.push(text);
            break;
          }
        case 'component':
          {
            this.printChildren(fiber, out);
            break;
          }
      }
    }
  }, {
    key: "printChildren",
    value: function printChildren(fiber, out) {
      var current = fiber.child;
      while (current !== null) {
        this.printFiber(CHECKED_CAST$default(current), out);
        current = CHECKED_CAST$default(current).sibling;
      }
    }
  }, {
    key: "renderFiber",
    value: function renderFiber(fiber) {
      try {
        React$INTERNAL$workInProgressFiber = fiber;
        React$INTERNAL$workInProgressState = null;
        switch (fiber.type.kind) {
          case 'component':
            {
              invariant$default(React$INTERNAL$renderPhaseUpdateQueue.length === 0, 'Expected no queued render updates');
              var render = CHECKED_CAST$default(fiber.type).component;
              var element = render(fiber.props);
              var iterationCount = 0;
              while (React$INTERNAL$renderPhaseUpdateQueue.length !== 0) {
                iterationCount++;
                invariant$default(iterationCount < 1000, 'Possible infinite loop with setState during render');
                var hasChanges = false;
                for (var update of React$INTERNAL$renderPhaseUpdateQueue) {
                  invariant$default(update.fiber === fiber, 'setState() during render is currently only supported when updating the component ' + 'being rendered. Setting state from another component is not supported.');
                  hasChanges = update.run() || hasChanges;
                }
                React$INTERNAL$renderPhaseUpdateQueue.length = 0;
                if (!hasChanges) {
                  break;
                }
                element = render(fiber.props);
              }
              fiber.child = this.reconcileFiber(fiber, fiber.child, element);
              break;
            }
          case 'host':
          case 'text':
            {
              // Nothing to reconcile, these nodes are visited by the main doWork() loop
              break;
            }
        }
      } finally {
        React$INTERNAL$workInProgressFiber = null;
        React$INTERNAL$workInProgressState = null;
      }
    }
  }, {
    key: "mountFiber",
    value: function mountFiber(elementOrString, parent) {
      var fiber;
      if (typeof elementOrString === 'object') {
        var element = CHECKED_CAST$default(elementOrString);
        if (typeof element.type === 'function') {
          var component = CHECKED_CAST$default(element.type);
          // const type: FiberType = {
          //   kind: 'component',
          //   component,
          // };
          var type = new React$INTERNAL$FiberTypeComponent(component);
          fiber = new React$INTERNAL$Fiber(type, element.props, element.key);
        } else {
          invariant$default(typeof element.type === 'string', 'Expected a host component name such as "div" or "span", got ' + CHECKED_CAST$default(element.type));
          // const type: FiberType = {
          //   kind: 'host',
          //   tag: element.type,
          // };
          var _type = new React$INTERNAL$FiberTypeHost(CHECKED_CAST$default(element.type));
          invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
          // const {children, ...props} = element.props;
          var children = element.props.children;
          var _props2 = Object.assign({}, element.props);
          delete _props2.children;
          fiber = new React$INTERNAL$Fiber(_type, _props2, element.key);
          if (Array.isArray(children)) {
            var _prev = null;
            for (var childElement of CHECKED_CAST$default(children)) {
              var child = this.mountFiber(CHECKED_CAST$default(childElement), fiber);
              if (_prev !== null) {
                CHECKED_CAST$default(_prev).sibling = child;
              } else {
                // set parent to point to first child
                fiber.child = child;
              }
              _prev = child;
            }
          } else if (typeof children === 'string') {
            var _child = new React$INTERNAL$Fiber({
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
        invariant$default(typeof elementOrString === 'string', 'Expected a string');
        // const type: FiberType = {
        //   kind: 'text',
        //   text: element,
        // };
        var _type2 = new React$INTERNAL$FiberTypeText(CHECKED_CAST$default(elementOrString));
        fiber = new React$INTERNAL$Fiber(_type2, {}, null);
      }
      fiber.parent = parent;
      return fiber;
    }
  }, {
    key: "reconcileFiber",
    value: function reconcileFiber(parent, prevChild, element) {
      if (prevChild !== null && CHECKED_CAST$default(prevChild).type === element.type) {
        var _prevChild = CHECKED_CAST$default(_prevChild);
        // Only host nodes have to be reconciled: otherwise this is a function component
        // and its children will be reconciled when they are later emitted in a host
        // position (ie as a direct result of render)
        if (_prevChild.type.kind === 'host') {
          invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
          // const {children, ...props} = element.props;
          var children = element.props.children;
          var _props3 = Object.assign({}, element.props);
          delete _props3.children;
          _prevChild.props = _props3;
          this.reconcileChildren(_prevChild, children);
        } else if (_prevChild.type.kind === 'component') {
          invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
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
        var childrenArray = CHECKED_CAST$default(children);
        // Fast-path for empty and single-element arrays
        if (childrenArray.length === 0) {
          parent.child = null;
        } else if (childrenArray.length === 1) {
          parent.child = this.reconcileFiber(parent, prevChild, childrenArray[0]);
          CHECKED_CAST$default(parent.child).sibling = null;
        } else {
          this.reconcileMultipleChildren(parent, childrenArray);
        }
      } else if (typeof children === 'string') {
        if (prevChild === null || CHECKED_CAST$default(prevChild).type.kind !== 'text') {
          var child = new React$INTERNAL$Fiber({
            kind: 'text',
            text: children
          }, {}, null);
          parent.child = child;
        } else {
          CHECKED_CAST$default(CHECKED_CAST$default(prevChild).type).text = CHECKED_CAST$default(children);
        }
      } else if (children != null) {
        parent.child = this.reconcileFiber(parent, prevChild, CHECKED_CAST$default(children));
        CHECKED_CAST$default(parent.child).sibling = null;
      } else {
        parent.child = null;
        if (prevChild !== null) {
          CHECKED_CAST$default(prevChild).parent = null;
        }
      }
    }
  }, {
    key: "reconcileMultipleChildren",
    value: function reconcileMultipleChildren(parent, children) {
      invariant$default(children.length > 1, 'Expected children to have multiple elements');
      // map existing children by key to make subsequent lookup O(log n)
      var keyedChildren = new Map();
      var current = parent.child;
      while (current !== null) {
        if (CHECKED_CAST$default(current).key !== null) {
          keyedChildren.set(CHECKED_CAST$default(current).key, current);
        }
        current = CHECKED_CAST$default(current).sibling;
      }
      var prev = null; // previous fiber at this key/index
      var prevByIndex = parent.child; // keep track of prev fiber at this index
      for (var childElement of children) {
        var _ref;
        var prevFiber = (_ref = childElement.key != null ? keyedChildren.get(childElement.key) : null) != null ? _ref : prevByIndex;
        var child = void 0;
        if (prevFiber != null) {
          child = this.reconcileFiber(parent, prevFiber, childElement);
        } else {
          child = this.mountFiber(childElement, parent);
        }
        if (prev !== null) {
          CHECKED_CAST$default(prev).sibling = child;
        } else {
          // set parent to point to first child
          parent.child = child;
        }
        prev = child;
        prevByIndex = prevByIndex !== null ? CHECKED_CAST$default(prevByIndex).sibling : null;
      }
    }
  }]);
  return React$INTERNAL$Root;
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
var React$INTERNAL$FiberType = /*#__PURE__*/_createClass(function React$INTERNAL$FiberType(kind) {
  "use strict";

  _classCallCheck(this, React$INTERNAL$FiberType);
  this.kind = kind;
});
var React$INTERNAL$FiberTypeComponent = /*#__PURE__*/function (_React$INTERNAL$Fiber) {
  "use strict";

  _inherits(React$INTERNAL$FiberTypeComponent, _React$INTERNAL$Fiber);
  var _super = _createSuper(React$INTERNAL$FiberTypeComponent);
  function React$INTERNAL$FiberTypeComponent(component) {
    var _this2;
    _classCallCheck(this, React$INTERNAL$FiberTypeComponent);
    _this2 = _super.call(this, 'component');
    _this2.component = component;
    return _this2;
  }
  return _createClass(React$INTERNAL$FiberTypeComponent);
}(React$INTERNAL$FiberType);
var React$INTERNAL$FiberTypeHost = /*#__PURE__*/function (_React$INTERNAL$Fiber2) {
  "use strict";

  _inherits(React$INTERNAL$FiberTypeHost, _React$INTERNAL$Fiber2);
  var _super2 = _createSuper(React$INTERNAL$FiberTypeHost);
  function React$INTERNAL$FiberTypeHost(tag) {
    var _this3;
    _classCallCheck(this, React$INTERNAL$FiberTypeHost);
    _this3 = _super2.call(this, 'host');
    _this3.tag = tag;
    return _this3;
  }
  return _createClass(React$INTERNAL$FiberTypeHost);
}(React$INTERNAL$FiberType);
var React$INTERNAL$FiberTypeText = /*#__PURE__*/function (_React$INTERNAL$Fiber3) {
  "use strict";

  _inherits(React$INTERNAL$FiberTypeText, _React$INTERNAL$Fiber3);
  var _super3 = _createSuper(React$INTERNAL$FiberTypeText);
  function React$INTERNAL$FiberTypeText(text) {
    var _this4;
    _classCallCheck(this, React$INTERNAL$FiberTypeText);
    _this4 = _super3.call(this, 'text');
    _this4.text = text;
    return _this4;
  }
  return _createClass(React$INTERNAL$FiberTypeText);
}(React$INTERNAL$FiberType);
/**
 * The type of component props as seen by the framework, because processing is heterogenous
 * the framework only looks at the identity of prop values and does not otherwise make any
 * assumptions about which props may exist and what their types are.
 */
/**
 * Data storage for the useState() hook
 */
var React$INTERNAL$State = /*#__PURE__*/_createClass(function React$INTERNAL$State(value) {
  "use strict";

  _classCallCheck(this, React$INTERNAL$State);
  this.value = value;
  this.next = null;
  this.prev = null;
});
/**
 * Represents a node in the UI tree, and may correspond to a user-defined function component,
 * a host node, or a text node.
 */
var React$INTERNAL$Fiber = /*#__PURE__*/_createClass(function React$INTERNAL$Fiber(type, props, key) {
  "use strict";

  _classCallCheck(this, React$INTERNAL$Fiber);
  this.type = type;
  this.props = props;
  this.key = key;
  this.parent = null;
  this.child = null;
  this.sibling = null;
  this.state = null;
});
function React$createElement(type, props, key) {
  'inline';

  return {
    type: type,
    props: props,
    key: key,
    ref: null
  };
}
/* file: index.js */
function index$INTERNAL$Title(props) {
  return React$createElement('h1', {
    children: props.children
  }, null);
}
function index$INTERNAL$MyComponent(_props) {
  return React$createElement('div', {
    children: [React$createElement(index$INTERNAL$Title, {
      children: 'Hello'
    }, null), ' world!']
  }, null);
}
function index$INTERNAL$run() {
  var N = 1;
  for (var i = 0; i < N; ++i) {
    var root = React$createRoot();
    var rendered = root.render(React$createElement(index$INTERNAL$MyComponent, {}, null));
  }
  print(rendered);
}
index$INTERNAL$run();
//# sourceMappingURL=MiniReact-es5.js.map
