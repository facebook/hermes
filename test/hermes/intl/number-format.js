/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print("get number format test");
// CHECK-LABEL: get number format test

const number = 1000.25;

print(new Intl.NumberFormat('en-US').format(number));
// CHECK-NEXT: 1,000.25

print(new Intl.NumberFormat('de-DE').format(number));
// CHECK-NEXT: 1.000,25

print(new Intl.NumberFormat('ar-EG').format(number));
// CHECK-NEXT: ١٬٠٠٠٫٢٥

print(new Intl.NumberFormat('en-GB', { style: 'currency', currency: 'GBP' }).format(number));
// CHECK-NEXT: £1,000.25

print(new Intl.NumberFormat('de-DE', { style: 'currency', currency: 'GBP' }).format(number));
// CHECK-NEXT: 1.000,25 £

print(new Intl.NumberFormat('ja-JP', { style: 'currency', currency: 'JPY' }).format(number));
// CHECK-NEXT: ￥1,000

print(new Intl.NumberFormat('es-CL', { style: 'currency', currency: 'CLF' }).format(number));
// CHECK-NEXT: CLF 1.000,2500

print(new Intl.NumberFormat('fr-TN', { style: 'currency', currency: 'TND' }).format(number));
// CHECK-NEXT: 1 000,250 DT

try {
    print(new Intl.NumberFormat('en-US', { maximumSignificantDigits: 0 }).format(number));
      print("Succeeded");
    } catch (e) {
      print("Caught", e.name, e.message);
    }
// CHECK-NEXT: Caught{{.*}}

print(new Intl.NumberFormat('en-US', { maximumSignificantDigits: 4 }).format(number));
// CHECK-NEXT: 1,000

print(new Intl.NumberFormat('en-US', { maximumSignificantDigits: 5 }).format(number));
// CHECK-NEXT: 1,000.3

print(new Intl.NumberFormat('en-US', { maximumSignificantDigits: 6 }).format(number));
// CHECK-NEXT: 1,000.25

print(new Intl.NumberFormat('en-US', { maximumSignificantDigits: 7 }).format(number));
// CHECK-NEXT: 1,000.25

print(new Intl.NumberFormat('en-US',  {
    style: 'percent'
}).format(0.33));
// CHECK-NEXT: 33%

print(new Intl.NumberFormat('ja-JP',  {
    style: 'decimal'
}).format(0.33));
// CHECK-NEXT: 0.33

const unitDisplays = ["short", "narrow", "long"];

unitDisplays.forEach(element => print(new Intl.NumberFormat('en-US',  {
    style: 'unit',
    unit: 'kilometer-per-hour',
    unitDisplay: element
}).format(100)));
// CHECK-NEXT: 100 km/h
// CHECK-NEXT: 100km/h
// CHECK-NEXT: 100 kilometers per hour

unitDisplays.forEach(element => print(new Intl.NumberFormat('en-US',  {
    style: 'unit',
    unit: 'megabyte-per-second',
    unitDisplay: element
}).format(100)));
// CHECK-NEXT: 100 MB/s
// CHECK-NEXT: 100MB/s
// CHECK-NEXT: 100 megabytes per second

unitDisplays.forEach(element => print(new Intl.NumberFormat('fr-FR',  {
    style: 'unit',
    unit: 'megabyte-per-second',
    unitDisplay: element
}).format(100)));
// CHECK-NEXT: 100 Mo/s
// CHECK-NEXT: 100 Mo/s
// CHECK-NEXT: 100 mégaoctets par seconde

unitDisplays.forEach(element => print(new Intl.NumberFormat('en-US',  {
    style: 'unit',
    unit: 'meter',
    unitDisplay: element
}).format(100)));
// CHECK-NEXT: 100 m
// CHECK-NEXT: 100m
// CHECK-NEXT: 100 meters

unitDisplays.forEach(element => print(new Intl.NumberFormat('en-GB',  {
    style: 'unit',
    unit: 'meter',
    unitDisplay: element
}).format(100)));
// CHECK-NEXT: 100 m
// CHECK-NEXT: 100m
// CHECK-NEXT: 100 metres

unitDisplays.forEach(element => print(new Intl.NumberFormat('ja-JP',  {
    style: 'unit',
    unit: 'acre',
    unitDisplay: element
}).format(100)));
// CHECK-NEXT: 100 ac
// CHECK-NEXT: 100ac
// CHECK-NEXT: 100 エーカー

try {
    print(new Intl.NumberFormat('en-US', { style: 'unit' }).format(0.33));
      print("Succeeded");
    } catch (e) {
      print("Caught", e.name, e.message);
    }
// CHECK-NEXT: Caught{{.*}}

try {
    print(new Intl.NumberFormat('en-US', { style: 'currency' }).format(0.33));
      print("Succeeded");
    } catch (e) {
      print("Caught", e.name, e.message);
    }
// CHECK-NEXT: Caught{{.*}}
