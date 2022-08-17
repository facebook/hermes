/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ="MST+7" %hermes -O %s | %FileCheck --match-full-lines %s
"use strict";

// Tests are being run under Mountain Standard Time, with DST disabled (TZ var).
// This allows for consistent results, the tests won't break every year.

// Note: the offset for MST is the same as that of Pacific Daylight Time.

print('Date');
// CHECK-LABEL: Date
print((new Date(2017, 2, 15, 15, 1, 37, 243)).getTime());
// CHECK-NEXT: 1489615297243
print((new Date(2017, 2, 15, 15, 1, 37, 243)).valueOf());
// CHECK-NEXT: 1489615297243
print((new Date(69, 6, 20, 20, 18)).getTime());
// CHECK-NEXT: -14157720000
print((new Date(84, 5)).getTime());
// CHECK-NEXT: 454921200000
print((new Date(446947200000)).getTime());
// CHECK-NEXT: 446947200000
print((new Date(0)).getTime());
// CHECK-NEXT: 0
print((new Date(1e81)).getTime());
// CHECK-NEXT: NaN
print((new Date(1e81, "")).getTime());
// CHECK-NEXT: NaN
print(new Date(2017, 0, 0).getTime() === new Date(2016, 11, 31).getTime());
// CHECK-NEXT: true
print(new Date(2017, 0, 1).getTime() === new Date(2016, 12, 1).getTime());
// CHECK-NEXT: true
print(Date());
// CHECK-NEXT: {{... ... .. .... ..:..:.. GMT.....}}
// {{... ... [0-9]{2} [0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2} GMT[-+][0-9]{4} }}
print(typeof Date());
// CHECK-NEXT: string
var lb = new Date().toString();
var a = Date();
var ub = new Date().toString();
print(Date.parse(lb) <= Date.parse(a));
// CHECK-NEXT: true
print(Date.parse(a) <= Date.parse(ub));
// CHECK-NEXT: true
var d = new Date(1929328);
d.toString = function() {throw new Error()};
d.valueOf = function() {throw new Error()};
var d2 = new Date(d);
print(d.getTime(), d2.getTime());
// CHECK-NEXT: 1929328 1929328
try {Date.prototype.setDate(123);} catch(e) {print('caught', e.name)}
// CHECK-NEXT: caught TypeError

print('toString');
// CHECK-LABEL: toString
print(new Date(112).toString());
// CHECK-NEXT: Wed Dec 31 1969 17:00:00 GMT-0700
print(new Date(2017, 2, 15, 15, 1, 37, 243).toString());
// CHECK-NEXT: Wed Mar 15 2017 15:01:37 GMT-0700
print(new Date(123123, 2, 15, 15, 1, 37, 243).toString());
// CHECK-NEXT: Thu Mar 15 123123 15:01:37 GMT-0700
print(new Date(-1, 2, 15, 15, 1, 37, 243).toString());
// CHECK-NEXT: Mon Mar 15 -0001 15:01:37 GMT-0700
print(new Date(NaN, 2, 15, 15, 1, 37, 243).toString());
// CHECK-NEXT: Invalid Date
print(new Date('2016T12:30').toString());
// CHECK-NEXT: Fri Jan 01 2016 12:30:00 GMT-0700

print('@@toPrimitive');
// CHECK-LABEL: @@toPrimitive
print('"' + new Date(112)[Symbol.toPrimitive].name + '"');
// CHECK-NEXT: "[Symbol.toPrimitive]"
print(new Date(112)[Symbol.toPrimitive]('string'));
// CHECK-NEXT: Wed Dec 31 1969 17:00:00 GMT-0700
print(new Date(112)[Symbol.toPrimitive]('default'));
// CHECK-NEXT: Wed Dec 31 1969 17:00:00 GMT-0700
print(new Date(112)[Symbol.toPrimitive]('number'));
// CHECK-NEXT: 112
try {
  print(new Date(112)[Symbol.toPrimitive](5));
} catch (e) {
  print('caught', e.name)
}
// CHECK-NEXT: caught TypeError

print('toISOString');
// CHECK-LABEL: toISOString
print(new Date(2017, 2, 15, 15, 1, 37, 243).toISOString());
// CHECK-NEXT: 2017-03-15T22:01:37.243Z
try {
  print(new Date(Infinity).toISOString());
} catch (e) {
  print(e);
}
// CHECK-NEXT: RangeError: {{.*}}

print('toUTCString');
// CHECK-LABEL: toUTCString
print(new Date(2017, 2, 15, 15, 1, 37, 243).toUTCString());
// CHECK-NEXT: Wed, 15 Mar 2017 22:01:37 GMT
print(new Date(-1, 0, 0, 0, 0, 0).toUTCString());
// CHECK-NEXT: Thu, 31 Dec -0002 07:00:00 GMT

print('toJSON');
// CHECK-LABEL: toJSON
print(new Date('2016T12:30').toJSON());
// CHECK-NEXT: 2016-01-01T19:30:00.000Z
print(Date.prototype.toJSON.call({valueOf: function() {return Infinity;}}));
// CHECK-NEXT: null
try {
  print(Date.prototype.toJSON.call({valueOf: function() {return 1;}}));
} catch (e) {
  print(e);
}
// CHECK-NEXT: TypeError: {{.*}}

print('toDateString');
// CHECK-LABEL: toDateString
print(new Date(112).toDateString());
// CHECK-NEXT: Wed Dec 31 1969
print(new Date(2017, 2, 15, 15, 1, 37, 243).toDateString());
// CHECK-NEXT: Wed Mar 15 2017
print(new Date(123123, 2, 15, 15, 1, 37, 243).toDateString());
// CHECK-NEXT: Thu Mar 15 123123
print(new Date(-1, 2, 15, 15, 1, 37, 243).toDateString());
// CHECK-NEXT: Mon Mar 15 -0001

print('toTimeString');
// CHECK-LABEL: toTimeString
print(new Date(112).toTimeString());
// CHECK-NEXT: 17:00:00 GMT-0700
print(new Date(2017, 2, 15, 15, 1, 37, 243).toTimeString());
// CHECK-NEXT: 15:01:37 GMT-0700

print('UTC');
// CHECK-LABEL: UTC
print(Date.UTC(1970));
// CHECK-NEXT: 0
print(Date.UTC(1971));
// CHECK-NEXT: 31536000000
print(Date.UTC(12, 3));
// CHECK-NEXT: -1822521600000
print(Date.UTC(2017, 2, 15, 15, 1, 37, 243));
// CHECK-NEXT: 1489590097243

print('now');
// CHECK-LABEL: now
print(Date.now());
// CHECK-NEXT: {{[0-9]+}}

print('parse');
// CHECK-LABEL: parse
var x = new Date(Math.floor(new Date().valueOf() / 1000) * 1000); // 0 ms
print(Date.parse(x.toString()) == x.valueOf());
// CHECK-NEXT: true
print(Date.parse(x.toISOString()) == x.valueOf());
// CHECK-NEXT: true
print(Date.parse(x.toUTCString()) == x.valueOf());
// CHECK-NEXT: true
print(Date.parse('asdf'));
// CHECK-NEXT: NaN
print(Date.parse('ĥ'));
// CHECK-NEXT: NaN
print(Date.parse('99999999999999999999999999'));
// CHECK-NEXT: NaN
print(Date.parse('99999999999999999999999999-12-13'));
// CHECK-NEXT: NaN
print(Date.parse('2016T12:30asdf'));
// CHECK-NEXT: NaN
print(Date.parse('2016T12:30Z-07:00'));
// CHECK-NEXT: NaN
print(Date.parse('2016T12:30-07'));
// CHECK-NEXT: NaN
print(Date.parse('2016-+01T12:30Z'));
// CHECK-NEXT: NaN
print(Date.parse('2016T12:30:00.000-07:00asdf'));
// CHECK-NEXT: NaN
print(Date.parse('2016T12:30Z'));
// CHECK-NEXT: 1451651400000
print(Date.parse('2016-01T12:30Z'));
// CHECK-NEXT: 1451651400000
print(Date.parse('2016-01-01T12:30Z'));
// CHECK-NEXT: 1451651400000
print(Date.parse('2016T12:30:00Z'));
// CHECK-NEXT: 1451651400000
print(Date.parse('2016T12:30:00.000Z'));
// CHECK-NEXT: 1451651400000
print(Date.parse('2016'));
// CHECK-NEXT: 1451606400000
print(Date.parse('2016T12:30'));
// CHECK-NEXT: 1451676600000
print(Date.parse('2016T12:30:00.000-07:00'));
// CHECK-NEXT: 1451676600000
print(Date.parse('2016T12:30:47.1-07:00'));
// CHECK-NEXT: 1451676647100
print(Date.parse('2016T12:30:47.1-0700'));
// CHECK-NEXT: 1451676647100
print(Date.parse('2016T12:30:47.12-07:00'));
// CHECK-NEXT: 1451676647120
print(Date.parse('2016T12:30:47.12-0700'));
// CHECK-NEXT: 1451676647120
print(Date.parse('2016T12:30:47.123-07:00'));
// CHECK-NEXT: 1451676647123
print(Date.parse('2016T12:30:47.123-0700'));
// CHECK-NEXT: 1451676647123
print(Date.parse('2016T12:30:47.1234-07:00'));
// CHECK-NEXT: 1451676647123
print(Date.parse('2016T12:30:47.1234-0700'));
// CHECK-NEXT: 1451676647123
print(Date.parse('2016T12:30:47.760738998-07:00'));
// CHECK-NEXT: 1451676647760
print(Date.parse('2016T12:30:47.760738998-0700'));
// CHECK-NEXT: 1451676647760
print(Date.parse('2016T12:30:47.760-07:00') === Date.parse('2016T12:30:47.760738998-07:00'));
+// CHECK-NEXT: true
print(Date.parse('Tue Jul 16 2019 13:15:25 GMT-0700 (Pacific Daylight Time)'));
// CHECK-NEXT: 1563308125000
print(Date.parse('Tue Jul 16 2019 13:15:25 GMT-0700'));
// CHECK-NEXT: 1563308125000
print(Date.parse('Tue Jul 16 2019 13:15:25 PDT'));
// CHECK-NEXT: 1563308125000
print(Date.parse('Tue Jul 16 2019 13:15:25 PST'));
// CHECK-NEXT: 1563311725000
print(Date.parse('Tue Jul 16 2019 13:15:25 MDT'));
// CHECK-NEXT: 1563304525000
print(Date.parse('Tue Jul 16 2019 13:15:25 MST'));
// CHECK-NEXT: 1563308125000
print(Date.parse('Tue Jul 16 2019 13:15:25 CDT'));
// CHECK-NEXT: 1563300925000
print(Date.parse('Tue Jul 16 2019 13:15:25 CST'));
// CHECK-NEXT: 1563304525000
print(Date.parse('Tue Jul 16 2019 13:15:25 EDT'));
// CHECK-NEXT: 1563297325000
print(Date.parse('Tue Jul 16 2019 13:15:25 EST'));
// CHECK-NEXT: 1563300925000
print(Date.parse('Tue Jul 16 2019 13:15:25 EST+0700'));
// CHECK-NEXT: NaN
print(Date.parse('Tue Jul 16 2019 13:15:25 -0700'));
// CHECK-NEXT: 1563308125000
print(Date.parse('Tue Jul 16 2019 13:15:25 GMT'));
// CHECK-NEXT: 1563282925000
print(Date.parse('Tue, 16 Jul 2019 13:15:25 GMT-0700'));
// CHECK-NEXT: 1563308125000
print(Date.parse('Tue, 16 Jul 2019 13:15:25 -0700'));
// CHECK-NEXT: 1563308125000
print(Date.parse('Tue, 16 Jul 2019 13:15:25 GMT'));
// CHECK-NEXT: 1563282925000
print(Date.parse('Wat Jul 16 2019 13:15:25 GMT-0700'));
// CHECK-NEXT: NaN
print(Date.parse('Mon Jul 16 2019 13:1525 GMT-0700'));
// CHECK-NEXT: NaN
print(Date.parse('Mon Jul 16 2019 13:1525 GMT'));
// CHECK-NEXT: NaN
print(Date.parse('Mon Jul 16 2019 00:00:00'));
// CHECK-NEXT: 1563260400000
print(Date.parse('Mon Jul 16 2019'));
// CHECK-NEXT: 1563260400000
print(Date.parse('2021-04-10T01:00:00.000-01:30'));
// CHECK-NEXT: 1618021800000
print(Date.parse('2021-04-10T01:00:00.000-0130'));
// CHECK-NEXT: 1618021800000

// Fault tolerance on garbages (marked as "G"s).
print('Fault tolerance');
// CHECK-LABEL: Fault tolerance
print(Date.parse('TueG 05 May 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue G G 05 May 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue ,, G ,, 05 May 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue 05 MayG 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue 05 May G 2020 00:00:00'));
// CHECK-NEXT: NaN
print(Date.parse('TueG May 05 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue G May 05 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue ,, G ,, May 05 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue MayG 05 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue May G 05 2020 00:00:00'));
// CHECK-NEXT: NaN

// Fault tolerance on spaces.
print(Date.parse('Tue  05 May 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue 05  May 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue 05 May  2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue 05 May 2020  00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue  May 05 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue May  05 2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue May 05  2020 00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue May 05 2020  00:00:00'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue May 05 2020 00:00:00  PDT'));
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue May 05 2020 00:00:00'.padEnd(60)));  // trailing spaces.
// CHECK-NEXT: 1588662000000
print(Date.parse('Tue May 05 2020'.padEnd(60)));
// CHECK-NEXT: 1588662000000

// Quick check that getters work; internal functions are unit tested instead.
print('getters');
// CHECK-LABEL: getters
var x = new Date('2016-02-15T18:03:57.263-07:00');
print(x.getTime());
// CHECK-NEXT: 1455584637263
print(x.getFullYear(), x.getMonth(), x.getDate(), x.getDay());
// CHECK-NEXT: 2016 1 15 1
print(x.getYear());
// CHECK-NEXT: 116
print(x.getUTCFullYear(), x.getUTCMonth(), x.getUTCDate(), x.getUTCDay());
// CHECK-NEXT: 2016 1 16 2
print(x.getHours(), x.getMinutes());
// CHECK-NEXT: 18 3
print(x.getUTCHours(), x.getUTCMinutes());
// CHECK-NEXT: 1 3
print(x.getSeconds(), x.getMilliseconds());
// CHECK-NEXT: 57 263
print(x.getUTCSeconds(), x.getUTCMilliseconds());
// CHECK-NEXT: 57 263
print(x.getTimezoneOffset());
// CHECK-NEXT: 420
var x = new Date('invalid date');
print(x.getFullYear());
// CHECK-NEXT: NaN

print('setters');
// CHECK-LABEL: setters
var t = 1468631037263;
var x = new Date(t);
print(x.setTime(123456789), x.getTime());
// CHECK-NEXT: 123456789 123456789
var x = new Date(t);
print(x.setMilliseconds(123), x.getTime());
// CHECK-NEXT: 1468631037123 1468631037123
var x = new Date(t);
print(x.setUTCMilliseconds(123), x.getTime());
// CHECK-NEXT: 1468631037123 1468631037123
var x = new Date(t);
print(x.setSeconds(1), x.getTime());
// CHECK-NEXT: 1468630981263 1468630981263
print(x.setSeconds(1, 123), x.getTime());
// CHECK-NEXT: 1468630981123 1468630981123
var x = new Date(t);
print(x.setUTCSeconds(1), x.getTime());
// CHECK-NEXT: 1468630981263 1468630981263
var x = new Date(t);
print(x.setMinutes(12), x.getTime());
// CHECK-NEXT: 1468631577263 1468631577263
var x = new Date(t);
print(x.setMinutes(12, 1, 123), x.getTime());
// CHECK-NEXT: 1468631521123 1468631521123
var x = new Date(t);
print(x.setUTCMinutes(12), x.getTime());
// CHECK-NEXT: 1468631577263 1468631577263
var x = new Date(t);
print(x.setHours(20), x.getTime());
// CHECK-NEXT: 1468638237263 1468638237263
var x = new Date(t);
print(x.setHours(20, 12, 1, 123), x.getTime());
// CHECK-NEXT: 1468638721123 1468638721123
var x = new Date(t);
print(x.setUTCHours(20), x.getTime());
// CHECK-NEXT: 1468699437263 1468699437263
var x = new Date(t);
print(x.setDate(25), x.getTime());
// CHECK-NEXT: 1469495037263 1469495037263
var x = new Date(t);
print(x.setUTCDate(25), x.getTime());
// CHECK-NEXT: 1469408637263 1469408637263
var x = new Date(t);
print(x.setMonth(8), x.getTime());
// CHECK-NEXT: 1473987837263 1473987837263
var x = new Date(t);
print(x.setMonth(8, 25), x.getTime());
// CHECK-NEXT: 1474851837263 1474851837263
var x = new Date(t);
print(x.setUTCMonth(8), x.getTime());
// CHECK-NEXT: 1473987837263 1473987837263
var x = new Date(t);
print(x.setFullYear(2001), x.getTime());
// CHECK-NEXT: 995245437263 995245437263
var x = new Date(t);
print(x.setFullYear(2001, 8, 25), x.getTime());
// CHECK-NEXT: 1001466237263 1001466237263
var x = new Date(t);
print(x.setUTCFullYear(2001), x.getTime());
// CHECK-NEXT: 995245437263 995245437263
var x = new Date(t);
print(x.setYear(2001), x.getTime());
// CHECK-NEXT: 995245437263 995245437263
var x = new Date(t);
print(x.setYear(95), x.getTime());
// CHECK-NEXT: 805856637263 805856637263
