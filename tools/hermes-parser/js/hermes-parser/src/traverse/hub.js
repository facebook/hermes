/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import type Scope from './scope';

export interface HubInterface {
  getCode(): ?string;
  getScope(): ?Scope;
  addHelper(name: string): Object;
  buildError(node: Object, msg: string, Error: Class<Error>): Error;
}

export default class Hub implements HubInterface {
  getCode() {}

  getScope() {}

  addHelper() {
    throw new Error('Helpers are not supported by the default hub.');
  }

  buildError(node, msg, Error = TypeError): Error {
    return new Error(msg);
  }
}
