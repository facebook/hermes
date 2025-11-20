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
// Iterator.prototype.forEach Implementation
// Adapted from https://github.com/es-shims/iterator-helpers/tree/main/Iterator.prototype.forEach at da02ab656dce1c4f73355290a3bd9e4c25ccc3b6
// ==========================

// From es-iterator-helpers/Iterator.prototype.forEach/implementation.js
var IteratorPrototypeForEach = function forEach(fn) {
	if (this instanceof forEach) {
		throw new TypeError('`forEach` is not a constructor');
	}

	var O = this; // step 1
	if (Type(O) !== 'Object') {
		throw new TypeError('`this` value must be an Object'); // step 2
	}

	if (!isCallable(fn)) {
		throw new TypeError('`fn` must be a function'); // step 3
	}

	var iterated = GetIteratorDirect(O); // step 4

	var counter = 0; // step 5

	// eslint-disable-next-line no-constant-condition
	while (true) { // step 6
		var value = IteratorStepValue(iterated); // step 6.a
		if (iterated['[[Done]]']) {
			return void undefined; // step 6.b
		}
		try {
			Call(fn, void undefined, [value, counter]); // step 6.c
		} catch (e) {
			IteratorClose(
				iterated,
				ThrowCompletion(e)
			); // steps 6.d
			throw e;
		} finally {
			counter += 1; // step 6.e
		}
	}
};

defineProperties(
	Iterator.prototype,
	{ forEach: IteratorPrototypeForEach },
	{ forEach: function () { return Iterator.prototype.forEach !== IteratorPrototypeForEach; } }
);
