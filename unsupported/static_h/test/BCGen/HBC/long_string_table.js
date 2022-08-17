/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -gc-sanitize-handles=0 -O %s | %hermes - -target=HBC -gc-sanitize-handles=0 -O | %FileCheck --match-full-lines %s

// This test is meant to make sure that Hermes generates long instructions for
// indexing into objects when it is necessary.
// Since such a test requires 2^16 lines of declaring strings, this test instead
// generates such a test and passes it back into hermes.
//
// This declares a set of functions which require 2^16 unique strings, in order to
// populate the string table. It then intializes an object with properties that are
// 2^16 away to ensure that both versions of the instructions work. This intialization
// is stored in a separate function called `foo`, since strings in global scope get added
// to the string table before the strings in other functions.
//
var limC = ""
print("function f() {\n  return 'a';");
for (var i = 0; i < (1 << 16) + 2; i++) {
    if (i % 1024 == 0)
        print("};\nfunction f" + i + "() {")
    limC = i;
    print("  this[" + i + "] = '" + i + "'");
}
print("}");
print("function foo() { x={}\n      x['a'] = 1\n       x['" + limC + "'] = 2\n       print(x['a'])\n       print(x['" + limC + "'])\n       }\nfoo()")
//CHECK: 1
//CHECK: 2
