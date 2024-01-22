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
function react_invariant$default(condition: boolean, format: string): void {
  'inline';

  if (!condition) {
    throw new Error(format);
  }
}
/* file: packages/sh/CHECKED_CAST.js */
function sh_CHECKED_CAST$default<T>(value: mixed): T {
  'inline';

  return (value: any);
}
/* file: packages/sh/microtask.js */
let sh_microtask$INTERNAL$microtaskQueue = [];
function sh_microtask$drainMicrotaskQueue(): void {
  for (let i = 0; i < sh_microtask$INTERNAL$microtaskQueue.length; i++) {
    sh_microtask$INTERNAL$microtaskQueue[i]();
    sh_microtask$INTERNAL$microtaskQueue[i] = undefined;
  }
  sh_microtask$INTERNAL$microtaskQueue = [];
}
function sh_microtask$queueMicrotask(callback: () => void): void {
  sh_microtask$INTERNAL$microtaskQueue.push(callback);
}
/* file: packages/sh/fastarray.js */
function sh_fastarray$join(arr: string[], sep: string): string {
  let result: string = '';
  for (let i: number = 0, e = arr.length; i < e; ++i) {
    if (i !== 0) result += sep;
    result += arr[i];
  }
  return result;
}
function sh_fastarray$reduce<TInput, TAcc>(arr: TInput[], fn: (acc: TAcc, item: TInput, index: number) => TAcc, initialValue: TAcc): TAcc {
  let acc = initialValue;
  for (let i = 0, e = arr.length; i < e; ++i) {
    acc = fn(acc, arr[i], i);
  }
  return acc;
}
function sh_fastarray$map<TInput, TOutput>(arr: TInput[], fn: (item: TInput, index: number) => TOutput): TOutput[] {
  const output: TOutput[] = [];
  for (let i = 0, e = arr.length; i < e; ++i) {
    output.push(fn(arr[i], i));
  }
  return output;
}
function sh_fastarray$includes<T>(arr: T[], searchElement: T): boolean {
  for (let i = 0, e = arr.length; i < e; ++i) {
    if (arr[i] === searchElement) {
      return true;
    }
  }
  return false;
}
/* file: packages/react/index.js */
function react_index$INTERNAL$padString(str: string, len: number): string {
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
type react_index$INTERNAL$React$ElementType = string | react_index$INTERNAL$Component /* TODO: React$AbstractComponent<empty, mixed> */ | number /* TODO: symbol */;
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
class react_index$INTERNAL$React$Element<ElementType> {
  type: ElementType;
  props: react_index$Props;
  key: react_index$INTERNAL$React$Key | null;
  ref: any;
  constructor(type: ElementType, props: react_index$Props, key: react_index$INTERNAL$React$Key | null, ref: any) {
    this.type = type;
    this.props = props;
    this.key = key;
    this.ref = ref;
  }
}
type react_index$React$MixedElement = react_index$INTERNAL$React$Element<react_index$INTERNAL$React$ElementType>;
type react_index$INTERNAL$React$NodeWithoutArray = react_index$React$MixedElement | string | null | void;
type react_index$React$Node = react_index$INTERNAL$React$NodeWithoutArray[] | react_index$INTERNAL$React$NodeWithoutArray;
/**
 * The type of the key that React uses to determine where items in a new list
 * have moved.
 */
type react_index$INTERNAL$React$Key = string | number;
const react_index$INTERNAL$REACT_FRAGMENT_TYPE: number = 1 /* Symbol.for('react.fragment') */;
/* eslint-disable lint/strictly-null, lint/react-state-props-mutation, lint/flow-react-element */

/**
 * The current root
 */
let react_index$INTERNAL$workInProgressRoot: react_index$INTERNAL$Root | null = null;
/**
 * The currently rendering fiber. Only set when a component is being rendered.
 */
let react_index$INTERNAL$workInProgressFiber: react_index$INTERNAL$Fiber | null = null;
/**
 * The previous state hook, or null if no state hook has been evaluated yet.
 */
let react_index$INTERNAL$workInProgressState: react_index$INTERNAL$State<mixed> | null = null;
/**
 * Queue of updates triggered *during* render.
 */
const react_index$INTERNAL$renderPhaseUpdateQueue: react_index$INTERNAL$Update<mixed>[] = [];
/**
 * Public API to create a new "root", this is where React attaches rendering to a host element.
 * In our case we don't actually have a real host, and currently only "render" to strings.
 */
function react_index$createRoot(): react_index$INTERNAL$Root {
  return new react_index$INTERNAL$Root();
}
/**
 * Hook to create (on initial render) or access (on update) a state, using the index of the useState
 * call within the component as the identity. Thus conditionally calling this API can cause state to
 * be lost.
 */
function react_index$useState<T>(
/**
 * Initial value of the state
 */
initial: T): [T, (value: T | ((prev: T) => T)) => void] {
  const root: react_index$INTERNAL$Root = sh_CHECKED_CAST$default<react_index$INTERNAL$Root>(react_index$INTERNAL$workInProgressRoot);
  const fiber: react_index$INTERNAL$Fiber = sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(react_index$INTERNAL$workInProgressFiber);
  react_invariant$default(fiber !== null && root !== null, 'useState() called outside of render');
  let state: react_index$INTERNAL$State<T>;
  const _workInProgressState: react_index$INTERNAL$State<mixed> | null = react_index$INTERNAL$workInProgressState;
  if (_workInProgressState === null) {
    // Get or initialize the first state on the fiber
    let nextState = fiber.state;
    if (nextState === null) {
      nextState = new react_index$INTERNAL$State<mixed>(initial);
      fiber.state = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = sh_CHECKED_CAST$default<react_index$INTERNAL$State<T>>(nextState);
  } else {
    let nextState = sh_CHECKED_CAST$default<react_index$INTERNAL$State<mixed>>(_workInProgressState).next;
    if (nextState === null) {
      nextState = new react_index$INTERNAL$State<mixed>(initial);
      sh_CHECKED_CAST$default<react_index$INTERNAL$State<mixed>>(_workInProgressState).next = nextState;
    }
    // NOTE: in case of a re-render we assume that the hook types match but
    // can't statically prove this
    state = sh_CHECKED_CAST$default<react_index$INTERNAL$State<T>>(nextState);
  }
  // NOTE: this should just work because of subtying, State<T> should be subtype of State<mixed>
  react_index$INTERNAL$workInProgressState = sh_CHECKED_CAST$default<react_index$INTERNAL$State<mixed>>(state);
  return [
  // Untyped check that the existing state value has the correct type,
  // This is safe if components follow the rules of hooks
  sh_CHECKED_CAST$default<T>(state.value), (updater: T | ((prev: T) => T)): void => {
    const update = new react_index$INTERNAL$Update<mixed>(fiber, sh_CHECKED_CAST$default<react_index$INTERNAL$State<mixed>>(state), sh_CHECKED_CAST$default<T | ((prev: T) => T)>(updater));
    if (react_index$INTERNAL$workInProgressFiber !== null) {
      // called during render
      react_index$INTERNAL$renderPhaseUpdateQueue.push(update);
    } else {
      root.notify(update);
    }
  }];
}
const react_index$INTERNAL$callbacks = new Map();
function react_index$callOnClickOrChange(id: string, event: any): void {
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
class react_index$INTERNAL$Update<T> {
  fiber: react_index$INTERNAL$Fiber;
  state: react_index$INTERNAL$State<T>;
  updater: T | ((prev: T) => T);
  constructor(fiber: react_index$INTERNAL$Fiber, state: react_index$INTERNAL$State<T>, updater: T | ((prev: T) => T)) {
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
      const fn = sh_CHECKED_CAST$default<(prev: T) => T>(updater);
      value = fn(state.value);
    } else {
      // NOTE: The type of Updater<T> is meant to expresss `T (not function) | T (function of T => T)`
      // thus the fact that updater is *not* a function here means it is a T
      value = sh_CHECKED_CAST$default<T>(updater);
    }
    const changed = !Object.is(state.value, value);
    state.value = value;
    return changed;
  }
}
class react_index$INTERNAL$Root {
  root: react_index$INTERNAL$Fiber | null;
  element: react_index$React$MixedElement | null;
  updateQueue: react_index$INTERNAL$Update<mixed>[];
  constructor() {
    this.root = null;
    this.element = null;
    this.updateQueue = ([]: react_index$INTERNAL$Update<mixed>[]);
  }
  notify(update: react_index$INTERNAL$Update<mixed>): void {
    this.updateQueue.push(update);
    if (this.updateQueue.length === 1) {
      sh_microtask$queueMicrotask((): void => {
        const element = this.element;
        react_invariant$default(element !== null, 'Expected an element to be set after rendering');
        this.doWork(sh_CHECKED_CAST$default<react_index$React$MixedElement>(element));
      });
    }
  }
  render(element: react_index$React$MixedElement): string {
    react_invariant$default(react_index$INTERNAL$workInProgressFiber === null && react_index$INTERNAL$workInProgressState === null, 'Cannot render, an existing render is in progress');
    const hasChanges = element !== this.element;
    this.element = element;
    if (hasChanges) {
      this.doWork(element);
    }
    react_invariant$default(this.root !== null, 'Expected root to be rendered');
    const root: react_index$INTERNAL$Fiber = sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(this.root);
    const output: string[] = [];
    this.printFiber(root, output, 0);
    return sh_fastarray$join(output, '\n');
  }
  doWork(element: react_index$React$MixedElement): void {
    let mustRender = this.root === null;
    for (const update of this.updateQueue) {
      mustRender = update.run() || mustRender;
    }
    this.updateQueue = ([]: react_index$INTERNAL$Update<mixed>[]);
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
      let fiber2: react_index$INTERNAL$Fiber = sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(fiber);
      this.renderFiber(fiber2);
      // advance to the next fiber
      if (fiber2.child !== null) {
        fiber = fiber2.child;
      } else if (fiber2.sibling !== null) {
        fiber = fiber2.sibling;
      } else {
        fiber = fiber2.parent;
        while (fiber !== null && sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(fiber).sibling === null) {
          fiber = sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(fiber).parent;
        }
        if (fiber !== null) {
          fiber = sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(fiber).sibling;
        }
      }
    }
    react_index$INTERNAL$workInProgressRoot = null;
  }
  printFiber(fiber: react_index$INTERNAL$Fiber, out: string[], level: number): void {
    switch (fiber.type.kind) {
      case 'host':
        {
          const tag = sh_CHECKED_CAST$default<react_index$INTERNAL$FiberTypeHost>(fiber.type).tag;
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
          const text = sh_CHECKED_CAST$default<react_index$INTERNAL$FiberTypeText>(fiber.type).text;
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
  printChildren(fiber: react_index$INTERNAL$Fiber, out: string[], level: number): void {
    let current: react_index$INTERNAL$Fiber | null = fiber.child;
    while (current !== null) {
      this.printFiber(sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(current), out, level);
      current = sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(current).sibling;
    }
  }
  renderFiber(fiber: react_index$INTERNAL$Fiber): void {
    try {
      react_index$INTERNAL$workInProgressFiber = fiber;
      react_index$INTERNAL$workInProgressState = null;
      switch (fiber.type.kind) {
        case 'component':
          {
            react_invariant$default(react_index$INTERNAL$renderPhaseUpdateQueue.length === 0, 'Expected no queued render updates');
            const render: (props: react_index$Props) => react_index$React$MixedElement = sh_CHECKED_CAST$default<react_index$INTERNAL$FiberTypeComponent>(fiber.type).component;
            let element = render(fiber.props);
            let iterationCount: number = 0;
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
  mountFiber(elementOrString: react_index$React$Node, parent: react_index$INTERNAL$Fiber | null): react_index$INTERNAL$Fiber {
    let fiber: react_index$INTERNAL$Fiber;
    // TODO: Support Array of Node's being returned from a component.
    if (typeof elementOrString === 'object') {
      const element = sh_CHECKED_CAST$default<react_index$React$MixedElement>(elementOrString);
      if (typeof element.type === 'function') {
        const component: react_index$INTERNAL$Component = sh_CHECKED_CAST$default<react_index$INTERNAL$Component>(element.type);
        const type: react_index$INTERNAL$FiberType = new react_index$INTERNAL$FiberTypeComponent(component);
        fiber = new react_index$INTERNAL$Fiber(type, (element.props: any), element.key);
      } else if (typeof element.type === 'string') {
        react_invariant$default(typeof element.type === 'string', 'Expected a host component name such as "div" or "span", got ' + typeof element.type);
        const type: react_index$INTERNAL$FiberType = new react_index$INTERNAL$FiberTypeHost(sh_CHECKED_CAST$default<string>(element.type));
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
              const type: react_index$INTERNAL$FiberType = new react_index$INTERNAL$FiberTypeFragment();
              fiber = new react_index$INTERNAL$Fiber(type, (element.props: any), element.key);
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
      const type = new react_index$INTERNAL$FiberTypeText(sh_CHECKED_CAST$default<string>(elementOrString));
      fiber = new react_index$INTERNAL$Fiber(type, {}, null);
    } else {
      throw new Error(`Unexpected element type of ${typeof elementOrString}`);
    }
    fiber.parent = parent;
    return fiber;
  }
  mountChildren(children: react_index$React$Node, parentFiber: react_index$INTERNAL$Fiber): void {
    if (Array.isArray(children)) {
      let prev: react_index$INTERNAL$Fiber | null = null;
      for (const childElement of sh_CHECKED_CAST$default<any[]>(children)) {
        if (childElement == null) {
          continue;
        }
        const child = this.mountFiber(sh_CHECKED_CAST$default<react_index$React$Node>(childElement), parentFiber);
        if (prev !== null) {
          sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(prev).sibling = child;
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
  reconcileFiber(parent: react_index$INTERNAL$Fiber, prevChild: react_index$INTERNAL$Fiber | null, element: react_index$React$MixedElement): react_index$INTERNAL$Fiber {
    if (prevChild !== null && sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(prevChild).type === element.type) {
      let prevChild: react_index$INTERNAL$Fiber = sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(prevChild);
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
            this.reconcileChildren(prevChild, (children: any));
            break;
          }
        case 'fragment':
          {
            react_invariant$default(element.props !== null && typeof element.props === 'object', 'Expected component props');
            const children = element.props.children;
            this.reconcileChildren(prevChild, (children: any));
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
  reconcileChildren(parent: react_index$INTERNAL$Fiber, children: react_index$React$Node): void {
    const prevChild: react_index$INTERNAL$Fiber | null = parent.child;
    if (Array.isArray(children)) {
      let childrenArray = sh_CHECKED_CAST$default<react_index$React$MixedElement[]>(children);
      // Fast-path for empty and single-element arrays
      if (childrenArray.length === 0) {
        parent.child = null;
      } else if (childrenArray.length === 1) {
        parent.child = this.reconcileFiber(parent, prevChild, childrenArray[0]);
        sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(parent.child).sibling = null;
      } else {
        this.reconcileMultipleChildren(parent, childrenArray);
      }
    } else if (typeof children === 'string') {
      if (prevChild === null || sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(prevChild).type.kind !== 'text') {
        const child = new react_index$INTERNAL$Fiber({
          kind: 'text',
          text: children
        }, {}, null);
        parent.child = child;
      } else {
        sh_CHECKED_CAST$default<react_index$INTERNAL$FiberTypeText>(sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(prevChild).type).text = sh_CHECKED_CAST$default<string>(children);
      }
    } else if (children != null) {
      parent.child = this.reconcileFiber(parent, prevChild, sh_CHECKED_CAST$default<react_index$React$MixedElement>(children));
      sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(parent.child).sibling = null;
    } else {
      parent.child = null;
      if (prevChild !== null) {
        sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(prevChild).parent = null;
      }
    }
  }
  reconcileMultipleChildren(parent: react_index$INTERNAL$Fiber, children: react_index$React$MixedElement[]): void {
    react_invariant$default(children.length > 1, 'Expected children to have multiple elements');
    // map existing children by key to make subsequent lookup O(log n)
    const keyedChildren: any = new Map();
    let current: react_index$INTERNAL$Fiber | null = parent.child;
    while (current !== null) {
      if (sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(current).key !== null) {
        keyedChildren.set(sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(current).key, current);
      }
      current = sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(current).sibling;
    }
    let prev: react_index$INTERNAL$Fiber | null = null; // previous fiber at this key/index
    let prevByIndex: react_index$INTERNAL$Fiber | null = parent.child; // keep track of prev fiber at this index
    for (const childElement of children) {
      const prevFiber = (childElement.key != null ? keyedChildren.get(childElement.key) : null) ?? prevByIndex;
      let child: react_index$INTERNAL$Fiber;
      if (prevFiber != null) {
        child = this.reconcileFiber(parent, prevFiber, childElement);
      } else {
        child = this.mountFiber(childElement, parent);
      }
      if (prev !== null) {
        sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(prev).sibling = child;
      } else {
        // set parent to point to first child
        parent.child = child;
      }
      prev = child;
      prevByIndex = prevByIndex !== null ? sh_CHECKED_CAST$default<react_index$INTERNAL$Fiber>(prevByIndex).sibling : null;
    }
  }
}
/**
 * Describes the `type` field of Fiber, which can hold different data depending on the fiber's kind:
 * - Component stores a function of props => element.
 * - Host stores the name of the host component, ie "div"
 * - Text stores the text itself.
 */
type react_index$INTERNAL$Component = (props: react_index$Props) => react_index$React$MixedElement;
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
  kind: string;
  constructor(kind: string) {
    this.kind = kind;
  }
}
class react_index$INTERNAL$FiberTypeComponent extends react_index$INTERNAL$FiberType {
  component: react_index$INTERNAL$Component;
  constructor(component: react_index$INTERNAL$Component) {
    super('component');
    this.component = component;
  }
}
class react_index$INTERNAL$FiberTypeHost extends react_index$INTERNAL$FiberType {
  tag: string;
  constructor(tag: string) {
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
type react_index$Props = any;
/**
 * Data storage for the useState() hook
 */
class react_index$INTERNAL$State<T> {
  value: T;
  next: react_index$INTERNAL$State<T> | null;
  prev: react_index$INTERNAL$State<T> | null;
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
class react_index$INTERNAL$Fiber {
  type: react_index$INTERNAL$FiberType;
  props: react_index$Props;
  parent: react_index$INTERNAL$Fiber | null;
  child: react_index$INTERNAL$Fiber | null;
  sibling: react_index$INTERNAL$Fiber | null;
  state: react_index$INTERNAL$State<mixed> | null;
  key: react_index$INTERNAL$React$Key | null;
  constructor(type: react_index$INTERNAL$FiberType, props: react_index$Props, key: react_index$INTERNAL$React$Key | null) {
    this.type = type;
    this.props = props;
    this.key = key;
    this.parent = null;
    this.child = null;
    this.sibling = null;
    this.state = null;
  }
}
function react_index$jsx(type: react_index$INTERNAL$React$ElementType, props: react_index$Props, key: react_index$INTERNAL$React$Key | null): react_index$React$MixedElement {
  'inline';

  return {
    type: type,
    props: props,
    key: key,
    ref: null
  };
}
function react_index$Fragment(props: react_index$Props): react_index$React$MixedElement {
  'inline';

  return {
    type: react_index$INTERNAL$REACT_FRAGMENT_TYPE,
    props: props,
    key: null,
    ref: null
  };
}
function react_index$forwardRef(comp: (props: react_index$Props, ref: mixed) => react_index$React$MixedElement): react_index$INTERNAL$Component {
  return (props: react_index$Props): react_index$React$MixedElement => comp(props, null);
}
/* file: packages/class-variance-authority/index.js */
function class_variance_authority_index$cva(base: string[] | string, variants: mixed): (opts: mixed) => string {
  const baseString: string = typeof base === 'string' ? sh_CHECKED_CAST$default<string>(base) : sh_fastarray$join(sh_CHECKED_CAST$default<string[]>(base), ' ');
  return (opts: mixed): string => baseString;
}
/* file: lib/utils.js */
function utils$cn(...rest: any): string {
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
}, ref): react_index$React$MixedElement => {
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
/* file: app/music/page.js */
function page$default(props: react_index$Props): react_index$React$MixedElement {
  const [toggle, setToggle] = react_index$useState<boolean>(true);
  return react_index$jsx(react_index$Fragment, {
    children: [react_index$jsx(button$Button, {
      id: "click-me",
      onClick: () => setToggle(!toggle),
      children: ['Click me: ', String(toggle)]
    }, null), react_index$jsx('span', {
      children: 'Other'
    }, null)]
  }, null);
}
/* file: app/music/index.js */
function index$INTERNAL$printIf1(i: number, str: string): void {
  if (i === 1) {
    print('===============================');
    print(str);
    print('===============================');
  }
}
function index$INTERNAL$run(N: number): void {
  for (let i: number = 1; i <= N; ++i) {
    const root = react_index$createRoot();
    const rootElement = react_index$jsx(page$default, {}, null);
    index$INTERNAL$printIf1(i, root.render(rootElement));
    react_index$callOnClickOrChange('click-me', null);
    sh_microtask$drainMicrotaskQueue();
    index$INTERNAL$printIf1(i, root.render(rootElement));
  }
}
index$INTERNAL$run(1);
//# sourceMappingURL=music.js.map
