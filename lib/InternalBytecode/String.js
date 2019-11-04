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

/**
 * Helper function for String.prototype.startsWith and String.prototypr.endsWith,
 * see their docuemntations for more details.
 * @param str a value that's coercible to string.
 * @param searchString string to search for in `this` string.
 * @param position if isStartsWith is true, this is position at which the search
 *                 starts, otherwise it's the position at which the search ends.
 * @param isStartsWith boolean flag for whether the caller function is
 *                     startsWith or endsWith.
 * @return true if searchString is found in str at the desired position, false
 *         false otherwise.
 */
function stringPrototypeStartsOrEndsWithHelper(
  str,
  searchString,
  position,
  isStartsWith,
) {
  // 1. Let O be ? RequireObjectCoercible(this value).
  if (str === undefined || str === null) {
    throw _TypeError('Value not coercible to object');
  }
  // 2. Let S be ? ToString(O).
  var S = HermesInternal.toString(str);
  // 3. Let isRegExp be ? IsRegExp(searchString).
  // 4. If isRegExp is true, throw a TypeError exception.
  if (HermesInternal.isRegExp(searchString)) {
    throw _TypeError('First argument must not be a RegExp.');
  }

  // 5. Let searchStr be ? ToString(searchString).
  var searchStr = HermesInternal.toString(searchString);
  // 6. Let len be the length of S.
  var len = S.length;
  // 7. Let pos be ? ToInteger(position). (startsWith)
  //    If endPosition is undefined, let pos be len, else let pos be
  //    ? ToInteger(endPosition). (endsWith)
  var pos = HermesInternal.toInteger(position);
  if (!isStartsWith && position === undefined) {
    pos = len;
  }

  var start, end;
  // 8. Let start be min(max(pos, 0), len). (startsWith)
  //    Let end be min(max(pos, 0), len). (endsWith)
  start = _MathMin(_MathMax(pos, 0), len);
  if (!isStartsWith) {
    end = start;
  }

  // 9. Let searchLength be the length of searchStr.
  var searchLength = searchStr.length;

  // 10. Let start be end - searchLength. (endsWith)
  // Note: there is no definition of end for startsWith, adding one
  //       here so we can have a general case for substring
  if (isStartsWith) {
    end = start + searchLength;
  } else {
    start = end - searchLength;
  }

  // 11. If searchLength + start is greater than len, return false. (startsWith)
  //     If start is less than 0, return false. (endsWith)
  if (start < 0 || end > len) return false;
  // 12. If the sequence of code units of S starting at start of length
  //     searchLength is the same as the full code unit sequence of searchStr,
  //     return true.
  // 13. Otherwise, return false.
  return (
    -1 !== HermesInternal.searchString(S, searchStr, false, start, len - end)
  );
}

/**
 * ES10.0 22.1.3.6
 * Checks to see that if searchString is a suffix of
 * `this`.substring(0, endPosition).
 * Will coerce both values to strings if they aren't already.
 * @this a value that's coercible to string.
 * @param searchString string to search for in `this` string.
 * @param [endPosition] position at which the search ends.
 * @return true if searchString is a suffix of `this`.substring(0, endPosition),
 *         false otherwise.
 */
String.prototype.endsWith = function endsWith(
  searchString,
  endPosition = undefined,
) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');
  return stringPrototypeStartsOrEndsWithHelper(
    this,
    searchString,
    endPosition,
    false,
  );
};

/**
 * ES10.0 22.1.3.20
 * Checks to see that if searchString is a prefix of `this`.substring(position).
 * Will coerce both values to strings if they aren't already.
 * @this a value that's coercible to string.
 * @param searchString string to search for in `this` string.
 * @param [position] position at which the search starts.
 * @return true if searchString is a prefix of `this`.substring(position),
 *         false otherwise.
 */
String.prototype.startsWith = function startsWith(
  searchString,
  position = undefined,
) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');
  return stringPrototypeStartsOrEndsWithHelper(
    this,
    searchString,
    position,
    true,
  );
};

/**
 * ES10.0 22.1.3.7
 * Checks to see that if searchString is a substring of
 * `this`.substring(position).
 * Will coerce both values to strings if they aren't already.
 * @this a value that's coercible to string.
 * @param searchString string to search for in `this` string.
 * @param [position] position at which the search starts.
 * @return true if searchString is a substring of `this`.substring(position),
 *         false otherwise.
 */
String.prototype.includes = function includes(
  searchString,
  position = undefined,
) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? RequireObjectCoercible(this value).
  if (this === undefined || this === null) {
    throw _TypeError('Value not coercible to object');
  }
  // 2. Let S be ? ToString(O).
  var S = HermesInternal.toString(this);
  // 3. Let isRegExp be ? IsRegExp(searchString).
  // 4. If isRegExp is true, throw a TypeError exception.
  if (HermesInternal.isRegExp(searchString)) {
    throw _TypeError('First argument to endsWith must not be a RegExp.');
  }

  // 5. Let searchStr be ? ToString(searchString).
  var searchStr = HermesInternal.toString(searchString);
  // 6. Let pos be ? ToInteger(position).
  var pos = HermesInternal.toInteger(position);
  // 7. Assert: If position is undefined, then pos is 0.
  // 8. Let len be the length of S.
  var len = S.length;
  // 9. Let start be min(max(pos, 0), len).
  var start = _MathMin(_MathMax(pos, 0), len);
  // 10. Let searchLen be the length of searchStr.
  var searchLength = searchStr.length;
  // 11. If there exists any integer k not smaller than start such that
  //     k + searchLen is not greater than len, and for all nonnegative integers
  //     j less than searchLen, the code unit at index k + j within S is the
  //     same as the code unit at index j within searchStr, return true;
  //     but if there is no such integer k, return false.
  return -1 !== HermesInternal.searchString(S, searchStr, false, start);
};

/**
 * Helper function for String.prototype.indexOf and String.prototype.lastIndexOf,
 * see their docuemntations for more details.
 * @param str a value that's coercible to string.
 * @param searchString string to search for in `this` string.
 * @param position position at which the search starts.
 * @param reverse boolean flag for whether str should be searched from
 *                left to right or right to left.
 * @return the first index at which the searchString can be found, if
 *         no such index exists, return -1.
 */
function stringPrototypeIndexOfHelper(str, searchString, position, reverse) {
  // 1. Let O be ? RequireObjectCoercible(this value).
  if (str === undefined || str === null) {
    throw _TypeError('Value not coercible to object');
  }
  // 2. Let S be ? ToString(O).
  var S = HermesInternal.toString(str);
  // 3. Let searchStr be ? ToString(searchString).
  var searchStr = HermesInternal.toString(searchString);

  // indexOf:
  // 4. Let pos be ? ToInteger(position).
  // 5. Assert: If position is undefined, then pos is 0.
  // lastIndexOf:
  // 4. Let numPos be ? ToNumber(position).
  // 5. Assert: If position is undefined, then numPos is NaN.
  // 6. If numPos is NaN, let pos be +∞; otherwise,
  //    let pos be ! ToInteger(numPos).
  var pos = HermesInternal.toInteger(position);
  if (reverse) {
    var numPos = +position;
    if (numPos !== numPos) {
      pos = Infinity;
    }
  }

  // 6. Let len be the length of S.
  var len = S.length;
  // 7. Let start be min(max(pos, 0), len).
  var start = _MathMin(_MathMax(pos, 0), len);
  // 8. Let searchLen be the length of searchStr.
  var searchLen = searchStr.length;
  // 9. Return the smallest possible integer k not smaller than start such that
  //    k + searchLen is not greater than len, and for all nonnegative integers
  //    j less than searchLen, the code unit at index k + j within S is the same
  //    as the code unit at index j within searchStr; but if there is no such
  //    integer k, return the value -1. (indexOf)
  //    Return the largest possible nonnegative integer k not larger than start
  //    such that k + searchLen is not greater than len, and for all nonnegative
  //    integers j less than searchLen, the code unit at index k + j within S is
  //    the same as the code unit at index j within searchStr; but if there is
  //    no such integer k, return the value -1. (lastIndexOf)
  var offset = start;
  if (reverse) offset = _MathMax(len - start - searchLen, 0);
  var k = HermesInternal.searchString(S, searchStr, reverse, offset);
  if (k < 0) return -1;
  if (reverse) return k - searchLen;
  return k;
}

/**
 * ES10.0 22.1.3.8
 * Tries to find searchString in `this`.substring(position). If found at one or
 * more indices, returns the smallest such index; otherwise, -1 is returned.
 * Will coerce both values to strings if they aren't already.
 * @this a value that's coercible to string.
 * @param searchString string to search for in `this` string.
 * @param [position] position at which the search starts.
 * @return the smallest index i such that searchString is a prefix of
 *         `this`.substring(i). If no such index exists, return -1.
 */
String.prototype.indexOf = function indexOf(
  searchString,
  position = undefined,
) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');
  return stringPrototypeIndexOfHelper(this, searchString, position, false);
};

/**
 * ES10.0 22.1.3.8
 * Tries to find searchString in `this`.substring(0, position). If found at one
 * or more indices, returns the largest such index; otherwise, -1 is returned.
 * Will coerce both values to strings if they aren't already.
 * @this a value that's coercible to string.
 * @param searchString string to search for in `this` string.
 * @param [position] position at which the search starts.
 * @return the largest index i such that searchString is a suffix of
 *         `this`.substring(0, i). If no such index exists, return -1.
 */
String.prototype.lastIndexOf = function lastIndexOf(
  searchString,
  position = undefined,
) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');
  return stringPrototypeIndexOfHelper(this, searchString, position, true);
};

/**
 * ES10.0 22.1.3.19
 * Partition `this` string on all occurrences of separator, and return an array
 * containing the resulting substrings from the partition.
 * @this a value that's coercible to string.
 * @param separator pattern with which to partition the string, can be both
 *                  string or RegExp.
 * @param limit maximum number of substrings that will result from the
 *              partition.
 * @return an array containing the resulting substrings from the partition.
 */
String.prototype.split = function split(separator, limit) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? RequireObjectCoercible(this value).
  if (this === undefined || this === null) {
    throw _TypeError('Value not coercible to object');
  }
  // 2. If separator is neither undefined nor null, then
  if (separator !== undefined && separator !== null) {
    // a. Let splitter be ? GetMethod(separator, @@split).
    var splitter = getMethod(separator, _SymbolSplit);
    // b. If splitter is not undefined, then
    if (splitter !== undefined) {
      // i. Return ? Call(splitter, separator, « O, limit »).
      // Note: `separator(O)` would pass an undefined `this`, so
      // we need to go through HermesInternal instead.
      return HermesInternal.executeCall(splitter, separator, this, limit);
    }
  }

  // 3. Let S be ? ToString(O).
  var S = HermesInternal.toString(this);
  // 4. Let A be ! ArrayCreate(0).
  var A = [];
  // 5. Let lengthA be 0.
  var lengthA = 0;
  // 6. If limit is undefined, let lim be 2^32 - 1;
  //    else let lim be ? ToUint32(limit).
  var lim = limit === undefined ? 0xffffffff : limit >>> 0;
  // 7. Let s be the length of S.
  var s = S.length;
  // 8. Let p be 0.
  var p = 0;
  // 9. Let R be ? ToString(separator).
  var R = HermesInternal.toString(separator);
  // 10. If lim = 0, return A.
  if (lim === 0) return A;
  // 11. If separator is undefined, then
  if (separator === undefined) {
    // a. Perform ! CreateDataProperty(A, "0", S).
    HermesInternal.jsArraySetElementAt(A, 0, S);
    // b. Return A.
    A.length = 1;
    return A;
  }

  var r = R.length;
  // 12. If s = 0, then
  if (s === 0) {
    // a. Let z be SplitMatch(S, 0, R).
    // b. If z is not false, return A.
    // Note: S is an empty string, and all SplitMatch does in this case
    // is checking if R is also an empty string.
    if (r === 0) return A;
    // c. Perform ! CreateDataProperty(A, "0", S).
    HermesInternal.jsArraySetElementAt(A, 0, S);
    // d. Return A.
    A.length = 1;
    return A;
  }

  // 13. Let q be p.
  var q = p;
  // 14. Repeat, while q ≠ s
  while (q < s) {
    // a. Let e be SplitMatch(S, q, R).
    // b. If e is false, increase q by 1.
    // Note: spec's implementation increments q by 1 until R is found in S at q,
    // which isn't very efficient. This implementation follows our C++
    // implementation and sets q to be the next match of R in S, or break
    // out if there is no more matches.
    var matchPos = HermesInternal.searchString(S, R, false, q);
    if (matchPos < 0) break;

    q = matchPos;
    var e = matchPos + r;
    // c. Else e is an integer index ≤ s,
    // i. If e = p, increase q by 1.
    if (e === p) {
      q += 1;
      // ii. Else e ≠ p,
    } else {
      // 1. Let T be the String value equal to the substring of S consisting
      //    of the code units at indices p (inclusive) through q (exclusive).
      var T = S.substring(p, q);
      // 2. Perform ! CreateDataProperty(A, ! ToString(lengthA), T).
      HermesInternal.jsArraySetElementAt(A, lengthA, T);
      // 3. Increment lengthA by 1.
      lengthA += 1;
      // 4. If lengthA = lim, return A.
      if (lengthA === lim) {
        A.length = lengthA;
        return A;
      }
      // 5. Set p to e.
      p = e;
      // 6. Set q to p.
      q = p;
    }
  }
  // 15. Let T be the String value equal to the substring of S consisting of
  //     the code units at indices p (inclusive) through s (exclusive).
  var T = S.substring(p);
  // 16. Perform ! CreateDataProperty(A, ! ToString(lengthA), T).
  HermesInternal.jsArraySetElementAt(A, lengthA, T);
  // 17. Return A.
  A.length = lengthA + 1;
  return A;
};

Object.defineProperty(String.prototype, 'charAt', builtinFuncDescriptor);
Object.defineProperty(String.prototype, 'match', builtinFuncDescriptor);
Object.defineProperty(String.prototype, 'search', builtinFuncDescriptor);
Object.defineProperty(String.prototype, 'endsWith', builtinFuncDescriptor);
Object.defineProperty(String.prototype, 'startsWith', builtinFuncDescriptor);
Object.defineProperty(String.prototype, 'includes', builtinFuncDescriptor);
Object.defineProperty(String.prototype, 'indexOf', builtinFuncDescriptor);
Object.defineProperty(String.prototype, 'lastIndexOf', builtinFuncDescriptor);
Object.defineProperty(String.prototype, 'split', builtinFuncDescriptor);

String.prototype.charAt.prototype = undefined;
String.prototype.match.prototype = undefined;
String.prototype.search.prototype = undefined;
String.prototype.endsWith.prototype = undefined;
String.prototype.startsWith.prototype = undefined;
String.prototype.includes.prototype = undefined;
String.prototype.indexOf.prototype = undefined;
String.prototype.lastIndexOf.prototype = undefined;
String.prototype.split.prototype = undefined;
