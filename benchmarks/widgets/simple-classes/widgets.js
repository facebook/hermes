/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

(function() {

const mapPrototypeGet: any = Map.prototype.get;
const mapPrototypeSet: any = Map.prototype.set;

function arrayPrototypeMap<T, U>(arr: T[], cb: T => U): U[] {
  'inline';
  var length: number = arr.length;
  var result: U[] = [];
  for (var i: number = 0; i < length; ++i) {
    var elem: T = arr[i];
    result.push(cb(elem));
  }
  return result;
}

function arrayPrototypeMapWithIndex<T, U>(arr: T[], cb: (T, number) => U): U[] {
  'inline';
  var length: number = arr.length;
  var result: U[] = [];
  for (var i: number = 0; i < length; ++i) {
    var elem: T = arr[i];
    result.push(cb(elem, i));
  }
  return result;
}

function arrayPrototypeFilter<T>(arr: T[], cb: T => any): T[] {
  'inline';
  var result: T[] = [];
  var resultlength: number = 0;
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: T = arr[i];
    if (cb(elem)) {
      result.push(elem);
    }
  }
  return result;
}

function arrayPrototypeIncludes<T>(arr: T[], x: T): bool {
  'inline';
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: T = arr[i];
    if (elem === x)
      return true;
  }
  return false;
}

// forEach overloads take callbacks which return 'any' for now,
// but eventually we'd probably want to make that return type generic in order
// to avoid unnecessary conversions to 'any.
// Note that Flow's type definitions have the callbacks return 'mixed'.

function arrayPrototypeForEach<T>(arr: T[], cb: T => any): void {
  'inline';
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: T = arr[i];
    cb(elem);
  }
}

function arrayPrototypeConcat<T>(
  arr1: T[],
  arr2: T[],
): T[] {
  return [...arr1, ...arr2];
}

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
    const component: Component = new NumberComponent(this.num);
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
    const component: Component = new NumberComponent(this.num);
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
    const component: Component = new StringComponent(this.path);
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
    const component: Component = new NumberComponent(13);
    const children: RenderNode[] = arrayPrototypeMap<Widget, RenderNode>(
      this.children,
      (child: Widget): RenderNode => RenderNode_createForChild(ctx, child),
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
    'inline'
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
    'inline'
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
  arrayPrototypeForEach<RenderNode>(oldChildren, (child: RenderNode) => $SHBuiltin.call(mapPrototypeSet, oldChildrenByKey, child.key, child));

  arrayPrototypeForEach<RenderNode>(newChildren, (child: RenderNode) => {
    const newKey = child.key;
    const oldChild: RenderNode | void =
      $SHBuiltin.call(mapPrototypeGet, oldChildrenByKey, newKey);
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
  arrayPrototypeForEach<VirtualEntity>(entities, (entity: VirtualEntity) => {
    const key: number = entity.key;
    const value: Component[] = entity.value;
    if ($SHBuiltin.call(mapPrototypeGet, map, key) == undefined) {
      $SHBuiltin.call(mapPrototypeSet, map, key, ([]: Component[]));
    }

    const components: Component[] = $SHBuiltin.call(mapPrototypeGet, map, key);
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

  const oldEntityIds: number[] = arrayPrototypeMap<VirtualEntity, number>(
    oldEntities,
    (entity: VirtualEntity): number => entity.key,
  );
  const newEntityIds: number[] = arrayPrototypeMap<VirtualEntity, number>(
    newEntities,
    (entity: VirtualEntity): number => entity.key
  );

  const createdEntities: number[] = arrayPrototypeFilter<number>(newEntityIds,
    (entityId: number) => !arrayPrototypeIncludes<number>(oldEntityIds, entityId),
  );
  const deletedEntities: number[] = arrayPrototypeFilter<number>(oldEntityIds,
    (entityId: number) => !arrayPrototypeIncludes<number>(newEntityIds, entityId),
  );

  const oldComponents: any = mapEntitiesToComponents(oldEntities);
  const newComponents: any = mapEntitiesToComponents(newEntities);

  arrayPrototypeForEach<number>(createdEntities, (entityId: number) => {
    const components: ComponentPair[] = arrayPrototypeMap<Component, ComponentPair>(
      $SHBuiltin.call(mapPrototypeGet, newComponents, entityId) || ([]: Component[]),
      (it: Component): ComponentPair => new ComponentPair(entityId, it),
    );
    createdComponents.push(...components);
  });

  newComponents.forEach((value: Component[], key: number) => {
    if ($SHBuiltin.call(mapPrototypeGet, oldComponents, key) == undefined) {
      return;
    }

    const oldComponentsForKey: Component[] = $SHBuiltin.call(mapPrototypeGet, oldComponents, key) || ([]: Component[]);
    const newComponentsForKey: Component[] = value;

    const deleted: Component[] = arrayPrototypeFilter<Component>(
      oldComponentsForKey,
      (it: Component) => !arrayPrototypeIncludes<Component>(newComponentsForKey, it),
    );
    const created: Component[] = arrayPrototypeFilter<Component>(
      newComponentsForKey,
      (it: Component) => !arrayPrototypeIncludes<Component>(oldComponentsForKey, it),
    );

    arrayPrototypeForEach<Component>(
      deleted,
      (it: Component) => deletedComponents.push(new ComponentPair(key, it)),
    );
    arrayPrototypeForEach<Component>(
      created,
      (it: Component) => createdComponents.push(new ComponentPair(key, it)),
    );
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
    'inline'
    this.x = x;
  }
}

class StringComponent {
  x: string;
  constructor(x: string) {
    'inline'
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
    'inline'
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
    'inline'
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
    'inline'
    this.key = key;
    this.id = id;
    this.components = components;
    this.children = children || ([]: RenderNode[]);
  }

  reduce(): VirtualEntity[] {
    const childrenEntities: VirtualEntity[] = [];
    arrayPrototypeForEach<RenderNode>(
      this.children,
      (child: RenderNode) => childrenEntities.push(...child.reduce()),
    );
    return arrayPrototypeConcat<VirtualEntity>(
      [new VirtualEntity(this.id, this.components)],
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
    'inline'
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
    const children: Widget[] = [
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
    return arrayPrototypeMapWithIndex<string, Widget>(
      models,
      (modelPath: string, index: number): Widget => {
        const buttonSize = sizes[index];
        const widget = new ButtonAndModel(new RenderData(
          modelPath,
          buttonSize,
        ));
        widget.key = `${modelPath}_${buttonSize}`;
        return widget;
      }
    );
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
const SIZES_LARGE: number[] = [
  64, 8, 8, 18, 7, 99, 9, 61, 27, 58, 30, 27, 95, 49, 37, 28, 87, 60, 95, 1, 58, 14, 90, 9, 57,
];
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
const MODELS_LARGE: string[] = [
  'sazGTSGrfY',
  'uEQjieLDUq',
  'jQKzwhnzYa',
  'buIwVjnNDI',
  'goBJPxAkFf',
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
];

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
