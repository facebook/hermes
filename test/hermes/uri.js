/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

print('encodeURI');
// CHECK-LABEL: encodeURI
print(encodeURI('https://.com/#sec-15.1.3.3'));
// CHECK-NEXT: https://.com/#sec-15.1.3.3
print(encodeURI('a b'));
// CHECK-NEXT: a%20b
print(encodeURI('ϱ'));
// CHECK-NEXT: %CF%B1
print(encodeURI('^\ud83d\udcd3$'));
// CHECK-NEXT: %5E%F0%9F%93%93$
try { print(encodeURI('\udc23')); } catch (e) { print('caught', e) }
// CHECK-NEXT: caught URIError: {{.*}}
try { print(encodeURI('\ud8d3\ud000')); } catch (e) { print('caught', e) }
// CHECK-NEXT: caught URIError: {{.*}}

print('encodeURIComponent');
// CHECK-LABEL: encodeURIComponent
print(encodeURIComponent('https://.com/#sec-15.1.3.3'));
// CHECK-NEXT: https%3A%2F%2F.com%2F%23sec-15.1.3.3
print(encodeURIComponent('a b'));
// CHECK-NEXT: a%20b
print(encodeURIComponent('ϱ'));
// CHECK-NEXT: %CF%B1
print(encodeURIComponent('^\ud83d\udcd3$'));
// CHECK-NEXT: %5E%F0%9F%93%93%24
try { print(encodeURIComponent('\udc23')); } catch (e) { print('caught', e) }
// CHECK-NEXT: caught URIError: {{.*}}
try {print(encodeURIComponent('\ud8d3\ud000'));} catch (e) {print('caught', e)}
// CHECK-NEXT: caught URIError: {{.*}}

print('decodeURI');
// CHECK-LABEL: decodeURI
print(decodeURI('https://.com/#sec-15.1.3.3?'));
// CHECK-NEXT: https://.com/#sec-15.1.3.3?
print(decodeURI('https://.com/%23sec-15.1.3.3%3f'));
// CHECK-NEXT: https://.com/%23sec-15.1.3.3%3f
print(decodeURI('a%20b'));
// CHECK-NEXT: a b
print(decodeURI('%CF%B1'));
// CHECK-NEXT: ϱ
print(decodeURI('%00').charCodeAt(0));
// CHECK-NEXT: 0
var x = decodeURI('%5E%F0%9F%93%93$');
print(x);
// CHECK-NEXT: ^📓$
print(x.length);
// CHECK-NEXT: 4
print(x.charCodeAt(0), x.charCodeAt(1), x.charCodeAt(2), x.charCodeAt(3));
// CHECK-NEXT: 94 55357 56531 36
try { print(decodeURI('%NO')); } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught URIError
try { print(decodeURI('%fa')); } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught URIError
try { print(decodeURI('%c3%f0')); } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught URIError
try { print(decodeURI('%c3%NO')); } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught URIError
try { print(decodeURI('%'+String.fromCharCode(23)+'1')); }
catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught URIError

print('decodeURIComponent');
// CHECK-LABEL: decodeURIComponent
print(decodeURIComponent('https://.com/%23sec-15.1.3.3'));
// CHECK-NEXT: https://.com/#sec-15.1.3.3
print(decodeURIComponent('https://.com/%23sec-15.1.3.3%3f'));
// CHECK-NEXT: https://.com/#sec-15.1.3.3?
print(decodeURIComponent('a%20b'));
// CHECK-NEXT: a b
print(decodeURIComponent('%CF%B1'));
// CHECK-NEXT: ϱ
var x = decodeURIComponent('%5E%F0%9F%93%93%24');
print(x);
// CHECK-NEXT: ^📓$
print(x.length);
// CHECK-NEXT: 4
print(x.charCodeAt(0), x.charCodeAt(1), x.charCodeAt(2), x.charCodeAt(3));
// CHECK-NEXT: 94 55357 56531 36
try { print(decodeURIComponent('%NO')); } catch (e) { print('caught', e) }
// CHECK-NEXT: caught URIError: {{.*}}
try { print(decodeURIComponent('%fa')); } catch (e) { print('caught', e) }
// CHECK-NEXT: caught URIError: {{.*}}
try { print(decodeURIComponent('%c3%f0')); } catch (e) { print('caught', e) }
// CHECK-NEXT: caught URIError: {{.*}}
try { print(decodeURIComponent('%c3%NO')); } catch (e) { print('caught', e) }
// CHECK-NEXT: caught URIError: {{.*}}
