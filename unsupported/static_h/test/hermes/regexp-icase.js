/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

print('RegExp icase');
// CHECK-LABEL: RegExp icase

// Case-insensitive range checks
print("1234".match(/[1-1]/ig));
// CHECK-NEXT: 1
print("A!xyz_".match(/[a-z]/ig));
// CHECK-NEXT: A,x,y,z
print("efghi^_[\\]`$".match(/[E-F]/ig));
// CHECK-NEXT: e,f
print("efghi^_[\\]`$".match(/[E-f]/ig));
// CHECK-NEXT: e,f,g,h,i,^,_,[,\,],`
print("\u039C".match(/\xb5/g));
// CHECK-NEXT: null
print("\u039C".match(/[\xb5-\xBA]/ig)); // note: output is greek capital mu M, not ASCII M
// CHECK-NEXT: Μ
print("\u0121".match(/[\xe0-\u0131]/ig));
// CHECK-NEXT: ġ
print("sxS\u017Fx".match(/[\u017F]/ig));
// CHECK-NEXT: ſ
print("sxS\u017Fx".match(/[s]/ig));
// CHECK-NEXT: s,S
print("sxS\u017Fx".match(/[s-s]/ig));
// CHECK-NEXT: s,S
print("sxS\u017Fx".match(/[S-S]/ig));
// CHECK-NEXT: s,S
print("Just. Match. Everything.".match(/[\0-\uffff]/ig));
// CHECK-NEXT: J,u,s,t,., ,M,a,t,c,h,., ,E,v,e,r,y,t,h,i,n,g,.

// Case insensitive tests
print(/a/i.exec("Aa"));
// CHECK-NEXT: A
print(/a/i.exec("aA"));
// CHECK-NEXT: a
print(/[a-z]*/i.exec("abcDEF123"));
// CHECK-NEXT: abcDEF
print(/([a-z]*)\1\1Z/i.exec("abcAbCABCz"));
// CHECK-NEXT: abcAbCABCz,abc
print(/[a-z]+/i.exec("ı"));
// CHECK-NEXT: null
print(/[\u03B1-\u03C9]/.exec("\u03D1")); // match math theta against lowercase greek letters
// CHECK-NEXT: null
print(/[\u03B1-\u03C9]/i.exec("\u03D1"));
// CHECK-NEXT: ϑ
print(/(.+)(ςΣ)(.+)(σ)/i.exec("Ὀδυσσεύς"));
// CHECK-NEXT: Ὀδυσσεύς,Ὀδυ,σσ,εύ,ς
