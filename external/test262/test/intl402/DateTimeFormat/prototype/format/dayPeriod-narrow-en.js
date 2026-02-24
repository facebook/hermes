// Copyright 2019 Leo Balter. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-createdatetimeformat
description: Checks basic handling of dayPeriod, narrow format.
features: [Intl.DateTimeFormat-dayPeriod]
locale: [en-US]
---*/

const d0000 = new Date(2017, 11, 12,  0, 0, 0, 0);
const d0100 = new Date(2017, 11, 12,  1, 0, 0, 0);
const d0200 = new Date(2017, 11, 12,  2, 0, 0, 0);
const d0300 = new Date(2017, 11, 12,  3, 0, 0, 0);
const d0400 = new Date(2017, 11, 12,  4, 0, 0, 0);
const d0500 = new Date(2017, 11, 12,  5, 0, 0, 0);
const d0600 = new Date(2017, 11, 12,  6, 0, 0, 0);
const d0700 = new Date(2017, 11, 12,  7, 0, 0, 0);
const d0800 = new Date(2017, 11, 12,  8, 0, 0, 0);
const d0900 = new Date(2017, 11, 12,  9, 0, 0, 0);
const d1000 = new Date(2017, 11, 12, 10, 0, 0, 0);
const d1100 = new Date(2017, 11, 12, 11, 0, 0, 0);
const d1200 = new Date(2017, 11, 12, 12, 0, 0, 0);
const d1300 = new Date(2017, 11, 12, 13, 0, 0, 0);
const d1400 = new Date(2017, 11, 12, 14, 0, 0, 0);
const d1500 = new Date(2017, 11, 12, 15, 0, 0, 0);
const d1600 = new Date(2017, 11, 12, 16, 0, 0, 0);
const d1700 = new Date(2017, 11, 12, 17, 0, 0, 0);
const d1800 = new Date(2017, 11, 12, 18, 0, 0, 0);
const d1900 = new Date(2017, 11, 12, 19, 0, 0, 0);
const d2000 = new Date(2017, 11, 12, 20, 0, 0, 0);
const d2100 = new Date(2017, 11, 12, 21, 0, 0, 0);
const d2200 = new Date(2017, 11, 12, 22, 0, 0, 0);
const d2300 = new Date(2017, 11, 12, 23, 0, 0, 0);

const narrow = new Intl.DateTimeFormat('en', {
  dayPeriod: 'narrow'
});

assert.sameValue(narrow.format(d0000), 'at night', '00:00, narrow format');
assert.sameValue(narrow.format(d0100), 'at night', '01:00, narrow format');
assert.sameValue(narrow.format(d0200), 'at night', '02:00, narrow format');
assert.sameValue(narrow.format(d0300), 'at night', '03:00, narrow format');
assert.sameValue(narrow.format(d0400), 'at night', '04:00, narrow format');
assert.sameValue(narrow.format(d0500), 'at night', '05:00, narrow format');
assert.sameValue(narrow.format(d0600), 'in the morning', '06:00, narrow format');
assert.sameValue(narrow.format(d0700), 'in the morning', '07:00, narrow format');
assert.sameValue(narrow.format(d0800), 'in the morning', '08:00, narrow format');
assert.sameValue(narrow.format(d0900), 'in the morning', '09:00, narrow format');
assert.sameValue(narrow.format(d1000), 'in the morning', '10:00, narrow format');
assert.sameValue(narrow.format(d1100), 'in the morning', '11:00, narrow format');
assert.sameValue(narrow.format(d1200), 'n', '12:00, narrow format');
assert.sameValue(narrow.format(d1300), 'in the afternoon', '13:00, narrow format');
assert.sameValue(narrow.format(d1400), 'in the afternoon', '14:00, narrow format');
assert.sameValue(narrow.format(d1500), 'in the afternoon', '15:00, narrow format');
assert.sameValue(narrow.format(d1600), 'in the afternoon', '16:00, narrow format');
assert.sameValue(narrow.format(d1700), 'in the afternoon', '17:00, narrow format');
assert.sameValue(narrow.format(d1800), 'in the evening', '18:00, narrow format');
assert.sameValue(narrow.format(d1900), 'in the evening', '19:00, narrow format');
assert.sameValue(narrow.format(d2000), 'in the evening', '20:00, narrow format');
assert.sameValue(narrow.format(d2100), 'at night', '21:00, narrow format');
assert.sameValue(narrow.format(d2200), 'at night', '22:00, narrow format');
assert.sameValue(narrow.format(d2300), 'at night', '23:00, narrow format');

const narrowNumeric = new Intl.DateTimeFormat('en', {
  dayPeriod: 'narrow',
  hour: 'numeric'
});

assert.sameValue(narrowNumeric.format(d0000), '12 at night', '00:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d0100), '1 at night', '01:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d0200), '2 at night', '02:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d0300), '3 at night', '03:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d0400), '4 at night', '04:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d0500), '5 at night', '05:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d0600), '6 in the morning', '06:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d0700), '7 in the morning', '07:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d0800), '8 in the morning', '08:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d0900), '9 in the morning', '09:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d1000), '10 in the morning', '10:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d1100), '11 in the morning', '11:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d1200), '12 n', '12:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d1300), '1 in the afternoon', '13:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d1400), '2 in the afternoon', '14:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d1500), '3 in the afternoon', '15:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d1600), '4 in the afternoon', '16:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d1700), '5 in the afternoon', '17:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d1800), '6 in the evening', '18:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d1900), '7 in the evening', '19:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d2000), '8 in the evening', '20:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d2100), '9 at night', '21:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d2200), '10 at night', '22:00, narrow-numeric');
assert.sameValue(narrowNumeric.format(d2300), '11 at night', '23:00, narrow-numeric');
