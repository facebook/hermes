/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

const MyError = function(message) {
  this.message = message;
};
MyError.prototype = new Error;
MyError.prototype.name = 'MyError';


try {
  throw new MyError('1234')
} catch (e) {
    if (e instanceof Error)
        print("Caught Error", e.stack);
    else
        print("Caught non-Error");
}
// CHECK: Caught Error MyError: 1234
// CHECK-NEXT:     at global{{.*}}


function MyError2() {
    this.message = "MyError2";
}
MyError2.prototype = Object.create(Error.prototype);
try {
    throw new MyError2();
} catch (e) {
    print("Caught", e, e.stack);
}

// CHECK-NEXT: Caught Error: MyError2 undefined
