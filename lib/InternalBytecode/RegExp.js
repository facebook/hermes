/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

/**
 * ES10.0 21.2.5.2.3
 * Return the index of the next character in string S, taking unicode
 * into consideration.
 * @param S string whose index needs to be advanced.
 * @param index current index of S.
 * @param unicode boolean flag for if the character at S[index] is treated
 *                as a unicode character.
 * @return the index of the next character in S. If the current character
 *         is a certain unicode character, then `index + 2`, else `index + 1`.
 */
function advanceStringIndex(S, index, unicode) {
  // 1. Assert: Type(S) is String.
  // 2. Assert: index is an integer such that 0 ≤ index ≤ 253 - 1.
  // 3. Assert: Type(unicode) is Boolean.
  // 4. If unicode is false, return index + 1.
  if (!unicode) return index + 1;
  // 5. Let length be the number of code units in S.
  var length = S.length;
  // 6. If index + 1 ≥ length, return index + 1.
  if (index + 1 >= length) return index + 1;
  // 7. Let first be the numeric value of the code unit at index index within S.
  // Note: the first variable defined here is still a string and not a number,
  // but it's sufficient for comparison purposes;
  var first = HermesInternal.executeCall(_StringPrototypeCharCodeAt, S, index);
  // 8. If first < 0xD800 or first > 0xDBFF, return index + 1.
  if (first < 0xd800 || first > 0xdbff) return index + 1;
  // 9. Let second be the numeric value of the code unit at index index + 1
  //    within S.
  var second = HermesInternal.executeCall(
    _StringPrototypeCharCodeAt,
    S,
    index + 1,
  );
  // 10. If second < 0xDC00 or second > 0xDFFF, return index + 1.
  if (second < 0xdc00 || second > 0xdfff) return index + 1;
  // 11. Return index + 2.
  return index + 2;
}

/**
 * ES10.0 21.2.5.7
 * Search for matches of `this` regexp pattern in the given string.
 * @this a RegExp-like object.
 * @param string string to search in.
 * @return an array of matches, or null if there is none.
 */
RegExp.prototype[_SymbolMatch] = function(string) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let rx be the this value.
  var rx = this;
  // 2. If Type(rx) is not Object, throw a TypeError exception.
  if (typeof rx !== 'object')
    throw _TypeError(
      'RegExp.prototype[@@match] should be called on a js object',
    );
  // 3. Let S be ? ToString(string).
  var S = HermesInternal.toString(string);
  // 4. Let global be ToBoolean(? Get(rx, "global")).
  // 5. If global is false, then
  if (!rx.global) {
    // a. Return ? RegExpExec(rx, S).
    return HermesInternal.regExpExec(rx, S);
  }
  // 6. Else global is true,
  // a. Let fullUnicode be ToBoolean(? Get(rx, "unicode")).
  var fullUnicode = !!rx.unicode;
  // b. Perform ? Set(rx, "lastIndex", 0, true).
  rx.lastIndex = 0;
  // c. Let A be ! ArrayCreate(0).
  var A = [];
  // d. Let n be 0.
  var n = 0;
  // e. Repeat,
  while (true) {
    // i. Let result be ? RegExpExec(rx, S).
    var result = HermesInternal.regExpExec(rx, S);
    // ii. If result is null, then
    if (result === null) {
      // 1. If n = 0, return null.
      if (n === 0) return null;
      // 2. Return A.
      A.length = n;
      return A;
    }
    // iii. Else result is not null,
    // 1. Let matchStr be ? ToString(? Get(result, "0")).
    var matchStr = HermesInternal.toString(result[0]);
    // 2. Let status be CreateDataProperty(A, ! ToString(n), matchStr).
    // 3. Assert: status is true.
    // Note: JSArray::setElementAt (which HermesInternal.jsArraySetElementAt
    // calls) doesn't return exceptional status
    HermesInternal.jsArraySetElementAt(A, n, matchStr);
    // 4. If matchStr is the empty String, then
    if (!matchStr) {
      // a. Let thisIndex be ? ToLength(? Get(rx, "lastIndex")).
      // b. Let nextIndex be AdvanceStringIndex(S, thisIndex, fullUnicode).
      // c. Perform ? Set(rx, "lastIndex", nextIndex, true).
      rx.lastIndex = advanceStringIndex(S, rx.lastIndex, fullUnicode);
    }
    // 5. Increment n.
    n += 1;
  }
};

/**
 * ES10.0 21.2.5.10
 * Search for the first match of `this` regexp pattern in the given string.
 * Keeps this.lastIndex the same value as before running this function, and
 * ignore the global flag.
 * @this a RegExp-like object.
 * @param string string to search in.
 * @return the index (of string) of the first match, or -1 if there's no match.
 */
RegExp.prototype[_SymbolSearch] = function(string) {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let rx be the this value.
  var rx = this;
  // 2. If Type(rx) is not Object, throw a TypeError exception.
  if (typeof rx !== 'object')
    throw _TypeError(
      'RegExp.prototype[@@search] should be called on a js object',
    );
  // 3. Let S be ? ToString(string).
  var S = HermesInternal.toString(string);
  // 4. Let previousLastIndex be ? Get(rx, "lastIndex").
  var previousLastIndex = rx.lastIndex;
  // 5. If SameValue(previousLastIndex, 0) is false, then
  // Note: -0 === +0 returns true, while SameValue(-0, +0) returns false
  if (
    previousLastIndex !== 0 ||
    (previousLastIndex === 0 && 1 / previousLastIndex < 0)
  ) {
    // a. Perform ? Set(rx, "lastIndex", 0, true).
    rx.lastIndex = 0;
  }
  // 6. Let result be ? RegExpExec(rx, S).
  var result = HermesInternal.regExpExec(rx, S);
  // 7. Let currentLastIndex be ? Get(rx, "lastIndex").
  var currentLastIndex = rx.lastIndex;
  // 8. If SameValue(currentLastIndex, previousLastIndex) is false, then
  // Note: Only differences between === and SameValue are
  //   1) SameValue(NaN, NaN) returns true, and
  //   2) SameValue(+0, -0) returns false
  // The condition of this if statement is
  //   1) currentLastIndex and previousLastIndex are not equal and not NaN, or
  //   2) currentLastIndex and previousLastIndex are equal, but one is positive
  //      and the other is negative.
  if (
    (currentLastIndex !== previousLastIndex &&
      currentLastIndex === currentLastIndex) ||
    (currentLastIndex === previousLastIndex &&
      1 / currentLastIndex !== 1 / previousLastIndex)
  ) {
    // a. Perform ? Set(rx, "lastIndex", previousLastIndex, true).
    rx.lastIndex = previousLastIndex;
  }
  // 9. If result is null, return -1.
  if (result === null) return -1;
  // 10. Return ? Get(result, "index").
  return result.index;
};

/**
 * ES10.0 21.2.5.15
 * Stringify the RegExp object.
 * @this a RegExp-like object.
 * @return a string representation of the RegExp object.
 */
RegExp.prototype.toString = function toString() {
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let R be the this value.
  // 2. If Type(R) is not Object, throw a TypeError exception.
  if (typeof this !== 'object')
    throw _TypeError('RegExp.prototype.toString() called on non-object');
  // 3. Let pattern be ? ToString(? Get(R, "source")).
  var pattern = HermesInternal.toString(this.source);
  // 4. Let flags be ? ToString(? Get(R, "flags")).
  var flags = HermesInternal.toString(this.flags);
  // 5. Let result be the string-concatenation of "/", pattern, "/", and flags.
  // 6. Return result.
  return '/' + pattern + '/' + flags;
};

/**
 * ES10.0 21.2.5.9
 * Search for `this` regexp pattern in string and replace the first
 * occurrence (or all occurences if global flag is true) with replaceValue.
 * @this a RegExp-like object.
 * @param string string to search `this` pattern in.
 * @param replaceValue value to be replaced with. Can be both string or
 *                     function. If it's a function, the return value will
 *                     be used.
 * @return the resulting string after all replacements.
 */
RegExp.prototype[_SymbolReplace] = function(string, replaceValue) {
  // 1. Let rx be the this value.
  var rx = this;
  // 2. If Type(rx) is not Object, throw a TypeError exception.
  if (typeof rx !== 'object')
    throw _TypeError('RegExp.prototype[@@replace] called on a non-object.');
  // 3. Let S be ? ToString(string).
  var S = HermesInternal.toString(string);
  // 4. Let lengthS be the number of code unit elements in S.
  var lengthS = S.length;
  // 5. Let functionalReplace be IsCallable(replaceValue).
  var functionalReplace = typeof replaceValue === 'function';
  // 6. If functionalReplace is false, then
  if (!functionalReplace) {
    // a. Let replaceValue be ? ToString(replaceValue).
    replaceValue = HermesInternal.toString(replaceValue);
  }
  // 7. Let global be ToBoolean(? Get(rx, "global")).
  var global = !!rx.global;
  // 8. If global is true, then
  if (global) {
    // a. Let fullUnicode be ToBoolean(? Get(rx, "unicode")).
    var fullUnicode = !!rx.unicode;
    // b. Perform ? Set(rx, "lastIndex", 0, true).
    rx.lastIndex = 0;
  }
  // 9. Let results be a new empty List.
  var results = [];
  var resultsLen = 0;
  // 10. Let done be false.
  // 11. Repeat, while done is false
  while (true) {
    // a. Let result be ? RegExpExec(rx, S).
    var result = HermesInternal.regExpExec(rx, S);
    // b. If result is null, set done to true.
    if (result === null) {
      break;
    }

    // c. Else result is not null,
    // i. Append result to the end of results.
    HermesInternal.jsArraySetElementAt(results, resultsLen++, result);
    // ii. If global is false, set done to true.
    if (!global) {
      break;
    }

    // iii. Else,
    // 1. Let matchStr be ? ToString(? Get(result, "0")).
    var matchStr = HermesInternal.toString(result[0]);
    // 2. If matchStr is the empty String, then
    if (matchStr.length === 0) {
      // a. Let thisIndex be ? ToLength(? Get(rx, "lastIndex")).
      // b. Let nextIndex be AdvanceStringIndex(S, thisIndex, fullUnicode).
      // c.Perform ? Set(rx, "lastIndex", nextIndex, true).
      rx.lastIndex = advanceStringIndex(S, rx.lastIndex, fullUnicode);
    }
  }
  results.length = resultsLen;

  // 12. Let accumulatedResult be the empty String value.
  var accumulatedResult = '';
  // 13. Let nextSourcePosition be 0.
  var nextSourcePosition = 0;
  // 14. For each result in results, do
  for (var i = 0; i < resultsLen; i++) {
    var result = results[i];
    // a. Let nCaptures be ? ToLength(? Get(result, "length")).
    var nCaptures = HermesInternal.toLength(result.length);
    // b. Set nCaptures to max(nCaptures - 1, 0).
    if (nCaptures > 0) {
      nCaptures -= 1;
    }
    // c. Let matched be ? ToString(? Get(result, "0")).
    var matched = HermesInternal.toString(result[0]);
    // d. Let matchLength be the number of code units in matched.
    var matchLength = matched.length;
    // e. Let position be ? ToInteger(? Get(result, "index")).
    // f. Set position to max(min(position, lengthS), 0).
    var position = HermesInternal.toInteger(result.index);
    position = _MathMax(_MathMin(position, lengthS), 0);
    // g. Let n be 1.
    var n = 1;
    // h. Let captures be a new empty List.
    var captures = new _Array(nCaptures);
    // i. Repeat, while n ≤ nCaptures
    while (n <= nCaptures) {
      // i. Let capN be ? Get(result, ! ToString(n)).
      var capN = result[n];
      // ii. If capN is not undefined, then
      if (capN !== undefined) {
        // 1. Set capN to ? ToString(capN).
        capN = HermesInternal.toString(capN);
      }
      // iii. Append capN as the last element of captures.
      HermesInternal.jsArraySetElementAt(captures, n - 1, capN);
      // iv. Let n be n+1.
      n += 1;
    }
    // j. If functionalReplace is true, then
    if (functionalReplace) {
      // i. Let replacerArgs be « matched ».
      // ii. Append in list order the elements of captures to the end of
      //     the List replacerArgs.
      // iii. Append position and S to replacerArgs.
      var capturesLen = captures.length;
      var replacerArgs = new _Array(3 + capturesLen);
      HermesInternal.jsArraySetElementAt(replacerArgs, 0, matched);
      for (var j = 0; j < capturesLen; j++) {
        HermesInternal.jsArraySetElementAt(replacerArgs, j + 1, captures[j]);
      }
      HermesInternal.jsArraySetElementAt(
        replacerArgs,
        capturesLen + 1,
        position,
      );
      HermesInternal.jsArraySetElementAt(replacerArgs, capturesLen + 2, S);
      // iv. Let replValue be ? Call(replaceValue, undefined, replacerArgs).
      // v. Let replacement be ? ToString(replValue).
      var replValue = HermesInternal.executeCall(
        _FunctionPrototypeApply,
        replaceValue,
        undefined,
        replacerArgs,
      );
      var replacement = HermesInternal.toString(replValue);
      // k. Else,
    } else {
      // i. Let replacement be GetSubstitution(matched, S, position,
      //    captures, replaceValue).
      var replacement = HermesInternal.getSubstitution(
        matched,
        S,
        position,
        captures,
        replaceValue,
      );
    }
    // l. If position ≥ nextSourcePosition, then
    if (position >= nextSourcePosition) {
      // i. NOTE: position should not normally move backwards. If it does, it is
      //    an indication of an ill-behaving RegExp subclass or use of an access
      //    triggered side-effect to change the global flag or other
      //    characteristics of rx. In such cases, the corresponding substitution
      //    is ignored.
      // ii. Let accumulatedResult be the String formed by concatenating the
      //     code units of the current value of accumulatedResult with the
      //     substring of S consisting of the code units from nextSourcePosition
      //     (inclusive) up to position (exclusive) and with the code units of
      //     replacement.
      accumulatedResult +=
        HermesInternal.executeCall(
          _StringPrototypeSubstring,
          S,
          nextSourcePosition,
          position,
        ) + replacement;
      // iii. Let nextSourcePosition be position + matchLength.
      nextSourcePosition = position + matchLength;
    }
  }
  // 15. If nextSourcePosition ≥ lengthS, return accumulatedResult.
  if (nextSourcePosition >= lengthS) return accumulatedResult;
  // 16. Return the String formed by concatenating the code units of
  //     accumulatedResult with the substring of S consisting of the code units
  //     from nextSourcePosition (inclusive) up through the final code unit of
  //     S (inclusive).
  return (
    accumulatedResult +
    HermesInternal.executeCall(_StringPrototypeSubstring, S, nextSourcePosition)
  );
};

Object.defineProperty(RegExp.prototype, _SymbolMatch, builtinFuncDescriptor);
Object.defineProperty(RegExp.prototype, _SymbolSearch, builtinFuncDescriptor);
Object.defineProperty(RegExp.prototype, 'toString', builtinFuncDescriptor);
Object.defineProperty(RegExp.prototype, _SymbolReplace, builtinFuncDescriptor);

RegExp.prototype[_SymbolMatch].prototype = undefined;
RegExp.prototype[_SymbolSearch].prototype = undefined;
RegExp.prototype.toString.prototype = undefined;
RegExp.prototype[_SymbolReplace].prototype = undefined;

Object.defineProperty(RegExp.prototype[_SymbolMatch], 'name', {
  value: '[Symbol.match]',
});
Object.defineProperty(RegExp.prototype[_SymbolSearch], 'name', {
  value: '[Symbol.search]',
});
Object.defineProperty(RegExp.prototype[_SymbolReplace], 'name', {
  value: '[Symbol.replace]',
});
