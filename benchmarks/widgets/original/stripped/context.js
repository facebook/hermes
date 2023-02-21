/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import { Widget as Widget } from "./widget";
export default class Context {
  constructor(key) {
    this.key = key;
    this.childCounter = 0;
  }
  static createForChild(parentCtx, child) {
    const widgetKey = child.key;
    const childKey =
      widgetKey !== null && widgetKey !== undefined
        ? widgetKey
        : `${child.constructor.name}_${parentCtx.childCounter++}`;
    const newKey = `${parentCtx.key}_${childKey}`;
    return new Context(newKey);
  }
}
