/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import { Widget as Widget } from "./widget";
import Context from "./context.js";
export class RenderNode {
  static idCounter = 0;
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
    return [[this.id, this.components]].concat(childrenEntities);
  }
  static create(ctx, components, children) {
    return new RenderNode(
      ctx.key,
      RenderNode.idCounter++,
      components,
      children
    );
  }
  static createForChild(ctx, child) {
    const childCtx = Context.createForChild(ctx, child);
    return child.reduce(childCtx);
  }
}
