/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import {
  Widget as Widget,
  ComposedWidget as ComposedWidget,
} from "./widget.js";
import { RenderNode as RenderNode } from "./render_node.js";
import Context from "./context.js";
export class Button extends Widget {
  constructor(num) {
    super();
    this.num = num;
  }
  reduce(ctx) {
    const component = { x: this.num };
    return RenderNode.create(ctx, [component], null);
  }
}

export class Floater extends Widget {
  constructor(num) {
    super();
    this.num = num;
  }
  reduce(ctx) {
    const component = { x: this.num };
    return RenderNode.create(ctx, [component], null);
  }
}

export class Gltf extends Widget {
  constructor(path) {
    super();
    this.path = path;
  }
  reduce(ctx) {
    const component = { x: this.path };
    return RenderNode.create(ctx, [component], null);
  }
}

export class Container extends Widget {
  constructor(children) {
    super();
    this.children = children;
  }
  reduce(ctx) {
    const component = { x: 13 };
    const children = this.children.map((child) =>
      RenderNode.createForChild(ctx, child)
    );
    return RenderNode.create(ctx, [component], children);
  }
}
