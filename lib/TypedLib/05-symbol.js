/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

class Symbol {
  constructor() {
    throw new TypeError("Symbol is not a constructor");
  }

  @Hermes.final
  static for(key: string): symbol {
    "inline";
    return globalThis.Symbol.for(key) as symbol;
  }
}
