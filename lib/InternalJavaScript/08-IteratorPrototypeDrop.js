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
// Iterator.prototype.drop Implementation
// Adapted from https://github.com/es-shims/iterator-helpers/tree/main/Iterator.prototype.drop at fbd6ffe (v1.2.2)
// ==========================

// From es-iterator-helpers/Iterator.prototype.drop/implementation.js
var IteratorPrototypeDrop = function drop(limit) {
	if (new.target !== undefined) {
		throw new TypeError('`drop` is not a constructor');
	}

	var O = this; // step 1
	if (!isObject(O)) {
		throw new TypeError('`this` value must be an Object'); // step 2
	}

	var iterated = { '[[Iterator]]': O, '[[NextMethod]]': undefined, '[[Done]]': false }; // step 3

	var numLimit;
	try {
		numLimit = ToNumber(limit); // step 4
	} catch (e) {
		// step 5
		IteratorClose(
			iterated,
			ThrowCompletion(e)
		);
	}
	if (isNaN(numLimit)) {
		IteratorClose(
			iterated,
			ThrowCompletion(new RangeError('`limit` must be a non-NaN number'))
		); // step 6
	}

	var integerLimit = ToIntegerOrInfinity(numLimit); // step 7
	if (integerLimit < 0) {
		IteratorClose(
			iterated,
			ThrowCompletion(new RangeError('`limit` must be a >= 0'))
		); // step 8
	}

	iterated = GetIteratorDirect(O); // step 9

	// step 10
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
	var remaining = integerLimit; // step 10.a
	var closure = function () { // step 10
		var next;
		while (remaining > 0) { // step 10.b
			if (remaining !== Infinity) { // step 10.b.i
				remaining -= 1; // step 10.b.i.1
			}

			next = IteratorStep(iterated); // step 10.b.ii
			if (!next) {
				// return void undefined; // step 10.b.iii
				return sentinel;
			}
		}
		// while (true) { // step 10.c
		var value = IteratorStepValue(iterated); // step 10.c.i
		if (iterated['[[Done]]']) {
			return sentinel; // step 10.c.ii
		}
		return value;
		// }
		// return void undefined;
	};
	SLOT.set(closure, '[[Sentinel]]', sentinel); // for the userland implementation
	SLOT.set(closure, '[[CloseIfAbrupt]]', closeIfAbrupt); // for the userland implementation

	var result = CreateIteratorFromClosure(closure, 'Iterator Helper', IteratorHelperPrototype, ['[[UnderlyingIterators]]']); // step 4

	SLOT.set(result, '[[UnderlyingIterators]]', [iterated]); // step 5

	return result; // step 6
};

defineProperties(
	Iterator.prototype,
	{ drop: IteratorPrototypeDrop },
	{ drop: function () { return Iterator.prototype.drop !== IteratorPrototypeDrop; } }
);
