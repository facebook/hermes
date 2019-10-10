/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */

/**
 * ES10.0 22.1.3.23
 * Reverse the order of all elements in an array-like object.
 * @this an array-like value.
 * @return the object version of this value.
 */
Array.prototype.reverse = function reverse() {
  // Array.prototype.reverse cannot be used as a constructor
  if (new.target) throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(this);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);
  // 3. Let middle be floor(len / 2).
  var middle = _MathFloor(len / 2);
  // 4. Let lower be 0.
  var lower = 0;

  var lowerValue, upperValue;

  // 5. Repeat, while lower ≠ middle
  while (lower !== middle) {
    // a. Let upper be len - lower - 1.
    var upper = len - lower - 1;

    // Note: steps b and c are about converting lower and upper to string,
    // which isn't necessary in our case since we can guarantee that lower
    // and upper are integer values

    // d. Let lowerExists be ? HasProperty(O, lowerP).
    var lowerExists = lower in O;
    // e. If lowerExists is true, then Let lowerValue be ? Get(O, lowerP).
    if (lowerExists) lowerValue = O[lower];

    // f. Let upperExists be ? HasProperty(O, upperP).
    var upperExists = upper in O;
    // g. If upperExists is true, then Let upperValue be ? Get(O, upperP).
    if (upperExists) upperValue = O[upper];

    // h. If lowerExists is true and upperExists is true, then
    if (lowerExists && upperExists) {
      // i. Perform ? Set(O, lowerP, upperValue, true).
      O[lower] = upperValue;
      // ii. Perform ? Set(O, upperP, lowerValue, true).
      O[upper] = lowerValue;
    // i. Else if lowerExists is false and upperExists is true, then
    } else if (upperExists) {
      // i. Perform ? Set(O, lowerP, upperValue, true).
      O[lower] = upperValue;
      // ii. Perform ? DeletePropertyOrThrow(O, upperP).
      delete O[upper];
    // j. Else if lowerExists is true and upperExists is false, then
    } else if (lowerExists) {
      // i. Perform ? DeletePropertyOrThrow(O, lowerP).
      delete O[lower];
      // ii. Perform ? Set(O, upperP, lowerValue, true).
      O[upper] = lowerValue;
    }

    // k. Else both lowerExists and upperExists are false, No action is required.

    // l. Increase lower by 1.
    lower += 1;
  }
  // 6. Return O.
  return O;
}

/**
 * ES10.0 22.1.3.18
 * Invoke a callback function on all elements in an array-like object, and
 * constructs a new array from the return values.
 * @this an array-like value.
 * @param callbackfn function to be invoked on the elements.
 * @param [thisArg] optional parameter which will be used as the <this> value
 *                  for each invocation of callbackfn.
 * @return the new array constructed.
 * @throw TypeError if callbackfn is not a function
 */
Array.prototype.map = function map(callbackfn, thisArg = undefined) {
  // Array.prototype.map cannot be used as a constructor
  if (new.target) throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(this);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If IsCallable(callbackfn) is false, throw a TypeError exception.
  if (typeof callbackfn !== 'function') {
    throw _TypeError('Array.prototype.map() requires a callable argument');
  }

  // 4. If thisArg is present, let T be thisArg; else let T be undefined.

  // 5. Let A be ? ArraySpeciesCreate(O, len).
  // Note: we do not support species constructors right now, just
  // constructing a Array with default Array constrcutor
  var A = new _Array(len);

  // 6. Let k be 0.
  var k = 0;

  // Flag for invoking callbackfn directly when thisArg is undefined,
  // which is faster than HermesInternal.executeCall
  var invokeDirectly = thisArg === undefined;

  // 7. Repeat, while k < len
  while (k < len) {
    // a. Let Pk be ! ToString(k).
    // b. Let kPresent be ? HasProperty(O, Pk).
    // c. If kPresent is true, then
    if (k in O) {
      // i. Let kValue be ? Get(O, Pk).
      // ii. Let mappedValue be ? Call(callbackfn, T, « kValue, k, O »).
      var mappedValue = invokeDirectly
        ? callbackfn(O[k], k, O)
        : HermesInternal.executeCall(callbackfn, thisArg, O[k], k, O);
      // iii. Perform ? CreateDataPropertyOrThrow(A, Pk, mappedValue).
      HermesInternal.jsArraySetElementAt(A, k, mappedValue);
    }
    // d. Increase k by 1.
    k += 1;
  }

  // 8. Return A.
  return A;
}

/**
 * Helper function for Array.prototype.indexOf and Array.prototypr.lastIndexOf,
 * see their docuemntations for more details.
 * @param arr an array-like value.
 * @param searchElement function to be invoked on the elements.
 * @param fromIndex index of the array to start the search from.
 * @param fromIndexPresent boolean flag for whether the fromIndex was present
 *                         in user's original call to indexOf or lastIndexOf.
 * @param reverse boolean flag for whether to search in descending order
 *                instead of ascending order
 * @return the first index at which the searchElement can be found, if
 *         no such index exists, return -1
 */
function arrayPrototypeIndexOfHelper(arr, searchElement, fromIndex, fromIndexPresent, reverse) {
  // Array.prototype.indexOf and Array.prototypr.lastIndexOf cannot be used as constructors
  if (new.target) throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(arr);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If len is 0, return -1.
  if (len === 0) return -1;

  // 4. Let n be ? ToInteger(fromIndex) (indexOf)
  //    If fromIndex is present, let n be ? ToInteger(fromIndex); else let n be len - 1. (lastIndexOf)
  var n;
  if (reverse && !fromIndexPresent) {
    n = len - 1;
  } else {
    // Note: toInteger(undefined) returns 0
    n = HermesInternal.toInteger(fromIndex);
  }

  // If n ≥ len, return -1. (indexOf)
  if (!reverse && n >= len) return -1;

  var k;
  // 5. If n ≥ 0, then
  if (n >= 0) {
    // a. If n is -0, let k be +0; else let k be n. (indexOf)
    //    If n is -0, let k be +0; else let k be min(n, len - 1). (lastIndexOf)
    if (n === 0) {
      k = 0;
    } else if (reverse) {
      k = _MathMin(n, len - 1);
    } else {
      k = n;
    }
  // 6. Else n < 0,
  } else {
    // a. Let k be len + n.
    k = len + n;
    // b. If k < 0, set k to 0. (indexOf)
    if (k < 0) {
      // Early return, this is not in the specs but is needed
      // for the boundary check below to work
      if (reverse) return -1;
      k = 0;
    }
  }

  var direction = reverse ? -1 : 1;
  var boundary = reverse ? -1 : len;

  // 7. Repeat, while k < len (indexOf)
  //    Repeat, while k ≥ 0 (lastIndexOf)
  while (k !== boundary) {
    // a. Let kPresent be ? HasProperty(O, ! ToString(k)).
    // b. If kPresent is true, then
      // i. Let elementK be ? Get(O, ! ToString(k)).
      // ii. Let same be the result of performing Strict Equality Comparison searchElement === elementK.
      // iii. If same is true, return k.
    if (k in O && O[k] === searchElement) return k;
    // c. Increase k by 1. (indexOf)
    //    Decrease k by 1. (lastIndexOf)
    k += direction;
  }
  // 8. Return -1.
  return -1;
}

/**
 * ES10.0 22.1.3.14
 * Tries to find searchElement in the elements of the array, in ascending order,
 * using the Strict Equality Comparison algorithm, and if found at one or
 * more indices, returns the smallest such index; otherwise, -1 is returned.
 * @this an array-like value.
 * @param searchElement element to search for.
 * @param [fromIndex] index of the array to start the search from.
 * @return the smallest index i such that Object(this)[i] === searchIndex,
 *         if no such index exists, return -1.
 */
Array.prototype.indexOf = function indexOf(searchElement, fromIndex = undefined) {
  return arrayPrototypeIndexOfHelper(this, searchElement, fromIndex, arguments.length > 1, false);
}

/**
 * ES10.0 22.1.3.17
 * Tries to find searchElement in the elements of the array, in descending order,
 * using the Strict Equality Comparison algorithm, and if found at one or
 * more indices, returns the largest such index; otherwise, -1 is returned.
 * @this an array-like value.
 * @param searchElement element to search for.
 * @param [fromIndex] index of the array to start the search from.
 * @return the largest index i such that Object(this)[i] === searchIndex,
 *         if no such index exists, return -1.
 */
Array.prototype.lastIndexOf = function lastIndexOf(searchElement, fromIndex = undefined) {
  return arrayPrototypeIndexOfHelper(this, searchElement, fromIndex, arguments.length > 1, true);
}

var arrayFuncDescriptor = {
  configurable: true,
  enumerable: false,
  writable: true,
};

Object.defineProperty(Array.prototype, 'reverse', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'map', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'indexOf', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'lastIndexOf', arrayFuncDescriptor);

Array.prototype.reverse.prototype = undefined;
Array.prototype.map.prototype = undefined;
Array.prototype.indexOf.prototype = undefined;
Array.prototype.lastIndexOf.prototype = undefined;
