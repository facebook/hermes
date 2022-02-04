/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// Test object literals hidden class caching behavior in dictionary mode.

// Create objects with more than 64 properties to make its hidden class go into dictionary mode.
var obj1 = {
  p1: 'val1',
  p2: 'val2',
  p3: 'val3',
  p4: 'val4',
  p5: 'val5',
  p6: 'val6',
  p7: 'val7',
  p8: 'val8',
  p9: 'val9',
  p10: 'val10',
  p11: 'val11',
  p12: 'val12',
  p13: 'val13',
  p14: 'val14',
  p15: 'val15',
  p16: 'val16',
  p17: 'val17',
  p18: 'val18',
  p19: 'val19',
  p20: 'val20',
  p21: 'val21',
  p22: 'val22',
  p23: 'val23',
  p24: 'val24',
  p25: 'val25',
  p26: 'val26',
  p27: 'val27',
  p28: 'val28',
  p29: 'val29',
  p30: 'val30',
  p31: 'val31',
  p32: 'val32',
  p33: 'val33',
  p34: 'val34',
  p35: 'val35',
  p36: 'val36',
  p37: 'val37',
  p38: 'val38',
  p39: 'val39',
  p40: 'val40',
  p41: 'val41',
  p42: 'val42',
  p43: 'val43',
  p44: 'val44',
  p45: 'val45',
  p46: 'val46',
  p47: 'val47',
  p48: 'val48',
  p49: 'val49',
  p50: 'val50',
  p51: 'val51',
  p52: 'val52',
  p53: 'val53',
  p54: 'val54',
  p55: 'val55',
  p56: 'val56',
  p57: 'val57',
  p58: 'val58',
  p59: 'val59',
  p60: 'val60',
  p61: 'val61',
  p62: 'val62',
  p63: 'val63',
  p64: 'val64',
  p65: 'val65',
  p66: 'val66',
  p67: 'val67',
  p68: 'val68',
  p69: 'val69',
}

var obj2 = {
  p1: 'another_val1',
  p2: 'another_val2',
  p3: 'another_val3',
  p4: 'another_val4',
  p5: 'another_val5',
  p6: 'another_val6',
  p7: 'another_val7',
  p8: 'another_val8',
  p9: 'another_val9',
  p10: 'another_val10',
  p11: 'another_val11',
  p12: 'another_val12',
  p13: 'another_val13',
  p14: 'another_val14',
  p15: 'another_val15',
  p16: 'another_val16',
  p17: 'another_val17',
  p18: 'another_val18',
  p19: 'another_val19',
  p20: 'another_val20',
  p21: 'another_val21',
  p22: 'another_val22',
  p23: 'another_val23',
  p24: 'another_val24',
  p25: 'another_val25',
  p26: 'another_val26',
  p27: 'another_val27',
  p28: 'another_val28',
  p29: 'another_val29',
  p30: 'another_val30',
  p31: 'another_val31',
  p32: 'another_val32',
  p33: 'another_val33',
  p34: 'another_val34',
  p35: 'another_val35',
  p36: 'another_val36',
  p37: 'another_val37',
  p38: 'another_val38',
  p39: 'another_val39',
  p40: 'another_val40',
  p41: 'another_val41',
  p42: 'another_val42',
  p43: 'another_val43',
  p44: 'another_val44',
  p45: 'another_val45',
  p46: 'another_val46',
  p47: 'another_val47',
  p48: 'another_val48',
  p49: 'another_val49',
  p50: 'another_val50',
  p51: 'another_val51',
  p52: 'another_val52',
  p53: 'another_val53',
  p54: 'another_val54',
  p55: 'another_val55',
  p56: 'another_val56',
  p57: 'another_val57',
  p58: 'another_val58',
  p59: 'another_val59',
  p60: 'another_val60',
  p61: 'another_val61',
  p62: 'another_val62',
  p63: 'another_val63',
  p64: 'another_val64',
  p65: 'another_val65',
  p66: 'another_val66',
  p67: 'another_val67',
  p68: 'another_val68',
  p69: 'another_val69',
}

// Under dictionary mode, hidden class becomes private which means it will not be shared between object literals even though they are of the same shape.
// So if we incorrectly shared the cached hidden class in dictionary mode between objects when we shouldn't, modification to obj2 will affect obj1.
obj2.added_prop = 'added_value';
delete obj2.p67;

print("start of test");
//CHECK: start of test

print(obj1.p11);
//CHECK-NEXT: val11

print(obj1.p60);
//CHECK-NEXT: val60

print(obj1.p69);
//CHECK-NEXT: val69

print(Object.keys(obj1).length);
//CHECK-NEXT: 69

print(obj2.p11);
//CHECK-NEXT: another_val11

print(obj2.p60);
//CHECK-NEXT: another_val60

print(obj2.p69);
//CHECK-NEXT: another_val69

print(Object.keys(obj2).length);
//CHECK-NEXT: 69

print(obj1.p67);
//CHECK-NEXT: val67

print(obj2.p67);
//CHECK-NEXT: undefined

print(obj1.added_prop);
//CHECK-NEXT: undefined

print(obj2.added_prop);
//CHECK-NEXT: added_value
