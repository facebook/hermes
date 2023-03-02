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
  constructor() {
    super();
  }

  render(): Widget {
    throw new Error('Implement this in a subclass');
  }

  reduce(ctx: Context): RenderNode {
    let child: Widget = this.render();
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
    const component: NumberComponent = new NumberComponent(this.num);
    return RenderNode_create(ctx, [component], null);
  }
}

class Floater extends Widget {
  num: number;

  constructor(num: number) {
    super();
    this.num = num;
  }

  reduce(ctx: Context): RenderNode {
    const component: NumberComponent = new NumberComponent(this.num);
    return RenderNode_create(ctx, [component], null);
  }
}

class Gltf extends Widget {
  path: string;

  constructor(path: string) {
    super();
    this.path = path;
  }

  reduce(ctx: Context): RenderNode {
    const component: StringComponent = new StringComponent(this.path);
    return RenderNode_create(ctx, [component], null);
  }
}

class Container extends Widget {
  children: Widget[];

  constructor(children: Widget[]) {
    super();
    this.children = children;
  }

  reduce(ctx: Context): RenderNode {
    const component: NumberComponent = new NumberComponent(13);
    const children: RenderNode[] = this.children.map(child =>
      RenderNode_createForChild(ctx, child),
    );
    return RenderNode_create(ctx, [component], children);
  }
}

// ==> app_runner.js <==

// type ComponentPair = [number, Component];
class ComponentPair {
  key: number;
  value: Component;

  constructor(key: number, value: Component) {
    this.key = key;
    this.value = value;
  }
}

// type SceneDiff = {
//   createdEntities: number[],
//   deletedEntities: number[],
//   createdComponents: ComponentPair[],
//   deletedComponents: ComponentPair[],
// };
class SceneDiff {
  createdEntities: number[];
  deletedEntities: number[];
  createdComponents: ComponentPair[];
  deletedComponents: ComponentPair[];

  constructor(
    createdEntities: number[],
    deletedEntities: number[],
    createdComponents: ComponentPair[],
    deletedComponents: ComponentPair[],
  ) {
    this.createdEntities = createdEntities;
    this.deletedEntities = deletedEntities;
    this.createdComponents = createdComponents;
    this.deletedComponents = deletedComponents;
  }
}

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
  const oldChildrenByKey: any = new Map();
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
): any {
  const map: any = new Map();
  entities.forEach((entity: VirtualEntity) => {
    const key: number = entity.key;
    const value: Component[] = entity.value;
    if (map.get(key) == undefined) {
      map.set(key, []);
    }

    const components: Component[] = map.get(key);
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

  const oldEntityIds: number[] = oldEntities.map(entity => entity.key);
  const newEntityIds: number[] = newEntities.map(entity => entity.key);

  const createdEntities: number[] = newEntityIds.filter(
    (entityId: number) => !oldEntityIds.includes(entityId),
  );
  const deletedEntities: number[] = oldEntityIds.filter(
    (entityId: number) => !newEntityIds.includes(entityId),
  );

  const oldComponents: any = mapEntitiesToComponents(oldEntities);
  const newComponents: any = mapEntitiesToComponents(newEntities);

  createdEntities.forEach((entityId: number) => {
    const components = (newComponents.get(entityId) || []).map(
      (it: Component) => new ComponentPair(entityId, it),
    );
    createdComponents.push(...components);
  });

  newComponents.forEach((value: Component[], key: number) => {
    if (oldComponents.get(key) == undefined) {
      return;
    }

    const oldComponentsForKey: Component[] = oldComponents.get(key) || [];
    const newComponentsForKey: Component[] = value;

    const deleted = oldComponentsForKey.filter(
      (it: Component) => !newComponentsForKey.includes(it),
    );
    const created = newComponentsForKey.filter(
      (it: Component) => !oldComponentsForKey.includes(it),
    );

    deleted.forEach(it => deletedComponents.push(new ComponentPair(key, it)));
    created.forEach(it => createdComponents.push(new ComponentPair(key, it)));
  });

  return new SceneDiff(
    createdEntities,
    deletedEntities,
    createdComponents,
    deletedComponents,
  );
}

function runTest(includeTreeSerialization: boolean): ?SceneDiff {
  /* Render a tree with 10 widgets */
  const oldCtx: Context = new Context('root');
  const oldWidgetTree: Widget = new TestApp(false).render();
  const oldRenderTree: RenderNode = oldWidgetTree.reduce(oldCtx);
  const oldEntityTree: VirtualEntity[] = oldRenderTree.reduce();

  /* Render a tree with 25 widgets, the first 5 of which are the same */
  const newCtx: Context = new Context('root');
  const newWidgetTree: Widget = new TestApp(true).render();
  const newRenderTree: RenderNode = newWidgetTree.reduce(newCtx);

  const reconciledRenderTree: RenderNode = reconcileRenderNode(
    newRenderTree,
    oldRenderTree,
  );
  const reconciledEntityTree: VirtualEntity[] = reconciledRenderTree.reduce();
  const diff: SceneDiff = diffTrees(reconciledEntityTree, oldEntityTree);
  return includeTreeSerialization ? diff : null;
}

// ==> components.js <==

class NumberComponent {
  x: number;
  constructor(x: number) {
    this.x = x;
  }
}

class StringComponent {
  x: string;
  constructor(x: string) {
    this.x = x;
  }
}

type Component = NumberComponent | StringComponent;

// ==> context.js <==

function Context_createForChild(parentCtx: Context, child: Widget): Context {
  const widgetKey = child.key;
  const childKey =
    widgetKey !== null && widgetKey !== undefined
      ? widgetKey
      : `_${parentCtx.childCounter++}`;
  const newKey = `${parentCtx.key}_${childKey}`;
  return new Context(newKey);
}

class Context {
  key: string;
  childCounter: number;

  constructor(key: string) {
    this.key = key;
    this.childCounter = 0;
  }
}

// ==> render_node.js <==

// type VirtualEntity = [number, Component[]];
class VirtualEntity {
  key: number;
  value: Component[];

  constructor(key: number, value: Component[]) {
    this.key = key;
    this.value = value;
  }
}

let RenderNode_idCounter: number = 0;

function RenderNode_create(
  ctx: Context,
  components: Component[],
  children: ?(RenderNode[]),
): RenderNode {
  return new RenderNode(ctx.key, RenderNode_idCounter++, components, children);
}

function RenderNode_createForChild(ctx: Context, child: Widget): RenderNode {
  const childCtx = Context_createForChild(ctx, child);
  return child.reduce(childCtx);
}

class RenderNode {
  key: string;
  id: number;
  components: Component[];
  children: RenderNode[];

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
    const childrenEntities: VirtualEntity[] = (this.children || []).flatMap(
      (child: RenderNode) => child.reduce(),
    );
    return [new VirtualEntity(this.id, this.components)].concat(
      childrenEntities,
    );
  }
}

// ==> test_app.js <==

// type RenderData = {
//   modelPath: string,
//   buttonSize: number,
// };
class RenderData {
  modelPath: string;
  buttonSize: number;

  constructor(modelPath: string, buttonSize: number) {
    this.modelPath = modelPath;
    this.buttonSize = buttonSize;
  }
}

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
      const widget = new ButtonAndModel(new RenderData(
        modelPath,
        buttonSize,
      ));
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
