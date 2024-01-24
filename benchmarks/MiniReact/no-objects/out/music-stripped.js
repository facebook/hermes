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
let sh_microtask$INTERNAL$microtaskQueue = [];
function sh_microtask$drainMicrotaskQueue() {
  for (let i = 0; i < sh_microtask$INTERNAL$microtaskQueue.length; i++) {
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
  let result = '';
  for (let i = 0, e = arr.length; i < e; ++i) {
    if (i !== 0) result += sep;
    result += arr[i];
  }
  return result;
}
function sh_fastarray$reduce(arr, fn, initialValue) {
  let acc = initialValue;
  for (let i = 0, e = arr.length; i < e; ++i) {
    acc = fn(acc, arr[i], i);
  }
  return acc;
}
function sh_fastarray$map(arr, fn) {
  const output = [];
  for (let i = 0, e = arr.length; i < e; ++i) {
    output.push(fn(arr[i], i));
  }
  return output;
}
function sh_fastarray$includes(arr, searchElement) {
  for (let i = 0, e = arr.length; i < e; ++i) {
    if (arr[i] === searchElement) {
      return true;
    }
  }
  return false;
}
/* file: packages/react/index.js */
function react_index$INTERNAL$padString(str, len) {
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
class react_index$INTERNAL$React$Element {
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

const react_index$INTERNAL$REACT_FRAGMENT_TYPE = 1 /* Symbol.for('react.fragment') */;
/* eslint-disable lint/strictly-null, lint/react-state-props-mutation, lint/flow-react-element */

/**
 * The current root
 */
let react_index$INTERNAL$workInProgressRoot = null;
/**
 * The currently rendering fiber. Only set when a component is being rendered.
 */
let react_index$INTERNAL$workInProgressFiber = null;
/**
 * The previous state hook, or null if no state hook has been evaluated yet.
 */
let react_index$INTERNAL$workInProgressState = null;
/**
 * Queue of updates triggered *during* render.
 */
const react_index$INTERNAL$renderPhaseUpdateQueue = [];
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
  const root = sh_CHECKED_CAST$default(react_index$INTERNAL$workInProgressRoot);
  const fiber = sh_CHECKED_CAST$default(react_index$INTERNAL$workInProgressFiber);
  react_invariant$default(fiber !== null && root !== null, 'useState() called outside of render');
  let state;
  const _workInProgressState = react_index$INTERNAL$workInProgressState;
  if (_workInProgressState === null) {
    // Get or initialize the first state on the fiber
    let nextState = fiber.state;
    if (nextState === null) {
      nextState = new react_index$INTERNAL$State(initial);
      fiber.state = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = sh_CHECKED_CAST$default(nextState);
  } else {
    let nextState = sh_CHECKED_CAST$default(_workInProgressState).next;
    if (nextState === null) {
      nextState = new react_index$INTERNAL$State(initial);
      sh_CHECKED_CAST$default(_workInProgressState).next = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = sh_CHECKED_CAST$default(nextState);
  }
  // NOTE: this should just work because of subtying, State<T> should be subtype of State<mixed>
  react_index$INTERNAL$workInProgressState = sh_CHECKED_CAST$default(state);
  return [
  // Untyped check that the existing state value has the correct type,
  // This is safe if components follow the rules of hooks
  sh_CHECKED_CAST$default(state.value), updater => {
    const update = new react_index$INTERNAL$Update(fiber, sh_CHECKED_CAST$default(state), sh_CHECKED_CAST$default(updater));
    if (react_index$INTERNAL$workInProgressFiber !== null) {
      // called during render
      react_index$INTERNAL$renderPhaseUpdateQueue.push(update);
    } else {
      root.notify(update);
    }
  }];
}
const react_index$INTERNAL$callbacks = new Map();
function react_index$callOnClickOrChange(id, event) {
  const callback = react_index$INTERNAL$callbacks.get(id);
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
class react_index$INTERNAL$Update {
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
      const fn = sh_CHECKED_CAST$default(updater);
      value = fn(state.value);
    } else {
      // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
      // thus the fact that updater is *not* a function here means it is a T
      value = sh_CHECKED_CAST$default(updater);
    }
    const changed = !Object.is(state.value, value);
    state.value = value;
    return changed;
  }
}
class react_index$INTERNAL$Root {
  constructor() {
    this.root = null;
    this.element = null;
    this.updateQueue = [];
  }
  notify(update) {
    this.updateQueue.push(update);
    if (this.updateQueue.length === 1) {
      sh_microtask$queueMicrotask(() => {
        const element = this.element;
        react_invariant$default(element !== null, 'Expected an element to be set after rendering');
        this.doWork(sh_CHECKED_CAST$default(element));
      });
    }
  }
  render(element) {
    react_invariant$default(react_index$INTERNAL$workInProgressFiber === null && react_index$INTERNAL$workInProgressState === null, 'Cannot render, an existing render is in progress');
    const hasChanges = element !== this.element;
    this.element = element;
    if (hasChanges) {
      this.doWork(element);
    }
    react_invariant$default(this.root !== null, 'Expected root to be rendered');
    const root = sh_CHECKED_CAST$default(this.root);
    const output = [];
    this.printFiber(root, output, 0);
    return sh_fastarray$join(output, '\n');
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
    react_index$INTERNAL$workInProgressRoot = this;
    let fiber = this.root;
    if (fiber === null) {
      fiber = this.mountFiber(element, null);
      this.root = fiber;
    }
    while (fiber !== null) {
      // Render the fiber, which creates child/sibling nodes
      let fiber2 = sh_CHECKED_CAST$default(fiber);
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
  printFiber(fiber, out, level) {
    switch (fiber.type.kind) {
      case 'host':
        {
          const tag = sh_CHECKED_CAST$default(fiber.type).tag;
          const padStr = react_index$INTERNAL$padString(' ', level);
          let str = padStr + '<' + tag;
          for (const [propName, propValue] of Object.entries(fiber.props)) {
            if (propValue == null || typeof propValue === 'function') {
              continue;
            }
            str += ` ${propName}=${JSON.stringify(propValue) ?? 'undefined'}`;
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
          const text = sh_CHECKED_CAST$default(fiber.type).text;
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
  printChildren(fiber, out, level) {
    let current = fiber.child;
    while (current !== null) {
      this.printFiber(sh_CHECKED_CAST$default(current), out, level);
      current = sh_CHECKED_CAST$default(current).sibling;
    }
  }
  renderFiber(fiber) {
    try {
      react_index$INTERNAL$workInProgressFiber = fiber;
      react_index$INTERNAL$workInProgressState = null;
      switch (fiber.type.kind) {
        case 'component':
          {
            react_invariant$default(react_index$INTERNAL$renderPhaseUpdateQueue.length === 0, 'Expected no queued render updates');
            const render = sh_CHECKED_CAST$default(fiber.type).component;
            let element = render(fiber.props);
            let iterationCount = 0;
            while (react_index$INTERNAL$renderPhaseUpdateQueue.length !== 0) {
              iterationCount++;
              react_invariant$default(iterationCount < 1000, 'Possible infinite loop with setState during render');
              let hasChanges = false;
              for (const update of react_index$INTERNAL$renderPhaseUpdateQueue) {
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
            const id = fiber.props.id;
            if (id != null) {
              const onClick = fiber.props.onClick;
              if (onClick != null) {
                react_index$INTERNAL$callbacks.set(id, onClick);
              }
              const onChange = fiber.props.onChange;
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
  mountFiber(elementOrString, parent) {
    let fiber;
    // TODO: Support Array of Node's being returned from a component.
    if (typeof elementOrString === 'object') {
      const element = sh_CHECKED_CAST$default(elementOrString);
      if (typeof element.type === 'function') {
        const component = sh_CHECKED_CAST$default(element.type);
        const type = new react_index$INTERNAL$FiberTypeComponent(component);
        fiber = new react_index$INTERNAL$Fiber(type, element.props, element.key);
      } else if (typeof element.type === 'string') {
        react_invariant$default(typeof element.type === 'string', 'Expected a host component name such as "div" or "span", got ' + typeof element.type);
        const type = new react_index$INTERNAL$FiberTypeHost(sh_CHECKED_CAST$default(element.type));
        react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
        // const {children, ...props} = element.props;
        const children = element.props.children;
        const props = {
          ...element.props
        };
        delete props.children;
        fiber = new react_index$INTERNAL$Fiber(type, props, element.key);
        this.mountChildren(children, fiber);
      } else {
        switch (element.type) {
          case react_index$INTERNAL$REACT_FRAGMENT_TYPE:
            {
              const type = new react_index$INTERNAL$FiberTypeFragment();
              fiber = new react_index$INTERNAL$Fiber(type, element.props, element.key);
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
      const type = new react_index$INTERNAL$FiberTypeText(sh_CHECKED_CAST$default(elementOrString));
      fiber = new react_index$INTERNAL$Fiber(type, {}, null);
    } else {
      throw new Error(`Unexpected element type of ${typeof elementOrString}`);
    }
    fiber.parent = parent;
    return fiber;
  }
  mountChildren(children, parentFiber) {
    if (Array.isArray(children)) {
      let prev = null;
      for (const childElement of sh_CHECKED_CAST$default(children)) {
        if (childElement == null) {
          continue;
        }
        const child = this.mountFiber(sh_CHECKED_CAST$default(childElement), parentFiber);
        if (prev !== null) {
          sh_CHECKED_CAST$default(prev).sibling = child;
        } else {
          // set parent to point to first child
          parentFiber.child = child;
        }
        prev = child;
      }
    } else if (children != null) {
      const child = this.mountFiber(children, parentFiber);
      parentFiber.child = child;
    }
  }
  reconcileFiber(parent, prevChild, element) {
    if (prevChild !== null && sh_CHECKED_CAST$default(prevChild).type === element.type) {
      let prevChild = sh_CHECKED_CAST$default(prevChild);
      // Only host and fragment nodes have to be reconciled: otherwise this is a
      // function component and its children will be reconciled when they are later
      // emitted in a host position (ie as a direct result of render)
      switch (prevChild.type.kind) {
        case 'host':
          {
            react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
            // const {children, ...props} = element.props;
            const children = element.props.children;
            const props = {
              ...element.props
            };
            delete props.children;
            prevChild.props = props;
            this.reconcileChildren(prevChild, children);
            break;
          }
        case 'fragment':
          {
            react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
            const children = element.props.children;
            this.reconcileChildren(prevChild, children);
            break;
          }
        case 'component':
          {
            react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
            prevChild.props = element.props;
            break;
          }
        default:
          {
            throw new Error(`Unknown node kind ${prevChild.type.kind}`);
          }
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
      let childrenArray = sh_CHECKED_CAST$default(children);
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
        const child = new react_index$INTERNAL$Fiber({
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
  reconcileMultipleChildren(parent, children) {
    react_invariant$default(children.length > 1, 'Expected children to have multiple elements');
    // map existing children by key to make subsequent lookup O(log n)
    const keyedChildren = new Map();
    let current = parent.child;
    while (current !== null) {
      if (sh_CHECKED_CAST$default(current).key !== null) {
        keyedChildren.set(sh_CHECKED_CAST$default(current).key, current);
      }
      current = sh_CHECKED_CAST$default(current).sibling;
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
        sh_CHECKED_CAST$default(prev).sibling = child;
      } else {
        // set parent to point to first child
        parent.child = child;
      }
      prev = child;
      prevByIndex = prevByIndex !== null ? sh_CHECKED_CAST$default(prevByIndex).sibling : null;
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

class react_index$INTERNAL$FiberType {
  constructor(kind) {
    this.kind = kind;
  }
}
class react_index$INTERNAL$FiberTypeComponent extends react_index$INTERNAL$FiberType {
  constructor(component) {
    super('component');
    this.component = component;
  }
}
class react_index$INTERNAL$FiberTypeHost extends react_index$INTERNAL$FiberType {
  constructor(tag) {
    super('host');
    this.tag = tag;
  }
}
class react_index$INTERNAL$FiberTypeFragment extends react_index$INTERNAL$FiberType {
  constructor() {
    super('fragment');
  }
}
class react_index$INTERNAL$FiberTypeText extends react_index$INTERNAL$FiberType {
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
class react_index$INTERNAL$State {
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
class react_index$INTERNAL$Fiber {
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
  return props => comp(props, null);
}
/* file: app/music/data/albums.js */
// TODO: Need Object support
// export type Album = {
//   name: string
//   artist: string
//   cover: string
// };

const albums$listenNowAlbums = [{
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
const albums$madeForYouAlbums = [{
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
  return react_index$jsx('img', {
    ...props
  }, null);
}
/* file: packages/@radix-ui/react-icons/index.js */
const radix_ui_react_icons_index$PlusCircledIcon = react_index$forwardRef( /*<SVGSVGElement, IconProps>*/({
  color = 'currentColor',
  ...props
}, forwardedRef) => {
  return react_index$jsx('svg', {
    width: "15",
    height: "15",
    viewBox: "0 0 15 15",
    fill: "none",
    xmlns: "http://www.w3.org/2000/svg",
    ...props,
    ref: forwardedRef,
    children: react_index$jsx('path', {
      d: "M7.49991 0.876892C3.84222 0.876892 0.877075 3.84204 0.877075 7.49972C0.877075 11.1574 3.84222 14.1226 7.49991 14.1226C11.1576 14.1226 14.1227 11.1574 14.1227 7.49972C14.1227 3.84204 11.1576 0.876892 7.49991 0.876892ZM1.82707 7.49972C1.82707 4.36671 4.36689 1.82689 7.49991 1.82689C10.6329 1.82689 13.1727 4.36671 13.1727 7.49972C13.1727 10.6327 10.6329 13.1726 7.49991 13.1726C4.36689 13.1726 1.82707 10.6327 1.82707 7.49972ZM7.50003 4C7.77617 4 8.00003 4.22386 8.00003 4.5V7H10.5C10.7762 7 11 7.22386 11 7.5C11 7.77614 10.7762 8 10.5 8H8.00003V10.5C8.00003 10.7761 7.77617 11 7.50003 11C7.22389 11 7.00003 10.7761 7.00003 10.5V8H4.50003C4.22389 8 4.00003 7.77614 4.00003 7.5C4.00003 7.22386 4.22389 7 4.50003 7H7.00003V4.5C7.00003 4.22386 7.22389 4 7.50003 4Z",
      fill: color,
      fillRule: "evenodd",
      clipRule: "evenodd"
    }, null)
  }, null);
});
/* file: packages/class-variance-authority/index.js */
function class_variance_authority_index$cva(base, variants) {
  const baseString = typeof base === 'string' ? sh_CHECKED_CAST$default(base) : sh_fastarray$join(sh_CHECKED_CAST$default(base), ' ');
  return opts => baseString;
}
/* file: lib/utils.js */
// TODO switch from legacy function when SH supports rest args.
function utils$cn(...rest) {
  return rest.join(' ');
}
/* file: registry/new-york/ui/button.js */
const button$buttonVariants = class_variance_authority_index$cva('inline-flex items-center justify-center whitespace-nowrap rounded-md text-sm font-medium transition-colors focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-ring disabled:pointer-events-none disabled:opacity-50', {
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
const button$Button = react_index$forwardRef(({
  className,
  variant,
  size,
  asChild = false,
  ...props
}, ref) => {
  return react_index$jsx('button', {
    className: utils$cn(button$buttonVariants({
      variant,
      size,
      className
    })),
    ref: ref,
    ...props
  }, null);
});
/* file: packages/@radix-ui/react-primitive/index.js */
// import * as ReactDOM from 'react-dom';
// import {Slot} from '@radix-ui/react-slot';
const radix_ui_react_primitive_index$INTERNAL$NODES = ['a', 'button', 'div', 'form', 'h2', 'h3', 'img', 'input', 'label', 'li', 'nav', 'ol', 'p', 'span', 'svg', 'ul'];

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

const radix_ui_react_primitive_index$Primitive = sh_fastarray$reduce(radix_ui_react_primitive_index$INTERNAL$NODES, (primitive, node, _i) => {
  const Node = react_index$forwardRef((props /* PrimitivePropsWithRef<typeof node> */, forwardedRef) => {
    const {
      asChild,
      ...primitiveProps
    } = props;
    const Comp = asChild ? Slot : node;
    // TODO?
    // React.useEffect(() => {
    //   (window as any)[Symbol.for('radix-ui')] = true;
    // }, []);

    return react_index$jsx(Comp, {
      ...primitiveProps,
      ref: forwardedRef
    }, null);
  });
  // TODO: SH doesn't support setting properties on functions
  // Node.displayName = `Primitive.${node}`;

  return {
    ...primitive,
    [node]: Node
  };
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

const radix_ui_react_primitive_index$Root = radix_ui_react_primitive_index$Primitive;
/* file: packages/@radix-ui/react-separator/index.js */
/* -------------------------------------------------------------------------------------------------
 *  Separator
 * -----------------------------------------------------------------------------------------------*/
const radix_ui_react_separator_index$INTERNAL$DEFAULT_ORIENTATION = 'horizontal';
const radix_ui_react_separator_index$INTERNAL$ORIENTATIONS = ['horizontal', 'vertical'];
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

const radix_ui_react_separator_index$Separator = react_index$forwardRef( /*<SeparatorElement, SeparatorProps>*/(props, forwardedRef) => {
  const {
    decorative,
    orientation: orientationProp = radix_ui_react_separator_index$INTERNAL$DEFAULT_ORIENTATION,
    ...domProps
  } = props;
  const orientation = radix_ui_react_separator_index$INTERNAL$isValidOrientation(orientationProp) ? orientationProp : radix_ui_react_separator_index$INTERNAL$DEFAULT_ORIENTATION;
  // `aria-orientation` defaults to `horizontal` so we only need it if `orientation` is vertical
  const ariaOrientation = orientation === 'vertical' ? orientation : undefined;
  const semanticProps = decorative ? {
    role: 'none'
  } : {
    'aria-orientation': ariaOrientation,
    role: 'separator'
  };
  return react_index$jsx(radix_ui_react_primitive_index$Primitive.div, {
    'data-orientation': orientation,
    ...semanticProps,
    ...domProps,
    ref: forwardedRef
  }, null);
});
function radix_ui_react_separator_index$INTERNAL$isValidOrientation(orientation) {
  return sh_fastarray$includes(radix_ui_react_separator_index$INTERNAL$ORIENTATIONS, orientation);
}
const radix_ui_react_separator_index$Root = radix_ui_react_separator_index$Separator;
/* file: registry/new-york/ui/separator.js */
const separator$Separator = react_index$forwardRef(
/*<
React.ElementRef<typeof SeparatorPrimitive.Root>,
React.ComponentPropsWithoutRef<typeof SeparatorPrimitive.Root>,
>*/
({
  className,
  orientation = 'horizontal',
  decorative = true,
  ...props
}, ref) => react_index$jsx(radix_ui_react_separator_index$Root, {
  ref: ref,
  decorative: decorative,
  orientation: orientation,
  className: utils$cn('shrink-0 bg-border', orientation === 'horizontal' ? 'h-[1px] w-full' : 'h-full w-[1px]', className),
  ...props
}, null));
/* file: packages/@radix-ui/react-tabs/index.js */
// TODO: https://github.com/radix-ui/primitives/blob/main/packages/react/tabs/src/Tabs.tsx
function radix_ui_react_tabs_index$List(props) {
  return react_index$jsx('div', {
    ...props
  }, null);
}
function radix_ui_react_tabs_index$Trigger(props) {
  return react_index$jsx('div', {
    ...props
  }, null);
}
function radix_ui_react_tabs_index$Content(props) {
  return react_index$jsx('div', {
    ...props
  }, null);
}
function radix_ui_react_tabs_index$Tabs(props) {
  return react_index$jsx('div', {
    ...props
  }, null);
}
const radix_ui_react_tabs_index$Root = radix_ui_react_tabs_index$Tabs;
/* file: registry/new-york/ui/tabs.js */
const tabs$Tabs = radix_ui_react_tabs_index$Root;
const tabs$TabsList = react_index$forwardRef(
/*<
React.ElementRef<typeof TabsPrimitive.List>,
React.ComponentPropsWithoutRef<typeof TabsPrimitive.List>,
>*/
({
  className,
  ...props
}, ref) => react_index$jsx(radix_ui_react_tabs_index$List, {
  ref: ref,
  className: utils$cn('inline-flex h-9 items-center justify-center rounded-lg bg-muted p-1 text-muted-foreground', className),
  ...props
}, null));
// TODO: SH does not support setting properties on functions
// TabsList.displayName = TabsPrimitive.List.displayName;

const tabs$TabsTrigger = react_index$forwardRef(
/*<
React.ElementRef<typeof TabsPrimitive.Trigger>,
React.ComponentPropsWithoutRef<typeof TabsPrimitive.Trigger>,
>*/
({
  className,
  ...props
}, ref) => react_index$jsx(radix_ui_react_tabs_index$Trigger, {
  ref: ref,
  className: utils$cn('inline-flex items-center justify-center whitespace-nowrap rounded-md px-3 py-1 text-sm font-medium ring-offset-background transition-all focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2 disabled:pointer-events-none disabled:opacity-50 data-[state=active]:bg-background data-[state=active]:text-foreground data-[state=active]:shadow', className),
  ...props
}, null));
// TODO: SH does not support setting properties on functions
// TabsTrigger.displayName = TabsPrimitive.Trigger.displayName;

const tabs$TabsContent = react_index$forwardRef(
/*<
React.ElementRef<typeof TabsPrimitive.Content>,
React.ComponentPropsWithoutRef<typeof TabsPrimitive.Content>,
>*/
({
  className,
  ...props
}, ref) => react_index$jsx(radix_ui_react_tabs_index$Content, {
  ref: ref,
  className: utils$cn('mt-2 ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2', className),
  ...props
}, null));
/* file: app/music/data/playlists.js */
const playlists$playlists = ['Recently Added', 'Recently Played', 'Top Songs', 'Top Albums', 'Top Artists', 'Logic Discography', 'Bedtime Beats', 'Feeling Happy', 'I miss Y2K Pop', 'Runtober', 'Mellow Days', 'Eminem Essentials'];
/* file: app/music/components/album-artwork.js */
// TODO: Support objects
// type AlbumArtworkProps = {
//   ...React.HTMLAttributes<HTMLDivElement>,
//   album: Album
//   aspectRatio?: "portrait" | "square"
//   width?: number
//   height?: number
// };

function album_artwork$AlbumArtwork({
  album,
  aspectRatio = 'portrait',
  width,
  height,
  className,
  ...props
}) {
  return react_index$jsx('div', {
    className: utils$cn('space-y-3', className),
    ...props,
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
  }, null);
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
                        children: [react_index$jsx(radix_ui_react_icons_index$PlusCircledIcon, {
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
                        children: sh_fastarray$map(albums$listenNowAlbums, (album, _i) => react_index$jsx(album_artwork$AlbumArtwork, {
                          album: album,
                          className: "w-[250px]",
                          aspectRatio: "portrait",
                          width: 250,
                          height: 330
                        }, album.name))
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
                        children: sh_fastarray$map(albums$madeForYouAlbums, (album, _i) => react_index$jsx(album_artwork$AlbumArtwork, {
                          album: album,
                          className: "w-[150px]",
                          aspectRatio: "square",
                          width: 150,
                          height: 150
                        }, album.name))
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
  for (let i = 1; i <= N; ++i) {
    const root = react_index$createRoot();
    const rootElement = react_index$jsx(page$default, {}, null);
    index$INTERNAL$printIf1(i, root.render(rootElement));

    // React.callOnClickOrChange('click-me', null);
    // drainMicrotaskQueue();
    // printIf1(i, root.render(rootElement));
  }
}
index$INTERNAL$run(1);
//# sourceMappingURL=music-stripped.js.map
