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
// Iterator.concat Implementation
// Adapted from https://github.com/es-shims/iterator-helpers/tree/main/Iterator.concat at b790e7409435f3bd3f43edbf9dc1d962818d8f46
// ==========================

// From es-iterator-helpers/Iterator.concat/implementation.js
var IteratorConcat = function concat() {
	if (this instanceof concat) {
		throw new TypeError('`Iterator.concat` is not a constructor');
	}

	var iterables = []; // step 1

	for (var i = 0; i < arguments.length; i += 1) { // step 2
		var item = arguments[i];
		if (!isObject(item)) {
			throw new TypeError('`Iterator.concat` requires all arguments to be objects'); // step 2.1
		}
		var method = getIteratorMethod(
			{
				AdvanceStringIndex: AdvanceStringIndex,
				GetMethod: GetMethod,
				IsArray: IsArray
			},
			item
		);
		if (typeof method === 'undefined') {
			throw new TypeError('`Iterator.concat` requires all arguments to be iterable'); // step 2.3
		}
		iterables[iterables.length] = { '[[OpenMethod]]': method, '[[Iterable]]': item }; // step 2.4
	}

	var sentinel = {};
	var iterablesIndex = 0;
	var iteratorRecord;
	var innerAlive = false;
	var openIters = []; // track the current open iterator for return() forwarding
	var closure = function () { // step 3
		if (iterablesIndex >= iterables.length) {
			return sentinel;
		}
		var iterable = iterables[iterablesIndex]; // step 3.a
		if (!innerAlive) {
			var iter = Call(iterable['[[OpenMethod]]'], iterable['[[Iterable]]']); // step 3.a.i
			if (!isObject(iter)) {
				throw new TypeError('`Iterator.concat` iterator method did not return an object'); // step 3.a.ii
			}
			iteratorRecord = GetIteratorDirect(iter); // step 3.a.iii
			openIters[0] = iteratorRecord; // track the open iterator
			innerAlive = true; // step 3.a.iv
		}

		if (innerAlive) { // step 3.a.v
			var innerValue = IteratorStepValue(iteratorRecord); // step 3.a.v.1
			if (iteratorRecord['[[Done]]']) { // step 3.a.v.2
				innerAlive = false; // step 3.a.v.2.a
				openIters.length = 0; // no open iterator now
			} else { // step 3.a.v.3
				return innerValue; // step 3.a.v.3.a
			}
		}

		iterablesIndex += 1;
		return closure();
	};

	var closeIfAbrupt = function (abruptCompletion) {
		if (!(abruptCompletion instanceof CompletionRecord)) {
			throw new TypeError('`abruptCompletion` must be a Completion Record');
		}
		iterablesIndex = iterables.length;
		innerAlive = false;
		if (openIters.length > 0) {
			var toClose = openIters.slice();
			openIters.length = 0; // prevent double-closing
			IteratorCloseAll(toClose, abruptCompletion);
		}
	};

	SLOT.set(closure, '[[Sentinel]]', sentinel); // for the userland implementation
	SLOT.set(closure, '[[CloseIfAbrupt]]', closeIfAbrupt); // for the userland implementation

	var gen = CreateIteratorFromClosure(closure, 'Iterator Helper', IteratorHelperPrototype, ['[[UnderlyingIterators]]']); // step 5
	SLOT.set(gen, '[[UnderlyingIterators]]', openIters); // step 6 - share the array reference
	return gen; // step 7
};

defineProperties(
	Iterator,
	{ concat: IteratorConcat },
	{ concat: function () { return Iterator.concat !== IteratorConcat; } }
);
