/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

import type {RenderNode} from './render_node.js';
import Context from './context.js';

export class Widget {
  key: ?string;

  reduce(ctx: Context): RenderNode {
    throw new Error('Implement this in a subclass');
  }
}

export class ComposedWidget extends Widget {
  render(): Widget {
    throw new Error('Implement this in a subclass');
  }

  reduce(ctx: Context): RenderNode {
    let child = this.render();
    return child.reduce(ctx);
  }
}
