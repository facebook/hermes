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

/**
 * Helper function for String.prototype.match and String.prototype.search,
 * see their docuemntations for more details.
 * @param O a value that's coercible to string.
 * @param regexp an object with the desired method, if it doesn't have
 *               the desired method, it'll be treated as the pattern of a
 *               RegExp object.
 * @param methodSymbol symbol of the desired method.
 * @return the result of calling the desired method of regexp on O.
 */
function stringPrototypeMatchSearchHelper(O, regexp, methodSymbol) {
  // Note: the comments below are steps for String.prototype.match,
  // steps for String.prototype.search are pretty much the same,
  // the only functional difference is that it would be @@search
  // instead of @@match.

  // 1. Let O be ? RequireObjectCoercible(this value).
  if (O === undefined || O === null) {
    throw _TypeError('Value not coercible to object');
  }

  // 2. If regexp is neither undefined nor null, then
  if (regexp !== undefined && regexp != null) {
    // a. Let matcher be ? GetMethod(regexp, @@match).
    var matcher = getMethod(regexp, methodSymbol);
    // b. If matcher is not undefined, then
    if (matcher !== undefined) {
      // i. Return ? Call(matcher, regexp, « O »).
      // Note: `matcher(O)` would pass an undefined `this`, so
      // we need to go through HermesInternal instead.
      return HermesInternal.executeCall(matcher, regexp, O);
    }
  }
  // 3. Let S be ? ToString(O).
  var S = HermesInternal.toString(O);
  // 4. Let rx be ? RegExpCreate(regexp, undefined).
  var rx = HermesInternal.regExpCreate(regexp, undefined);
  // 5. Return ? Invoke(rx, @@match, « S »).
  return rx[methodSymbol](S);
}

/**
 * ES10.0 21.1.3.11
 * Calls the @@match method of regexp on `this` string. In most
 * use cases, regexp should be a RegExp object.
 * `this` will be coerced into a string if it's not already.
 * @this a value that's coercible to string.
 * @param regexp an object with a @@match method, if it doesn't
 *               have one, it'll be treated as the pattern of a
 *               RegExp object.
 * @return the result of calling the @@match method of regexp on
 *         `this` string.
 */
String.prototype.match = function match(regexp) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');
  return stringPrototypeMatchSearchHelper(this, regexp, _SymbolMatch);
};

/**
 * ES10.0 21.1.3.17
 * Calls the @@search method of regexp on `this` string. In most
 * use cases, regexp should be a RegExp object.
 * `this` will be coerced into a string if it's not already.
 * @this a value that's coercible to string.
 * @param regexp an object with a @@search method, if it doesn't
 *               have one, it'll be treated as the pattern of a
 *               RegExp object.
 * @return the result of calling the @@search method of regexp on
 *         `this` string.
 */
String.prototype.search = function search(regexp) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');
  return stringPrototypeMatchSearchHelper(this, regexp, _SymbolSearch);
};

Object.defineProperty(String.prototype, 'charAt', builtinFuncDescriptor);
Object.defineProperty(String.prototype, 'match', builtinFuncDescriptor);
Object.defineProperty(String.prototype, 'search', builtinFuncDescriptor);

String.prototype.charAt.prototype = undefined;
String.prototype.match.prototype = undefined;
String.prototype.search.prototype = undefined;
