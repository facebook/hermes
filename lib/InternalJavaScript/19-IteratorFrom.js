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
// Iterator.from Implementation
// Adapted from https://github.com/es-shims/iterator-helpers/tree/main/Iterator.from at da02ab656dce1c4f73355290a3bd9e4c25ccc3b6
// ==========================

	// From es-iterator-helpers/WrapForValidIteratorPrototype/index.js
var $WrapForValidIteratorPrototype = /* GetIntrinsic('%WrapForValidIteratorPrototype%', true) || */ {
		__proto__: Iterator.prototype,
		next: function next() {
			var O = this; // step 1

			// RequireInternalSlot(O, [[Iterated]]); // step 2
			SLOT.assert(O, '[[Iterated]]');

			var iteratorRecord = SLOT.get(O, '[[Iterated]]'); // step 3

			return Call(iteratorRecord['[[NextMethod]]'], iteratorRecord['[[Iterator]]']); // step 4
		},
		'return': function () {
			var O = this; // step 1

			// RequireInternalSlot(O, [[Iterated]]); // step 2
			SLOT.assert(O, '[[Iterated]]');

			var iterator = SLOT.get(O, '[[Iterated]]')['[[Iterator]]']; // step 3

			if (Type(iterator) !== 'Object') {
				throw new TypeError('iterator must be an Object'); // step 4
			}

			var returnMethod = GetMethod(iterator, 'return'); // step 5

			if (typeof returnMethod === 'undefined') { // step 6
				return CreateIterResultObject(undefined, true); // step 6.a
			}
			return Call(returnMethod, iterator); // step 7
		}
	};

var IteratorFrom = function from(O) {
	if (this instanceof from) {
		throw new TypeError('`Iterator.from` is not a constructor');
	}

	var iteratorRecord = GetIteratorFlattenable(O, 'ITERATE-STRINGS'); // step 1

	var hasInstance = OrdinaryHasInstance(Iterator, iteratorRecord['[[Iterator]]']); // step 2

	if (hasInstance) { // step 3
		return iteratorRecord['[[Iterator]]']; // step 3.a
	}

	var wrapper = OrdinaryObjectCreate($WrapForValidIteratorPrototype); // , ['[[Iterated]]']); // step 4

	SLOT.set(wrapper, '[[Iterated]]', iteratorRecord); // step 5

	return wrapper; // step 6
};

// Note: Iterator.from is attached to the Iterator constructor, not Iterator.prototype
defineProperties(
	Iterator,
	{ from: IteratorFrom },
	{ from: function () { return Iterator.from !== IteratorFrom; } }
);
