/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

var globalVar = "omega";

function func1(closure, f1param) { // frame 4
    var f1v1 = "alpha";
    var f1v2 = "beta";
    function func1b() { // frame 3
        var f1bv1 = "gamma";
        function func1c() { // frame 2
            var f1cv1 = 19;
            closure();
        }
        func1c();
    }
    func1b();
}

function func2() { // frame 1
    var f2v1 = "baker";
    var f2v2 = "charlie";
    function func2b() { // frame 0
        var f2bv1 = "dog";
        debugger;
        print(globalVar);
        print(f2bv1);
    }
    func2b();
}

func1(func2, "tau")

// CHECK: Break on 'debugger' statement in func2b: {{.*}}:32:9
// CHECK-NEXT: Checking literals
// CHECK-NEXT: { a: bc }
// CHECK-NEXT: {
// CHECK-NEXT:     d: ef
// CHECK-NEXT:     g: [object Object]
// CHECK-NEXT: }
// CHECK-NEXT: Selected frame 0
// CHECK-NEXT: 0: omega
// CHECK-NEXT: 1: dog
// CHECK-NEXT: 2: charlie
// CHECK-NEXT: 3: dog && charlie
// CHECK-NEXT: Selected frame 1
// CHECK-NEXT: 4: baker
// CHECK-NEXT: 5: charlie
// CHECK-NEXT: Selected frame 2
// CHECK-NEXT: 6: 19gammaalpha
// CHECK-NEXT: Selected frame 3
// CHECK-NEXT: 7: gammaalpha
// CHECK-NEXT: Selected frame 2
// CHECK-NEXT: 8: tau
// CHECK-NEXT: Selected frame 3
// CHECK-NEXT: 9: tau
// CHECK-NEXT: Selected frame 4
// CHECK-NEXT: 10: tau
// CHECK-NEXT: 11: omega
// CHECK-NEXT: Selected frame 2
// CHECK-NEXT: setting globalVar
// CHECK-NEXT: Selected frame 0
// CHECK-NEXT: setting f2bv1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: var modified by debugger
// CHECK-NEXT: another var modified by debugger
