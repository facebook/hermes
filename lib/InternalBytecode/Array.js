/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

/**
 * ES10.0 22.1.2.1
 * Construct an object using the given constructor, and fill it with the
 * values from items. User can optional pass in a mapfn function which
 * is invoked on the values of items and return values are stored in the
 * new object.
 * @this a constructor value. If it's not a valid constructor, the default
 *       Array constructor will be used.
 * @param items values to construct the array from. If it is iterable,
 *              its iterator will be used to extract he values, else it'll
 *              iterated like an array (ie. indexed from 0 to len).
 * @param [mapfn] function to be invoked on the elements of items.
 * @param [thisArg] value to use as the <this> value for each invocation
 *                  of mapfn.
 * @return the new object constructed.
 * @throw TypeError if mapfn is defined and not a function.
 *        TypeError if there are too many values in items to fit into an array.
 */
Array.from = function from(items, mapfn = undefined, thisArg = undefined) {
  // Array.prototype.from cannot be used as a constructor
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let C be the this value.
  var C = this;
  // 2. If mapfn is undefined, let mapping be false.
  var mapping = mapfn !== undefined;
  // 3. Else,
  if (mapping) {
    // a. If IsCallable(mapfn) is false, throw a TypeError exception.
    if (typeof mapfn !== 'function') {
      throw _TypeError('Mapping function is not callable.');
    }
    // b. If thisArg is present, let T be thisArg; else let T be undefined.
    // c. Let mapping be true.
  }

  var mappedValue;
  var propertyDescriptor = {
    writable: true,
    enumerable: true,
    configurable: true,
  };

  // Flag for invoking callbackfn directly when thisArg is undefined,
  // which is faster than HermesInternal.executeCall
  var invokeDirectly = thisArg === undefined;

  // 4. Let usingIterator be ? GetMethod(items, @@iterator).
  // 5. If usingIterator is not undefined, then
  if (getMethod(items, _SymbolIterator) !== undefined) {
    // a. If IsConstructor(C) is true, then
    // i. Let A be ? Construct(C).
    // b. Else,
    // i. Let A be ! ArrayCreate(0).
    var A = HermesInternal.isConstructor(C) ? new C() : [];

    // Note: the for ... of loop semantic takes care of any calls to
    // GetIterator, IteratorStep, and IteratorClose.

    // c. Let iteratorRecord be ? GetIterator(items, sync, usingIterator).
    // d. Let k be 0.
    var k = 0;

    var isArrayA = _ArrayIsArray(A);
    // e. Repeat,
    for (var nextValue of items) {
      // i. If k ≥ 2 ^ 53 - 1, then
      if (k >= 0x1ffffffffffff) {
        // 1. Let error be ThrowCompletion(a newly created TypeError object).
        // 2. Return ? IteratorClose(iteratorRecord, error).
        throw _TypeError('Array.from result out of space');
      }
      // ii. Let Pk be ! ToString(k).

      // Note: if next is false, loop will automatically terminate,
      //       so step iv is implemented outside of the loop
      // iii. Let next be ? IteratorStep(iteratorRecord).
      // iv. If next is false, then
      // 1. Perform ? Set(A, "length", k, true).
      // 2. Return A.

      // v. Let nextValue be ? IteratorValue(next).

      // vi. If mapping is true, then
      if (mapping) {
        // 1. Let mappedValue be Call(mapfn, T, « nextValue, k »).
        mappedValue = invokeDirectly
          ? mapfn(nextValue, k)
          : HermesInternal.executeCall(mapfn, thisArg, nextValue, k);
        // 2. If mappedValue is an abrupt completion, return ? IteratorClose(iteratorRecord, mappedValue).
        // 3. Set mappedValue to mappedValue.[[Value]].
        // vii. Else, let mappedValue be nextValue.
      } else {
        mappedValue = nextValue;
      }
      // viii. Let defineStatus be CreateDataPropertyOrThrow(A, Pk, mappedValue).
      if (isArrayA) {
        HermesInternal.jsArraySetElementAt(A, k, mappedValue);
      } else {
        propertyDescriptor.value = mappedValue;
        _ObjectDefineProperty(A, k, propertyDescriptor);
      }
      // ix. If defineStatus is an abrupt completion, return ? IteratorClose(iteratorRecord, defineStatus).
      // x. Increase k by 1.
      k += 1;
    }

    A.length = k;
    return A;
  }

  // 6. NOTE: items is not an Iterable so assume it is an array-like object.
  // 7. Let arrayLike be ! ToObject(items).
  var arrayLike = HermesInternal.toObject(items);
  // 8. Let len be ? ToLength(? Get(arrayLike, "length")).
  var len = HermesInternal.toLength(arrayLike.length);
  // 9. If IsConstructor(C) is true, then
  // a. Let A be ? Construct(C, « len »).
  // 10. Else,
  // a. Let A be ? ArrayCreate(len).
  var A = HermesInternal.isConstructor(C) ? new C(len) : new _Array(len);

  var isArrayA = _ArrayIsArray(A);

  // 11. Let k be 0.
  var k = 0;
  // 12. Repeat, while k < len
  while (k < len) {
    // a. Let Pk be ! ToString(k).
    // b. Let kValue be ? Get(arrayLike, Pk).
    var kValue = arrayLike[k];
    // c. If mapping is true, then
    // i. Let mappedValue be ? Call(mapfn, T, « kValue, k »).
    // d. Else, let mappedValue be kValue.
    mappedValue = mapping
      ? invokeDirectly
        ? mapfn(kValue, k)
        : HermesInternal.executeCall(mapfn, thisArg, kValue, k)
      : kValue;
    // e. Perform ? CreateDataPropertyOrThrow(A, Pk, mappedValue).
    if (isArrayA) {
      HermesInternal.jsArraySetElementAt(A, k, mappedValue);
    } else {
      propertyDescriptor.value = mappedValue;
      _ObjectDefineProperty(A, k, propertyDescriptor);
    }
    // f. Increase k by 1.
    k += 1;
  }
  // 13. Perform ? Set(A, "length", len, true).
  A.length = len;
  // 14. Return A.
  return A;
};

/**
 * ES10.0 22.1.3.23
 * Reverse the order of all elements in an array-like object.
 * @this an array-like value.
 * @return the object version of this value.
 */
Array.prototype.reverse = function reverse() {
  // Array.prototype.reverse cannot be used as a constructor
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

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
};

/**
 * ES10.0 22.1.3.18
 * Invoke a callback function on all elements in an array-like object, and
 * constructs a new array from the return values.
 * @this an array-like value.
 * @param callbackfn function to be invoked on the elements.
 * @param [thisArg] optional parameter which will be used as the <this> value
 *                  for each invocation of callbackfn.
 * @return the new array constructed.
 */
Array.prototype.map = function map(callbackfn, thisArg = undefined) {
  // Array.prototype.map cannot be used as a constructor
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

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
};

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
function arrayPrototypeIndexOfHelper(
  arr,
  searchElement,
  fromIndex,
  fromIndexPresent,
  reverse,
) {
  // Array.prototype.indexOf and Array.prototypr.lastIndexOf cannot be used as
  // constructors
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(arr);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If len is 0, return -1.
  if (len === 0) return -1;

  // 4. Let n be ? ToInteger(fromIndex) (indexOf)
  //    If fromIndex is present, let n be ? ToInteger(fromIndex);
  //    else let n be len - 1. (lastIndexOf)
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
    // ii. Let same be the result of performing Strict Equality Comparison
    //     searchElement === elementK.
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
Array.prototype.indexOf = function indexOf(
  searchElement,
  fromIndex = undefined,
) {
  return arrayPrototypeIndexOfHelper(
    this,
    searchElement,
    fromIndex,
    arguments.length > 1,
    false,
  );
};

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
Array.prototype.lastIndexOf = function lastIndexOf(
  searchElement,
  fromIndex = undefined,
) {
  return arrayPrototypeIndexOfHelper(
    this,
    searchElement,
    fromIndex,
    arguments.length > 1,
    true,
  );
};

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
 */
function arrayPrototypeEverySomeHelper(arr, callbackfn, thisArg, isEvery) {
  // Array.prototype.every and Array.prototype.some cannot be used as constructors
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(arr);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If IsCallable(callbackfn) is false, throw a TypeError exception.
  if (typeof callbackfn !== 'function') {
    if (isEvery)
      throw _TypeError('Array.prototype.every() requires a callable argument');
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
      var testResult = !!HermesInternal.executeCall(
        callbackfn,
        thisArg,
        O[k],
        k,
        O,
      );
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
 */
Array.prototype.every = function every(callbackfn, thisArg = undefined) {
  return arrayPrototypeEverySomeHelper(this, callbackfn, thisArg, true);
};

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
 */
Array.prototype.some = function some(callbackfn, thisArg = undefined) {
  return arrayPrototypeEverySomeHelper(this, callbackfn, thisArg, false);
};

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
Array.prototype.fill = function fill(
  value,
  start = undefined,
  end = undefined,
) {
  // Array.prototype.fill cannot be used as a constructor
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(this);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. Let relativeStart be ? ToInteger(start).
  var relativeStart = HermesInternal.toInteger(start);
  // 4. If relativeStart < 0, let k be max((len + relativeStart), 0);
  //    else let k be min(relativeStart, len).
  var k =
    relativeStart < 0
      ? _MathMax(len + relativeStart, 0)
      : _MathMin(relativeStart, len);

  // 5. If end is undefined, let relativeEnd be len;
  //    else let relativeEnd be ? ToInteger(end).
  var relativeEnd = end === undefined ? len : HermesInternal.toInteger(end);
  // 6. If relativeEnd < 0, let final be max((len + relativeEnd), 0);
  //    else let final be min(relativeEnd, len).
  var final =
    relativeEnd < 0
      ? _MathMax(len + relativeEnd, 0)
      : _MathMin(relativeEnd, len);

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
};

/**
 * ES10.0 22.1.3.7
 * Invoke a callback function on all elements in an array-like object, and
 * constructs a new array for elements that returned truthy values.
 * @this an array-like value.
 * @param callbackfn function to be invoked on the elements.
 * @param [thisArg] optional parameter which will be used as the <this> value
 *                  for each invocation of callbackfn.
 * @return the new array constructed.
 */
Array.prototype.filter = function filter(callbackfn, thisArg = undefined) {
  // Array.prototype.filter cannot be used as a constructor
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

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
};

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
 */
function arrayPrototypeFindHelper(arr, predicate, thisArg, returnElement) {
  // Array.prototype.find and Array.prototype.findIndex cannot be used as
  // constructors
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

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
};

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
};

/**
 * ES10.0 22.1.3.19
 * Remove and return the last element of the array
 * @this an array-like value.
 * @return the removed element
 */
Array.prototype.pop = function pop() {
  // Array.prototype.pop cannot be used as a constructor
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(this);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If len is zero, then
  if (len === 0) {
    // a. Perform ? Set(O, "length", 0, true).
    O.length = 0;
    // b. Return undefined.
    return undefined;
  }

  // 4. Else len > 0,
  // a. Let newLen be len - 1.
  var newLen = len - 1;
  // b. Let index be ! ToString(newLen).
  // c. Let element be ? Get(O, index).
  var element = O[newLen];
  // d. Perform ? DeletePropertyOrThrow(O, index).
  delete O[newLen];
  // e. Perform ? Set(O, "length", newLen, true).
  O.length = newLen;
  // f. Return element.
  return element;
};

/**
 * Helper function for Array.prototype.reduce and Array.prototypr.reduceRight,
 * see their docuemntations for more details.
 * @param arr an array-like value.
 * @param callbackfn function to be invoked on the elements.
 * @param initialValue initial value of the aggregated result.
 * @param initialValuePresent boolean flag for whether initialValue was
 *                            present in the original call.
 * @param fromLeft boolean flag for whether the elements should be
 *                 from left to right or right to left.
 * @return an aggregated result after invoking callbackfn on all elements.
 */
function arrayPrototypeReduceHelper(
  arr,
  callbackfn,
  initialValue,
  initialValuePresent,
  fromLeft,
) {
  // Array.prototype.reduce and Array.prototype.reduceRight cannot be used as
  // constructors
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(arr);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If IsCallable(callbackfn) is false, throw a TypeError exception.
  if (typeof callbackfn !== 'function') {
    throw _TypeError('Array.prototype.reduce() requires a callable argument');
  }

  // 4. If len is 0 and initialValue is not present, throw a TypeError exception.
  if (len === 0 && !initialValuePresent) {
    throw _TypeError(
      'Array.prototype.reduce() requires an intial value with empty array',
    );
  }

  // 5. Let k be 0. (reduce)
  //    Let k be len - 1. (reduceRight)
  var k = fromLeft ? 0 : len - 1;
  var boundary = fromLeft ? len : -1;
  var direction = fromLeft ? 1 : -1;

  // 6. Let accumulator be undefined.
  var accumulator = undefined;

  // 7. If initialValue is present, then
  if (initialValuePresent) {
    // a. Set accumulator to initialValue.
    accumulator = initialValue;
    // 8. Else initialValue is not present,
  } else {
    // a. Let kPresent be false.
    var kPresent = false;
    // b. Repeat, while kPresent is false and k < len (reduce)
    //    Repeat, while kPresent is false and k ≥ 0 (reduceRight)
    while (!kPresent && k != boundary) {
      // i. Let Pk be ! ToString(k).
      // ii. Set kPresent to ? HasProperty(O, Pk).
      kPresent = k in O;
      // iii. If kPresent is true, then
      // 1. Set accumulator to ? Get(O, Pk).
      if (kPresent) {
        accumulator = O[k];
      }
      // iv. Increase k by 1. (reduce)
      //     Decrease k by 1. (reduceRight)
      k += direction;
    }
    // c. If kPresent is false, throw a TypeError exception.
    if (!kPresent) {
      throw _TypeError(
        'Array.prototype.reduce() requires an intial value with empty array',
      );
    }
  }

  // 9. Repeat, while k < len (reduce)
  //    Repeat, while k ≥ 0 (reduceRight)
  while (k != boundary) {
    // a. Let Pk be ! ToString(k).
    // b. Let kPresent be ? HasProperty(O, Pk).
    // c. If kPresent is true, then
    if (k in O) {
      // i. Let kValue be ? Get(O, Pk).
      // ii. Set accumulator to
      //     ? Call(callbackfn, undefined, « accumulator, kValue, k, O »).
      accumulator = callbackfn(accumulator, O[k], k, O);
    }
    // d. Increase k by 1. (reduce)
    //    Decrease k by 1. (reduceRight)
    k += direction;
  }

  // 10. Return accumulator.
  return accumulator;
}

/**
 * ES10.0 22.1.3.21
 * Invoke a reducer function (callbackfn) on every element in the array, in
 * ascending order. The arguments to the reducer function are: aggregated
 * result from previous calls, current element, current index, and given array.
 * @this an array-like value.
 * @param callbackfn function to be invoked on the elements
 * @param [initialValue] initial value of the aggregated result
 * @return an aggregated result after invoking callbackfn on all elements
 */
Array.prototype.reduce = function reduce(callbackfn, initialValue = undefined) {
  return arrayPrototypeReduceHelper(
    this,
    callbackfn,
    initialValue,
    arguments.length >= 2,
    true,
  );
};

/**
 * ES10.0 22.1.3.22
 * Invoke a reducer function (callbackfn) on every element in the array, in
 * descending order. The arguments to the reducer function are: aggregated
 * result from previous calls, current element, current index, and given array.
 * @this an array-like value.
 * @param callbackfn function to be invoked on the elements
 * @param [initialValue] initial value of the aggregated result
 * @return an aggregated result after invoking callbackfn on all elements
 */
Array.prototype.reduceRight = function reduceRight(
  callbackfn,
  initialValue = undefined,
) {
  return arrayPrototypeReduceHelper(
    this,
    callbackfn,
    initialValue,
    arguments.length >= 2,
    false,
  );
};

/**
 * ES10.0 22.1.3.24
 * Remove and return the first element of the array
 * @this an array-like value.
 * @return the removed element
 */
Array.prototype.shift = function shift() {
  // Array.prototype.shift cannot be used as a constructor
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(this);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If len is zero, then
  if (len === 0) {
    // a. Perform ? Set(O, "length", 0, true).
    O.length = 0;
    // b. Return undefined.
    return undefined;
  }

  // 4. Let first be ? Get(O, "0").
  var first = O[0];
  // 5. Let k be 1.
  var k = 1;

  // 6. Repeat, while k < len
  while (k < len) {
    // a. Let from be ! ToString(k).
    // b. Let to be ! ToString(k - 1).
    var to = k - 1;
    // c. Let fromPresent be ? HasProperty(O, from).
    // d. If fromPresent is true, then
    if (k in O) {
      // i. Let fromVal be ? Get(O, from).
      // ii. Perform ? Set(O, to, fromVal, true).
      O[to] = O[k];
      // e. Else fromPresent is false,
    } else {
      // i. Perform ? DeletePropertyOrThrow(O, to).
      delete O[to];
    }
    // f. Increase k by 1.
    k += 1;
  }
  // 7. Perform ? DeletePropertyOrThrow(O, ! ToString(len - 1)).
  delete O[len - 1];
  // 8. Perform ? Set(O, "length", len - 1, true).
  O.length = len - 1;
  // 9. Return first.
  return first;
};

/**
 * ES10.0 22.1.3.31
 * Append all arguments to the front of the array
 * @this an array-like value.
 * @return new length of the array
 */
Array.prototype.unshift = function unshift() {
  // Array.prototype.unshift cannot be used as a constructor
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(this);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. Let argCount be the number of actual arguments.
  var argCount = arguments.length;

  // 4. If argCount > 0, then
  if (argCount > 0) {
    // a. If len + argCount > 2 ^ 53 - 1, throw a TypeError exception.
    if (len + argCount > 0x1ffffffffffff)
      throw _TypeError('Array.prototype.unshift result out of space');
    // b. Let k be len.
    var k = len;
    // c. Repeat, while k > 0,
    while (k > 0) {
      // i. Let from be ! ToString(k - 1).
      var from = k - 1;
      // ii. Let to be ! ToString(k + argCount - 1).
      var to = k + argCount - 1;
      // iii. Let fromPresent be ? HasProperty(O, from).
      // iv. If fromPresent is true, then
      if (from in O) {
        // 1. Let fromValue be ? Get(O, from).
        // 2. Perform ? Set(O, to, fromValue, true).
        O[to] = O[from];
        // v. Else fromPresent is false,
      } else {
        // 1. Perform ? DeletePropertyOrThrow(O, to).
        delete O[to];
      }

      // vi. Decrease k by 1.
      k -= 1;
    }
    // d. Let j be 0.
    var j = 0;
    // e. Let items be a List whose elements are, in left to right order,
    //    the arguments that were passed to this function invocation.
    // f. Repeat, while items is not empty
    while (j < argCount) {
      // i. Remove the first element from items and let E be the value of
      //    that element.
      // ii. Perform ? Set(O, ! ToString(j), E, true).
      O[j] = arguments[j];
      // iii. Increase j by 1.
      j += 1;
    }
  }
  // 5. Perform ? Set(O, "length", len + argCount, true).
  O.length = len + argCount;
  // 6. Return len + argCount.
  return len + argCount;
};

Object.defineProperty(Array.prototype.unshift, 'length', {
  value: 1,
  configurable: true,
  enumerable: false,
  writable: false,
});

/**
 * ES10.0 22.1.3.13
 * Tries to find searchElement in the array, in ascending order,
 * using the SameValueZero algorithm.
 * @this an array-like value.
 * @param searchElement function to be invoked on the elements.
 * @param fromIndex index of the array to start the search from.
 * @return true if searchElement is found in the array, false otherwise.
 */
Array.prototype.includes = function includes(
  searchElement,
  fromIndex = undefined,
) {
  // Array.prototype.includes cannot be used as a constructor
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(this);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. If len is 0, return false.
  if (len === 0) return false;

  // 4. Let n be ? ToInteger(fromIndex) (indexOf)
  var n = HermesInternal.toInteger(fromIndex);

  // 5. Assert: If fromIndex is undefined, then n is 0.

  var k;
  // 5. If n ≥ 0, then
  if (n >= 0) {
    // a. Let k be n.
    k = n;
    // 6. Else n < 0,
  } else {
    // a. Let k be len + n.
    k = len + n;
    // b. If k < 0, set k to 0.
    if (k < 0) {
      k = 0;
    }
  }

  // 7. Repeat, while k < len
  while (k < len) {
    // a. Let elementK be the result of ? Get(O, ! ToString(k)).
    var elementK = O[k];
    // b. If SameValueZero(searchElement, elementK) is true, return true.
    // Note: Only difference between SameValueZero and strict equality is that
    //       SameValueZero(NaN, NaN) is true
    if (
      elementK === searchElement ||
      (elementK !== elementK && searchElement !== searchElement)
    ) {
      return true;
    }
    // c. Increase k by 1.
    k += 1;
  }
  // 8. Return false.
  return false;
};

/**
 * ES10.0 22.1.3.3
 * Copy part of an array (from start index until end index) to the part
 * starting at target index. This would not change the length of the array.
 * @this an array-like value.
 * @param target starting index of the region which elements will be copied to.
 * @param start starting index of the region which elements will be copied from.
 * @param [end] ending index of the region which elements will be copied from,
 *.             defaults to the length of the array.
 * @return the object version of this value.
 */
Array.prototype.copyWithin = function copyWithin(
  target,
  start,
  end = undefined,
) {
  // Array.prototype.copyWithin cannot be used as a constructor
  if (new.target)
    throw _TypeError('This function cannot be used as a constructor.');

  // 1. Let O be ? ToObject(this value).
  var O = HermesInternal.toObject(this);
  // 2. Let len be ? ToLength(? Get(O, "length")).
  var len = HermesInternal.toLength(O.length);

  // 3. Let relativeTarget be ? ToInteger(target).
  var relativeTarget = HermesInternal.toInteger(target);
  // 4. If relativeTarget < 0, let to be max((len + relativeTarget), 0);
  //    else let to be min(relativeTarget, len).
  var to =
    relativeTarget < 0
      ? _MathMax(len + relativeTarget, 0)
      : _MathMin(relativeTarget, len);
  // 5. Let relativeStart be ? ToInteger(start).
  var relativeStart = HermesInternal.toInteger(start);
  // 6. If relativeStart < 0, let from be max((len + relativeStart), 0);
  //    else let from be min(relativeStart, len).
  var from =
    relativeStart < 0
      ? _MathMax(len + relativeStart, 0)
      : _MathMin(relativeStart, len);
  // 7. If end is undefined, let relativeEnd be len;
  //    else let relativeEnd be ? ToInteger(end).
  var relativeEnd = end === undefined ? len : HermesInternal.toInteger(end);
  // 8. If relativeEnd < 0, let final be max((len + relativeEnd), 0);
  //    else let final be min(relativeEnd, len).
  var final =
    relativeEnd < 0
      ? _MathMax(len + relativeEnd, 0)
      : _MathMin(relativeEnd, len);
  // 9. Let count be min(final - from, len - to).
  var count = _MathMin(final - from, len - to);

  var direction = 1;
  // 10. If from < to and to < from + count, then
  if (from < to && to < from + count) {
    // a. Let direction be -1.
    direction = -1;
    // b. Set from to from + count - 1.
    from = from + count - 1;
    // c. Set to to to + count - 1.
    to = to + count - 1;
  }
  // 11. Else,
  // a. Let direction be 1.

  // 12. Repeat, while count > 0
  while (count > 0) {
    // a. Let fromKey be ! ToString(from).
    // b. Let toKey be ! ToString(to).
    // c. Let fromPresent be ? HasProperty(O, fromKey).
    // d. If fromPresent is true, then
    if (from in O) {
      // i. Let fromVal be ? Get(O, fromKey).
      // ii.Perform ? Set(O, toKey, fromVal, true).
      O[to] = O[from];
      // e. Else fromPresent is false,
    } else {
      // i. Perform ? DeletePropertyOrThrow(O, toKey).
      delete O[to];
    }
    // f. Set from to from + direction.
    from += direction;
    // g. Set to to to + direction.
    to += direction;
    // h. Decrease count by 1.
    count -= 1;
  }

  // 13. Return O.
  return O;
};

var arrayFuncDescriptor = {
  configurable: true,
  enumerable: false,
  writable: true,
};

Object.defineProperty(Array, 'from', arrayFuncDescriptor);
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
Object.defineProperty(Array.prototype, 'pop', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'reduce', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'reduceRight', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'shift', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'unshift', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'includes', arrayFuncDescriptor);
Object.defineProperty(Array.prototype, 'copyWithin', arrayFuncDescriptor);

Array.from.prototype = undefined;
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
Array.prototype.pop.prototype = undefined;
Array.prototype.reduce.prototype = undefined;
Array.prototype.reduceRight.prototype = undefined;
Array.prototype.shift.prototype = undefined;
Array.prototype.unshift.prototype = undefined;
Array.prototype.includes.prototype = undefined;
Array.prototype.copyWithin.prototype = undefined;
