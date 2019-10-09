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

var arrayFuncDescriptor = {
  configurable: true,
  enumerable: false,
  writable: true,
};

Object.defineProperty(Array.prototype, 'reverse', arrayFuncDescriptor);

Array.prototype.reverse.prototype = undefined;
