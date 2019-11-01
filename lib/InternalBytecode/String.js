/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

/**
 * ES10.0 21.1.3.1
 * Coerce the this value to a string if it's not a string already,
 * and returns the character at pos position in the string.
 * @this a value that's coercible to string.
 * @param pos position to get the character from.
 * @return the character at the given position of the given string.
 */
String.prototype.charAt = function charAt(pos) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? RequireObjectCoercible(this value).
  if (this === undefined || this === null) {
    throw _TypeError('Value not coercible to object');
  }
  // 2. Let S be ? ToString(O).
  var S = HermesInternal.toString(this);
  // 3. Let position be ? ToInteger(pos).
  var position = HermesInternal.toInteger(pos);
  // 4. Let size be the length of S.
  var size = S.length;
  // 5. If position < 0 or position ≥ size, return the empty String.
  if (position < 0 || position >= size) return '';
  // 6. Return the String value of length 1, containing one code unit
  //    from S, namely the code unit at index position.
  return S[position];
};

Object.defineProperty(String.prototype, 'charAt', builtinFuncDescriptor);

String.prototype.charAt.prototype = undefined;
