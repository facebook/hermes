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
// Iterator.prototype.map Implementation
// Adapted from https://github.com/es-shims/iterator-helpers/tree/main/Iterator.prototype.map at da02ab656dce1c4f73355290a3bd9e4c25ccc3b6
// ==========================

// From es-iterator-helpers/Iterator.prototype.map/implementation.js
var IteratorPrototypeMap = function map(mapper) {
	if (this instanceof map) {
		throw new TypeError('`map` is not a constructor');
	}

	var O = this; // step 1
	if (Type(O) !== 'Object') {
		throw new TypeError('`this` value must be an Object'); // step 2
	}

	if (!isCallable(mapper)) {
		throw new TypeError('`mapper` must be a function'); // step 3
	}

	var iterated = GetIteratorDirect(O); // step 4

	var closeIfAbrupt = function (abruptCompletion) {
		if (!(abruptCompletion instanceof CompletionRecord)) {
			throw new TypeError('`abruptCompletion` must be a Completion Record');
		}
		IteratorClose(
			iterated,
			abruptCompletion
		);
	};

	var sentinel = {};
	var counter = 0; // step 6.a
	var closure = function () {
		// while (true) { // step 6.b
		var value = IteratorStepValue(iterated); // step 6.b.i
		if (iterated['[[Done]]']) {
			return sentinel; // step 6.b.ii
		}

		var mapped;
		try {
			mapped = Call(mapper, void undefined, [value, counter]); // step 6.b.iii
			// yield mapped // step 6.b.vi
			return mapped;
		} catch (e) {
			// close iterator // step 6.b.v, 6.b.vii
			closeIfAbrupt(ThrowCompletion(e));
			throw e;
		} finally {
			counter += 1; // step 6.b.viii
		}
		// }
	};
	SLOT.set(closure, '[[Sentinel]]', sentinel); // for the userland implementation
	SLOT.set(closure, '[[CloseIfAbrupt]]', closeIfAbrupt); // for the userland implementation

	var result = CreateIteratorFromClosure(closure, 'Iterator Helper', IteratorHelperPrototype, ['[[UnderlyingIterators]]']); // step 7

	SLOT.set(result, '[[UnderlyingIterators]]', [iterated]); // step 8

	return result; // step 9
};

defineProperties(
	Iterator.prototype,
	{ map: IteratorPrototypeMap },
	{ map: function () { return Iterator.prototype.map !== IteratorPrototypeMap; } }
);
