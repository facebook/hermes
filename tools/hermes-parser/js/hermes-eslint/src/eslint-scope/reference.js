/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import type {Identifier, ESNode as Node} from 'hermes-estree';
import type {Scope} from './scope';
import type Variable from './variable';

export type ReadWriteFlagType = 0x1 | 0x2 | 0x3;

const ReadWriteFlag = {
  READ: 0x1,
  WRITE: 0x2,
  RW: 0x3,
};

export type ImplicitGlobal = {
  pattern: Identifier,
  node: Node,
};

/**
 * A Reference represents a single occurrence of an identifier in code.
 * @class Reference
 */
class Reference {
  /**
   * Identifier syntax node.
   */
  identifier: Identifier;

  /**
   * Reference to the enclosing Scope.
   */
  from: Scope;

  /**
   * The variable this reference is resolved with.
   */
  resolved: ?Variable = null;

  /**
   * The read-write mode of the reference.
   */
  flag: ReadWriteFlagType;

  /**
   * If reference is writeable, this is the tree being written to it.
   */
  writeExpr: ?Node;

  /**
   * Whether the Reference is to write of initialization.
   */
  init: ?boolean;

  __maybeImplicitGlobal: ?ImplicitGlobal;
  __isTypeReference: boolean;

  constructor(
    ident: Identifier,
    scope: Scope,
    flag: ReadWriteFlagType,
    writeExpr: ?Node,
    maybeImplicitGlobal: ?ImplicitGlobal,
    init: boolean,
    isTypeReference: boolean,
  ) {
    this.identifier = ident;
    this.from = scope;
    this.flag = flag;

    if (this.isWrite()) {
      this.writeExpr = writeExpr;
      this.init = init;
    }

    this.__maybeImplicitGlobal = maybeImplicitGlobal;
    this.__isTypeReference = isTypeReference;
  }

  /**
   * Whether the reference is static.
   */
  isStatic(): boolean {
    return this.resolved != null && this.resolved.scope.isStatic();
  }

  /**
   * Whether the reference is writeable.
   */
  isWrite(): boolean {
    return !!(this.flag & ReadWriteFlag.WRITE);
  }

  /**
   * Whether the reference is readable.
   */
  isRead(): boolean {
    return !!(this.flag & ReadWriteFlag.READ);
  }

  /**
   * Whether the reference is read-only.
   */
  isReadOnly(): boolean {
    return this.flag === ReadWriteFlag.READ;
  }

  /**
   * Whether the reference is write-only.
   */
  isWriteOnly(): boolean {
    return this.flag === ReadWriteFlag.WRITE;
  }

  /**
   * Whether the reference is read-write.
   */
  isReadWrite(): boolean {
    return this.flag === ReadWriteFlag.RW;
  }

  /**
   * Whether the reference is for a value.
   */
  isValueReference(): boolean {
    return !this.__isTypeReference;
  }

  /**
   * Whether the reference is for a type.
   */
  isTypeReference(): boolean {
    return this.__isTypeReference;
  }
}

module.exports = {
  ReadWriteFlag,
  Reference,
};
