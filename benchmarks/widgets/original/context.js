/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {Widget} from './widget';

export default class Context {
  key: string;
  childCounter: number;

  constructor(key: string) {
    this.key = key;
    this.childCounter = 0;
  }

  static createForChild(parentCtx: Context, child: Widget): Context {
    const widgetKey = child.key;
    const childKey =
      widgetKey !== null && widgetKey !== undefined
        ? widgetKey
        : `${child.constructor.name}_${parentCtx.childCounter++}`;
    const newKey = `${parentCtx.key}_${childKey}`;
    return new Context(newKey);
  }
}
