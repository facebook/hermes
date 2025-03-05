/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ=GMT %hermes %s
// REQUIRES: intl
// UNSUPPORTED: apple

// Apple implementation does not support formatRange and formatRangeToParts

function assert(pred, str) {
  if (!pred) {
    throw new Error('assertion failed' + (str === undefined ? '' : ': ' + str));
  }
}

function normalize(str) {
  // Normalize all white space characters to ASCII space.
  return str.replace(/\s/g, ' ');
}

function verifyParts(testName, expected, actual) {
  expected.forEach((expectedPart, index) => {
    const actualPart = actual.at(index);
    assert(
      actualPart,
      `${testName} failed, missing part '${JSON.stringify(expectedPart)}':
   expected='${JSON.stringify(expected)}',
   actual='${JSON.stringify(actual)}'`
    );
    for (const [key, value] of Object.entries(expectedPart)) {
      assert(
        Object.hasOwn(actualPart, key),
        `${testName} failed, missing key '${key}' in a part:
           expectedPart='${JSON.stringify(expectedPart)}'
           actualPart='${JSON.stringify(actualPart)}'
           expected='${JSON.stringify(expected)}'
           actual='${JSON.stringify(actual)}'`
      );
      const actualPartValue = actualPart[key];
      const pass =
        value instanceof RegExp ? value.test(actualPartValue) : normalize(value) === normalize(actualPartValue);
      assert(
        pass,
        `${testName} failed, for key='${key}', expected='${value}', actual='${actualPartValue}', in a part:
           expectedPart='${JSON.stringify(expectedPart)}'
           actualPart='${JSON.stringify(actualPart)}'
           expected='${JSON.stringify(expected)}'
           actual='${JSON.stringify(actual)}'`
      );
    }
  });
}

function testFormatRange(testName, expected, locales, options, startDate, endDate) {
  const actual = new Intl.DateTimeFormat(locales, options).formatRange(startDate, endDate);
  const expectedResult = typeof expected === 'function' ? expected(actual) : expected;
  const pass =
    expectedResult instanceof RegExp ? expectedResult.test(actual) : normalize(expectedResult) === normalize(actual);
  assert(pass, `${testName} failed, expected='${expectedResult}', actual='${actual}'`);
}

function testFormatRangeToParts(testName, expected, locales, options, startDate, endDate) {
  const actual = new Intl.DateTimeFormat(locales, options).formatRangeToParts(startDate, endDate);
  const expectedResult = typeof expected === 'function' ? expected(actual) : expected;
  verifyParts(testName, expectedResult, actual);
}

let testName = 'Test-All-Numeric';
const startDate = new Date(Date.UTC(1990, 0, 10, 5, 20, 0)); // numeric input
const endDate = new Date(Date.UTC(2020, 5, 20, 11, 30, 0)); // numeric input
let expected = '1/10/1990, 5:20 AM – 6/20/2020, 11:30 AM';
let locales = 'en-US';
let options = {
  year: 'numeric',
  month: 'numeric',
  day: 'numeric',
  hour: 'numeric',
  minute: 'numeric',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: '1', type: 'month' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '10', type: 'day' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '1990', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '5', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '20', type: 'minute' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'AM', type: 'dayPeriod' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: '6', type: 'month' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '20', type: 'day' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2020', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '11', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '30', type: 'minute' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'AM', type: 'dayPeriod' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-All-2-digit-Era-WkDay-DayPeriod-Narrow';
startDate = new Date(Date.UTC(1990, 0, 5, 11, 30, 0)); // numeric input
endDate = new Date(Date.UTC(2050, 11, 25, 18, 45, 0)); // numeric input
expected = 'F, 05 01 90 A, 11:30 in the morning – S, 25 12 50 A, 6:45 in the evening';
locales = 'en-IN';
options = {
  era: 'narrow',
  weekday: 'narrow',
  year: '2-digit',
  month: '2-digit',
  day: '2-digit',
  hour: '2-digit',
  minute: '2-digit',
  dayPeriod: 'narrow',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: 'F', type: 'weekday' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '05', type: 'day' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: '01', type: 'month' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: '90', type: 'year' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'A', type: 'era' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '11', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '30', type: 'minute' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'in the morning', type: 'dayPeriod' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: 'S', type: 'weekday' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '25', type: 'day' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: '12', type: 'month' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: '50', type: 'year' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'A', type: 'era' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '6', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '45', type: 'minute' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'in the evening', type: 'dayPeriod' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-All-Short-Yr-2Digit';
startDate = new Date(Date.UTC(1950, 2, 10, 14, 30, 0)); // numeric input
endDate = new Date(Date.UTC(2040, 10, 30, 15, 45, 0)); // numeric input
expected = 'Mar 50 AD Fri, UTC ├am/pm: in the afternoon┤ – Nov 40 AD Fri, UTC ├am/pm: in the afternoon┤';
locales = 'en-GB';
options = {
  era: 'short',
  weekday: 'short',
  month: 'short',
  year: '2-digit',
  dayPeriod: 'short',
  timeZoneName: 'short',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: 'Mar', type: 'month' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: '50', type: 'year' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'AD', type: 'era' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Fri', type: 'weekday' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: 'UTC', type: 'timeZoneName' },
  { source: 'startRange', value: ' ├am/pm: ', type: 'literal' },
  { source: 'startRange', value: 'in the afternoon', type: 'dayPeriod' },
  { source: 'shared', value: '┤ – ', type: 'literal' },
  { source: 'endRange', value: 'Nov', type: 'month' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: '40', type: 'year' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'AD', type: 'era' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Fri', type: 'weekday' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: 'UTC', type: 'timeZoneName' },
  { source: 'endRange', value: ' ├am/pm: ', type: 'literal' },
  { source: 'endRange', value: 'in the afternoon', type: 'dayPeriod' },
  { source: 'shared', value: '┤', type: 'literal' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-All-Narrow-Yr-Numeric-Tz-ShortOffset';
startDate = new Date(Date.UTC(2000, 5, 20, 12, 15, 0)); // numeric input
endDate = new Date(Date.UTC(2025, 8, 25, 18, 30, 0)); // numeric input
expected = 'J 2000 A T, GMT ├AM/PM: n┤ – S 2025 A T, GMT ├AM/PM: in the evening┤';
locales = 'en';
options = {
  era: 'narrow',
  weekday: 'narrow',
  month: 'narrow',
  year: 'numeric',
  dayPeriod: 'narrow',
  timeZoneName: 'shortOffset',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: 'J', type: 'month' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: '2000', type: 'year' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'A', type: 'era' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'T', type: 'weekday' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: 'GMT', type: 'timeZoneName' },
  { source: 'startRange', value: ' ├AM/PM: ', type: 'literal' },
  { source: 'startRange', value: 'n', type: 'dayPeriod' },
  { source: 'shared', value: '┤ – ', type: 'literal' },
  { source: 'endRange', value: 'S', type: 'month' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: '2025', type: 'year' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'A', type: 'era' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'T', type: 'weekday' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: 'GMT', type: 'timeZoneName' },
  { source: 'endRange', value: ' ├AM/PM: ', type: 'literal' },
  { source: 'endRange', value: 'in the evening', type: 'dayPeriod' },
  { source: 'shared', value: '┤', type: 'literal' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-All-Long';
startDate = new Date(Date.UTC(1900, 1, 15, 4, 20, 0)); // numeric input
endDate = new Date(Date.UTC(2000, 6, 30, 11, 40, 0)); // numeric input
expected =
  /^February 00 Anno Domini Thursday(?:,| at) Coordinated Universal Time ├AM\/PM: at night┤\s–\sJuly 00 Anno Domini Sunday(?:,| at) Coordinated Universal Time ├AM\/PM: in the morning┤$/;
locales = 'en-US';
options = {
  era: 'long',
  weekday: 'long',
  month: 'long',
  year: '2-digit',
  dayPeriod: 'long',
  timeZoneName: 'long',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: 'February', type: 'month' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: '00', type: 'year' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Anno Domini', type: 'era' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Thursday', type: 'weekday' },
  { source: 'startRange', value: /^(?:,| at) $/, type: 'literal' },
  { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'startRange', value: ' ├AM/PM: ', type: 'literal' },
  { source: 'startRange', value: 'at night', type: 'dayPeriod' },
  { source: 'shared', value: '┤ – ', type: 'literal' },
  { source: 'endRange', value: 'July', type: 'month' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: '00', type: 'year' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Anno Domini', type: 'era' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Sunday', type: 'weekday' },
  { source: 'endRange', value: /^(?:,| at) $/, type: 'literal' },
  { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'endRange', value: ' ├AM/PM: ', type: 'literal' },
  { source: 'endRange', value: 'in the morning', type: 'dayPeriod' },
  { source: 'shared', value: '┤', type: 'literal' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-All-Default';
startDate = new Date(Date.UTC(1980, 5, 10, 2, 15, 0)); // numeric input
endDate = new Date(Date.UTC(2020, 11, 20, 10, 45, 0)); // numeric input
expected = '6/10/1980 – 12/20/2020';
locales = 'en-US';
options = {};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: '6', type: 'month' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '10', type: 'day' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '1980', type: 'year' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: '12', type: 'month' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '20', type: 'day' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2020', type: 'year' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Hour-DayPeriod-Timezone';
startDate = new Date(Date.UTC(2012, 11, 17, 12, 10, 0));
endDate = new Date(Date.UTC(2013, 12, 17, 1, 20, 0));
expected = /^17\/12\/2012, 1 (?:pm|in the afternoon)\s–\s17\/1\/2014, 2 (?:am|at night)$/;
locales = 'en-IN';
options = {
  hour: 'numeric',
  dayPeriod: 'short',
  timeZone: 'Europe/Paris',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: '17', type: 'day' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '12', type: 'month' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '2012', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '1', type: 'hour' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: /^(?:pm|in the afternoon)$/, type: 'dayPeriod' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: '17', type: 'day' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '1', type: 'month' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2014', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '2', type: 'hour' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: /^(?:am|at night)$/, type: 'dayPeriod' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-12HrLocale-DateStyle-TimeStyle';
startDate = new Date(Date.UTC(1900, 1, 15, 14, 20, 0)); // numeric input
endDate = new Date(Date.UTC(2000, 6, 30, 16, 40, 0)); // numeric input
expected = 'Feb 15, 1900, 2:20:00 PM Coordinated Universal Time – Jul 30, 2000, 4:40:00 PM Coordinated Universal Time';
locales = 'en-US';
options = {
  dateStyle: 'medium',
  timeStyle: 'full',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: 'Feb', type: 'month' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: '15', type: 'day' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '1900', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '2', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '20', type: 'minute' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '00', type: 'second' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'PM', type: 'dayPeriod' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: 'Jul', type: 'month' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: '30', type: 'day' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '2000', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '4', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '40', type: 'minute' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '00', type: 'second' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'PM', type: 'dayPeriod' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-24HrLocale-DateStyle-TimeStyle';
startDate = new Date(Date.UTC(1900, 1, 15, 14, 20, 0)); // numeric input
endDate = new Date(Date.UTC(2000, 6, 30, 16, 40, 0)); // numeric input
expected =
  /^15\. februar 1900 (?:kl\. )?14\.20\.00 Koordineret universaltid-30\. juli 2000 (?:kl\. )?16\.40\.00 Koordineret universaltid$/;
locales = 'da-DK';
options = {
  dateStyle: 'long',
  timeStyle: 'full',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: '15', type: 'day' },
  { source: 'startRange', value: '. ', type: 'literal' },
  { source: 'startRange', value: 'februar', type: 'month' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: '1900', type: 'year' },
  { source: 'startRange', value: /^ (?:kl\. )?$/, type: 'literal' },
  { source: 'startRange', value: '14', type: 'hour' },
  { source: 'startRange', value: '.', type: 'literal' },
  { source: 'startRange', value: '20', type: 'minute' },
  { source: 'startRange', value: '.', type: 'literal' },
  { source: 'startRange', value: '00', type: 'second' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Koordineret universaltid', type: 'timeZoneName' },
  { source: 'shared', value: '-', type: 'literal' },
  { source: 'endRange', value: '30', type: 'day' },
  { source: 'endRange', value: '. ', type: 'literal' },
  { source: 'endRange', value: 'juli', type: 'month' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: '2000', type: 'year' },
  { source: 'endRange', value: /^ (?:kl\. )?$/, type: 'literal' },
  { source: 'endRange', value: '16', type: 'hour' },
  { source: 'endRange', value: '.', type: 'literal' },
  { source: 'endRange', value: '40', type: 'minute' },
  { source: 'endRange', value: '.', type: 'literal' },
  { source: 'endRange', value: '00', type: 'second' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Koordineret universaltid', type: 'timeZoneName' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Locale-DateStyle-TimeStyle-h11';
startDate = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
endDate = new Date(Date.UTC(2024, 1, 15, 0, 50, 0)); // numeric input
expected = (actual) => {
  if (actual.indexOf('12:40:00') >= 0) {
    // Before ICU 67.1, the hour cycle of UDateIntervalFormat cannot be forced
    // to one that differs from the locale's default 12 or 24-hour cycles.
    // See https://unicode-org.atlassian.net/browse/ICU-20887.
    print(`WARNING: ${testName} is using incorrect hour cycle expectation, only ok if ICU version is before 67.1.`);
    return 'Feb 15, 2023, 12:40:00 AM Coordinated Universal Time – Feb 15, 2024, 12:50:00 AM Coordinated Universal Time';
  }
  return 'Feb 15, 2023, 0:40:00 AM Coordinated Universal Time – Feb 15, 2024, 0:50:00 AM Coordinated Universal Time';
};
locales = 'en-US';
options = {
  dateStyle: 'medium',
  timeStyle: 'full',
  hourCycle: 'h11',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = (actual) => {
  for (const part of actual) {
    if (part.type === 'hour' && part.value === '12') {
      print(`WARNING: ${testName} is using incorrect hour cycle expectation, only ok if ICU version is before 67.1.`);
      return [
        { source: 'startRange', value: 'Feb', type: 'month' },
        { source: 'startRange', value: ' ', type: 'literal' },
        { source: 'startRange', value: '15', type: 'day' },
        { source: 'startRange', value: ', ', type: 'literal' },
        { source: 'startRange', value: '2023', type: 'year' },
        { source: 'startRange', value: ', ', type: 'literal' },
        { source: 'startRange', value: '12', type: 'hour' },
        { source: 'startRange', value: ':', type: 'literal' },
        { source: 'startRange', value: '40', type: 'minute' },
        { source: 'startRange', value: ':', type: 'literal' },
        { source: 'startRange', value: '00', type: 'second' },
        { source: 'startRange', value: ' ', type: 'literal' },
        { source: 'startRange', value: 'AM', type: 'dayPeriod' },
        { source: 'startRange', value: ' ', type: 'literal' },
        { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
        { source: 'shared', value: ' – ', type: 'literal' },
        { source: 'endRange', value: 'Feb', type: 'month' },
        { source: 'endRange', value: ' ', type: 'literal' },
        { source: 'endRange', value: '15', type: 'day' },
        { source: 'endRange', value: ', ', type: 'literal' },
        { source: 'endRange', value: '2024', type: 'year' },
        { source: 'endRange', value: ', ', type: 'literal' },
        { source: 'endRange', value: '12', type: 'hour' },
        { source: 'endRange', value: ':', type: 'literal' },
        { source: 'endRange', value: '50', type: 'minute' },
        { source: 'endRange', value: ':', type: 'literal' },
        { source: 'endRange', value: '00', type: 'second' },
        { source: 'endRange', value: ' ', type: 'literal' },
        { source: 'endRange', value: 'AM', type: 'dayPeriod' },
        { source: 'endRange', value: ' ', type: 'literal' },
        { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
      ];
    }
  }
  return [
    { source: 'startRange', value: 'Feb', type: 'month' },
    { source: 'startRange', value: ' ', type: 'literal' },
    { source: 'startRange', value: '15', type: 'day' },
    { source: 'startRange', value: ', ', type: 'literal' },
    { source: 'startRange', value: '2023', type: 'year' },
    { source: 'startRange', value: ', ', type: 'literal' },
    { source: 'startRange', value: '0', type: 'hour' },
    { source: 'startRange', value: ':', type: 'literal' },
    { source: 'startRange', value: '40', type: 'minute' },
    { source: 'startRange', value: ':', type: 'literal' },
    { source: 'startRange', value: '00', type: 'second' },
    { source: 'startRange', value: ' ', type: 'literal' },
    { source: 'startRange', value: 'AM', type: 'dayPeriod' },
    { source: 'startRange', value: ' ', type: 'literal' },
    { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
    { source: 'shared', value: ' – ', type: 'literal' },
    { source: 'endRange', value: 'Feb', type: 'month' },
    { source: 'endRange', value: ' ', type: 'literal' },
    { source: 'endRange', value: '15', type: 'day' },
    { source: 'endRange', value: ', ', type: 'literal' },
    { source: 'endRange', value: '2024', type: 'year' },
    { source: 'endRange', value: ', ', type: 'literal' },
    { source: 'endRange', value: '0', type: 'hour' },
    { source: 'endRange', value: ':', type: 'literal' },
    { source: 'endRange', value: '50', type: 'minute' },
    { source: 'endRange', value: ':', type: 'literal' },
    { source: 'endRange', value: '00', type: 'second' },
    { source: 'endRange', value: ' ', type: 'literal' },
    { source: 'endRange', value: 'AM', type: 'dayPeriod' },
    { source: 'endRange', value: ' ', type: 'literal' },
    { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  ];
};
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Locale-DateStyle-TimeStyle-h12';
startDate = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
endDate = new Date(Date.UTC(2024, 1, 15, 0, 50, 0)); // numeric input
expected =
  'Feb 15, 2023, 12:40:00 AM Coordinated Universal Time – Feb 15, 2024, 12:50:00 AM Coordinated Universal Time';
locales = 'en-US';
options = {
  dateStyle: 'medium',
  timeStyle: 'full',
  hourCycle: 'h12',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: 'Feb', type: 'month' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: '15', type: 'day' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '2023', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '12', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '40', type: 'minute' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '00', type: 'second' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'AM', type: 'dayPeriod' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: 'Feb', type: 'month' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: '15', type: 'day' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '2024', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '12', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '50', type: 'minute' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '00', type: 'second' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'AM', type: 'dayPeriod' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Locale-DateStyle-TimeStyle-h23';
startDate = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
endDate = new Date(Date.UTC(2024, 1, 15, 1, 50, 0)); // numeric input
expected = 'Feb 15, 2023, 00:40:00 Coordinated Universal Time – Feb 15, 2024, 01:50:00 Coordinated Universal Time';
locales = 'en-US';
options = {
  dateStyle: 'medium',
  timeStyle: 'full',
  hourCycle: 'h23',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: 'Feb', type: 'month' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: '15', type: 'day' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '2023', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '00', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '40', type: 'minute' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '00', type: 'second' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: 'Feb', type: 'month' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: '15', type: 'day' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '2024', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '01', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '50', type: 'minute' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '00', type: 'second' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Locale-DateStyle-TimeStyle-h24';
startDate = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
endDate = new Date(Date.UTC(2024, 1, 15, 1, 50, 0)); // numeric input
expected = (actual) => {
  if (actual.indexOf('00:40:00') >= 0) {
    // Before ICU 67.1, the hour cycle of UDateIntervalFormat cannot be forced
    // to one that differs from the locale's default 12 or 24-hour cycles.
    // See https://unicode-org.atlassian.net/browse/ICU-20887.
    print(`WARNING: ${testName} is using incorrect hour cycle expectation, only ok if ICU version is before 67.1.`);
    return 'Feb 15, 2023, 00:40:00 Coordinated Universal Time – Feb 15, 2024, 01:50:00 Coordinated Universal Time';
  }
  return 'Feb 15, 2023, 24:40:00 Coordinated Universal Time – Feb 15, 2024, 01:50:00 Coordinated Universal Time';
};
locales = 'en-US';
options = {
  dateStyle: 'medium',
  timeStyle: 'full',
  hourCycle: 'h24',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = (actual) => {
  for (const part of actual) {
    if (part.type === 'hour' && part.value === '00') {
      print(`WARNING: ${testName} is using incorrect hour cycle expectation, only ok if ICU version is before 67.1.`);
      return [
        { source: 'startRange', value: 'Feb', type: 'month' },
        { source: 'startRange', value: ' ', type: 'literal' },
        { source: 'startRange', value: '15', type: 'day' },
        { source: 'startRange', value: ', ', type: 'literal' },
        { source: 'startRange', value: '2023', type: 'year' },
        { source: 'startRange', value: ', ', type: 'literal' },
        { source: 'startRange', value: '00', type: 'hour' },
        { source: 'startRange', value: ':', type: 'literal' },
        { source: 'startRange', value: '40', type: 'minute' },
        { source: 'startRange', value: ':', type: 'literal' },
        { source: 'startRange', value: '00', type: 'second' },
        { source: 'startRange', value: ' ', type: 'literal' },
        { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
        { source: 'shared', value: ' – ', type: 'literal' },
        { source: 'endRange', value: 'Feb', type: 'month' },
        { source: 'endRange', value: ' ', type: 'literal' },
        { source: 'endRange', value: '15', type: 'day' },
        { source: 'endRange', value: ', ', type: 'literal' },
        { source: 'endRange', value: '2024', type: 'year' },
        { source: 'endRange', value: ', ', type: 'literal' },
        { source: 'endRange', value: '01', type: 'hour' },
        { source: 'endRange', value: ':', type: 'literal' },
        { source: 'endRange', value: '50', type: 'minute' },
        { source: 'endRange', value: ':', type: 'literal' },
        { source: 'endRange', value: '00', type: 'second' },
        { source: 'endRange', value: ' ', type: 'literal' },
        { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
      ];
    }
  }
  return [
    { source: 'startRange', value: 'Feb', type: 'month' },
    { source: 'startRange', value: ' ', type: 'literal' },
    { source: 'startRange', value: '15', type: 'day' },
    { source: 'startRange', value: ', ', type: 'literal' },
    { source: 'startRange', value: '2023', type: 'year' },
    { source: 'startRange', value: ', ', type: 'literal' },
    { source: 'startRange', value: '24', type: 'hour' },
    { source: 'startRange', value: ':', type: 'literal' },
    { source: 'startRange', value: '40', type: 'minute' },
    { source: 'startRange', value: ':', type: 'literal' },
    { source: 'startRange', value: '00', type: 'second' },
    { source: 'startRange', value: ' ', type: 'literal' },
    { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
    { source: 'shared', value: ' – ', type: 'literal' },
    { source: 'endRange', value: 'Feb', type: 'month' },
    { source: 'endRange', value: ' ', type: 'literal' },
    { source: 'endRange', value: '15', type: 'day' },
    { source: 'endRange', value: ', ', type: 'literal' },
    { source: 'endRange', value: '2024', type: 'year' },
    { source: 'endRange', value: ', ', type: 'literal' },
    { source: 'endRange', value: '01', type: 'hour' },
    { source: 'endRange', value: ':', type: 'literal' },
    { source: 'endRange', value: '50', type: 'minute' },
    { source: 'endRange', value: ':', type: 'literal' },
    { source: 'endRange', value: '00', type: 'second' },
    { source: 'endRange', value: ' ', type: 'literal' },
    { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  ];
};
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Hour12-with-True';
startDate = new Date(Date.UTC(2023, 1, 15, 15, 40, 0)); // numeric input
endDate = new Date(Date.UTC(2024, 1, 15, 18, 50, 0)); // numeric input
expected = '15/2/2023, 3:40:00 pm Coordinated Universal Time – 15/2/2024, 6:50:00 pm Coordinated Universal Time';
locales = 'en-IN';
options = {
  hour12: true,
  timeStyle: 'full',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: '15', type: 'day' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '2', type: 'month' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '2023', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '3', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '40', type: 'minute' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '00', type: 'second' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'pm', type: 'dayPeriod' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: '15', type: 'day' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2', type: 'month' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2024', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '6', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '50', type: 'minute' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '00', type: 'second' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'pm', type: 'dayPeriod' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Hour12-Override-h23';
startDate = new Date(Date.UTC(2023, 1, 15, 13, 40, 0)); // numeric input
endDate = new Date(Date.UTC(2024, 1, 15, 23, 50, 0)); // numeric input
expected = '15/2/2023, 1:40:00 pm Coordinated Universal Time – 15/2/2024, 11:50:00 pm Coordinated Universal Time';
locales = 'en-IN';
options = {
  hour12: true,
  timeStyle: 'full',
  hourCycle: 'h23',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: '15', type: 'day' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '2', type: 'month' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '2023', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '1', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '40', type: 'minute' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '00', type: 'second' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'pm', type: 'dayPeriod' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: '15', type: 'day' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2', type: 'month' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2024', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '11', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '50', type: 'minute' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '00', type: 'second' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'pm', type: 'dayPeriod' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Hour12-Override-h24';
startDate = new Date(Date.UTC(2023, 1, 15, 15, 40, 0)); // numeric input
endDate = new Date(Date.UTC(2024, 1, 15, 24, 50, 0)); // numeric input
expected = '15/02/2023, 3:40:00 pm Coordinated Universal Time – 16/02/2024, 12:50:00 am Coordinated Universal Time';
locales = 'en-GB';
options = {
  hour12: true,
  timeStyle: 'full',
  hourCycle: 'h24',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: '15', type: 'day' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '02', type: 'month' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '2023', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '3', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '40', type: 'minute' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '00', type: 'second' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'pm', type: 'dayPeriod' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: '16', type: 'day' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '02', type: 'month' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2024', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '12', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '50', type: 'minute' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '00', type: 'second' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'am', type: 'dayPeriod' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Hour12-with-False';
startDate = new Date(Date.UTC(2023, 1, 15, 15, 40, 0)); // numeric input
endDate = new Date(Date.UTC(2024, 1, 15, 18, 50, 0)); // numeric input
expected = '15/02/2023, 15:40:00 Coordinated Universal Time – 15/02/2024, 18:50:00 Coordinated Universal Time';
locales = 'en-GB';
options = {
  hour12: false,
  timeStyle: 'full',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: '15', type: 'day' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '02', type: 'month' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '2023', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '15', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '40', type: 'minute' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '00', type: 'second' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: '15', type: 'day' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '02', type: 'month' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2024', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '18', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '50', type: 'minute' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '00', type: 'second' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Hour12-Override-h11';
startDate = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
endDate = new Date(Date.UTC(2024, 1, 15, 11, 50, 0)); // numeric input
expected = '15/02/2023, 00:40:00 Coordinated Universal Time – 15/02/2024, 11:50:00 Coordinated Universal Time';
locales = 'en-GB';
options = {
  hour12: false,
  timeStyle: 'full',
  hourCycle: 'h11',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: '15', type: 'day' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '02', type: 'month' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '2023', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '00', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '40', type: 'minute' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '00', type: 'second' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: '15', type: 'day' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '02', type: 'month' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2024', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '11', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '50', type: 'minute' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '00', type: 'second' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

testName = 'Test-Hour12-Override-h12';
startDate = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
endDate = new Date(Date.UTC(2024, 1, 15, 24, 50, 0)); // numeric input
expected = '2/15/2023, 00:40:00 Coordinated Universal Time – 2/16/2024, 00:50:00 Coordinated Universal Time';
locales = 'en-US';
options = {
  hour12: false,
  timeStyle: 'full',
  hourCycle: 'h12',
};
testFormatRange(testName, expected, locales, options, startDate, endDate);
expected = [
  { source: 'startRange', value: '2', type: 'month' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '15', type: 'day' },
  { source: 'startRange', value: '/', type: 'literal' },
  { source: 'startRange', value: '2023', type: 'year' },
  { source: 'startRange', value: ', ', type: 'literal' },
  { source: 'startRange', value: '00', type: 'hour' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '40', type: 'minute' },
  { source: 'startRange', value: ':', type: 'literal' },
  { source: 'startRange', value: '00', type: 'second' },
  { source: 'startRange', value: ' ', type: 'literal' },
  { source: 'startRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
  { source: 'shared', value: ' – ', type: 'literal' },
  { source: 'endRange', value: '2', type: 'month' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '16', type: 'day' },
  { source: 'endRange', value: '/', type: 'literal' },
  { source: 'endRange', value: '2024', type: 'year' },
  { source: 'endRange', value: ', ', type: 'literal' },
  { source: 'endRange', value: '00', type: 'hour' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '50', type: 'minute' },
  { source: 'endRange', value: ':', type: 'literal' },
  { source: 'endRange', value: '00', type: 'second' },
  { source: 'endRange', value: ' ', type: 'literal' },
  { source: 'endRange', value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatRangeToParts(testName, expected, locales, options, startDate, endDate);

function expectException(testName, locales, options, expectedError, startDate, endDate) {
  let exceptionIsThrown = false;
  try {
    let actual = new Intl.DateTimeFormat(locales, options).formatRange(startDate, endDate);
  } catch (e) {
    exceptionIsThrown = true;
    assert(
      e.name === expectedError.name && e.message === expectedError.message,
      `test='${testName}' failed, expected='${expectedError}', actual='${e.toString()}'`
    );
  }
  assert(exceptionIsThrown, `${testName} failed, expected ${expectedError.name} exception was not thrown.`);
}

testName = 'Test-Invalid-Locale';
locales = ['es-ES', 'foobarbaz', 'es-US'];
options = {
  year: 'numeric',
  month: 'numeric',
  day: 'numeric',
  hour: 'numeric',
  minute: 'numeric',
};
expectedError = new RangeError('Invalid language tag: foobarbaz');
expectException(testName, locales, options, expectedError, startDate, endDate);

testName = 'Test-Invalid-Timezone';
options = { timeZone: 'FOO' };
locales = 'en-US';
expectedError = new RangeError('Invalid time zone: FOO');
expectException(testName, locales, options, expectedError, startDate, endDate);

options = { fractionalSecondDigits: 0 };
expectedError = new RangeError('fractionalSecondDigits value is invalid.');
expectException(testName, locales, options, expectedError, startDate, endDate);
