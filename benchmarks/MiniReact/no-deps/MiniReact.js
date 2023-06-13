/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

function invariant(condition: boolean, format: string) {
  if (!condition) {
    var error = new Error(format);
  }
}

function queueMicrotask(callback: (...args: Array<mixed>) => void) {
  HermesInternal.enqueueJob(callback);
}

/**
 * The type of an element in React. A React element may be a:
 *
 * - String. These elements are intrinsics that depend on the React renderer
 *   implementation.
 * - React component. See `ComponentType` for more information about its
 *   different variants.
 */
type React$ElementType =
  | string
  | Component /* TODO: React$AbstractComponent<empty, mixed> */;

/**
 * Type of a React element. React elements are commonly created using JSX
 * literals, which desugar to React.createElement calls (see below).
 */
type React$Element<
  ElementType: React$ElementType,
> = {|
  type: ElementType,
  props: Props,
  key: React$Key | null,
  ref: any,
|};

type React$MixedElement = React$Element<React$ElementType>;

/**
 * The type of the key that React uses to determine where items in a new list
 * have moved.
 */
type React$Key = string | number;

/* eslint-disable lint/strictly-null, lint/react-state-props-mutation, lint/flow-react-element */

/**
 * The current root
 */
let workInProgressRoot: Root | null = null;

/**
 * The currently rendering fiber. Only set when a component is being rendered.
 */
let workInProgressFiber: Fiber | null = null;

/**
 * The previous state hook, or null if no state hook has been evaluated yet.
 */
let workInProgressState: State<mixed> | null = null;

/**
 * Queue of updates triggered *during* render.
 */
const renderPhaseUpdateQueue: Array<Update<mixed>> = [];

/**
 * Public API to create a new "root", this is where React attaches rendering to a host element.
 * In our case we don't actually have a real host, and currently only "render" to strings.
 */
export function createRoot(): Root {
  return new Root();
}

/**
 * Hook to create (on initial render) or access (on update) a state, using the index of the useState
 * call within the component as the identity. Thus conditionally calling this API can cause state to
 * be lost.
 */
export function useState<T>(
  /**
   * Initial value of the state
   */
  initial: T
): [T, SetState<T>] {
  const root = workInProgressRoot;
  const fiber = workInProgressFiber;
  invariant(
    fiber !== null && root !== null,
    'useState() called outside of render'
  );

  let state: State<T>;
  const _workInProgressState = workInProgressState;
  if (_workInProgressState === null) {
    // Get or initialize the first state on the fiber
    let nextState = fiber.state;
    if (nextState === null) {
      nextState = new State(initial);
      fiber.state = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = UNSAFE_CAST<State<T>>(nextState);
  } else {
    let nextState = _workInProgressState.next;
    if (nextState === null) {
      nextState = new State(initial);
      _workInProgressState.next = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = UNSAFE_CAST<State<T>>(nextState);
  }
  // NOTE: this should just work because of subtying, State<T> should be subtype of State<mixed>
  workInProgressState = UNSAFE_CAST<State<mixed>>(state);
  return [
    // Untyped check that the existing state value has the correct type,
    // This is safe if components follow the rules of hooks
    UNSAFE_CAST<T>(state.value),
    updater => {
      const update = new Update<mixed>(
        fiber,
        UNSAFE_CAST<State<mixed>>(state),
        UNSAFE_CAST<Updater<mixed>>(updater)
      );
      if (workInProgressFiber !== null) {
        // called during render
        renderPhaseUpdateQueue.push(update);
      } else {
        root.notify(update);
      }
    },
  ];
}

/**
 * The type of value that may be passed to the setState function (second part of useState return value).
 * - T: the new value
 * - (prev: T) => T: a function to compute the new value from the old value
 */
type Updater<T> = T | ((prev: T) => T);

/**
 * The type of the setState function (second element of the array returned by useState).
 */
export type SetState<T> = (value: Updater<T>) => void;

/**
 * A queued state update.
 */
class Update<T> {
  fiber: Fiber; // used to check state updates that occur during render to see if they came from the current component.
  state: State<T>;
  updater: Updater<T>;

  constructor(fiber: Fiber, state: State<T>, updater: Updater<T>) {
    this.fiber = fiber;
    this.state = state;
    this.updater = updater;
  }

  /**
   * Process the state update
   */
  run(): boolean {
    const state = this.state;
    let value: T = state.value;
    const updater = this.updater;
    if (typeof updater === 'function') {
      // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
      // thus the fact that updater is a function here menas its a function of T => T.
      const fn = UNSAFE_CAST<(prev: T) => T>(updater);
      value = fn(state.value);
    } else {
      // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
      // thus the fact that updater is *not* a function here means it is a T
      value = UNSAFE_CAST<T>(updater);
    }
    const changed = !Object.is(state.value, value);
    state.value = value;
    return changed;
  }
}

class Root {
  /**
   * The fiber representing the root node (`element`), null until
   * render is first called.
   */
  root: Fiber | null = null;

  /**
   * The last rendered root element, initially null.
   */
  element: React$MixedElement | null = null;

  /**
   * Queue of updates (state changes) to apply on the next render
   */
  updateQueue: Array<Update<mixed>> = [];

  /**
   * Notify the root that an update is scheduled
   */
  notify(update: Update<mixed>): void {
    this.updateQueue.push(update);
    if (this.updateQueue.length === 1) {
      queueMicrotask((..._args) => {
        const element = this.element;
        invariant(
          element !== null,
          'Expected an element to be set after rendering'
        );
        this.doWork(element);
      });
    }
  }

  /**
   * Drive any remaining work to completion and return the rendered result
   */
  render(element: React$MixedElement): string {
    invariant(
      workInProgressFiber === null && workInProgressState === null,
      'Cannot render, an existing render is in progress'
    );
    const hasChanges = element !== this.element;
    this.element = element;
    if (hasChanges) {
      this.doWork(element);
    }

    const root = this.root;
    invariant(root !== null, 'Expected root to be rendered');
    const output: Array<string> = [];
    this.printFiber(root, output);
    return output.join('');
  }

  doWork(element: React$MixedElement): void {
    let mustRender = this.root === null;
    for (const update of this.updateQueue) {
      mustRender = update.run() || mustRender;
    }
    this.updateQueue.length = 0;
    if (!mustRender) {
      return;
    }

    // Visit the tree in pre-order, rendering each node
    // and then processing its children
    // eslint-disable-next-line consistent-this
    workInProgressRoot = this;
    let fiber = this.root;
    if (fiber === null) {
      fiber = this.mountFiber(element, null);
      this.root = fiber;
    }
    while (fiber !== null) {
      // Render the fiber, which creates child/sibling nodes
      this.renderFiber(fiber);
      // advance to the next fiber
      if (fiber.child !== null) {
        fiber = fiber.child;
      } else if (fiber.sibling !== null) {
        fiber = fiber.sibling;
      } else {
        fiber = fiber.parent;
        while (fiber !== null && fiber.sibling === null) {
          fiber = fiber.parent;
        }
        if (fiber !== null) {
          fiber = fiber.sibling;
        }
      }
    }
    workInProgressRoot = null;
  }

  /**
   * Prints a representation of the output DOM as HTML, emitting HTML snippets to @param out.
   */
  printFiber(fiber: Fiber, out: Array<string>): void {
    switch (fiber.type.kind) {
      case 'host': {
        const tag = fiber.type.tag;
        out.push('<' + tag);
        for (const [prop, value] of Object.entries(fiber.props)) {
          out.push(` ${prop}=${JSON.stringify(value) ?? 'undefined'}`);
        }
        out.push('>');
        this.printChildren(fiber, out);
        out.push('</' + tag + '>');
        break;
      }
      case 'text': {
        const text = fiber.type.text;
        out.push(text);
        break;
      }
      case 'component': {
        this.printChildren(fiber, out);
        break;
      }
    }
  }

  printChildren(fiber: Fiber, out: Array<string>): void {
    let current: Fiber | null = fiber.child;
    while (current !== null) {
      this.printFiber(current, out);
      current = current.sibling;
    }
  }

  /**
   * Renders and reconciles the output of the given @param fiber. Note that this does not *render*
   * children, it only reconciles the current output of the fiber with the previous children.
   */
  renderFiber(fiber: Fiber): void {
    try {
      workInProgressFiber = fiber;
      workInProgressState = null;
      switch (fiber.type.kind) {
        case 'component': {
          invariant(
            renderPhaseUpdateQueue.length === 0,
            'Expected no queued render updates'
          );
          const render: (props: Props) => React$MixedElement =
            fiber.type.component;
          let element = render(fiber.props);
          let iterationCount: number = 0;
          while (renderPhaseUpdateQueue.length !== 0) {
            iterationCount++;
            invariant(
              iterationCount < 1000,
              'Possible infinite loop with setState during render'
            );
            let hasChanges = false;
            for (const update of renderPhaseUpdateQueue) {
              invariant(
                update.fiber === fiber,
                'setState() during render is currently only supported when updating the component ' +
                  'being rendered. Setting state from another component is not supported.'
              );
              hasChanges = update.run() || hasChanges;
            }
            renderPhaseUpdateQueue.length = 0;
            if (!hasChanges) {
              break;
            }
            element = render(fiber.props);
          }
          fiber.child = this.reconcileFiber(fiber, fiber.child, element);
          break;
        }
        case 'host':
        case 'text': {
          // Nothing to reconcile, these nodes are visited by the main doWork() loop
          break;
        }
      }
    } finally {
      workInProgressFiber = null;
      workInProgressState = null;
    }
  }

  /**
   * Create a new fiber for the given element. Used when there is no fiber at
   * a given tree position which can be reused.
   */
  mountFiber(element: React$MixedElement, parent: Fiber | null): Fiber {
    let fiber: Fiber;
    if (typeof element.type === 'function') {
      const component: Component = element.type;
      const type: FiberType = {
        kind: 'component',
        component,
      };
      fiber = new Fiber(type, (element.props: $FlowFixMe), element.key);
    } else if (element !== null && typeof element === 'object') {
      invariant(
        typeof element.type === 'string',
        'Expected a host component name such as "div" or "span", got ' +
          element.type
      );
      const type: FiberType = {
        kind: 'host',
        tag: element.type,
      };
      invariant(
        element.props !== null && typeof element.props === 'object',
        'Expected component props'
      );
      const {children, ...props} = element.props;
      fiber = new Fiber(type, props, element.key);
      if (Array.isArray(children)) {
        let prev: Fiber | null = null;
        for (const childElement of children) {
          const child = this.mountFiber((childElement: $FlowFixMe), fiber);
          if (prev !== null) {
            prev.sibling = child;
          } else {
            // set parent to point to first child
            fiber.child = child;
          }
          prev = child;
        }
      } else if (typeof children === 'string') {
        const child = new Fiber({kind: 'text', text: children}, {}, null);
        child.parent = fiber;
        fiber.child = child;
      } else if (children != null) {
        const child = this.mountFiber((children: $FlowFixMe), fiber);
        fiber.child = child;
      }
    } else {
      invariant(typeof element === 'string', 'Expected a string');
      const type: FiberType = {
        kind: 'text',
        text: element,
      };
      fiber = new Fiber(type, {}, null);
    }
    fiber.parent = parent;
    return fiber;
  }

  /**
   * Update's @param parent's child to reflect the latest desired output as described by @param element.
   * This either reuses @param prevChild if present and the component type matches, otherwise it creates
   * a new Fiber.
   */
  reconcileFiber(
    parent: Fiber,
    prevChild: Fiber | null,
    element: React$MixedElement
  ): Fiber {
    if (prevChild !== null && prevChild.type === element.type) {
      // Only host nodes have to be reconciled: otherwise this is a function component
      // and its children will be reconciled when they are later emitted in a host
      // position (ie as a direct result of render)
      if (prevChild.type.kind === 'host') {
        invariant(
          element.props !== null && typeof element.props === 'object',
          'Expected component props'
        );
        const {children, ...props} = element.props;
        prevChild.props = props;
        this.reconcileChildren(prevChild, (children: $FlowFixMe));
      } else if (prevChild.type.kind === 'component') {
        invariant(
          element.props !== null && typeof element.props === 'object',
          'Expected component props'
        );
        prevChild.props = element.props;
      }
      return prevChild;
    } else {
      const child = this.mountFiber(element, parent);
      return child;
    }
  }

  /**
   * Reconciles the @param parent fiber's children nodes.
   */
  reconcileChildren(
    parent: Fiber,
    children:
      | Array<React$MixedElement>
      | React$MixedElement
      | string
      | null
      | void
  ): void {
    const prevChild: Fiber | null = parent.child;
    if (Array.isArray(children)) {
      // Fast-path for empty and single-element arrays
      if (children.length === 0) {
        parent.child = null;
      } else if (children.length === 1) {
        parent.child = this.reconcileFiber(parent, prevChild, children[0]);
        parent.child.sibling = null;
      } else {
        this.reconcileMultipleChildren(parent, children);
      }
    } else if (typeof children === 'string') {
      if (prevChild === null || prevChild.type.kind !== 'text') {
        const child = new Fiber({kind: 'text', text: children}, {}, null);
        parent.child = child;
      } else {
        prevChild.type.text = children;
      }
    } else if (children != null) {
      parent.child = this.reconcileFiber(parent, prevChild, children);
      parent.child.sibling = null;
    } else {
      parent.child = null;
      if (prevChild !== null) {
        prevChild.parent = null;
      }
    }
  }

  /**
   * Reconciles the @param parent fiber's children when the children are known to
   * have 2+ items. Note that the algorithm works for 0+ elements but a fast-path
   * should be used for 0/1 item cases.
   */
  reconcileMultipleChildren(
    parent: Fiber,
    children: Array<React$MixedElement>
  ): void {
    invariant(
      children.length > 1,
      'Expected children to have multiple elements'
    );
    // map existing children by key to make subsequent lookup O(log n)
    const keyedChildren: Map<React$Key, Fiber> = new Map();
    let current: Fiber | null = parent.child;
    while (current !== null) {
      if (current.key !== null) {
        keyedChildren.set(current.key, current);
      }
      current = current.sibling;
    }
    let prev: Fiber | null = null; // previous fiber at this key/index
    let prevByIndex: Fiber | null = parent.child; // keep track of prev fiber at this index
    for (const childElement of children) {
      const prevFiber =
        (childElement.key != null
          ? keyedChildren.get(childElement.key)
          : null) ?? prevByIndex;
      let child: Fiber;
      if (prevFiber != null) {
        child = this.reconcileFiber(parent, prevFiber, childElement);
      } else {
        child = this.mountFiber(childElement, parent);
      }
      if (prev !== null) {
        prev.sibling = child;
      } else {
        // set parent to point to first child
        parent.child = child;
      }
      prev = child;
      prevByIndex = prevByIndex !== null ? prevByIndex.sibling : null;
    }
  }
}

/**
 * Describes the `type` field of Fiber, which can hold different data depending on the fiber's kind:
 * - Component stores a function of props => element.
 * - Host stores the name of the host component, ie "div"
 * - Text stores the text itself.
 */
type Component = (props: Props) => React$MixedElement;
type FiberType =
  | {
      kind: 'component',
      component: Component,
    }
  | {
      kind: 'host',
      tag: string,
    }
  | {
      kind: 'text',
      text: string,
    };

/**
 * The type of component props as seen by the framework, because processing is heterogenous
 * the framework only looks at the identity of prop values and does not otherwise make any
 * assumptions about which props may exist and what their types are.
 */
type Props = {+[prop: string]: mixed};

/**
 * Data storage for the useState() hook
 */
class State<T> {
  value: T;
  next: State<T> | null = null;
  prev: State<T> | null = null;

  constructor(value: T) {
    this.value = value;
  }
}

/**
 * Represents a node in the UI tree, and may correspond to a user-defined function component,
 * a host node, or a text node.
 */
class Fiber {
  type: FiberType;
  props: Props;
  parent: Fiber | null = null;
  child: Fiber | null = null;
  sibling: Fiber | null = null;
  state: State<mixed> | null = null;
  key: React$Key | null;

  constructor(type: FiberType, props: Props, key: React$Key | null) {
    this.type = type;
    this.props = props;
    this.key = key;
  }
}

function UNSAFE_CAST<T>(value: mixed): T {
  return (value: $FlowFixMe);
}
