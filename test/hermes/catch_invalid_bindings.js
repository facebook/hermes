/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
'use strict';

print("catch");
//CHECK-LABEL: catch

try {
  eval('try{}catch(eval){}')
} catch (err){
  print(err instanceof SyntaxError);
}
//CHECK-NEXT: true
try {
  eval('try{}catch(arguments){}')
} catch (err){
  print(err instanceof SyntaxError);
}
//CHECK-NEXT: true
