/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes -gc-sanitize-handles=0 %t.hbc | %FileCheck --match-full-lines %s
// REQUIRES: !slow_debug

var arr = [];

// discover the current max array size...
try
{
  // ~/fbsource/xplat/hermes/include/hermes/VM/AlignedStorage.h
  var kLogSize = 22;
  var kSize = 1 << kLogSize;

  var sizeof_HermesValue = 8;

  // this is an approximation of upper limit of Arraystorage::maxElements which
  // we want to be too big so bind throws a RangeError and we can discover the
  // actual maximum elements.

  arr.length = (kSize / sizeof_HermesValue);

  // BoundFunction::create calls ArrayStorage::create which will throw a RangeError
  // as the requested capacity is too big.
  Function.prototype.bind.apply( () => {}, arr );
}
catch( e )
{
  // catch the range error and use a regex to pull out the max elements size

  // RangeError: Requested an array size larger than the max allowable:
  // Requested elements = 524288, max elements = 514045

  arr.length = /max elements = ([\d]+)/.exec(e.message)[1];
}

print( "arr.length=" + arr.length );

function foo()
{
  try
  {
    Function.prototype.bind.apply(
      Function.prototype.bind.apply(
        Function.prototype.bind.apply(
          gc,
          arr
        ),
        arr
      ),
      arr
    )(); // Note: This will call the bound gc method.
  }
  catch(e)
  {
    // Swallow the "RangeError: Maximum call stack size exceeded" exception
    // during BoundFunction::_boundCall so that foo can return to the caller.
    print( "Swallowing Exception: " + e );
  }
}

try {
Function.prototype.bind.apply(
  Function.prototype.bind.apply(
    Function.prototype.bind.apply(
      Function.prototype.bind.apply(
        () => {
            print( "Hello World" );
        },
        arr
      ),
      arr
    ),
    arr
  ),
  arr,
  // These params are not used by functionPrototypeApply, but
  // will be evaluated before functionPrototypeApply is called.
  foo(), // performs an explicit call to gc (as foo binds and calls gc)
  foo() // force gc a second time during array allocation, triggering a null pointer write AV.
)(); // Note: hermes will crash before actually calling the bound anonymous function.
} catch (e) {
  print("Caught Exception: " + e);
}
print("success");

// CHECK: Swallowing Exception: RangeError: Maximum call stack size exceeded
// CHECK-NEXT: Swallowing Exception: RangeError: Maximum call stack size exceeded
// CHECK-NEXT: Caught Exception: RangeError: Maximum call stack size exceeded
// CHECK: success
