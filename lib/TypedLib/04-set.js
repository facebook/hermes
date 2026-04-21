/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

class Set<T> {
  /// Native implementation of this Set.
  #impl: any;

  constructor() {
    "inline";
    this.#impl = new globalThis.Set();
  }

  @Hermes.final
  add(v: T): Set<T> {
    "inline";
    this.#impl.add(v);
    return this;
  }

  @Hermes.final
  has(v: T): bool {
    "inline";
    return this.#impl.has(v) as bool;
  }

  @Hermes.final
  delete(v: T): bool {
    "inline";
    return this.#impl.delete(v) as bool;
  }

  @Hermes.final
  clear(): void {
    "inline";
    this.#impl.clear();
  }

  @Hermes.final
  forEach(cb: (v: T, k?: T, s?: Set<T>) => any): void {
    "inline";
    this.#impl.forEach((v, k, s) => {
      return cb(v, k, this);
    });
  }
}
