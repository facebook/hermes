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

var arrayIterProto = GetIntrinsic('%ArrayIteratorPrototype%', true);
var iterProto = arrayIterProto && getProto(arrayIterProto);

var result = (iterProto !== Object.prototype && iterProto) || {};

// Check for symbol support and add Symbol.iterator if needed
if (typeof Symbol !== 'undefined' && Symbol.iterator) {
	if (!(Symbol.iterator in result)) {
		// needed when result === iterProto, or, node 0.11.15 - 3
		var iter = setFunctionName(function SymbolIterator() {
			return this;
		}, '[Symbol.iterator]', true);

		defineDataProperty(result, Symbol.iterator, iter, true);
	}
}

var iteratorPrototype = result;

// === Main Iterator Constructor ===
// From Iterator/implementation.js

var $defineProperty = hasPropertyDescriptors() && GetIntrinsic('%Object.defineProperty%', true);

var $isPrototypeOf = callBound('Object.prototype.isPrototypeOf');

var $Iterator = typeof Iterator === 'function' ? Iterator : function Iterator() {
	if (
		!(this instanceof Iterator)
		|| this.constructor === Iterator
		|| !$isPrototypeOf(Iterator, this.constructor)
	) {
		throw new TypeError('`Iterator` can not be called or constructed directly');
	}
};

defineProperties(
	iteratorPrototype,
	{ constructor: $Iterator },
	{ constructor: function () { return $Iterator.constructor !== $Iterator; } }
);

Object.defineProperty(iteratorPrototype, 'constructor', {
	configurable: true,
	enumerable: false,
	get: function () {
		return $Iterator;
	},
	set: function (v) {
		SetterThatIgnoresPrototypeProperties(this, iteratorPrototype, 'constructor', v);
		return undefined;
	}
});

Object.defineProperty(iteratorPrototype, Symbol.toStringTag, {
	configurable: true,
	enumerable: false,
	get: function () {
		return 'Iterator';
	},
	set: function (v) {
		SetterThatIgnoresPrototypeProperties(this, iteratorPrototype, Symbol.toStringTag, v);
		return undefined;
	}
});

$Iterator.prototype = iteratorPrototype;

$defineProperty($Iterator, 'prototype', { writable: false });

defineProperties(
	globalThis,
	{ Iterator: $Iterator },
	{ Iterator: function () { return Iterator !== $Iterator; } }
);
