/* @nolint */
/**
 * MIT License
 * Portions Copyright (c) Meta Platforms, Inc. and affiliates.
 * Copyright (c) 2022 ECMAScript Shims
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// ==========================
// Iterator.prototype.reduce Implementation
// Adapted from https://github.com/es-shims/iterator-helpers/tree/main/Iterator.prototype.reduce at da02ab656dce1c4f73355290a3bd9e4c25ccc3b6
// ==========================

// From es-iterator-helpers/Iterator.prototype.reduce/implementation.js
var IteratorPrototypeReduce = function reduce(reducer) {
	if (this instanceof reduce) {
		throw new TypeError('`reduce` is not a constructor');
	}

	var O = this; // step 1
	if (Type(O) !== 'Object') {
		throw new TypeError('`this` value must be an Object'); // step 2
	}

	if (!isCallable(reducer)) {
		throw new TypeError('`reducer` must be a function'); // step 3
	}

	var iterated = GetIteratorDirect(O); // step 4

	var accumulator;
	var counter;
	if (arguments.length < 2) { // step 6
		accumulator = IteratorStepValue(iterated); // step 6.a
		if (iterated['[[Done]]']) {
			throw new TypeError('Reduce of empty iterator with no initial value');
		}
		counter = 1;
	} else { // step 7
		accumulator = arguments[1]; // step 7.a
		counter = 0;
	}

	// eslint-disable-next-line no-constant-condition
	while (true) { // step 8
		var value = IteratorStepValue(iterated); // step 8.a
		if (iterated['[[Done]]']) {
			return accumulator; // step 8.b
		}
		try {
			var result = Call(reducer, void undefined, [accumulator, value, counter]); // step 8.d
			accumulator = result; // step 8.f
		} catch (e) {
			// close iterator // step 8.e
			IteratorClose(
				iterated,
				ThrowCompletion(e)
			);
		}
		counter += 1; // step 8.g
	}
};

defineProperties(
	Iterator.prototype,
	{ reduce: IteratorPrototypeReduce },
	{ reduce: function () { return Iterator.prototype.reduce !== IteratorPrototypeReduce; } }
);
