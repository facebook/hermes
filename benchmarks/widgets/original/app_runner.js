/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {RenderNode} from './render_node.js';
import type {VirtualEntity} from './render_node.js';
import {TestApp} from './test_app.js';
import Context from './context.js';
import type {Component} from './components.js';

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

export function runTest(includeTreeSerialization: boolean): ?SceneDiff {
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
