/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

(function() {

// ==> widget.js <==

class Widget {
  key: ?string;

  reduce(ctx: Context): RenderNode {
    throw new Error('Implement this in a subclass');
  }
}

class ComposedWidget extends Widget {
  render(): Widget {
    throw new Error('Implement this in a subclass');
  }

  reduce(ctx: Context): RenderNode {
    let child = this.render();
    return child.reduce(ctx);
  }
}

// ==> widgets.js <==

class Button extends Widget {
  num: number;

  constructor(num: number) {
    super();
    this.num = num;
  }

  reduce(ctx: Context): RenderNode {
    const component: NumberComponent = {x: this.num};
    return RenderNode.create(ctx, [component], null);
  }
}

class Floater extends Widget {
  num: number;

  constructor(num: number) {
    super();
    this.num = num;
  }

  reduce(ctx: Context): RenderNode {
    const component: NumberComponent = {x: this.num};
    return RenderNode.create(ctx, [component], null);
  }
}

class Gltf extends Widget {
  path: string;

  constructor(path: string) {
    super();
    this.path = path;
  }

  reduce(ctx: Context): RenderNode {
    const component: StringComponent = {x: this.path};
    return RenderNode.create(ctx, [component], null);
  }
}

class Container extends Widget {
  children: Widget[];

  constructor(children: Widget[]) {
    super();
    this.children = children;
  }

  reduce(ctx: Context): RenderNode {
    const component: NumberComponent = {x: 13};
    const children = this.children.map(child =>
      RenderNode.createForChild(ctx, child),
    );
    return RenderNode.create(ctx, [component], children);
  }
}

// ==> app_runner.js <==

type ComponentPair = [number, Component];
type SceneDiff = {
  createdEntities: number[],
  deletedEntities: number[],
  createdComponents: ComponentPair[],
  deletedComponents: ComponentPair[],
};

function reconcileRenderNode(
  newNode: RenderNode,
  oldNode: RenderNode,
): RenderNode {
  return new RenderNode(
    newNode.key,
    oldNode.id,
    newNode.components,
    reconcileChildren(newNode.children, oldNode.children),
  );
}

function reconcileChildren(
  newChildren: RenderNode[],
  oldChildren: RenderNode[],
): RenderNode[] {
  const outChildren: RenderNode[] = [];
  const oldChildrenByKey: Map<string, RenderNode> = new Map();
  oldChildren.forEach(child => oldChildrenByKey.set(child.key, child));

  newChildren.forEach(child => {
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

function mapEntitiesToComponents(
  entities: VirtualEntity[],
): Map<number, Component[]> {
  const map: Map<number, Component[]> = new Map();
  entities.forEach(entity => {
    const key = entity[0];
    const value = entity[1];
    if (map.get(key) == undefined) {
      map.set(key, []);
    }

    const components = map.get(key);
    if (components !== undefined) {
      components.push(...value);
    } else {
      throw new Error('components shouldnt be undefined');
    }
  });

  return map;
}

function diffTrees(
  newEntities: VirtualEntity[],
  oldEntities: VirtualEntity[],
): SceneDiff {
  const createdComponents: ComponentPair[] = [];
  const deletedComponents: ComponentPair[] = [];

  const oldEntityIds = oldEntities.map(entity => entity[0]);
  const newEntityIds = newEntities.map(entity => entity[0]);

  const createdEntities = newEntityIds.filter(
    entityId => !oldEntityIds.includes(entityId),
  );
  const deletedEntities = oldEntityIds.filter(
    entityId => !newEntityIds.includes(entityId),
  );

  const oldComponents = mapEntitiesToComponents(oldEntities);
  const newComponents = mapEntitiesToComponents(newEntities);

  createdEntities.forEach(entityId => {
    const components = (newComponents.get(entityId) || []).map(it => [
      entityId,
      it,
    ]);
    createdComponents.push(...components);
  });

  newComponents.forEach((value, key) => {
    if (oldComponents.get(key) == undefined) {
      return;
    }

    const oldComponentsForKey = oldComponents.get(key) || [];
    const newComponentsForKey = value;

    const deleted = oldComponentsForKey.filter(
      it => !newComponentsForKey.includes(it),
    );
    const created = newComponentsForKey.filter(
      it => !oldComponentsForKey.includes(it),
    );

    deleted.forEach(it => deletedComponents.push([key, it]));
    created.forEach(it => createdComponents.push([key, it]));
  });

  return {
    createdEntities: createdEntities,
    deletedEntities: deletedEntities,
    createdComponents: createdComponents,
    deletedComponents: deletedComponents,
  };
}

function runTest(includeTreeSerialization: boolean): ?SceneDiff {
  /* Render a tree with 10 widgets */
  const oldCtx = new Context('root');
  const oldWidgetTree = new TestApp(false).render();
  const oldRenderTree = oldWidgetTree.reduce(oldCtx);
  const oldEntityTree = oldRenderTree.reduce();

  /* Render a tree with 25 widgets, the first 5 of which are the same */
  const newCtx = new Context('root');
  const newWidgetTree = new TestApp(true).render();
  const newRenderTree = newWidgetTree.reduce(newCtx);

  const reconciledRenderTree = reconcileRenderNode(
    newRenderTree,
    oldRenderTree,
  );
  const reconciledEntityTree = reconciledRenderTree.reduce();
  const diff = diffTrees(reconciledEntityTree, oldEntityTree);
  return includeTreeSerialization ? diff : null;
}

// ==> components.js <==

type NumberComponent = {
  x: number,
};

type StringComponent = {
  x: string,
};

type Component = NumberComponent | StringComponent;

// ==> context.js <==

class Context {
  key: string;
  childCounter: number;

  constructor(key: string) {
    this.key = key;
    this.childCounter = 0;
  }

  static createForChild(parentCtx: Context, child: Widget): Context {
    const widgetKey = child.key;
    const childKey =
      widgetKey !== null && widgetKey !== undefined
        ? widgetKey
        : `${child.constructor.name}_${parentCtx.childCounter++}`;
    const newKey = `${parentCtx.key}_${childKey}`;
    return new Context(newKey);
  }
}

// ==> render_node.js <==

type VirtualEntity = [number, Component[]];

class RenderNode {
  key: string;
  id: number;
  components: Component[];
  children: RenderNode[];
  static idCounter: number = 0;

  constructor(
    key: string,
    id: number,
    components: Component[],
    children: ?(RenderNode[]),
  ) {
    this.key = key;
    this.id = id;
    this.components = components;
    this.children = children || [];
  }

  reduce(): VirtualEntity[] {
    const childrenEntities = (this.children || []).flatMap(child =>
      child.reduce(),
    );
    return [[this.id, this.components]].concat(childrenEntities);
  }

  static create(
    ctx: Context,
    components: Component[],
    children: ?(RenderNode[]),
  ): RenderNode {
    return new RenderNode(
      ctx.key,
      RenderNode.idCounter++,
      components,
      children,
    );
  }

  static createForChild(ctx: Context, child: Widget): RenderNode {
    const childCtx = Context.createForChild(ctx, child);
    return child.reduce(childCtx);
  }
}

// ==> test_app.js <==

type RenderData = {
  modelPath: string,
  buttonSize: number,
};

class ButtonAndModel extends ComposedWidget {
  data: RenderData;

  constructor(data: RenderData) {
    super();
    this.data = data;
  }

  render(): Widget {
    const children = [
      new Button(this.data.buttonSize),
      new Gltf(this.data.modelPath),
      new Floater(Math.sqrt(this.data.buttonSize)),
    ];

    return new Container(children);
  }
}

class TestApp extends ComposedWidget {
  renderLarge: boolean;

  constructor(renderLarge: boolean) {
    super();
    this.renderLarge = renderLarge;
  }

  getWidgets(sizes: number[], models: string[]): Widget[] {
    if (sizes.length != models.length) {
      throw new Error('sizes and models must have same length');
    }
    return models.map((modelPath, index) => {
      const buttonSize = sizes[index];
      const widget = new ButtonAndModel({
        modelPath: modelPath,
        buttonSize: buttonSize,
      });
      widget.key = `${modelPath}_${buttonSize}`;
      return widget;
    });
  }

  getChildren(): Widget[] {
    if (this.renderLarge) {
      return this.getWidgets(SIZES_LARGE, MODELS_LARGE);
    }
    return this.getWidgets(SIZES_SMALL, MODELS_SMALL);
  }

  render(): Widget {
    const children = this.getChildren();
    return new Container(children);
  }
}

// ==> test_app_data.js <==

const SIZES_SMALL: number[] = [64, 8, 8, 18, 7, 24, 84, 4, 29, 58];
const SIZES_LARGE: number[] = SIZES_SMALL.slice(0, 5).concat([
  99, 9, 61, 27, 58, 30, 27, 95, 49, 37, 28, 87, 60, 95, 1, 58, 14, 90, 9, 57,
]);
const MODELS_SMALL: string[] = [
  'sazGTSGrfY',
  'uEQjieLDUq',
  'jQKzwhnzYa',
  'buIwVjnNDI',
  'goBJPxAkFf',
  'uKihCBaMwm',
  'VAyeIqqnSU',
  'bMNULcHsKb',
  'NBMEpcDimq',
  'wMCIoQQbNg',
];
const MODELS_LARGE: string[] = MODELS_SMALL.slice(0, 5).concat([
  'oCtHzjLczM',
  'GJVcmhfddz',
  'nbpEAplbzQ',
  'yNXDBUcDys',
  'IZZQpqwiXa',
  'AAroNFkOBf',
  'flsXwiIaQG',
  'qazjSVkFcR',
  'PefkCqwfKJ',
  'yJDvizIEDY',
  'XauGblPeuo',
  'ZnvLBVjEom',
  'UeosvyfoBE',
  'BFeZAIAHQq',
  'iOYUWXWXhr',
  'WpOfaJwOlm',
  'sVHOxutIGB',
  'qOikwyZSWx',
  'KJGEPQxUKU',
  'cqrRvLCCYB',
]);

// ==> index.js <==

globalThis.runFullTest = runTest;

// ==> main.js <==

const printDiff = false;

if (printDiff) {
    print(JSON.stringify(runFullTest(true)));
} else {
    print("Running...");
    // Warmup.
    var i: number;
    for(i = 0; i < 50; ++i)
      runFullTest(false);
    // The actual execution.
    let t1 = Date.now();
    for(i = 0; i < 5000; ++i)
      runFullTest(false);
    print(Date.now() - t1, "ms", i, "iterations");
}

})();
