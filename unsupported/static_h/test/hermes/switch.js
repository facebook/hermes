/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

function f(x) {
  var str = "";
  switch (x) {
    case 0:
      return "regular";
    case 1:
    case 2:
      return "multicase";
    case 3:
      str +="fall";
    case 4:
      str +="through";
      return str;
    default:
      return "default";
  }
}

function sparse_numeric(x) {
    var r = 0;
    switch (x) {
        case 100:
            r += 1;
        case 200:
            r += 2;
            break;
        case 300:
            r += 100;
            break;
        default:
            r += 33;
    }

    return r;
}

// test multiple jump tables in one func
function multi_dense_switch(x) {

    var y = 0;
    switch (x) {
        case 1:
            y = 10
            break;
        case 2:
            y = 11
        case 3:
            y += 1
            break;
        case 5:
            y = 10
            break;
        case 6:
            y = 11
        case 8:
            y += 1
            break;
        case 9:
            y = 10
            break;
        case 10:
            y = 11
        case 11:
            y += 1
            break;
        case 12:
            y = 10
            break;
        case 13:
            y = 11
        case 14:
            y += 1
            break;
        default:
            y += 2
    }

    switch (y) {
        case 10:
            print(10);
            break;
        case 11:
            print(20);
            break;
        case 12:
            print(30);
            break;
        case 13:
            print(40);
        case 14:
            print(50);
            break;
        case 15:
            print(60);
            break;
        case 16:
            print(70);
            break;
        case 17:
            print(80);

        case 18:
            print(90);
            break;
        case 19:
            print(100);
            break;
        case 20:
            print(110);
            break;
        case 21:
            print(120);

    }

}

function negative_dense(x) {
    switch (x) {
        case -2:
            print (-2);
            break;
        case -1:
            print (-1);
            break;
        case 0:
            print (0);
            break;
        case 1:
            print(1);
            break;
    }
}


//CHECK: start
print("start");

//CHECK-NEXT: regular
print(f(0));

//CHECK-NEXT: multicase
print(f(1));

//CHECK-NEXT: multicase
print(f(2));

//CHECK-NEXT: fallthrough
print(f(3));

//CHECK-NEXT: through
print(f(4));

//CHECK-NEXT: default
print(f("something else"));

//CHECK: sparse_numeric
print("sparse_numeric")

//CHECK: 2
print(sparse_numeric(200))

//CHECK: 3
print(sparse_numeric(100))

//CHECK: 100
print(sparse_numeric(300))

//CHECK: 33
print(sparse_numeric(1))

//CHECK: 10
multi_dense_switch(1)

//CHECK: 30
multi_dense_switch(2)

//CHECK: -1
negative_dense(-1)
