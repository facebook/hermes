/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// Makes sure we don't sink object initialization into loops
// If `o` is sunk into its only use, `arr` is populated with different values

count = 0
arr = []

function main() {
	var o = {}
	for (var i = 0; i < 5; i++) {
		counter(o);
	}
}

function counter(obj) {
	if (!obj.x) { obj.x = 0; }
	count++;
	obj.x += count;
	arr.push(obj.x);
}

main();
// CHECK-LABEL: 1,3,6,10,15
print(arr);
