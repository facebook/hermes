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

/**
 * Helper function for Array.prototype.every and Array.prototypr.some,
 * see their docuemntations for more details.
 * @this an array-like value.
 * @param callbackfn function to be invoked on the elements.
 * @param thisArg parameter which will be used as the <this> value
 *                for each invocation of callbackfn.
 * @return In case of every, return true if callbackfn returns true for
 *         all elements in the array, false otherwise.
 *         In case of some, return false if callbackfn returns false for
 *         all elements in the array, true otherwise.
 * @throw TypeError if callbackfn is not a function
 */
function arrayPrototypeEverySomeHelper(arr, callbackfn, thisArg, isEvery) {
  // Array.prototype.every and Array.prototype.some cannot be used as constructors
  if (new.target) throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(arr);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If IsCallable(callbackfn) is false, throw a TypeError exception.
  if (typeof callbackfn !== 'function') {
    if (isEvery) throw _TypeError('Array.prototype.every() requires a callable argument');
    throw _TypeError('Array.prototype.some() requires a callable argument');
  }

  // 4. If thisArg is present, let T be thisArg; else let T be undefined.

  // 5. Let k be 0.
  var k = 0;

  // fast path which calls the function directly when thisArg is undefined
  if (thisArg === undefined) {
    while (k < len) {
      if (k in O) {
        var testResult = callbackfn(O[k], k, O);
        if (isEvery && !testResult) return false;
        if (!isEvery && testResult) return true;
      }
      k += 1;
    }
    return isEvery ? true : false;
  }

  // 6. Repeat, while k < len
  while (k < len) {
    // a. Let Pk be ! ToString(k).
    // b. Let kPresent be ? HasProperty(O, Pk).
    // c. If kPresent is true, then
    if (k in O) {
      // i. Let kValue be ? Get(O, Pk).
      // ii. Let Let testResult be ToBoolean(? Call(callbackfn, T, « kValue, k, O »)).
      var testResult = !!HermesInternal.executeCall(callbackfn, thisArg, O[k], k, O);
      // iii. If testResult is false, return false. (every)
      //      If testResult is false, return false. (some)
      if (isEvery && !testResult) return false;
      if (!isEvery && testResult) return true;
    }
    // d. Increase k by 1.
    k += 1;
  }

  // 7. Return true. (every)
  //    Return false. (some)
  return isEvery;
}

/**
 * ES10.0 22.1.3.5
 * Invoke a callback function on elements in an array-like object until
 * a return value is falsy or there's no more elments.
 * @this an array-like value.
 * @param callbackfn function to be invoked on the elements.
 * @param [thisArg] optional parameter which will be used as the <this> value
 *                  for each invocation of callbackfn.
 * @return true if callbackfn returns true for all elements in the array,
 *         false otherwise.
 * @throw TypeError if callbackfn is not a function
 */
Array.prototype.every = function every(callbackfn, thisArg = undefined) {
  return arrayPrototypeEverySomeHelper(this, callbackfn, thisArg, true);
}

/**
 * ES10.0 22.1.3.26
 * Invoke a callback function on elements in an array-like object until
 * a return value is truthy or there's no more elments.
 * @this an array-like value.
 * @param callbackfn function to be invoked on the elements.
 * @param [thisArg] optional parameter which will be used as the <this> value
 *                  for each invocation of callbackfn.
 * @return false if callbackfn returns false for all elements in the array,
 *         true otherwise.
 * @throw TypeError if callbackfn is not a function
 */
Array.prototype.some = function some(callbackfn, thisArg = undefined) {
  return arrayPrototypeEverySomeHelper(this, callbackfn, thisArg, false);
}

/**
 * ES10.0 22.1.3.6
 * Modifies the array such that elements with indices in the range [start,end)
 * are replaced with the given value.
 * @this an array-like value.
 * @param value value to replace the elements with.
 * @param [start] starting index of the range to fill, defaults to 0
 * @param [end] index after the last index in the range to fill, defaults
 *              to the length of the array
 * @return the modified array
 */
Array.prototype.fill = function fill(value, start = undefined, end = undefined) {
  // Array.prototype.fill cannot be used as a constructor
  if (new.target) throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(this);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. Let relativeStart be ? ToInteger(start).
  var relativeStart = HermesInternal.toInteger(start);
  // 4. If relativeStart < 0, let k be max((len + relativeStart), 0); else let k be min(relativeStart, len).
  var k = relativeStart < 0 ? _MathMax(len + relativeStart, 0) : _MathMin(relativeStart, len);

  // 5. If end is undefined, let relativeEnd be len; else let relativeEnd be ? ToInteger(end).
  var relativeEnd = end === undefined ? len : HermesInternal.toInteger(end);
  // 6. If relativeEnd < 0, let final be max((len + relativeEnd), 0); else let final be min(relativeEnd, len).
  var final = relativeEnd < 0 ? _MathMax(len + relativeEnd, 0) : _MathMin(relativeEnd, len);

  // 7. Repeat, while k < final
  while (k < final) {
    // a. Let Pk be ! ToString(k).
    // b. Perform ? Set(O, Pk, value, true).
    O[k] = value;
    // c. Increase k by 1.
    k += 1;
  }

  // 8. Return O.
  return O;
}

/**
 * ES10.0 22.1.3.7
 * Invoke a callback function on all elements in an array-like object, and
 * constructs a new array for elements that returned truthy values.
 * @this an array-like value.
 * @param callbackfn function to be invoked on the elements.
 * @param [thisArg] optional parameter which will be used as the <this> value
 *                  for each invocation of callbackfn.
 * @return the new array constructed.
 * @throw TypeError if callbackfn is not a function
 */
Array.prototype.filter = function filter(callbackfn, thisArg = undefined) {
  // Array.prototype.filter cannot be used as a constructor
  if (new.target) throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(this);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If IsCallable(callbackfn) is false, throw a TypeError exception.
  if (typeof callbackfn !== 'function') {
    throw _TypeError('Array.prototype.filter() requires a callable argument');
  }

  // 4. If thisArg is present, let T be thisArg; else let T be undefined.

  // 5. Let A be ? ArraySpeciesCreate(O, len).
  // Note: we do not support species constructors right now, just
  // constructing a Array with default Array constrcutor.
  // Allocating extra space here so we won't need to repeatedly
  // resize as we add elements.
  var A = new _Array(len);

  // 6. Let k be 0.
  var k = 0;
  // 7. Let to be 0.
  var to = 0;

  // fast path which calls the function directly when thisArg is undefined
  if (thisArg === undefined) {
    while (k < len) {
      if (k in O) {
        var kValue = O[k];
        if (callbackfn(kValue, k, O)) {
          HermesInternal.jsArraySetElementAt(A, to, kValue);
          to += 1;
        }
      }
      k += 1;
    }
  } else {
    // 8. Repeat, while k < len
    while (k < len) {
      // a. Let Pk be ! ToString(k).
      // b. Let kPresent be ? HasProperty(O, Pk).
      // c. If kPresent is true, then
      if (k in O) {
        // i. Let kValue be ? Get(O, Pk).
        var kValue = O[k];
        // ii. Let selected be ToBoolean(? Call(callbackfn, T, « kValue, k, O »)).
        // iii. If selected is true, then
        if (HermesInternal.executeCall(callbackfn, thisArg, kValue, k, O)) {
          // 1. Perform ? CreateDataPropertyOrThrow(A, ! ToString(to), kValue).
          HermesInternal.jsArraySetElementAt(A, to, kValue);
          // 2. Increase to by 1.
          to += 1;
        }
      }
      // d. Increase k by 1.
      k += 1;
    }
  }

  A.length = to;
  // 9. Return A.
  return A;
}

/**
 * Helper function for Array.prototype.find and Array.prototypr.findIndex,
 * see their doc-comments for more details.
 * @param arr an array-like value.
 * @param predicate function to be invoked on the elements.
 * @param thisArg parameter which will be used as the <this> value
 *                for each invocation of predicate.
 * @param returnElement boolean flag for whether to return the element found
 *                      or index of the element
 * @return if returnElement is true, returns the first element which makes
 *         the predicate return a truthy value, and undefined if no such
 *         element exists.
 *         If returnElement is false, return index of the first element which
 *         makes the predicate return a truthy value, and -1 if no such
 *         element exists.
 * @throw TypeError if predicate is not a function
 */
function arrayPrototypeFindHelper(arr, predicate, thisArg, returnElement) {
  // Array.prototype.find and Array.prototype.findIndex cannot be used as constructors
  if (new.target) throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(arr);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If IsCallable(predicate) is false, throw a TypeError exception.
  if (typeof predicate !== 'function') {
    throw _TypeError('Find argument must be a function');
  }

  // 4. If thisArg is present, let T be thisArg; else let T be undefined.

  // 5. Let k be 0.
  var k = 0;

  // fast path which calls the function directly when thisArg is undefined
  if (thisArg === undefined) {
    while (k < len) {
      var kValue = O[k];
      if (predicate(kValue, k, O)) return returnElement ? kValue : k;
      k += 1;
    }
    return returnElement ? undefined : -1;
  }

  // 6. Repeat, while k < len
  while (k < len) {
    // a. Let Pk be ! ToString(k).
    // b. Let kValue be ? Get(O, Pk).
    var kValue = O[k];
    // c. Let testResult be ToBoolean(? Call(predicate, T, « kValue, k, O »)).
    // d. If testResult is true, return kValue. (Array.prototype.find)
    // d. If testResult is true, return k. (Array.prototype.findIndex)
    if (HermesInternal.executeCall(predicate, thisArg, kValue, k, O)) {
      return returnElement ? kValue : k;
    }
    // e. Increase k by 1.
    k += 1;
  }

  // 7. Return undefined. (Array.prototype.find)
  // 7. Return -1. (Array.prototype.findIndex)
  return returnElement ? undefined : -1;
}

/**
 * ES10.0 22.1.3.8
 * Invoke a predicate function on elements in an array-like object until
 * a return value is truthy or there's no more elments.
 * @this an array-like value.
 * @param predicate function to be invoked on the elements.
 * @param [thisArg] optional parameter which will be used as the <this> value
 *                  for each invocation of predicate.
 * @return the first element which makes the predicate return a truthy value,
 *         and undefined if no such element exists
 */
Array.prototype.find = function find(predicate, thisArg = undefined) {
  return arrayPrototypeFindHelper(this, predicate, thisArg, true);
}

/**
 * ES10.0 22.1.3.9
 * Invoke a predicate function on elements in an array-like object until
 * a return value is truthy or there's no more elments.
 * @this an array-like value.
 * @param predicate function to be invoked on the elements.
 * @param [thisArg] optional parameter which will be used as the <this> value
 *                  for each invocation of predicate.
 * @return index of the first element which makes the predicate return a
 *         truthy value, and -1 if no such element exists
 */
Array.prototype.findIndex = function findIndex(predicate, thisArg = undefined) {
  return arrayPrototypeFindHelper(this, predicate, thisArg, false);
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
Object.defineProperty(Array.prototype, 'every', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'some', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'fill', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'filter', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'find', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'findIndex', arrayFuncDescriptor);

Array.prototype.reverse.prototype = undefined;
Array.prototype.map.prototype = undefined;
Array.prototype.indexOf.prototype = undefined;
Array.prototype.lastIndexOf.prototype = undefined;
Array.prototype.every.prototype = undefined;
Array.prototype.some.prototype = undefined;
Array.prototype.fill.prototype = undefined;
Array.prototype.filter.prototype = undefined;
Array.prototype.find.prototype = undefined;
Array.prototype.findIndex.prototype = undefined;
