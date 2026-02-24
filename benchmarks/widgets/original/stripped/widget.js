/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import Context from "./context.js";
export class Widget {
  reduce(ctx) {
    throw new Error("Implement this in a subclass");
  }
}

export class ComposedWidget extends Widget {
  render() {
    throw new Error("Implement this in a subclass");
  }
  reduce(ctx) {
    let child = this.render();
    return child.reduce(ctx);
  }
}
