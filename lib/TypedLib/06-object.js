/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

class Object {
  @Hermes.final
  static create(proto: any): any {
    'inline';
    return globalThis.Object.create(proto);
  }

  /// TODO: Return a proper FastArray here.
  @Hermes.final
  static keys(obj: any): any {
    'inline';
    return globalThis.Object.keys(obj);
  }

  /// TODO: Return a proper FastArray here.
  @Hermes.final
  static values(obj: any): any {
    'inline';
    return globalThis.Object.values(obj);
  }

  /// TODO: Return a proper FastArray here.
  @Hermes.final
  static entries(obj: any): any {
    'inline';
    return globalThis.Object.entries(obj);
  }

  @Hermes.final
  static is<T>(a: T, b: T): boolean {
    'inline';
    return globalThis.Object.is(a, b) as boolean;
  }

  @Hermes.final
  static getPrototypeOf(a: any): any {
    'inline';
    return globalThis.Object.getPrototypeOf(a);
  }

  @Hermes.final
  toString(): string {
    'inline';
    return globalThis.Object.prototype.toString.call(this) as string;
  }

  @Hermes.final
  hasOwnProperty(prop: mixed): boolean {
    'inline';
    return globalThis.Object.prototype.hasOwnProperty.call(
      this,
      prop
    ) as boolean;
  }
}
