/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {NumberComponent, StringComponent} from './components.js';
import {Widget, ComposedWidget} from './widget.js';
import {RenderNode} from './render_node.js';
import Context from './context.js';

export class Button extends Widget {
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

export class Floater extends Widget {
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

export class Gltf extends Widget {
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

export class Container extends Widget {
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
