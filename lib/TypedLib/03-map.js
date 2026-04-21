/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

class Map<K, V> {
  /// Native implementation of this Map.
  #impl: any;

  constructor() {
    "inline";
    this.#impl = new globalThis.Map();
  }

  @Hermes.final
  get(k: K): V | void {
    "inline";
    return this.#impl.get(k) as (V | void);
  }

  @Hermes.final
  set(k: K, v: V): Map<K, V> {
    "inline";
    this.#impl.set(k, v);
    return this;
  }

  @Hermes.final
  has(k: K): bool {
    "inline";
    return this.#impl.has(k) as bool;
  }

  @Hermes.final
  delete(k: K): bool {
    "inline";
    return this.#impl.delete(k) as bool;
  }

  @Hermes.final
  clear(): void {
    "inline";
    this.#impl.clear();
  }

  @Hermes.final
  forEach(cb: (v: V, k?: K, m?: Map<K, V>) => any): void {
    "inline";
    this.#impl.forEach((v, k, m) => {
      return cb(v, k, this);
    });
  }
}
