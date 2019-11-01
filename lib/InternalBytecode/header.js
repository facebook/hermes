/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// header.js + footer.js wraps around other js lib files to create a local scaope
// so that local variables / functions defined wihtin cannot be modified by users
(function() {
  'use strict';

  var builtinFuncDescriptor = {
    configurable: true,
    enumerable: false,
    writable: true,
  };

  var _Array = Array;
  var _ArrayIsArray = Array.isArray;
  var _MathFloor = Math.floor;
  var _MathMax = Math.max;
  var _MathMin = Math.min;
  var _ObjectDefineProperty = Object.defineProperty;
  var _SymbolIterator = Symbol.iterator;
  var _TypeError = TypeError;

  /**
   * ES10.0 7.3.9
   * Get property P from object V and verify that it's either
   * undefined, null, or a function.
   * @param V the object which to get the property from, will be
   *          coerced to an object if it's not already.
   * @param P the property key.
   * @return property P of object V.
   */
  function getMethod(V, P) {
    // 1. Assert: IsPropertyKey(P) is true.
    // 2. Let func be ? GetV(V, P).
    // The steps for GetV(V, P) are:
    //   1. Assert: IsPropertyKey(P) is true.
    //   2. Let O be ? ToObject(V).
    //   3. Return ? O.[[Get]](P, V).
    var O = HermesInternal.toObject(V);
    var func = O[P];
    // 3. If func is either undefined or null, return undefined.
    if (func === undefined || func === null) return undefined;
    // 4. If IsCallable(func) is false, throw a TypeError exception.
    if (typeof func != 'function')
      throw _TypeError('Could not get callable method from object');
    // 5. Return func.
    return func;
  }
