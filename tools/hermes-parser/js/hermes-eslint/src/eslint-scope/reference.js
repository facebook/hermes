/**
 * Portions Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

/*
  Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
'use strict';

const READ = 0x1;
const WRITE = 0x2;
const RW = READ | WRITE;

/**
 * A Reference represents a single occurrence of an identifier in code.
 * @class Reference
 */
class Reference {
  constructor(
    ident,
    scope,
    flag,
    writeExpr,
    maybeImplicitGlobal,
    init,
    isTypeReference,
  ) {
    /**
     * Identifier syntax node.
     * @member {espreeIdentifier} Reference#identifier
     */
    this.identifier = ident;

    /**
     * Reference to the enclosing Scope.
     * @member {Scope} Reference#from
     */
    this.from = scope;

    /**
     * The variable this reference is resolved with.
     * @member {Variable} Reference#resolved
     */
    this.resolved = null;

    /**
     * The read-write mode of the reference. (Value is one of {@link
     * Reference.READ}, {@link Reference.RW}, {@link Reference.WRITE}).
     * @member {number} Reference#flag
     * @private
     */
    this.flag = flag;
    if (this.isWrite()) {
      /**
       * If reference is writeable, this is the tree being written to it.
       * @member {espreeNode} Reference#writeExpr
       */
      this.writeExpr = writeExpr;

      /**
       * Whether the Reference is to write of initialization.
       * @member {boolean} Reference#init
       */
      this.init = init;
    }
    this.__maybeImplicitGlobal = maybeImplicitGlobal;

    this.__isTypeReference = isTypeReference;
  }

  /**
   * Whether the reference is static.
   * @method Reference#isStatic
   * @returns {boolean} static
   */
  isStatic() {
    return this.resolved && this.resolved.scope.isStatic();
  }

  /**
   * Whether the reference is writeable.
   * @method Reference#isWrite
   * @returns {boolean} write
   */
  isWrite() {
    return !!(this.flag & Reference.WRITE);
  }

  /**
   * Whether the reference is readable.
   * @method Reference#isRead
   * @returns {boolean} read
   */
  isRead() {
    return !!(this.flag & Reference.READ);
  }

  /**
   * Whether the reference is read-only.
   * @method Reference#isReadOnly
   * @returns {boolean} read only
   */
  isReadOnly() {
    return this.flag === Reference.READ;
  }

  /**
   * Whether the reference is write-only.
   * @method Reference#isWriteOnly
   * @returns {boolean} write only
   */
  isWriteOnly() {
    return this.flag === Reference.WRITE;
  }

  /**
   * Whether the reference is read-write.
   * @method Reference#isReadWrite
   * @returns {boolean} read write
   */
  isReadWrite() {
    return this.flag === Reference.RW;
  }

  /**
   * Whether the reference is for a value.
   */
  isValueReference() {
    return !this.__isTypeReference;
  }

  /**
   * Whether the reference is for a type.
   */
  isTypeReference() {
    return this.__isTypeReference;
  }
}

/**
 * @constant Reference.READ
 * @private
 */
Reference.READ = READ;

/**
 * @constant Reference.WRITE
 * @private
 */
Reference.WRITE = WRITE;

/**
 * @constant Reference.RW
 * @private
 */
Reference.RW = RW;

module.exports = Reference;
