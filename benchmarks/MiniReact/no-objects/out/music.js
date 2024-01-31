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
function M$react_invariant$default(condition: boolean, format: string): void {
  'inline';

  if (!condition) {
    throw new Error(format);
  }
}
/* file: packages/sh/CHECKED_CAST.js */
function M$sh_CHECKED_CAST$default<T>(value: mixed): T {
  'inline';

  return (value: any);
}
/* file: packages/sh/microtask.js */
let M$sh_microtask$INTERNAL$microtaskQueue = [];
function M$sh_microtask$drainMicrotaskQueue(): void {
  for (let i = 0; i < M$sh_microtask$INTERNAL$microtaskQueue.length; i++) {
    M$sh_microtask$INTERNAL$microtaskQueue[i]();
    M$sh_microtask$INTERNAL$microtaskQueue[i] = undefined;
  }
  M$sh_microtask$INTERNAL$microtaskQueue = [];
}
function M$sh_microtask$queueMicrotask(callback: () => void): void {
  M$sh_microtask$INTERNAL$microtaskQueue.push(callback);
}
/* file: packages/sh/fastarray.js */
function M$sh_fastarray$join(arr: string[], sep: string): string {
  let result: string = '';
  for (let i: number = 0, e = arr.length; i < e; ++i) {
    if (i !== 0) result += sep;
    result += arr[i];
  }
  return result;
}
function M$sh_fastarray$reduce<TInput, TAcc>(arr: TInput[], fn: (acc: TAcc, item: TInput, index: number) => TAcc, initialValue: TAcc): TAcc {
  let acc = initialValue;
  for (let i = 0, e = arr.length; i < e; ++i) {
    acc = fn(acc, arr[i], i);
  }
  return acc;
}
function M$sh_fastarray$map<TInput, TOutput>(arr: TInput[], fn: (item: TInput, index: number) => TOutput): TOutput[] {
  const output: TOutput[] = [];
  for (let i = 0, e = arr.length; i < e; ++i) {
    output.push(fn(arr[i], i));
  }
  return output;
}
function M$sh_fastarray$includes<T>(arr: T[], searchElement: T): boolean {
  for (let i = 0, e = arr.length; i < e; ++i) {
    if (arr[i] === searchElement) {
      return true;
    }
  }
  return false;
}
/* file: packages/react/index.js */
function M$react_index$INTERNAL$padString(str: string, len: number): string {
  let result: string = '';
  for (let i: number = 0; i < len; i++) {
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
type M$react_index$INTERNAL$React$ElementType = string | M$react_index$INTERNAL$Component /* TODO: React$AbstractComponent<empty, mixed> */ | number /* TODO: symbol */;
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
class M$react_index$INTERNAL$React$Element<ElementType> {
  type: ElementType;
  props: M$react_index$Props;
  key: M$react_index$INTERNAL$React$Key | null;
  ref: any;
  constructor(type: ElementType, props: M$react_index$Props, key: M$react_index$INTERNAL$React$Key | null, ref: any) {
    this.type = type;
    this.props = props;
    this.key = key;
    this.ref = ref;
  }
}
type M$react_index$React$MixedElement = M$react_index$INTERNAL$React$Element<M$react_index$INTERNAL$React$ElementType>;
type M$react_index$INTERNAL$React$NodeWithoutArray = M$react_index$React$MixedElement | string | null | void;
type M$react_index$React$Node = M$react_index$INTERNAL$React$NodeWithoutArray[] | M$react_index$INTERNAL$React$NodeWithoutArray;
/**
 * The type of the key that React uses to determine where items in a new list
 * have moved.
 */
type M$react_index$INTERNAL$React$Key = string | number;
const M$react_index$INTERNAL$REACT_FRAGMENT_TYPE: number = 1 /* Symbol.for('react.fragment') */;
/* eslint-disable lint/strictly-null, lint/react-state-props-mutation, lint/flow-react-element */

/**
 * The current root
 */
let M$react_index$INTERNAL$workInProgressRoot: M$react_index$INTERNAL$Root | null = null;
/**
 * The currently rendering fiber. Only set when a component is being rendered.
 */
let M$react_index$INTERNAL$workInProgressFiber: M$react_index$INTERNAL$Fiber | null = null;
/**
 * The previous state hook, or null if no state hook has been evaluated yet.
 */
let M$react_index$INTERNAL$workInProgressState: M$react_index$INTERNAL$State<mixed> | null = null;
/**
 * Queue of updates triggered *during* render.
 */
const M$react_index$INTERNAL$renderPhaseUpdateQueue: M$react_index$INTERNAL$Update<mixed>[] = [];
/**
 * Public API to create a new "root", this is where React attaches rendering to a host element.
 * In our case we don't actually have a real host, and currently only "render" to strings.
 */
function M$react_index$createRoot(): M$react_index$INTERNAL$Root {
  return new M$react_index$INTERNAL$Root();
}
/**
 * Hook to create (on initial render) or access (on update) a state, using the index of the useState
 * call within the component as the identity. Thus conditionally calling this API can cause state to
 * be lost.
 */
function M$react_index$useState<T>(
/**
 * Initial value of the state
 */
initial: T): [T, (value: T | ((prev: T) => T)) => void] {
  const root: M$react_index$INTERNAL$Root = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Root>(M$react_index$INTERNAL$workInProgressRoot);
  const fiber: M$react_index$INTERNAL$Fiber = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(M$react_index$INTERNAL$workInProgressFiber);
  M$react_invariant$default(fiber !== null && root !== null, 'useState() called outside of render');
  let state: M$react_index$INTERNAL$State<T>;
  const _workInProgressState: M$react_index$INTERNAL$State<mixed> | null = M$react_index$INTERNAL$workInProgressState;
  if (_workInProgressState === null) {
    // Get or initialize the first state on the fiber
    let nextState = fiber.state;
    if (nextState === null) {
      nextState = new M$react_index$INTERNAL$State<mixed>(initial);
      fiber.state = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$State<T>>(nextState);
  } else {
    let nextState = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$State<mixed>>(_workInProgressState).next;
    if (nextState === null) {
      nextState = new M$react_index$INTERNAL$State<mixed>(initial);
      M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$State<mixed>>(_workInProgressState).next = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$State<T>>(nextState);
  }
  // NOTE: this should just work because of subtying, State<T> should be subtype of State<mixed>
  M$react_index$INTERNAL$workInProgressState = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$State<mixed>>(state);
  return [
  // Untyped check that the existing state value has the correct type,
  // This is safe if components follow the rules of hooks
  M$sh_CHECKED_CAST$default<T>(state.value), (updater: T | ((prev: T) => T)): void => {
    const update = new M$react_index$INTERNAL$Update<mixed>(fiber, M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$State<mixed>>(state), M$sh_CHECKED_CAST$default<T | ((prev: T) => T)>(updater));
    if (M$react_index$INTERNAL$workInProgressFiber !== null) {
      // called during render
      M$react_index$INTERNAL$renderPhaseUpdateQueue.push(update);
    } else {
      root.notify(update);
    }
  }];
}
const M$react_index$INTERNAL$callbacks = new Map();
function M$react_index$callOnClickOrChange(id: string, event: any): void {
  const callback = M$react_index$INTERNAL$callbacks.get(id);
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
class M$react_index$INTERNAL$Update<T> {
  fiber: M$react_index$INTERNAL$Fiber;
  state: M$react_index$INTERNAL$State<T>;
  updater: T | ((prev: T) => T);
  constructor(fiber: M$react_index$INTERNAL$Fiber, state: M$react_index$INTERNAL$State<T>, updater: T | ((prev: T) => T)) {
    this.fiber = fiber;
    this.state = state;
    this.updater = updater;
  }
  run(): boolean {
    const state = this.state;
    let value: T = state.value;
    const updater = this.updater;
    if (typeof updater === 'function') {
      // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
      // thus the fact that updater is a function here menas its a function of T => T.
      const fn = M$sh_CHECKED_CAST$default<(prev: T) => T>(updater);
      value = fn(state.value);
    } else {
      // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
      // thus the fact that updater is *not* a function here means it is a T
      value = M$sh_CHECKED_CAST$default<T>(updater);
    }
    const changed = !Object.is(state.value, value);
    state.value = value;
    return changed;
  }
}
class M$react_index$INTERNAL$Root {
  root: M$react_index$INTERNAL$Fiber | null;
  element: M$react_index$React$MixedElement | null;
  updateQueue: M$react_index$INTERNAL$Update<mixed>[];
  constructor() {
    this.root = null;
    this.element = null;
    this.updateQueue = ([]: M$react_index$INTERNAL$Update<mixed>[]);
  }
  notify(update: M$react_index$INTERNAL$Update<mixed>): void {
    this.updateQueue.push(update);
    if (this.updateQueue.length === 1) {
      M$sh_microtask$queueMicrotask((): void => {
        const element = this.element;
        M$react_invariant$default(element !== null, 'Expected an element to be set after rendering');
        this.doWork(M$sh_CHECKED_CAST$default<M$react_index$React$MixedElement>(element));
      });
    }
  }
  render(element: M$react_index$React$MixedElement): string {
    M$react_invariant$default(M$react_index$INTERNAL$workInProgressFiber === null && M$react_index$INTERNAL$workInProgressState === null, 'Cannot render, an existing render is in progress');
    const hasChanges = element !== this.element;
    this.element = element;
    if (hasChanges) {
      this.doWork(element);
    }
    M$react_invariant$default(this.root !== null, 'Expected root to be rendered');
    const root: M$react_index$INTERNAL$Fiber = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(this.root);
    const output: string[] = [];
    this.printFiber(root, output, 0);
    return M$sh_fastarray$join(output, '\n');
  }
  doWork(element: M$react_index$React$MixedElement): void {
    let mustRender = this.root === null;
    for (const update of this.updateQueue) {
      mustRender = update.run() || mustRender;
    }
    this.updateQueue = ([]: M$react_index$INTERNAL$Update<mixed>[]);
    if (!mustRender) {
      return;
    }
    // Visit the tree in pre-order, rendering each node
    // and then processing its children
    // eslint-disable-next-line consistent-this
    M$react_index$INTERNAL$workInProgressRoot = this;
    let fiber = this.root;
    if (fiber === null) {
      fiber = this.mountFiber(element, null);
      this.root = fiber;
    }
    while (fiber !== null) {
      // Render the fiber, which creates child/sibling nodes
      let fiber2: M$react_index$INTERNAL$Fiber = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(fiber);
      this.renderFiber(fiber2);
      // advance to the next fiber
      if (fiber2.child !== null) {
        fiber = fiber2.child;
      } else if (fiber2.sibling !== null) {
        fiber = fiber2.sibling;
      } else {
        fiber = fiber2.parent;
        while (fiber !== null && M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(fiber).sibling === null) {
          fiber = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(fiber).parent;
        }
        if (fiber !== null) {
          fiber = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(fiber).sibling;
        }
      }
    }
    M$react_index$INTERNAL$workInProgressRoot = null;
  }
  printFiber(fiber: M$react_index$INTERNAL$Fiber, out: string[], level: number): void {
    switch (fiber.type.kind) {
      case 'host':
        {
          const tag = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$FiberTypeHost>(fiber.type).tag;
          const padStr = M$react_index$INTERNAL$padString(' ', level);
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
          const text = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$FiberTypeText>(fiber.type).text;
          if (text !== '') {
            out.push(M$react_index$INTERNAL$padString(' ', level) + text);
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
  printChildren(fiber: M$react_index$INTERNAL$Fiber, out: string[], level: number): void {
    let current: M$react_index$INTERNAL$Fiber | null = fiber.child;
    while (current !== null) {
      this.printFiber(M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(current), out, level);
      current = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(current).sibling;
    }
  }
  renderFiber(fiber: M$react_index$INTERNAL$Fiber): void {
    try {
      M$react_index$INTERNAL$workInProgressFiber = fiber;
      M$react_index$INTERNAL$workInProgressState = null;
      switch (fiber.type.kind) {
        case 'component':
          {
            M$react_invariant$default(M$react_index$INTERNAL$renderPhaseUpdateQueue.length === 0, 'Expected no queued render updates');
            const render: (props: M$react_index$Props) => M$react_index$React$MixedElement = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$FiberTypeComponent>(fiber.type).component;
            let element = render(fiber.props);
            let iterationCount: number = 0;
            while (M$react_index$INTERNAL$renderPhaseUpdateQueue.length !== 0) {
              iterationCount++;
              M$react_invariant$default(iterationCount < 1000, 'Possible infinite loop with setState during render');
              let hasChanges = false;
              for (const update of M$react_index$INTERNAL$renderPhaseUpdateQueue) {
                M$react_invariant$default(update.fiber === fiber, 'setState() during render is currently only supported when updating the component ' + 'being rendered. Setting state from another component is not supported.');
                hasChanges = update.run() || hasChanges;
              }
              M$react_index$INTERNAL$renderPhaseUpdateQueue.length = 0;
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
                M$react_index$INTERNAL$callbacks.set(id, onClick);
              }
              const onChange = fiber.props.onChange;
              if (onChange != null) {
                M$react_index$INTERNAL$callbacks.set(id, onChange);
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
      M$react_index$INTERNAL$workInProgressFiber = null;
      M$react_index$INTERNAL$workInProgressState = null;
    }
  }
  mountFiber(elementOrString: M$react_index$React$Node, parent: M$react_index$INTERNAL$Fiber | null): M$react_index$INTERNAL$Fiber {
    let fiber: M$react_index$INTERNAL$Fiber;
    // TODO: Support Array of Node's being returned from a component.
    if (typeof elementOrString === 'object') {
      const element = M$sh_CHECKED_CAST$default<M$react_index$React$MixedElement>(elementOrString);
      if (typeof element.type === 'function') {
        const component: M$react_index$INTERNAL$Component = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Component>(element.type);
        const type: M$react_index$INTERNAL$FiberType = new M$react_index$INTERNAL$FiberTypeComponent(component);
        fiber = new M$react_index$INTERNAL$Fiber(type, (element.props: any), element.key);
      } else if (typeof element.type === 'string') {
        M$react_invariant$default(typeof element.type === 'string', 'Expected a host component name such as "div" or "span", got ' + typeof element.type);
        const type: M$react_index$INTERNAL$FiberType = new M$react_index$INTERNAL$FiberTypeHost(M$sh_CHECKED_CAST$default<string>(element.type));
        M$react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
        // const {children, ...props} = element.props;
        const children = element.props.children;
        const props = {
          ...element.props
        };
        delete props.children;
        fiber = new M$react_index$INTERNAL$Fiber(type, props, element.key);
        this.mountChildren(children, fiber);
      } else {
        switch (element.type) {
          case M$react_index$INTERNAL$REACT_FRAGMENT_TYPE:
            {
              const type: M$react_index$INTERNAL$FiberType = new M$react_index$INTERNAL$FiberTypeFragment();
              fiber = new M$react_index$INTERNAL$Fiber(type, (element.props: any), element.key);
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
      const type = new M$react_index$INTERNAL$FiberTypeText(M$sh_CHECKED_CAST$default<string>(elementOrString));
      fiber = new M$react_index$INTERNAL$Fiber(type, {}, null);
    } else {
      throw new Error(`Unexpected element type of ${typeof elementOrString}`);
    }
    fiber.parent = parent;
    return fiber;
  }
  mountChildren(children: M$react_index$React$Node, parentFiber: M$react_index$INTERNAL$Fiber): void {
    if (Array.isArray(children)) {
      let prev: M$react_index$INTERNAL$Fiber | null = null;
      for (const childElement of M$sh_CHECKED_CAST$default<any[]>(children)) {
        if (childElement == null) {
          continue;
        }
        const child = this.mountFiber(M$sh_CHECKED_CAST$default<M$react_index$React$Node>(childElement), parentFiber);
        if (prev !== null) {
          M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(prev).sibling = child;
        } else {
          // set parent to point to first child
          parentFiber.child = child;
        }
        prev = child;
      }
    } else if (children != null) {
      const child = this.mountFiber((children: any), parentFiber);
      parentFiber.child = child;
    }
  }
  reconcileFiber(parent: M$react_index$INTERNAL$Fiber, prevChild: M$react_index$INTERNAL$Fiber | null, element: M$react_index$React$MixedElement): M$react_index$INTERNAL$Fiber {
    if (prevChild !== null && M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(prevChild).type === element.type) {
      let prevChild: M$react_index$INTERNAL$Fiber = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(prevChild);
      // Only host and fragment nodes have to be reconciled: otherwise this is a
      // function component and its children will be reconciled when they are later
      // emitted in a host position (ie as a direct result of render)
      switch (prevChild.type.kind) {
        case 'host':
          {
            M$react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
            // const {children, ...props} = element.props;
            const children = element.props.children;
            const props = {
              ...element.props
            };
            delete props.children;
            prevChild.props = props;
            this.reconcileChildren(prevChild, (children: any));
            break;
          }
        case 'fragment':
          {
            M$react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
            const children = element.props.children;
            this.reconcileChildren(prevChild, (children: any));
            break;
          }
        case 'component':
          {
            M$react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
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
  reconcileChildren(parent: M$react_index$INTERNAL$Fiber, children: M$react_index$React$Node): void {
    const prevChild: M$react_index$INTERNAL$Fiber | null = parent.child;
    if (Array.isArray(children)) {
      let childrenArray = M$sh_CHECKED_CAST$default<M$react_index$React$MixedElement[]>(children);
      // Fast-path for empty and single-element arrays
      if (childrenArray.length === 0) {
        parent.child = null;
      } else if (childrenArray.length === 1) {
        parent.child = this.reconcileFiber(parent, prevChild, childrenArray[0]);
        M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(parent.child).sibling = null;
      } else {
        this.reconcileMultipleChildren(parent, childrenArray);
      }
    } else if (typeof children === 'string') {
      if (prevChild === null || M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(prevChild).type.kind !== 'text') {
        const child = new M$react_index$INTERNAL$Fiber({
          kind: 'text',
          text: children
        }, {}, null);
        parent.child = child;
      } else {
        M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$FiberTypeText>(M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(prevChild).type).text = M$sh_CHECKED_CAST$default<string>(children);
      }
    } else if (children != null) {
      parent.child = this.reconcileFiber(parent, prevChild, M$sh_CHECKED_CAST$default<M$react_index$React$MixedElement>(children));
      M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(parent.child).sibling = null;
    } else {
      parent.child = null;
      if (prevChild !== null) {
        M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(prevChild).parent = null;
      }
    }
  }
  reconcileMultipleChildren(parent: M$react_index$INTERNAL$Fiber, children: M$react_index$React$MixedElement[]): void {
    M$react_invariant$default(children.length > 1, 'Expected children to have multiple elements');
    // map existing children by key to make subsequent lookup O(log n)
    const keyedChildren: any = new Map();
    let current: M$react_index$INTERNAL$Fiber | null = parent.child;
    while (current !== null) {
      if (M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(current).key !== null) {
        keyedChildren.set(M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(current).key, current);
      }
      current = M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(current).sibling;
    }
    let prev: M$react_index$INTERNAL$Fiber | null = null; // previous fiber at this key/index
    let prevByIndex: M$react_index$INTERNAL$Fiber | null = parent.child; // keep track of prev fiber at this index
    for (const childElement of children) {
      const prevFiber = (childElement.key != null ? keyedChildren.get(childElement.key) : null) ?? prevByIndex;
      let child: M$react_index$INTERNAL$Fiber;
      if (prevFiber != null) {
        child = this.reconcileFiber(parent, prevFiber, childElement);
      } else {
        child = this.mountFiber(childElement, parent);
      }
      if (prev !== null) {
        M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(prev).sibling = child;
      } else {
        // set parent to point to first child
        parent.child = child;
      }
      prev = child;
      prevByIndex = prevByIndex !== null ? M$sh_CHECKED_CAST$default<M$react_index$INTERNAL$Fiber>(prevByIndex).sibling : null;
    }
  }
}
/**
 * Describes the `type` field of Fiber, which can hold different data depending on the fiber's kind:
 * - Component stores a function of props => element.
 * - Host stores the name of the host component, ie "div"
 * - Text stores the text itself.
 */
type M$react_index$INTERNAL$Component = (props: M$react_index$Props) => M$react_index$React$MixedElement;
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

class M$react_index$INTERNAL$FiberType {
  kind: string;
  constructor(kind: string) {
    this.kind = kind;
  }
}
class M$react_index$INTERNAL$FiberTypeComponent extends M$react_index$INTERNAL$FiberType {
  component: M$react_index$INTERNAL$Component;
  constructor(component: M$react_index$INTERNAL$Component) {
    super('component');
    this.component = component;
  }
}
class M$react_index$INTERNAL$FiberTypeHost extends M$react_index$INTERNAL$FiberType {
  tag: string;
  constructor(tag: string) {
    super('host');
    this.tag = tag;
  }
}
class M$react_index$INTERNAL$FiberTypeFragment extends M$react_index$INTERNAL$FiberType {
  constructor() {
    super('fragment');
  }
}
class M$react_index$INTERNAL$FiberTypeText extends M$react_index$INTERNAL$FiberType {
  text: string;
  constructor(text: string) {
    super('text');
    this.text = text;
  }
}
/**
 * The type of component props as seen by the framework, because processing is heterogenous
 * the framework only looks at the identity of prop values and does not otherwise make any
 * assumptions about which props may exist and what their types are.
 */
type M$react_index$Props = any;
/**
 * Data storage for the useState() hook
 */
class M$react_index$INTERNAL$State<T> {
  value: T;
  next: M$react_index$INTERNAL$State<T> | null;
  prev: M$react_index$INTERNAL$State<T> | null;
  constructor(value: T) {
    this.value = value;
    this.next = null;
    this.prev = null;
  }
}
/**
 * Represents a node in the UI tree, and may correspond to a user-defined function component,
 * a host node, or a text node.
 */
class M$react_index$INTERNAL$Fiber {
  type: M$react_index$INTERNAL$FiberType;
  props: M$react_index$Props;
  parent: M$react_index$INTERNAL$Fiber | null;
  child: M$react_index$INTERNAL$Fiber | null;
  sibling: M$react_index$INTERNAL$Fiber | null;
  state: M$react_index$INTERNAL$State<mixed> | null;
  key: M$react_index$INTERNAL$React$Key | null;
  constructor(type: M$react_index$INTERNAL$FiberType, props: M$react_index$Props, key: M$react_index$INTERNAL$React$Key | null) {
    this.type = type;
    this.props = props;
    this.key = key;
    this.parent = null;
    this.child = null;
    this.sibling = null;
    this.state = null;
  }
}
function M$react_index$jsx(type: M$react_index$INTERNAL$React$ElementType, props: M$react_index$Props, key: M$react_index$INTERNAL$React$Key | null): M$react_index$React$MixedElement {
  'inline';

  return {
    type: type,
    props: props,
    key: key,
    ref: null
  };
}
function M$react_index$Fragment(props: M$react_index$Props): M$react_index$React$MixedElement {
  'inline';

  return {
    type: M$react_index$INTERNAL$REACT_FRAGMENT_TYPE,
    props: props,
    key: null,
    ref: null
  };
}
function M$react_index$forwardRef(comp: (props: M$react_index$Props, ref: mixed) => M$react_index$React$MixedElement): M$react_index$INTERNAL$Component {
  return (props: M$react_index$Props): M$react_index$React$MixedElement => comp(props, null);
}
/* file: app/music/data/albums.js */
type M$albums$Album = any;
// TODO: Need Object support
// export type Album = {
//   name: string
//   artist: string
//   cover: string
// };

const M$albums$listenNowAlbums: M$albums$Album[] = [{
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
const M$albums$madeForYouAlbums: M$albums$Album[] = [{
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
function M$next_image$default(props: M$react_index$Props): M$react_index$React$MixedElement {
  return M$react_index$jsx('img', {
    ...props
  }, null);
}
/* file: packages/@radix-ui/react-icons/index.js */
const M$radix_ui_react_icons_index$PlusCircledIcon = M$react_index$forwardRef( /*<SVGSVGElement, IconProps>*/({
  color = 'currentColor',
  ...props
}: M$react_index$Props, forwardedRef: any): M$react_index$React$MixedElement => {
  return M$react_index$jsx('svg', {
    width: "15",
    height: "15",
    viewBox: "0 0 15 15",
    fill: "none",
    xmlns: "http://www.w3.org/2000/svg",
    ...props,
    ref: forwardedRef,
    children: M$react_index$jsx('path', {
      d: "M7.49991 0.876892C3.84222 0.876892 0.877075 3.84204 0.877075 7.49972C0.877075 11.1574 3.84222 14.1226 7.49991 14.1226C11.1576 14.1226 14.1227 11.1574 14.1227 7.49972C14.1227 3.84204 11.1576 0.876892 7.49991 0.876892ZM1.82707 7.49972C1.82707 4.36671 4.36689 1.82689 7.49991 1.82689C10.6329 1.82689 13.1727 4.36671 13.1727 7.49972C13.1727 10.6327 10.6329 13.1726 7.49991 13.1726C4.36689 13.1726 1.82707 10.6327 1.82707 7.49972ZM7.50003 4C7.77617 4 8.00003 4.22386 8.00003 4.5V7H10.5C10.7762 7 11 7.22386 11 7.5C11 7.77614 10.7762 8 10.5 8H8.00003V10.5C8.00003 10.7761 7.77617 11 7.50003 11C7.22389 11 7.00003 10.7761 7.00003 10.5V8H4.50003C4.22389 8 4.00003 7.77614 4.00003 7.5C4.00003 7.22386 4.22389 7 4.50003 7H7.00003V4.5C7.00003 4.22386 7.22389 4 7.50003 4Z",
      fill: color,
      fillRule: "evenodd",
      clipRule: "evenodd"
    }, null)
  }, null);
});
/* file: packages/class-variance-authority/index.js */
function M$class_variance_authority_index$cva(base: string[] | string, variants: mixed): (opts: mixed) => string {
  const baseString: string = typeof base === 'string' ? M$sh_CHECKED_CAST$default<string>(base) : M$sh_fastarray$join(M$sh_CHECKED_CAST$default<string[]>(base), ' ');
  return (opts: mixed): string => baseString;
}
/* file: lib/utils.js */
// TODO switch from legacy function when SH supports rest args.
function M$utils$cn(...rest) {
  return rest.join(' ');
}
/* file: registry/new-york/ui/button.js */
const M$button$buttonVariants = M$class_variance_authority_index$cva('inline-flex items-center justify-center whitespace-nowrap rounded-md text-sm font-medium transition-colors focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-ring disabled:pointer-events-none disabled:opacity-50', {
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
const M$button$Button = M$react_index$forwardRef(({
  className,
  variant,
  size,
  asChild = false,
  ...props
}, ref): M$react_index$React$MixedElement => {
  return M$react_index$jsx('button', {
    className: M$utils$cn(M$button$buttonVariants({
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
const M$radix_ui_react_primitive_index$INTERNAL$NODES: string[] = ['a', 'button', 'div', 'form', 'h2', 'h3', 'img', 'input', 'label', 'li', 'nav', 'ol', 'p', 'span', 'svg', 'ul'];
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
type M$radix_ui_react_primitive_index$INTERNAL$Primitives = any;
// type PrimitivePropsWithRef<E extends React.ElementType> =
//   React.ComponentPropsWithRef<E> & {
//     asChild?: boolean,
//   };

// interface PrimitiveForwardRefComponent<E extends React.ElementType>
//   extends React.ForwardRefExoticComponent<PrimitivePropsWithRef<E>> {}

/* -------------------------------------------------------------------------------------------------
 * Primitive
 * -----------------------------------------------------------------------------------------------*/

const M$radix_ui_react_primitive_index$Primitive = M$sh_fastarray$reduce<string, M$radix_ui_react_primitive_index$INTERNAL$Primitives>(M$radix_ui_react_primitive_index$INTERNAL$NODES, (primitive: M$radix_ui_react_primitive_index$INTERNAL$Primitives, node: string, _i: number): M$radix_ui_react_primitive_index$INTERNAL$Primitives => {
  const Node = M$react_index$forwardRef((props /* PrimitivePropsWithRef<typeof node> */: any, forwardedRef: any): M$react_index$React$MixedElement => {
    const {
      asChild,
      ...primitiveProps
    } = props;
    const Comp: any = asChild ? Slot : node;
    // TODO?
    // React.useEffect(() => {
    //   (window as any)[Symbol.for('radix-ui')] = true;
    // }, []);

    return M$react_index$jsx(Comp, {
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
}, ({}: M$radix_ui_react_primitive_index$INTERNAL$Primitives));
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

const M$radix_ui_react_primitive_index$Root = M$radix_ui_react_primitive_index$Primitive;
/* file: packages/@radix-ui/react-separator/index.js */
/* -------------------------------------------------------------------------------------------------
 *  Separator
 * -----------------------------------------------------------------------------------------------*/
const M$radix_ui_react_separator_index$INTERNAL$DEFAULT_ORIENTATION = 'horizontal';
const M$radix_ui_react_separator_index$INTERNAL$ORIENTATIONS: string[] = ['horizontal', 'vertical'];
type M$radix_ui_react_separator_index$SeparatorProps = any;
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

const M$radix_ui_react_separator_index$Separator = M$react_index$forwardRef( /*<SeparatorElement, SeparatorProps>*/(props: M$react_index$Props, forwardedRef: any): M$react_index$React$MixedElement => {
  const {
    decorative,
    orientation: orientationProp = M$radix_ui_react_separator_index$INTERNAL$DEFAULT_ORIENTATION,
    ...domProps
  } = props;
  const orientation = M$radix_ui_react_separator_index$INTERNAL$isValidOrientation(orientationProp) ? orientationProp : M$radix_ui_react_separator_index$INTERNAL$DEFAULT_ORIENTATION;
  // `aria-orientation` defaults to `horizontal` so we only need it if `orientation` is vertical
  const ariaOrientation = orientation === 'vertical' ? orientation : undefined;
  const semanticProps = decorative ? {
    role: 'none'
  } : {
    'aria-orientation': ariaOrientation,
    role: 'separator'
  };
  return M$react_index$jsx(M$radix_ui_react_primitive_index$Primitive.div, {
    'data-orientation': orientation,
    ...semanticProps,
    ...domProps,
    ref: forwardedRef
  }, null);
});
function M$radix_ui_react_separator_index$INTERNAL$isValidOrientation(orientation: string): boolean {
  return M$sh_fastarray$includes<string>(M$radix_ui_react_separator_index$INTERNAL$ORIENTATIONS, orientation);
}
const M$radix_ui_react_separator_index$Root = M$radix_ui_react_separator_index$Separator;
/* file: registry/new-york/ui/separator.js */
const M$separator$Separator = M$react_index$forwardRef(
/*<
React.ElementRef<typeof SeparatorPrimitive.Root>,
React.ComponentPropsWithoutRef<typeof SeparatorPrimitive.Root>,
>*/
({
  className,
  orientation = 'horizontal',
  decorative = true,
  ...props
}: M$react_index$Props, ref: any): M$react_index$React$MixedElement => M$react_index$jsx(M$radix_ui_react_separator_index$Root, {
  ref: ref,
  decorative: decorative,
  orientation: orientation,
  className: M$utils$cn('shrink-0 bg-border', orientation === 'horizontal' ? 'h-[1px] w-full' : 'h-full w-[1px]', className),
  ...props
}, null));
/* file: packages/@radix-ui/react-tabs/index.js */
// TODO: https://github.com/radix-ui/primitives/blob/main/packages/react/tabs/src/Tabs.tsx
function M$radix_ui_react_tabs_index$List(props: M$react_index$Props): M$react_index$React$MixedElement {
  return M$react_index$jsx('div', {
    ...props
  }, null);
}
function M$radix_ui_react_tabs_index$Trigger(props: M$react_index$Props): M$react_index$React$MixedElement {
  return M$react_index$jsx('div', {
    ...props
  }, null);
}
function M$radix_ui_react_tabs_index$Content(props: M$react_index$Props): M$react_index$React$MixedElement {
  return M$react_index$jsx('div', {
    ...props
  }, null);
}
function M$radix_ui_react_tabs_index$Tabs(props: M$react_index$Props): M$react_index$React$MixedElement {
  return M$react_index$jsx('div', {
    ...props
  }, null);
}
const M$radix_ui_react_tabs_index$Root = M$radix_ui_react_tabs_index$Tabs;
/* file: registry/new-york/ui/tabs.js */
const M$tabs$Tabs = M$radix_ui_react_tabs_index$Root;
const M$tabs$TabsList = M$react_index$forwardRef(
/*<
React.ElementRef<typeof TabsPrimitive.List>,
React.ComponentPropsWithoutRef<typeof TabsPrimitive.List>,
>*/
({
  className,
  ...props
}: M$react_index$Props, ref: any): M$react_index$React$MixedElement => M$react_index$jsx(M$radix_ui_react_tabs_index$List, {
  ref: ref,
  className: M$utils$cn('inline-flex h-9 items-center justify-center rounded-lg bg-muted p-1 text-muted-foreground', className),
  ...props
}, null));
// TODO: SH does not support setting properties on functions
// TabsList.displayName = TabsPrimitive.List.displayName;

const M$tabs$TabsTrigger = M$react_index$forwardRef(
/*<
React.ElementRef<typeof TabsPrimitive.Trigger>,
React.ComponentPropsWithoutRef<typeof TabsPrimitive.Trigger>,
>*/
({
  className,
  ...props
}: M$react_index$Props, ref: any): M$react_index$React$MixedElement => M$react_index$jsx(M$radix_ui_react_tabs_index$Trigger, {
  ref: ref,
  className: M$utils$cn('inline-flex items-center justify-center whitespace-nowrap rounded-md px-3 py-1 text-sm font-medium ring-offset-background transition-all focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2 disabled:pointer-events-none disabled:opacity-50 data-[state=active]:bg-background data-[state=active]:text-foreground data-[state=active]:shadow', className),
  ...props
}, null));
// TODO: SH does not support setting properties on functions
// TabsTrigger.displayName = TabsPrimitive.Trigger.displayName;

const M$tabs$TabsContent = M$react_index$forwardRef(
/*<
React.ElementRef<typeof TabsPrimitive.Content>,
React.ComponentPropsWithoutRef<typeof TabsPrimitive.Content>,
>*/
({
  className,
  ...props
}: M$react_index$Props, ref: any): M$react_index$React$MixedElement => M$react_index$jsx(M$radix_ui_react_tabs_index$Content, {
  ref: ref,
  className: M$utils$cn('mt-2 ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2', className),
  ...props
}, null));
/* file: app/music/data/playlists.js */
type M$playlists$Playlist = string;
const M$playlists$playlists = ['Recently Added', 'Recently Played', 'Top Songs', 'Top Albums', 'Top Artists', 'Logic Discography', 'Bedtime Beats', 'Feeling Happy', 'I miss Y2K Pop', 'Runtober', 'Mellow Days', 'Eminem Essentials'];
/* file: app/music/components/album-artwork.js */
type M$album_artwork$INTERNAL$AlbumArtworkProps = M$react_index$Props;
// TODO: Support objects
// type AlbumArtworkProps = {
//   ...React.HTMLAttributes<HTMLDivElement>,
//   album: Album
//   aspectRatio?: "portrait" | "square"
//   width?: number
//   height?: number
// };

function M$album_artwork$AlbumArtwork({
  album,
  aspectRatio = 'portrait',
  width,
  height,
  className,
  ...props
}: M$album_artwork$INTERNAL$AlbumArtworkProps): M$react_index$React$MixedElement {
  return M$react_index$jsx('div', {
    className: M$utils$cn('space-y-3', className),
    ...props,
    children: [null, M$react_index$jsx('div', {
      className: "overflow-hidden rounded-md",
      children: M$react_index$jsx(M$next_image$default, {
        src: album.cover,
        alt: album.name,
        width: width,
        height: height,
        className: M$utils$cn('h-auto w-auto object-cover transition-all hover:scale-105', aspectRatio === 'portrait' ? 'aspect-[3/4]' : 'aspect-square')
      }, null)
    }, null), null, null, M$react_index$jsx('div', {
      className: "space-y-1 text-sm",
      children: [M$react_index$jsx('h3', {
        className: "font-medium leading-none",
        children: album.name
      }, null), M$react_index$jsx('p', {
        className: "text-xs text-muted-foreground",
        children: album.artist
      }, null)]
    }, null)]
  }, null);
}
/* file: app/music/page.js */
function M$page$default(props: M$react_index$Props): M$react_index$React$MixedElement {
  return M$react_index$jsx(M$react_index$Fragment, {
    children: [M$react_index$jsx('div', {
      className: "md:hidden",
      children: [M$react_index$jsx(M$next_image$default, {
        src: "/examples/music-light.png",
        width: 1280,
        height: 1114,
        alt: "Music",
        className: "block dark:hidden"
      }, null), M$react_index$jsx(M$next_image$default, {
        src: "/examples/music-dark.png",
        width: 1280,
        height: 1114,
        alt: "Music",
        className: "hidden dark:block"
      }, null)]
    }, null), M$react_index$jsx('div', {
      className: "hidden md:block",
      children: [null, M$react_index$jsx('div', {
        className: "border-t",
        children: M$react_index$jsx('div', {
          className: "bg-background",
          children: M$react_index$jsx('div', {
            className: "grid lg:grid-cols-5",
            children: [null, M$react_index$jsx('div', {
              className: "col-span-3 lg:col-span-4 lg:border-l",
              children: M$react_index$jsx('div', {
                className: "h-full px-4 py-6 lg:px-8",
                children: M$react_index$jsx(M$tabs$Tabs, {
                  defaultValue: "music",
                  className: "h-full space-y-6",
                  children: [M$react_index$jsx('div', {
                    className: "space-between flex items-center",
                    children: [M$react_index$jsx(M$tabs$TabsList, {
                      children: [M$react_index$jsx(M$tabs$TabsTrigger, {
                        value: "music",
                        className: "relative",
                        children: 'Music'
                      }, null), M$react_index$jsx(M$tabs$TabsTrigger, {
                        value: "podcasts",
                        children: 'Podcasts'
                      }, null), M$react_index$jsx(M$tabs$TabsTrigger, {
                        value: "live",
                        disabled: true,
                        children: 'Live'
                      }, null)]
                    }, null), M$react_index$jsx('div', {
                      className: "ml-auto mr-4",
                      children: M$react_index$jsx(M$button$Button, {
                        children: [M$react_index$jsx(M$radix_ui_react_icons_index$PlusCircledIcon, {
                          className: "mr-2 h-4 w-4"
                        }, null), 'Add music']
                      }, null)
                    }, null)]
                  }, null), M$react_index$jsx(M$tabs$TabsContent, {
                    value: "music",
                    className: "border-none p-0 outline-none",
                    children: [M$react_index$jsx('div', {
                      className: "flex items-center justify-between",
                      children: M$react_index$jsx('div', {
                        className: "space-y-1",
                        children: [M$react_index$jsx('h2', {
                          className: "text-2xl font-semibold tracking-tight",
                          children: 'Listen Now'
                        }, null), M$react_index$jsx('p', {
                          className: "text-sm text-muted-foreground",
                          children: 'Top picks for you. Updated daily.'
                        }, null)]
                      }, null)
                    }, null), M$react_index$jsx(M$separator$Separator, {
                      className: "my-4"
                    }, null), M$react_index$jsx('div', {
                      className: "relative",
                      children: [null, M$react_index$jsx('div', {
                        className: "flex space-x-4 pb-4",
                        children: M$sh_fastarray$map<M$albums$Album, M$react_index$React$MixedElement>(M$albums$listenNowAlbums, (album: M$albums$Album, _i: number): M$react_index$React$MixedElement => M$react_index$jsx(M$album_artwork$AlbumArtwork, {
                          album: album,
                          className: "w-[250px]",
                          aspectRatio: "portrait",
                          width: 250,
                          height: 330
                        }, album.name))
                      }, null), null]
                    }, null), M$react_index$jsx('div', {
                      className: "mt-6 space-y-1",
                      children: [M$react_index$jsx('h2', {
                        className: "text-2xl font-semibold tracking-tight",
                        children: 'Made for You'
                      }, null), M$react_index$jsx('p', {
                        className: "text-sm text-muted-foreground",
                        children: 'Your personal playlists. Updated daily.'
                      }, null)]
                    }, null), M$react_index$jsx(M$separator$Separator, {
                      className: "my-4"
                    }, null), M$react_index$jsx('div', {
                      className: "relative",
                      children: [null, M$react_index$jsx('div', {
                        className: "flex space-x-4 pb-4",
                        children: M$sh_fastarray$map<M$albums$Album, M$react_index$React$MixedElement>(M$albums$madeForYouAlbums, (album: M$albums$Album, _i: number): M$react_index$React$MixedElement => M$react_index$jsx(M$album_artwork$AlbumArtwork, {
                          album: album,
                          className: "w-[150px]",
                          aspectRatio: "square",
                          width: 150,
                          height: 150
                        }, album.name))
                      }, null), null]
                    }, null)]
                  }, null), M$react_index$jsx(M$tabs$TabsContent, {
                    value: "podcasts",
                    className: "h-full flex-col border-none p-0 data-[state=active]:flex",
                    children: [M$react_index$jsx('div', {
                      className: "flex items-center justify-between",
                      children: M$react_index$jsx('div', {
                        className: "space-y-1",
                        children: [M$react_index$jsx('h2', {
                          className: "text-2xl font-semibold tracking-tight",
                          children: 'New Episodes'
                        }, null), M$react_index$jsx('p', {
                          className: "text-sm text-muted-foreground",
                          children: 'Your favorite podcasts. Updated daily.'
                        }, null)]
                      }, null)
                    }, null), M$react_index$jsx(M$separator$Separator, {
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
function M$index$INTERNAL$printIf1(i: number, str: string): void {
  if (i === 1) {
    print('===============================');
    print(str);
    print('===============================');
  }
}
function M$index$INTERNAL$run(N: number): void {
  for (let i: number = 1; i <= N; ++i) {
    const root = M$react_index$createRoot();
    const rootElement = M$react_index$jsx(M$page$default, {}, null);
    M$index$INTERNAL$printIf1(i, root.render(rootElement));

    // React.callOnClickOrChange('click-me', null);
    // drainMicrotaskQueue();
    // printIf1(i, root.render(rootElement));
  }
}
M$index$INTERNAL$run(1);
//# sourceMappingURL=music.js.map
