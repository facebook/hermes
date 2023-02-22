/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

(function () {
  class Widget {
    reduce(ctx) {
      throw new Error("Implement this in a subclass");
    }
  }

  class ComposedWidget extends Widget {
    render() {
      throw new Error("Implement this in a subclass");
    }
    reduce(ctx) {
      let child = this.render();
      return child.reduce(ctx);
    }
  }

  class Button extends Widget {
    constructor(num) {
      super();
      this.num = num;
    }
    reduce(ctx) {
      const component = new NumberComponent(this.num);
      return RenderNode_create(ctx, [component], null);
    }
  }

  class Floater extends Widget {
    constructor(num) {
      super();
      this.num = num;
    }
    reduce(ctx) {
      const component = new NumberComponent(this.num);
      return RenderNode_create(ctx, [component], null);
    }
  }

  class Gltf extends Widget {
    constructor(path) {
      super();
      this.path = path;
    }
    reduce(ctx) {
      const component = new StringComponent(this.path);
      return RenderNode_create(ctx, [component], null);
    }
  }

  class Container extends Widget {
    constructor(children) {
      super();
      this.children = children;
    }
    reduce(ctx) {
      const component = new NumberComponent(13);
      const children = this.children.map((child) =>
        RenderNode_createForChild(ctx, child)
      );
      return RenderNode_create(ctx, [component], children);
    }
  }

  class ComponentPair {
    constructor(key, value) {
      this.key = key;
      this.value = value;
    }
  }

  class SceneDiff {
    constructor(
      createdEntities,
      deletedEntities,
      createdComponents,
      deletedComponents
    ) {
      this.createdEntities = createdEntities;
      this.deletedEntities = deletedEntities;
      this.createdComponents = createdComponents;
      this.deletedComponents = deletedComponents;
    }
  }

  function reconcileRenderNode(newNode, oldNode) {
    return new RenderNode(
      newNode.key,
      oldNode.id,
      newNode.components,
      reconcileChildren(newNode.children, oldNode.children)
    );
  }
  function reconcileChildren(newChildren, oldChildren) {
    const outChildren = [];
    const oldChildrenByKey = new Map();
    oldChildren.forEach((child) => oldChildrenByKey.set(child.key, child));
    newChildren.forEach((child) => {
      const newKey = child.key;
      const oldChild = oldChildrenByKey.get(newKey);
      if (oldChild !== undefined) {
        outChildren.push(reconcileRenderNode(child, oldChild));
      } else {
        outChildren.push(child);
      }
    });
    return outChildren;
  }
  function mapEntitiesToComponents(entities) {
    const map = new Map();
    entities.forEach((entity) => {
      const key = entity.key;
      const value = entity.value;
      if (map.get(key) == undefined) {
        map.set(key, []);
      }
      const components = map.get(key);
      if (components !== undefined) {
        components.push(...value);
      } else {
        throw new Error("components shouldnt be undefined");
      }
    });
    return map;
  }
  function diffTrees(newEntities, oldEntities) {
    const createdComponents = [];
    const deletedComponents = [];
    const oldEntityIds = oldEntities.map((entity) => entity.key);
    const newEntityIds = newEntities.map((entity) => entity.key);
    const createdEntities = newEntityIds.filter(
      (entityId) => !oldEntityIds.includes(entityId)
    );
    const deletedEntities = oldEntityIds.filter(
      (entityId) => !newEntityIds.includes(entityId)
    );
    const oldComponents = mapEntitiesToComponents(oldEntities);
    const newComponents = mapEntitiesToComponents(newEntities);
    createdEntities.forEach((entityId) => {
      const components = (newComponents.get(entityId) || []).map(
        (it) => new ComponentPair(entityId, it)
      );
      createdComponents.push(...components);
    });
    newComponents.forEach((value, key) => {
      if (oldComponents.get(key) == undefined) {
        return;
      }
      const oldComponentsForKey = oldComponents.get(key) || [];
      const newComponentsForKey = value;
      const deleted = oldComponentsForKey.filter(
        (it) => !newComponentsForKey.includes(it)
      );
      const created = newComponentsForKey.filter(
        (it) => !oldComponentsForKey.includes(it)
      );
      deleted.forEach((it) =>
        deletedComponents.push(new ComponentPair(key, it))
      );
      created.forEach((it) =>
        createdComponents.push(new ComponentPair(key, it))
      );
    });
    return new SceneDiff(
      createdEntities,
      deletedEntities,
      createdComponents,
      deletedComponents
    );
  }
  function runTest(includeTreeSerialization) {
    const oldCtx = new Context("root");
    const oldWidgetTree = new TestApp(false).render();
    const oldRenderTree = oldWidgetTree.reduce(oldCtx);
    const oldEntityTree = oldRenderTree.reduce();
    const newCtx = new Context("root");
    const newWidgetTree = new TestApp(true).render();
    const newRenderTree = newWidgetTree.reduce(newCtx);
    const reconciledRenderTree = reconcileRenderNode(
      newRenderTree,
      oldRenderTree
    );
    const reconciledEntityTree = reconciledRenderTree.reduce();
    const diff = diffTrees(reconciledEntityTree, oldEntityTree);
    return includeTreeSerialization ? diff : null;
  }
  class NumberComponent {
    constructor(x) {
      this.x = x;
    }
  }

  class StringComponent {
    constructor(x) {
      this.x = x;
    }
  }

  function Context_createForChild(parentCtx, child) {
    const widgetKey = child.key;
    const childKey =
      widgetKey !== null && widgetKey !== undefined
        ? widgetKey
        : `${child.constructor.name}_${parentCtx.childCounter++}`;
    const newKey = `${parentCtx.key}_${childKey}`;
    return new Context(newKey);
  }
  class Context {
    constructor(key) {
      this.key = key;
      this.childCounter = 0;
    }
  }

  class VirtualEntity {
    constructor(key, value) {
      this.key = key;
      this.value = value;
    }
  }

  let RenderNode_idCounter = 0;
  function RenderNode_create(ctx, components, children) {
    return new RenderNode(
      ctx.key,
      RenderNode_idCounter++,
      components,
      children
    );
  }
  function RenderNode_createForChild(ctx, child) {
    const childCtx = Context_createForChild(ctx, child);
    return child.reduce(childCtx);
  }
  class RenderNode {
    constructor(key, id, components, children) {
      this.key = key;
      this.id = id;
      this.components = components;
      this.children = children || [];
    }
    reduce() {
      const childrenEntities = (this.children || []).flatMap((child) =>
        child.reduce()
      );
      return [new VirtualEntity(this.id, this.components)].concat(
        childrenEntities
      );
    }
  }

  class RenderData {
    constructor(modelPath, buttonSize) {
      this.modelPath = modelPath;
      this.buttonSize = buttonSize;
    }
  }

  class ButtonAndModel extends ComposedWidget {
    constructor(data) {
      super();
      this.data = data;
    }
    render() {
      const children = [
        new Button(this.data.buttonSize),
        new Gltf(this.data.modelPath),
        new Floater(Math.sqrt(this.data.buttonSize)),
      ];
      return new Container(children);
    }
  }

  class TestApp extends ComposedWidget {
    constructor(renderLarge) {
      super();
      this.renderLarge = renderLarge;
    }
    getWidgets(sizes, models) {
      if (sizes.length != models.length) {
        throw new Error("sizes and models must have same length");
      }
      return models.map((modelPath, index) => {
        const buttonSize = sizes[index];
        const widget = new ButtonAndModel(
          new RenderData(modelPath, buttonSize)
        );
        widget.key = `${modelPath}_${buttonSize}`;
        return widget;
      });
    }
    getChildren() {
      if (this.renderLarge) {
        return this.getWidgets(SIZES_LARGE, MODELS_LARGE);
      }
      return this.getWidgets(SIZES_SMALL, MODELS_SMALL);
    }
    render() {
      const children = this.getChildren();
      return new Container(children);
    }
  }

  const SIZES_SMALL = [64, 8, 8, 18, 7, 24, 84, 4, 29, 58];
  const SIZES_LARGE = SIZES_SMALL.slice(0, 5).concat([
    99, 9, 61, 27, 58, 30, 27, 95, 49, 37, 28, 87, 60, 95, 1, 58, 14, 90, 9, 57,
  ]);
  const MODELS_SMALL = [
    "sazGTSGrfY",
    "uEQjieLDUq",
    "jQKzwhnzYa",
    "buIwVjnNDI",
    "goBJPxAkFf",
    "uKihCBaMwm",
    "VAyeIqqnSU",
    "bMNULcHsKb",
    "NBMEpcDimq",
    "wMCIoQQbNg",
  ];
  const MODELS_LARGE = MODELS_SMALL.slice(0, 5).concat([
    "oCtHzjLczM",
    "GJVcmhfddz",
    "nbpEAplbzQ",
    "yNXDBUcDys",
    "IZZQpqwiXa",
    "AAroNFkOBf",
    "flsXwiIaQG",
    "qazjSVkFcR",
    "PefkCqwfKJ",
    "yJDvizIEDY",
    "XauGblPeuo",
    "ZnvLBVjEom",
    "UeosvyfoBE",
    "BFeZAIAHQq",
    "iOYUWXWXhr",
    "WpOfaJwOlm",
    "sVHOxutIGB",
    "qOikwyZSWx",
    "KJGEPQxUKU",
    "cqrRvLCCYB",
  ]);
  globalThis.runFullTest = runTest;
  const printDiff = false;
  if (printDiff) {
    print(JSON.stringify(runFullTest(true)));
  } else {
    print("Running...");
    for (var i = 0; i < 50; ++i) runFullTest(false);
    let t1 = Date.now();
    for (var i = 0; i < 5000; ++i) runFullTest(false);
    print(Date.now() - t1, "ms", i, "iterations");
  }
})();
