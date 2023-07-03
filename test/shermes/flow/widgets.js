/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

// RUN: %shermes -typed -exec %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

(function() {

const mapPrototypeGet: any = Map.prototype.get;
const mapPrototypeSet: any = Map.prototype.set;

function arrayPrototypeMap_Widget(arr: Widget[], cb: any): any {
  'inline';
  var length: number = arr.length;
  var result: Widget[] = [];
  for (var i: number = 0; i < length; ++i) {
    var elem: Widget = arr[i];
    result.push(cb(elem));
  }
  return result;
}

function arrayPrototypeMap_Component(arr: Component[], cb: any): any {
  'inline';
  var length: number = arr.length;
  var result: Component[] = [];
  for (var i: number = 0; i < length; ++i) {
    var elem: Component = arr[i];
    result.push(cb(elem));
  }
  return result;
}

function arrayPrototypeMap_VirtualEntity(arr: VirtualEntity[], cb: any): any {
  'inline';
  var length: number = arr.length;
  var result: VirtualEntity[] = [];
  for (var i: number = 0; i < length; ++i) {
    var elem: VirtualEntity = arr[i];
    result.push(cb(elem));
  }
  return result;
}

function arrayPrototypeMap_StringWithIndexToWidget(arr: string[], cb: any): any {
  'inline';
  var length: number = arr.length;
  var result: Widget[] = [];
  for (var i: number = 0; i < length; ++i) {
    var elem: string = arr[i];
    result.push(cb(elem, i));
  }
  return result;
}

function arrayPrototypeFilter_number(arr: number[], cb: any): number[] {
  'inline';
  var result: number[] = [];
  var resultlength: number = 0;
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: number = arr[i];
    if (cb(elem)) {
      result.push(elem);
    }
  }
  return result;
}

function arrayPrototypeFilter_Component(arr: Component[], cb: any): Component[] {
  'inline';
  var result: Component[] = [];
  var resultlength: number = 0;
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: Component = arr[i];
    if (cb(elem)) {
      result.push(elem);
    }
  }
  return result;
}

function arrayPrototypeIncludes_number(arr: number[], x: any) {
  'inline';
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: number = arr[i];
    if (elem === x)
      return true;
  }
  return false;
}

function arrayPrototypeIncludes_Component(arr: Component[], x: any) {
  'inline';
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: Component = arr[i];
    if (elem === x)
      return true;
  }
  return false;
}

function arrayPrototypeForEach_number(arr: number[], cb: any) {
  'inline';
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: number = arr[i];
    cb(elem, i);
  }
}

function arrayPrototypeForEach_RenderNode(arr: RenderNode[], cb: any) {
  'inline';
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: RenderNode = arr[i];
    cb(elem, i);
  }
}

function arrayPrototypeForEach_VirtualEntity(arr: VirtualEntity[], cb: any) {
  'inline';
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: VirtualEntity = arr[i];
    cb(elem, i);
  }
}

function arrayPrototypeForEach_Component(arr: Component[], cb: any) {
  'inline';
  var length: number = arr.length;
  for (var i: number = 0; i < length; ++i) {
    var elem: Component = arr[i];
    cb(elem, i);
  }
}

function arrayPrototypeConcat_VirtualEntity(
  arr1: VirtualEntity[],
  arr2: VirtualEntity[],
): VirtualEntity[] {
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
    const children: RenderNode[] = arrayPrototypeMap_Widget(
      this.children,
      child => RenderNode_createForChild(ctx, child),
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
  arrayPrototypeForEach_RenderNode(oldChildren, (child: RenderNode) => $SHBuiltin.call(mapPrototypeSet, oldChildrenByKey, child.key, child));

  arrayPrototypeForEach_RenderNode(newChildren, (child: RenderNode) => {
    const newKey = child.key;
    const oldChild = $SHBuiltin.call(mapPrototypeGet, oldChildrenByKey, newKey);
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
  arrayPrototypeForEach_VirtualEntity(entities, (entity: VirtualEntity) => {
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

  const oldEntityIds: number[] = arrayPrototypeMap_VirtualEntity(
    oldEntities,
    (entity: VirtualEntity) => entity.key,
  );
  const newEntityIds: number[] = arrayPrototypeMap_VirtualEntity(
    newEntities,
    (entity: VirtualEntity) => entity.key
  );

  const createdEntities: number[] = arrayPrototypeFilter_number(newEntityIds,
    (entityId: number) => !arrayPrototypeIncludes_number(oldEntityIds, entityId),
  );
  const deletedEntities: number[] = arrayPrototypeFilter_number(oldEntityIds,
    (entityId: number) => !arrayPrototypeIncludes_number(newEntityIds, entityId),
  );

  const oldComponents: any = mapEntitiesToComponents(oldEntities);
  const newComponents: any = mapEntitiesToComponents(newEntities);

  arrayPrototypeForEach_number(createdEntities, (entityId: number) => {
    const components: Component[] = arrayPrototypeMap_Component($SHBuiltin.call(mapPrototypeGet, newComponents, entityId) || ([]: Component[]),
      (it: Component) => new ComponentPair(entityId, it),
    );
    createdComponents.push(...components);
  });

  newComponents.forEach((value: Component[], key: number) => {
    if ($SHBuiltin.call(mapPrototypeGet, oldComponents, key) == undefined) {
      return;
    }

    const oldComponentsForKey: Component[] = $SHBuiltin.call(mapPrototypeGet, oldComponents, key) || ([]: Component[]);
    const newComponentsForKey: Component[] = value;

    const deleted: Component[] = arrayPrototypeFilter_Component(
      oldComponentsForKey,
      (it: Component) => !arrayPrototypeIncludes_Component(newComponentsForKey, it),
    );
    const created: Component[] = arrayPrototypeFilter_Component(
      newComponentsForKey,
      (it: Component) => !arrayPrototypeIncludes_Component(oldComponentsForKey, it),
    );

    arrayPrototypeForEach_Component(
      deleted,
      (it: Component) => deletedComponents.push(new ComponentPair(key, it)),
    );
    arrayPrototypeForEach_Component(
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
    arrayPrototypeForEach_RenderNode(
      this.children,
      (child: RenderNode) => childrenEntities.push(...child.reduce()),
    );
    return arrayPrototypeConcat_VirtualEntity(
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
    return arrayPrototypeMap_StringWithIndexToWidget(models, (modelPath: string, index: number) => {
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

const printDiff = true;

if (printDiff) {
    print(JSON.stringify(runFullTest(true), null, 2));
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

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "createdEntities": [
// CHECK-NEXT:    64,
// CHECK-NEXT:    61,
// CHECK-NEXT:    62,
// CHECK-NEXT:    63,
// CHECK-NEXT:    68,
// CHECK-NEXT:    65,
// CHECK-NEXT:    66,
// CHECK-NEXT:    67,
// CHECK-NEXT:    72,
// CHECK-NEXT:    69,
// CHECK-NEXT:    70,
// CHECK-NEXT:    71,
// CHECK-NEXT:    76,
// CHECK-NEXT:    73,
// CHECK-NEXT:    74,
// CHECK-NEXT:    75,
// CHECK-NEXT:    80,
// CHECK-NEXT:    77,
// CHECK-NEXT:    78,
// CHECK-NEXT:    79,
// CHECK-NEXT:    84,
// CHECK-NEXT:    81,
// CHECK-NEXT:    82,
// CHECK-NEXT:    83,
// CHECK-NEXT:    88,
// CHECK-NEXT:    85,
// CHECK-NEXT:    86,
// CHECK-NEXT:    87,
// CHECK-NEXT:    92,
// CHECK-NEXT:    89,
// CHECK-NEXT:    90,
// CHECK-NEXT:    91,
// CHECK-NEXT:    96,
// CHECK-NEXT:    93,
// CHECK-NEXT:    94,
// CHECK-NEXT:    95,
// CHECK-NEXT:    100,
// CHECK-NEXT:    97,
// CHECK-NEXT:    98,
// CHECK-NEXT:    99,
// CHECK-NEXT:    104,
// CHECK-NEXT:    101,
// CHECK-NEXT:    102,
// CHECK-NEXT:    103,
// CHECK-NEXT:    108,
// CHECK-NEXT:    105,
// CHECK-NEXT:    106,
// CHECK-NEXT:    107,
// CHECK-NEXT:    112,
// CHECK-NEXT:    109,
// CHECK-NEXT:    110,
// CHECK-NEXT:    111,
// CHECK-NEXT:    116,
// CHECK-NEXT:    113,
// CHECK-NEXT:    114,
// CHECK-NEXT:    115,
// CHECK-NEXT:    120,
// CHECK-NEXT:    117,
// CHECK-NEXT:    118,
// CHECK-NEXT:    119,
// CHECK-NEXT:    124,
// CHECK-NEXT:    121,
// CHECK-NEXT:    122,
// CHECK-NEXT:    123,
// CHECK-NEXT:    128,
// CHECK-NEXT:    125,
// CHECK-NEXT:    126,
// CHECK-NEXT:    127,
// CHECK-NEXT:    132,
// CHECK-NEXT:    129,
// CHECK-NEXT:    130,
// CHECK-NEXT:    131,
// CHECK-NEXT:    136,
// CHECK-NEXT:    133,
// CHECK-NEXT:    134,
// CHECK-NEXT:    135,
// CHECK-NEXT:    140,
// CHECK-NEXT:    137,
// CHECK-NEXT:    138,
// CHECK-NEXT:    139
// CHECK-NEXT:  ],
// CHECK-NEXT:  "deletedEntities": [
// CHECK-NEXT:    23,
// CHECK-NEXT:    20,
// CHECK-NEXT:    21,
// CHECK-NEXT:    22,
// CHECK-NEXT:    27,
// CHECK-NEXT:    24,
// CHECK-NEXT:    25,
// CHECK-NEXT:    26,
// CHECK-NEXT:    31,
// CHECK-NEXT:    28,
// CHECK-NEXT:    29,
// CHECK-NEXT:    30,
// CHECK-NEXT:    35,
// CHECK-NEXT:    32,
// CHECK-NEXT:    33,
// CHECK-NEXT:    34,
// CHECK-NEXT:    39,
// CHECK-NEXT:    36,
// CHECK-NEXT:    37,
// CHECK-NEXT:    38
// CHECK-NEXT:  ],
// CHECK-NEXT:  "createdComponents": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 64,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 61,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 99
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 62,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "oCtHzjLczM"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 63,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 9.9498743710662
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 68,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 65,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 9
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 66,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "GJVcmhfddz"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 67,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 3
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 72,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 69,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 61
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 70,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "nbpEAplbzQ"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 71,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 7.810249675906654
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 76,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 73,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 27
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 74,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "yNXDBUcDys"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 75,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 5.196152422706632
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 80,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 77,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 58
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 78,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "IZZQpqwiXa"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 79,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 7.615773105863909
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 84,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 81,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 30
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 82,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "AAroNFkOBf"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 83,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 5.477225575051661
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 88,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 85,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 27
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 86,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "flsXwiIaQG"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 87,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 5.196152422706632
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 92,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 89,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 95
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 90,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "qazjSVkFcR"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 91,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 9.746794344808963
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 96,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 93,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 49
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 94,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "PefkCqwfKJ"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 95,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 7
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 100,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 97,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 37
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 98,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "yJDvizIEDY"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 99,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 6.082762530298219
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 104,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 101,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 28
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 102,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "XauGblPeuo"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 103,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 5.291502622129181
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 108,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 105,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 87
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 106,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "ZnvLBVjEom"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 107,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 9.327379053088816
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 112,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 109,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 60
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 110,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "UeosvyfoBE"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 111,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 7.745966692414834
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 116,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 113,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 95
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 114,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "BFeZAIAHQq"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 115,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 9.746794344808963
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 120,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 117,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 1
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 118,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "iOYUWXWXhr"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 119,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 1
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 124,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 121,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 58
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 122,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "WpOfaJwOlm"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 123,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 7.615773105863909
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 128,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 125,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 14
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 126,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "sVHOxutIGB"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 127,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 3.7416573867739413
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 132,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 129,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 90
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 130,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "qOikwyZSWx"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 131,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 9.486832980505138
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 136,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 133,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 9
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 134,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "KJGEPQxUKU"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 135,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 3
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 140,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 137,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 57
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 138,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "cqrRvLCCYB"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 139,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 7.54983443527075
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 40,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 3,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 0,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 64
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 1,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "sazGTSGrfY"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 2,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 8
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 7,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 4,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 8
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 5,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "uEQjieLDUq"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 6,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 2.8284271247461903
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 11,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 8,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 8
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 9,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "jQKzwhnzYa"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 10,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 2.8284271247461903
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 15,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 12,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 18
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 13,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "buIwVjnNDI"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 14,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 4.242640687119285
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 19,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 16,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 7
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 17,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "goBJPxAkFf"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 18,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 2.6457513110645907
// CHECK-NEXT:      }
// CHECK-NEXT:    }
// CHECK-NEXT:  ],
// CHECK-NEXT:  "deletedComponents": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 40,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 3,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 0,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 64
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 1,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "sazGTSGrfY"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 2,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 8
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 7,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 4,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 8
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 5,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "uEQjieLDUq"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 6,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 2.8284271247461903
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 11,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 8,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 8
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 9,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "jQKzwhnzYa"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 10,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 2.8284271247461903
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 15,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 12,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 18
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 13,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "buIwVjnNDI"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 14,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 4.242640687119285
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 19,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 13
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 16,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 7
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 17,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": "goBJPxAkFf"
// CHECK-NEXT:      }
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "key": 18,
// CHECK-NEXT:      "value": {
// CHECK-NEXT:        "x": 2.6457513110645907
// CHECK-NEXT:      }
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
