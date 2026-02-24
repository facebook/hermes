/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

(function () {
  function invariant(condition, format) {
    if (!condition) {
      var error = new Error(format);
    }
  }
  function queueMicrotask(callback) {
    HermesInternal.enqueueJob(callback);
  }
  let workInProgressRoot = null;
  let workInProgressFiber = null;
  let workInProgressState = null;
  const renderPhaseUpdateQueue = [];
  function createRoot() {
    return new Root();
  }
  function useState(initial) {
    const root = workInProgressRoot;
    const fiber = workInProgressFiber;
    invariant(
      fiber !== null && root !== null,
      "useState() called outside of render"
    );
    let state;
    const _workInProgressState = workInProgressState;
    if (_workInProgressState === null) {
      let nextState = fiber.state;
      if (nextState === null) {
        nextState = new State(initial);
        fiber.state = nextState;
      }
      state = UNSAFE_CAST(nextState);
    } else {
      let nextState = _workInProgressState.next;
      if (nextState === null) {
        nextState = new State(initial);
        _workInProgressState.next = nextState;
      }
      state = UNSAFE_CAST(nextState);
    }
    workInProgressState = UNSAFE_CAST(state);
    return [
      UNSAFE_CAST(state.value),
      (updater) => {
        const update = new Update(
          fiber,
          UNSAFE_CAST(state),
          UNSAFE_CAST(updater)
        );
        if (workInProgressFiber !== null) {
          renderPhaseUpdateQueue.push(update);
        } else {
          root.notify(update);
        }
      },
    ];
  }
  class Update {
    constructor(fiber, state, updater) {
      this.fiber = fiber;
      this.state = state;
      this.updater = updater;
    }
    run() {
      const state = this.state;
      let value = state.value;
      const updater = this.updater;
      if (typeof updater === "function") {
        const fn = UNSAFE_CAST(updater);
        value = fn(state.value);
      } else {
        value = UNSAFE_CAST(updater);
      }
      const changed = !Object.is(state.value, value);
      state.value = value;
      return changed;
    }
  }

  class Root {
    root = null;
    element = null;
    updateQueue = [];
    notify(update) {
      this.updateQueue.push(update);
      if (this.updateQueue.length === 1) {
        queueMicrotask((..._args) => {
          const element = this.element;
          invariant(
            element !== null,
            "Expected an element to be set after rendering"
          );
          this.doWork(element);
        });
      }
    }
    render(element) {
      invariant(
        workInProgressFiber === null && workInProgressState === null,
        "Cannot render, an existing render is in progress"
      );
      const hasChanges = element !== this.element;
      this.element = element;
      if (hasChanges) {
        this.doWork(element);
      }
      const root = this.root;
      invariant(root !== null, "Expected root to be rendered");
      const output = [];
      this.printFiber(root, output);
      return output.join("");
    }
    doWork(element) {
      let mustRender = this.root === null;
      for (const update of this.updateQueue) {
        mustRender = update.run() || mustRender;
      }
      this.updateQueue.length = 0;
      if (!mustRender) {
        return;
      }
      workInProgressRoot = this;
      let fiber = this.root;
      if (fiber === null) {
        fiber = this.mountFiber(element, null);
        this.root = fiber;
      }
      while (fiber !== null) {
        this.renderFiber(fiber);
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
    printFiber(fiber, out) {
      switch (fiber.type.kind) {
        case "host": {
          const tag = fiber.type.tag;
          out.push("<" + tag);
          for (const [prop, value] of Object.entries(fiber.props)) {
            out.push(` ${prop}=${JSON.stringify(value) ?? "undefined"}`);
          }
          out.push(">");
          this.printChildren(fiber, out);
          out.push("</" + tag + ">");
          break;
        }
        case "text": {
          const text = fiber.type.text;
          out.push(text);
          break;
        }
        case "component": {
          this.printChildren(fiber, out);
          break;
        }
      }
    }
    printChildren(fiber, out) {
      let current = fiber.child;
      while (current !== null) {
        this.printFiber(current, out);
        current = current.sibling;
      }
    }
    renderFiber(fiber) {
      try {
        workInProgressFiber = fiber;
        workInProgressState = null;
        switch (fiber.type.kind) {
          case "component": {
            invariant(
              renderPhaseUpdateQueue.length === 0,
              "Expected no queued render updates"
            );
            const render = fiber.type.component;
            let element = render(fiber.props);
            let iterationCount = 0;
            while (renderPhaseUpdateQueue.length !== 0) {
              iterationCount++;
              invariant(
                iterationCount < 1000,
                "Possible infinite loop with setState during render"
              );
              let hasChanges = false;
              for (const update of renderPhaseUpdateQueue) {
                invariant(
                  update.fiber === fiber,
                  "setState() during render is currently only supported when updating the component " +
                    "being rendered. Setting state from another component is not supported."
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
          case "host":
          case "text": {
            break;
          }
        }
      } finally {
        workInProgressFiber = null;
        workInProgressState = null;
      }
    }
    mountFiber(element, parent) {
      let fiber;
      if (typeof element.type === "function") {
        const component = element.type;
        const type = { kind: "component", component };
        fiber = new Fiber(type, element.props, element.key);
      } else if (element !== null && typeof element === "object") {
        invariant(
          typeof element.type === "string",
          'Expected a host component name such as "div" or "span", got ' +
            element.type
        );
        const type = { kind: "host", tag: element.type };
        invariant(
          element.props !== null && typeof element.props === "object",
          "Expected component props"
        );
        const { children, ...props } = element.props;
        fiber = new Fiber(type, props, element.key);
        if (Array.isArray(children)) {
          let prev = null;
          for (const childElement of children) {
            const child = this.mountFiber(childElement, fiber);
            if (prev !== null) {
              prev.sibling = child;
            } else {
              fiber.child = child;
            }
            prev = child;
          }
        } else if (typeof children === "string") {
          const child = new Fiber({ kind: "text", text: children }, {}, null);
          child.parent = fiber;
          fiber.child = child;
        } else if (children != null) {
          const child = this.mountFiber(children, fiber);
          fiber.child = child;
        }
      } else {
        invariant(typeof element === "string", "Expected a string");
        const type = { kind: "text", text: element };
        fiber = new Fiber(type, {}, null);
      }
      fiber.parent = parent;
      return fiber;
    }
    reconcileFiber(parent, prevChild, element) {
      if (prevChild !== null && prevChild.type === element.type) {
        if (prevChild.type.kind === "host") {
          invariant(
            element.props !== null && typeof element.props === "object",
            "Expected component props"
          );
          const { children, ...props } = element.props;
          prevChild.props = props;
          this.reconcileChildren(prevChild, children);
        } else if (prevChild.type.kind === "component") {
          invariant(
            element.props !== null && typeof element.props === "object",
            "Expected component props"
          );
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
        if (children.length === 0) {
          parent.child = null;
        } else if (children.length === 1) {
          parent.child = this.reconcileFiber(parent, prevChild, children[0]);
          parent.child.sibling = null;
        } else {
          this.reconcileMultipleChildren(parent, children);
        }
      } else if (typeof children === "string") {
        if (prevChild === null || prevChild.type.kind !== "text") {
          const child = new Fiber({ kind: "text", text: children }, {}, null);
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
    reconcileMultipleChildren(parent, children) {
      invariant(
        children.length > 1,
        "Expected children to have multiple elements"
      );
      const keyedChildren = new Map();
      let current = parent.child;
      while (current !== null) {
        if (current.key !== null) {
          keyedChildren.set(current.key, current);
        }
        current = current.sibling;
      }
      let prev = null;
      let prevByIndex = parent.child;
      for (const childElement of children) {
        const prevFiber =
          (childElement.key != null
            ? keyedChildren.get(childElement.key)
            : null) ?? prevByIndex;
        let child;
        if (prevFiber != null) {
          child = this.reconcileFiber(parent, prevFiber, childElement);
        } else {
          child = this.mountFiber(childElement, parent);
        }
        if (prev !== null) {
          prev.sibling = child;
        } else {
          parent.child = child;
        }
        prev = child;
        prevByIndex = prevByIndex !== null ? prevByIndex.sibling : null;
      }
    }
  }

  class State {
    next = null;
    prev = null;
    constructor(value) {
      this.value = value;
    }
  }

  class Fiber {
    parent = null;
    child = null;
    sibling = null;
    state = null;
    constructor(type, props, key) {
      this.type = type;
      this.props = props;
      this.key = key;
    }
  }

  function UNSAFE_CAST(value) {
    return value;
  }
  function createElement(type, props, key) {
    return { type: type, props: props, key: key, ref: null };
  }
  function run() {
    var N = 1000000;
    for (var i = 0; i < N; ++i) {
      var root = createRoot();
      var rendered = root.render(
        createElement("div", {
          children: [createElement("h1", { children: ["Hello"] }), " world!"],
        })
      );
    }
    if (rendered !== `<div><h1>Hello</h1> world!</div>`)
      throw Error("failed: " + rendered);
  }
  run();
})();
