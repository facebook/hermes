/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// This is a regression test that ensures SymbolIDs cached in
// StringPrimitives are properly marked.

function tester(propname) {
	var prop;

	function foo() {
	    var o = {};
	    o[propname] = 10;

	    // prop shoould contain a uniqued name.
	    prop = Object.getOwnPropertyNames(o)[0];
	}

	foo();

	// This should collect the object and its symbols.
	gc();

	var o = {};

	o[prop] = 10;
	var prop = Object.getOwnPropertyNames(o)[0];
	print(prop[0]);
}

var a = "aa";
var b = "bb";
tester(a + b);
// CHECK: a

// Test with a string long enough to be external.
var x = "x";
for (var i=0; i < 18; i++) x += x;
tester(x);
// CHECK: x
