/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {Component} from './components.js';
import {Widget} from './widget';
import Context from './context.js';

export type VirtualEntity = [number, Component[]];

export class RenderNode {
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
