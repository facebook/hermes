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
// Iterator Helper Prototype Implementation
// Adapted from https://github.com/es-shims/iterator-helpers/tree/main/IteratorHelperPrototype at da02ab656dce1c4f73355290a3bd9e4c25ccc3b6
// ==========================

// From es-iterator-helpers/IteratorHelperPrototype/index.js
var o = { // in an object, for name inference
	'return': function () {
		var O = this; // step 1

		SLOT.assert(O, '[[UnderlyingIterators]]'); // step 2

		SLOT.assert(O, '[[GeneratorState]]'); // step 3

		if (SLOT.get(O, '[[GeneratorState]]') === 'suspendedStart') { // step 4
			SLOT.set(O, '[[GeneratorState]]', 'completed'); // step 4.a
			IteratorCloseAll(SLOT.get(O, '[[UnderlyingIterators]]'), ReturnCompletion(void undefined)); // step 4.c
			return CreateIterResultObject(void undefined, true); // step 4.d
		}

		var C = ReturnCompletion(void undefined); // step 5

		return GeneratorResumeAbrupt(O, C, 'Iterator Helper'); // step 6
	}
};

var IteratorHelperPrototype = {
	__proto__: Iterator.prototype,
	next: function next() {
		return GeneratorResume(this, void undefined, 'Iterator Helper');
	},
	'return': o['return']
};
Object.defineProperty(IteratorHelperPrototype, Symbol.toStringTag, {
	configurable: true,
	enumerable: false,
	value: 'Iterator Helper'
});
