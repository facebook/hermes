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
// Iterator.prototype.flatMap Implementation
// Adapted from https://github.com/es-shims/iterator-helpers/tree/main/Iterator.prototype.flatMap at da02ab656dce1c4f73355290a3bd9e4c25ccc3b6
// ==========================

// From es-iterator-helpers/Iterator.prototype.flatMap/implementation.js
var IteratorPrototypeFlatMap = function flatMap(mapper) {
	if (this instanceof flatMap) {
		throw new TypeError('`flatMap` is not a constructor');
	}

	var O = this; // step 1
	if (Type(O) !== 'Object') {
		throw new TypeError('`this` value must be an Object'); // step 2
	}

	if (!isCallable(mapper)) {
		throw new TypeError('`mapper` must be a function'); // step 3
	}

	var iterated = GetIteratorDirect(O); // step 4

	var sentinel = { sentinel: true };
	var innerIterator = sentinel;

	var closeIfAbrupt = function (abruptCompletion) {
		if (!(abruptCompletion instanceof CompletionRecord)) {
			throw new TypeError('`abruptCompletion` must be a Completion Record');
		}
		try {
			if (innerIterator !== sentinel) {
				IteratorClose(
					innerIterator,
					abruptCompletion
				);
			}
		} finally {
			innerIterator = sentinel;

			IteratorClose(
				iterated,
				abruptCompletion
			);
		}
	};

	var counter = 0; // step 5.a
	var innerAlive = false;
	var closure = function () {
		// while (true) { // step 5.b
		if (innerIterator === sentinel) {
			var value = IteratorStepValue(iterated); // step 5.b.i
			if (iterated['[[Done]]']) {
				innerAlive = false;
				innerIterator = sentinel;
				// return void undefined; // step 5.b.ii
				return sentinel;
			}
		}

		if (innerIterator === sentinel) {
			innerAlive = true; // step 5.b.viii
			try {
				var mapped = Call(mapper, void undefined, [value, counter]); // step 5.b.iv
				// yield mapped // step 5.b.vi
				innerIterator = GetIteratorFlattenable(mapped, 'REJECT-STRINGS'); // step 5.b.vi
			} catch (e) {
				innerAlive = false;
				innerIterator = sentinel;
				closeIfAbrupt(ThrowCompletion(e)); // steps 5.b.v, 5.b.vii
			} finally {
				counter += 1; // step 5.b.x
			}
		}
		// while (innerAlive) { // step 5.b.ix
		if (innerAlive) {
			// step 5.b.ix.4
			var innerValue;
			try {
				innerValue = IteratorStepValue(innerIterator); // step 5.b.ix.4.a
			} catch (e) {
				innerAlive = false;
				innerIterator = sentinel;
				closeIfAbrupt(ThrowCompletion(e)); // step 5.b.ix.4.b
			}
			if (innerIterator['[[Done]]']) {
				innerAlive = false;
				innerIterator = sentinel;
				return closure();
			}
			return innerValue; // step 5.b.ix.4.c
		}
		// }
		// return void undefined;
		return sentinel;
	};
	SLOT.set(closure, '[[Sentinel]]', sentinel); // for the userland implementation
	SLOT.set(closure, '[[CloseIfAbrupt]]', closeIfAbrupt); // for the userland implementation

	var result = CreateIteratorFromClosure(closure, 'Iterator Helper', IteratorHelperPrototype, ['[[UnderlyingIterators]]']); // step 7

	SLOT.set(result, '[[UnderlyingIterators]]', [iterated]); // step 8

	return result; // step 9
};

defineProperties(
	Iterator.prototype,
	{ flatMap: IteratorPrototypeFlatMap },
	{ flatMap: function () { return Iterator.prototype.flatMap !== IteratorPrototypeFlatMap; } }
);
