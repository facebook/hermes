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
/* file: microtask.js */
let microtask$INTERNAL$microtaskQueue = [];
function microtask$drainMicrotaskQueue() {
  for (let i = 0; i < microtask$INTERNAL$microtaskQueue.length; i++) {
    microtask$INTERNAL$microtaskQueue[i]();
    microtask$INTERNAL$microtaskQueue[i] = undefined;
  }
  microtask$INTERNAL$microtaskQueue = [];
}
function microtask$queueMicrotask(callback) {
  microtask$INTERNAL$microtaskQueue.push(callback);
}
/* file: React.js */
function React$INTERNAL$fastArrayJoin(arr, sep) {
  let result = '';
  for (let i = 0, e = arr.length; i < e; ++i) {
    if (i !== 0) result += sep;
    result += arr[i];
  }
  return result;
}
function React$INTERNAL$padString(str, len) {
  let result = '';
  for (let i = 0; i < len; i++) {
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
class React$INTERNAL$React$Element {
  constructor(type, props, key, ref) {
    this.type = type;
    this.props = props;
    this.key = key;
    this.ref = ref;
  }
}

/**
 * The type of the key that React uses to determine where items in a new list
 * have moved.
 */

/* eslint-disable lint/strictly-null, lint/react-state-props-mutation, lint/flow-react-element */

/**
 * The current root
 */
let React$INTERNAL$workInProgressRoot = null;
/**
 * The currently rendering fiber. Only set when a component is being rendered.
 */
let React$INTERNAL$workInProgressFiber = null;
/**
 * The previous state hook, or null if no state hook has been evaluated yet.
 */
let React$INTERNAL$workInProgressState = null;
/**
 * Queue of updates triggered *during* render.
 */
const React$INTERNAL$renderPhaseUpdateQueue = [];
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
  const root = CHECKED_CAST$default(React$INTERNAL$workInProgressRoot);
  const fiber = CHECKED_CAST$default(React$INTERNAL$workInProgressFiber);
  invariant$default(fiber !== null && root !== null, 'useState() called outside of render');
  let state;
  const _workInProgressState = React$INTERNAL$workInProgressState;
  if (_workInProgressState === null) {
    // Get or initialize the first state on the fiber
    let nextState = fiber.state;
    if (nextState === null) {
      nextState = new React$INTERNAL$State(initial);
      fiber.state = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = CHECKED_CAST$default(nextState);
  } else {
    let nextState = CHECKED_CAST$default(_workInProgressState).next;
    if (nextState === null) {
      nextState = new React$INTERNAL$State(initial);
      CHECKED_CAST$default(_workInProgressState).next = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = CHECKED_CAST$default(nextState);
  }
  // NOTE: this should just work because of subtying, State<T> should be subtype of State<mixed>
  React$INTERNAL$workInProgressState = CHECKED_CAST$default(state);
  return [
  // Untyped check that the existing state value has the correct type,
  // This is safe if components follow the rules of hooks
  CHECKED_CAST$default(state.value), updater => {
    const update = new React$INTERNAL$Update(fiber, CHECKED_CAST$default(state), CHECKED_CAST$default(updater));
    if (React$INTERNAL$workInProgressFiber !== null) {
      // called during render
      React$INTERNAL$renderPhaseUpdateQueue.push(update);
    } else {
      root.notify(update);
    }
  }];
}
const React$INTERNAL$callbacks = new Map();
function React$callOnClickOrChange(id, event) {
  const callback = React$INTERNAL$callbacks.get(id);
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
class React$INTERNAL$Update {
  constructor(fiber, state, updater) {
    this.fiber = fiber;
    this.state = state;
    this.updater = updater;
  }
  run() {
    const state = this.state;
    let value = state.value;
    const updater = this.updater;
    if (typeof updater === 'function') {
      // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
      // thus the fact that updater is a function here menas its a function of T => T.
      const fn = CHECKED_CAST$default(updater);
      value = fn(state.value);
    } else {
      // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
      // thus the fact that updater is *not* a function here means it is a T
      value = CHECKED_CAST$default(updater);
    }
    const changed = !Object.is(state.value, value);
    state.value = value;
    return changed;
  }
}
class React$INTERNAL$Root {
  constructor() {
    this.root = null;
    this.element = null;
    this.updateQueue = [];
  }
  notify(update) {
    this.updateQueue.push(update);
    if (this.updateQueue.length === 1) {
      microtask$queueMicrotask(() => {
        const element = this.element;
        invariant$default(element !== null, 'Expected an element to be set after rendering');
        this.doWork(CHECKED_CAST$default(element));
      });
    }
  }
  render(element) {
    invariant$default(React$INTERNAL$workInProgressFiber === null && React$INTERNAL$workInProgressState === null, 'Cannot render, an existing render is in progress');
    const hasChanges = element !== this.element;
    this.element = element;
    if (hasChanges) {
      this.doWork(element);
    }
    invariant$default(this.root !== null, 'Expected root to be rendered');
    const root = CHECKED_CAST$default(this.root);
    const output = [];
    this.printFiber(root, output, 0);
    // return output.join('');
    return React$INTERNAL$fastArrayJoin(output, '\n');
  }
  doWork(element) {
    let mustRender = this.root === null;
    for (const update of this.updateQueue) {
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
    let fiber = this.root;
    if (fiber === null) {
      fiber = this.mountFiber(element, null);
      this.root = fiber;
    }
    while (fiber !== null) {
      // Render the fiber, which creates child/sibling nodes
      let fiber2 = CHECKED_CAST$default(fiber);
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
  printFiber(fiber, out, level) {
    switch (fiber.type.kind) {
      case 'host':
        {
          const tag = CHECKED_CAST$default(fiber.type).tag;
          const padStr = React$INTERNAL$padString(' ', level);
          let str = padStr + '<' + tag;
          for (const [propName, propValue] of Object.entries(fiber.props)) {
            if (typeof propValue === 'function') {
              continue;
            }
            str += ` ${propName}=${JSON.stringify(propValue) ?? 'undefined'}`;
          }
          str += '>';
          out.push(str);
          this.printChildren(fiber, out, level + 1);
          out.push(padStr + '</' + tag + '>');
          break;
        }
      case 'text':
        {
          const text = CHECKED_CAST$default(fiber.type).text;
          if (text !== '') {
            out.push(React$INTERNAL$padString(' ', level) + text);
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
  printChildren(fiber, out, level) {
    let current = fiber.child;
    while (current !== null) {
      this.printFiber(CHECKED_CAST$default(current), out, level);
      current = CHECKED_CAST$default(current).sibling;
    }
  }
  renderFiber(fiber) {
    try {
      React$INTERNAL$workInProgressFiber = fiber;
      React$INTERNAL$workInProgressState = null;
      switch (fiber.type.kind) {
        case 'component':
          {
            invariant$default(React$INTERNAL$renderPhaseUpdateQueue.length === 0, 'Expected no queued render updates');
            const render = CHECKED_CAST$default(fiber.type).component;
            let element = render(fiber.props);
            let iterationCount = 0;
            while (React$INTERNAL$renderPhaseUpdateQueue.length !== 0) {
              iterationCount++;
              invariant$default(iterationCount < 1000, 'Possible infinite loop with setState during render');
              let hasChanges = false;
              for (const update of React$INTERNAL$renderPhaseUpdateQueue) {
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
          {
            const id = fiber.props.id;
            if (id != null) {
              const onClick = fiber.props.onClick;
              if (onClick != null) {
                React$INTERNAL$callbacks.set(id, onClick);
              }
              const onChange = fiber.props.onChange;
              if (onChange != null) {
                React$INTERNAL$callbacks.set(id, onChange);
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
      React$INTERNAL$workInProgressFiber = null;
      React$INTERNAL$workInProgressState = null;
    }
  }
  mountFiber(elementOrString, parent) {
    let fiber;
    if (typeof elementOrString === 'object') {
      const element = CHECKED_CAST$default(elementOrString);
      if (typeof element.type === 'function') {
        const component = CHECKED_CAST$default(element.type);
        // const type: FiberType = {
        //   kind: 'component',
        //   component,
        // };
        const type = new React$INTERNAL$FiberTypeComponent(component);
        fiber = new React$INTERNAL$Fiber(type, element.props, element.key);
      } else {
        invariant$default(typeof element.type === 'string', 'Expected a host component name such as "div" or "span", got ' + CHECKED_CAST$default(element.type));
        // const type: FiberType = {
        //   kind: 'host',
        //   tag: element.type,
        // };
        const type = new React$INTERNAL$FiberTypeHost(CHECKED_CAST$default(element.type));
        invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
        // const {children, ...props} = element.props;
        const children = element.props.children;
        const props = {
          ...element.props
        };
        delete props.children;
        fiber = new React$INTERNAL$Fiber(type, props, element.key);
        if (Array.isArray(children)) {
          let prev = null;
          for (const childElement of CHECKED_CAST$default(children)) {
            const child = this.mountFiber(CHECKED_CAST$default(childElement), fiber);
            if (prev !== null) {
              CHECKED_CAST$default(prev).sibling = child;
            } else {
              // set parent to point to first child
              fiber.child = child;
            }
            prev = child;
          }
        } else if (typeof children === 'string') {
          const child = new React$INTERNAL$Fiber({
            kind: 'text',
            text: children
          }, {}, null);
          child.parent = fiber;
          fiber.child = child;
        } else if (children != null) {
          const child = this.mountFiber(children, fiber);
          fiber.child = child;
        }
      }
    } else {
      invariant$default(typeof elementOrString === 'string', 'Expected a string');
      // const type: FiberType = {
      //   kind: 'text',
      //   text: element,
      // };
      const type = new React$INTERNAL$FiberTypeText(CHECKED_CAST$default(elementOrString));
      fiber = new React$INTERNAL$Fiber(type, {}, null);
    }
    fiber.parent = parent;
    return fiber;
  }
  reconcileFiber(parent, prevChild, element) {
    if (prevChild !== null && CHECKED_CAST$default(prevChild).type === element.type) {
      let prevChild = CHECKED_CAST$default(prevChild);
      // Only host nodes have to be reconciled: otherwise this is a function component
      // and its children will be reconciled when they are later emitted in a host
      // position (ie as a direct result of render)
      if (prevChild.type.kind === 'host') {
        invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
        // const {children, ...props} = element.props;
        const children = element.props.children;
        const props = {
          ...element.props
        };
        delete props.children;
        prevChild.props = props;
        this.reconcileChildren(prevChild, children);
      } else if (prevChild.type.kind === 'component') {
        invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
        prevChild.props = element.props;
      }
      return prevChild;
    } else {
      const child = this.mountFiber(element, parent);
      return child;
    }
  }
  reconcileChildren(parent, children) {
    const prevChild = parent.child;
    if (Array.isArray(children)) {
      let childrenArray = CHECKED_CAST$default(children);
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
        const child = new React$INTERNAL$Fiber({
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
  reconcileMultipleChildren(parent, children) {
    invariant$default(children.length > 1, 'Expected children to have multiple elements');
    // map existing children by key to make subsequent lookup O(log n)
    const keyedChildren = new Map();
    let current = parent.child;
    while (current !== null) {
      if (CHECKED_CAST$default(current).key !== null) {
        keyedChildren.set(CHECKED_CAST$default(current).key, current);
      }
      current = CHECKED_CAST$default(current).sibling;
    }
    let prev = null; // previous fiber at this key/index
    let prevByIndex = parent.child; // keep track of prev fiber at this index
    for (const childElement of children) {
      const prevFiber = (childElement.key != null ? keyedChildren.get(childElement.key) : null) ?? prevByIndex;
      let child;
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
}

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

class React$INTERNAL$FiberType {
  constructor(kind) {
    this.kind = kind;
  }
}
class React$INTERNAL$FiberTypeComponent extends React$INTERNAL$FiberType {
  constructor(component) {
    super('component');
    this.component = component;
  }
}
class React$INTERNAL$FiberTypeHost extends React$INTERNAL$FiberType {
  constructor(tag) {
    super('host');
    this.tag = tag;
  }
}
class React$INTERNAL$FiberTypeText extends React$INTERNAL$FiberType {
  constructor(text) {
    super('text');
    this.text = text;
  }
}

/**
 * The type of component props as seen by the framework, because processing is heterogenous
 * the framework only looks at the identity of prop values and does not otherwise make any
 * assumptions about which props may exist and what their types are.
 */

/**
 * Data storage for the useState() hook
 */
class React$INTERNAL$State {
  constructor(value) {
    this.value = value;
    this.next = null;
    this.prev = null;
  }
}
/**
 * Represents a node in the UI tree, and may correspond to a user-defined function component,
 * a host node, or a text node.
 */
class React$INTERNAL$Fiber {
  constructor(type, props, key) {
    this.type = type;
    this.props = props;
    this.key = key;
    this.parent = null;
    this.child = null;
    this.sibling = null;
    this.state = null;
  }
}
function React$jsx(type, props, key) {
  'inline';

  return {
    type: type,
    props: props,
    key: key,
    ref: null
  };
}
function React$Fragment(props) {
  // TODO: Get this to work.
  return props.children;
}
/* file: App.js */
function App$INTERNAL$Button(props) {
  return React$jsx('button', {
    id: props.id,
    onClick: props.onClick,
    children: 'Click me'
  }, null);
}
function App$INTERNAL$Input(props) {
  return React$jsx('input', {
    id: props.id,
    type: "text",
    onChange: props.onChange,
    value: props.value
  }, null);
}
function App$INTERNAL$TextArea(props) {
  return React$jsx('textarea', {
    onChange: props.onChange,
    children: props.value
  }, null);
}
function App$INTERNAL$Select(props) {
  const children = [];
  for (let i = 0; i < props.options.length; i++) {
    const option = props.options[i];
    children.push(React$jsx('option', {
      value: option.value,
      children: option.label
    }, option.value));
  }
  return React$jsx('select', {
    onChange: props.onChange,
    children: children
  }, null);
}
function App$INTERNAL$Checkbox(props) {
  return React$jsx('input', {
    type: "checkbox",
    checked: props.checked,
    onChange: props.onChange
  }, null);
}
function App$INTERNAL$Radio(props) {
  return React$jsx('input', {
    type: "radio",
    checked: props.checked,
    onChange: props.onChange
  }, null);
}
function App$INTERNAL$Slider(props) {
  return React$jsx('input', {
    type: "range",
    min: props.min,
    max: props.max,
    step: props.step,
    value: props.value,
    onChange: props.onChange
  }, null);
}
function App$INTERNAL$ProgressBar(props) {
  return React$jsx('div', {
    style: {
      width: `${props.progress}%`
    }
  }, null);
}
function App$INTERNAL$Spinner(props) {
  return React$jsx('div', {
    className: "spinner",
    children: 'Loading...'
  }, null);
}
function App$INTERNAL$Modal(props) {
  if (!props.isOpen) {
    return React$jsx('div', {
      className: "modal closed"
    }, null);
  }
  return React$jsx('div', {
    className: "modal open",
    children: [React$jsx('div', {
      className: "overlay",
      onClick: props.onClose,
      children: 'X'
    }, null), React$jsx('div', {
      className: "content",
      children: props.children
    }, null)]
  }, null);
}
function App$INTERNAL$Tooltip(props) {
  if (!props.isOpen) {
    return React$jsx('div', {
      className: "tooltip closed"
    }, null);
  }
  return React$jsx('div', {
    className: "tooltip open",
    children: [React$jsx('div', {
      className: "arrow"
    }, null), React$jsx('div', {
      className: "content",
      children: props.children
    }, null)]
  }, null);
}
function App$default(props) {
  const [text, setText] = React$useState('');
  const [number, setNumber] = React$useState(0);
  const [isChecked, setIsChecked] = React$useState(false);
  const [isSelected, setIsSelected] = React$useState(false);
  const [isOpen, setIsOpen] = React$useState(false);
  const [isTooltipOpen, setIsTooltipOpen] = React$useState(true);
  return React$jsx('div', {
    children: [React$jsx('h1', {
      children: 'React Benchmark'
    }, null), React$jsx(App$INTERNAL$Button, {
      id: "toggle-modal",
      onClick: () => setIsOpen(!isOpen),
      children: 'Toggle Modal'
    }, null), React$jsx(App$INTERNAL$Modal, {
      isOpen: isOpen,
      onClose: () => setIsOpen(false),
      children: [React$jsx('h2', {
        children: 'Modal Content'
      }, null), React$jsx('p', {
        children: 'This is some modal content.'
      }, null), React$jsx(App$INTERNAL$Tooltip, {
        isOpen: isTooltipOpen,
        onClose: () => setIsTooltipOpen(false),
        children: [React$jsx('h3', {
          children: 'Tooltip Content'
        }, null), React$jsx('p', {
          children: 'This is some tooltip content.'
        }, null)]
      }, null)]
    }, null), React$jsx('div', {
      children: [React$jsx('h2', {
        children: 'Form Elements'
      }, null), React$jsx(App$INTERNAL$Input, {
        id: "update-text",
        value: text,
        onChange: e => setText(e.target.value)
      }, null), React$jsx(App$INTERNAL$TextArea, {
        value: text,
        onChange: e => setText(e.target.value)
      }, null), React$jsx(App$INTERNAL$Select, {
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
        onChange: e => setNumber(parseInt(e.target.value))
      }, null), React$jsx(App$INTERNAL$Checkbox, {
        checked: isChecked,
        onChange: e => setIsChecked(e.target.checked)
      }, null), React$jsx(App$INTERNAL$Radio, {
        checked: isSelected,
        onChange: e => setIsSelected(e.target.checked)
      }, null), React$jsx(App$INTERNAL$Slider, {
        min: 0,
        max: 100,
        step: 1,
        value: number,
        onChange: e => setNumber(parseInt(e.target.value))
      }, null), React$jsx(App$INTERNAL$ProgressBar, {
        progress: number
      }, null), React$jsx(App$INTERNAL$Spinner, {}, null)]
    }, null)]
  }, null);
}
/* file: index.js */
function index$INTERNAL$printIf1(i, str) {
  if (i === 1) {
    print('===============================');
    print(str);
    print('===============================');
  }
}
function index$INTERNAL$run(N) {
  for (let i = 1; i <= N; ++i) {
    const root = React$createRoot();
    const rootElement = React$jsx(App$default, {}, null);
    index$INTERNAL$printIf1(i, root.render(rootElement));
    React$callOnClickOrChange('toggle-modal', null);
    React$callOnClickOrChange('update-text', {
      target: {
        value: '!!!!! some text !!!!!'
      }
    });
    microtask$drainMicrotaskQueue();
    index$INTERNAL$printIf1(i, root.render(rootElement));
  }
}
index$INTERNAL$run(1);
//# sourceMappingURL=MiniReact-stripped.js.map
