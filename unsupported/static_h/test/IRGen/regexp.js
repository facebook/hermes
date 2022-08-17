/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK-LABEL:function simple_test0()
//CHECK-NEXT:frame = [re, str, newstr]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [re]
//CHECK-NEXT:    %1 = StoreFrameInst undefined : undefined, [str]
//CHECK-NEXT:    %2 = StoreFrameInst undefined : undefined, [newstr]
//CHECK-NEXT:    %3 = CreateRegExpInst "(\\\\w+)\\\\s(\\\\w+)" : string, "" : string
//CHECK-NEXT:    %4 = StoreFrameInst %3 : regexp, [re]
//CHECK-NEXT:    %5 = StoreFrameInst "John Smith" : string, [str]
//CHECK-NEXT:    %6 = LoadFrameInst [str]
//CHECK-NEXT:    %7 = LoadPropertyInst %6, "replace" : string
//CHECK-NEXT:    %8 = LoadFrameInst [re]
//CHECK-NEXT:    %9 = CallInst %7, %6, %8, "$2, $1" : string
//CHECK-NEXT:    %10 = StoreFrameInst %9, [newstr]
//CHECK-NEXT:    %11 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function simple_test0() {
  var re = /(\w+)\s(\w+)/;
  var str = 'John Smith';
  var newstr = str.replace(re, '$2, $1');
}

//CHECK-LABEL:function simple_test1()
//CHECK-NEXT:frame = [re0, re1, re2, re3, re4]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [re0]
//CHECK-NEXT:    %1 = StoreFrameInst undefined : undefined, [re1]
//CHECK-NEXT:    %2 = StoreFrameInst undefined : undefined, [re2]
//CHECK-NEXT:    %3 = StoreFrameInst undefined : undefined, [re3]
//CHECK-NEXT:    %4 = StoreFrameInst undefined : undefined, [re4]
//CHECK-NEXT:    %5 = CreateRegExpInst "\\\\w+\\\\s" : string, "g" : string
//CHECK-NEXT:    %6 = StoreFrameInst %5 : regexp, [re0]
//CHECK-NEXT:    %7 = CreateRegExpInst "\\\\w+\\\\s" : string, "g" : string
//CHECK-NEXT:    %8 = StoreFrameInst %7 : regexp, [re1]
//CHECK-NEXT:    %9 = CreateRegExpInst "\\\\w+" : string, "" : string
//CHECK-NEXT:    %10 = StoreFrameInst %9 : regexp, [re2]
//CHECK-NEXT:    %11 = CreateRegExpInst "\\\\w+" : string, "g" : string
//CHECK-NEXT:    %12 = StoreFrameInst %11 : regexp, [re3]
//CHECK-NEXT:    %13 = CreateRegExpInst " " : string, "" : string
//CHECK-NEXT:    %14 = StoreFrameInst %13 : regexp, [re4]
//CHECK-NEXT:    %15 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function simple_test1() {
  var re0 = /\w+\s/g;
  var re1 = /\w+\s/g;
  var re2 = /\w+/;
  var re3 = /\w+/g;
  var re4 = / /;
}


