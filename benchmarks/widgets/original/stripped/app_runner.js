/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import { RenderNode as RenderNode } from "./render_node.js";
import { TestApp as TestApp } from "./test_app.js";
import Context from "./context.js";
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
    const key = entity[0];
    const value = entity[1];
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
  const oldEntityIds = oldEntities.map((entity) => entity[0]);
  const newEntityIds = newEntities.map((entity) => entity[0]);
  const createdEntities = newEntityIds.filter(
    (entityId) => !oldEntityIds.includes(entityId)
  );
  const deletedEntities = oldEntityIds.filter(
    (entityId) => !newEntityIds.includes(entityId)
  );
  const oldComponents = mapEntitiesToComponents(oldEntities);
  const newComponents = mapEntitiesToComponents(newEntities);
  createdEntities.forEach((entityId) => {
    const components = (newComponents.get(entityId) || []).map((it) => [
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
      (it) => !newComponentsForKey.includes(it)
    );
    const created = newComponentsForKey.filter(
      (it) => !oldComponentsForKey.includes(it)
    );
    deleted.forEach((it) => deletedComponents.push([key, it]));
    created.forEach((it) => createdComponents.push([key, it]));
  });
  return {
    createdEntities: createdEntities,
    deletedEntities: deletedEntities,
    createdComponents: createdComponents,
    deletedComponents: deletedComponents,
  };
}
export function runTest(includeTreeSerialization) {
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
