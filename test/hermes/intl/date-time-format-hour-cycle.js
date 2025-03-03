/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ=GMT %hermes %s
// REQUIRES: intl
// UNSUPPORTED: apple

// Apple implementation does not honor hour cycle options when
// dateStyle and/or timeStyle is specified.

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

function testFormat(testName, expected, locales, options, date) {
  let actual = new Intl.DateTimeFormat(locales, options).format(date);
  const pass = expected instanceof RegExp ? expected.test(actual) : normalize(expected) === normalize(actual);
  assert(pass, `${testName} failed, expected='${expected}', actual='${actual}'`);
}

function testFormatToParts(testName, expected, locales, options, date) {
  let actual = new Intl.DateTimeFormat(locales, options).formatToParts(date);
  verifyParts(testName, expected, actual);
}

let testName = 'Test-Locale-DateStyle-TimeStyle-11HrHourCycle';
let date = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
let expected = /^February 15, 2023(?:,| at) 0:40:00\sAM Coordinated Universal Time$/;
let locales = 'en-US';
let options = {
  dateStyle: 'long',
  timeStyle: 'full',
  hourCycle: 'h11',
};
testFormat(testName, expected, locales, options, date);
expected = [
  { value: 'February', type: 'month' },
  { value: ' ', type: 'literal' },
  { value: '15', type: 'day' },
  { value: ', ', type: 'literal' },
  { value: '2023', type: 'year' },
  { value: /^(?:,| at) $/, type: 'literal' },
  { value: '0', type: 'hour' },
  { value: ':', type: 'literal' },
  { value: '40', type: 'minute' },
  { value: ':', type: 'literal' },
  { value: '00', type: 'second' },
  { value: ' ', type: 'literal' },
  { value: 'AM', type: 'dayPeriod' },
  { value: ' ', type: 'literal' },
  { value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatToParts(testName, expected, locales, options, date);

testName = 'Test-Locale-DateStyle-TimeStyle-12HrHourCycle';
date = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
expected = 'February 15, 2023 at 12:40:00 AM Coordinated Universal Time';
locales = 'en-US';
options = {
  dateStyle: 'long',
  timeStyle: 'full',
  hourCycle: 'h12',
};
testFormat(testName, expected, locales, options, date);
expected = [
  { value: 'February', type: 'month' },
  { value: ' ', type: 'literal' },
  { value: '15', type: 'day' },
  { value: ', ', type: 'literal' },
  { value: '2023', type: 'year' },
  { value: ' at ', type: 'literal' },
  { value: '12', type: 'hour' },
  { value: ':', type: 'literal' },
  { value: '40', type: 'minute' },
  { value: ':', type: 'literal' },
  { value: '00', type: 'second' },
  { value: ' ', type: 'literal' },
  { value: 'AM', type: 'dayPeriod' },
  { value: ' ', type: 'literal' },
  { value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatToParts(testName, expected, locales, options, date);

testName = 'Test-Locale-DateStyle-TimeStyle-23HrHourCycle';
date = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
expected = /^February 15, 2023(?:,| at) 00:40:00 Coordinated Universal Time$/;
locales = 'en-US';
options = {
  dateStyle: 'long',
  timeStyle: 'full',
  hourCycle: 'h23',
};
testFormat(testName, expected, locales, options, date);
expected = [
  { value: 'February', type: 'month' },
  { value: ' ', type: 'literal' },
  { value: '15', type: 'day' },
  { value: ', ', type: 'literal' },
  { value: '2023', type: 'year' },
  { value: /^(?:,| at) $/, type: 'literal' },
  { value: '00', type: 'hour' },
  { value: ':', type: 'literal' },
  { value: '40', type: 'minute' },
  { value: ':', type: 'literal' },
  { value: '00', type: 'second' },
  { value: ' ', type: 'literal' },
  { value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatToParts(testName, expected, locales, options, date);

testName = 'Test-Locale-DateStyle-TimeStyle-24HrHourCycle';
date = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
expected = /^February 15, 2023(?:,| at) 24:40:00 Coordinated Universal Time$/;
locales = 'en-US';
options = {
  dateStyle: 'long',
  timeStyle: 'full',
  hourCycle: 'h24',
};
testFormat(testName, expected, locales, options, date);
expected = [
  { value: 'February', type: 'month' },
  { value: ' ', type: 'literal' },
  { value: '15', type: 'day' },
  { value: ', ', type: 'literal' },
  { value: '2023', type: 'year' },
  { value: /^(?:,| at) $/, type: 'literal' },
  { value: '24', type: 'hour' },
  { value: ':', type: 'literal' },
  { value: '40', type: 'minute' },
  { value: ':', type: 'literal' },
  { value: '00', type: 'second' },
  { value: ' ', type: 'literal' },
  { value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatToParts(testName, expected, locales, options, date);

testName = 'Test-Hour12-with-True';
date = new Date(Date.UTC(2023, 1, 15, 15, 40, 0)); // numeric input
expected = '15 February 2023 at 3:40:00 pm Coordinated Universal Time';
locales = 'en-IN';
options = {
  hour12: true,
  timeStyle: 'full',
  dateStyle: 'long',
};
testFormat(testName, expected, locales, options, date);
expected = [
  { value: '15', type: 'day' },
  { value: ' ', type: 'literal' },
  { value: 'February', type: 'month' },
  { value: ' ', type: 'literal' },
  { value: '2023', type: 'year' },
  { value: ' at ', type: 'literal' },
  { value: '3', type: 'hour' },
  { value: ':', type: 'literal' },
  { value: '40', type: 'minute' },
  { value: ':', type: 'literal' },
  { value: '00', type: 'second' },
  { value: ' ', type: 'literal' },
  { value: 'pm', type: 'dayPeriod' },
  { value: ' ', type: 'literal' },
  { value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatToParts(testName, expected, locales, options, date);

testName = 'Test-Hour12-23HrHourCycle';
date = new Date(Date.UTC(2023, 1, 15, 13, 40, 0)); // numeric input
expected = '15 February 2023 at 1:40:00 pm Coordinated Universal Time';
locales = 'en-IN';
options = {
  hour12: true,
  timeStyle: 'full',
  dateStyle: 'long',
  hourCycle: 'h23',
};
testFormat(testName, expected, locales, options, date);
expected = [
  { value: '15', type: 'day' },
  { value: ' ', type: 'literal' },
  { value: 'February', type: 'month' },
  { value: ' ', type: 'literal' },
  { value: '2023', type: 'year' },
  { value: ' at ', type: 'literal' },
  { value: '1', type: 'hour' },
  { value: ':', type: 'literal' },
  { value: '40', type: 'minute' },
  { value: ':', type: 'literal' },
  { value: '00', type: 'second' },
  { value: ' ', type: 'literal' },
  { value: 'pm', type: 'dayPeriod' },
  { value: ' ', type: 'literal' },
  { value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatToParts(testName, expected, locales, options, date);

testName = 'Test-Hour12-24HrHourCycle';
date = new Date(Date.UTC(2023, 1, 15, 15, 40, 0)); // numeric input
expected = /^15 February 2023(?:,| at) 3:40:00\spm Coordinated Universal Time$/;
locales = 'en-GB';
options = {
  hour12: true,
  timeStyle: 'full',
  dateStyle: 'long',
  hourCycle: 'h24',
};
testFormat(testName, expected, locales, options, date);
expected = [
  { value: '15', type: 'day' },
  { value: ' ', type: 'literal' },
  { value: 'February', type: 'month' },
  { value: ' ', type: 'literal' },
  { value: '2023', type: 'year' },
  { value: /^(?:,| at) $/, type: 'literal' },
  { value: '3', type: 'hour' },
  { value: ':', type: 'literal' },
  { value: '40', type: 'minute' },
  { value: ':', type: 'literal' },
  { value: '00', type: 'second' },
  { value: ' ', type: 'literal' },
  { value: 'pm', type: 'dayPeriod' },
  { value: ' ', type: 'literal' },
  { value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatToParts(testName, expected, locales, options, date);

testName = 'Test-Hour12-with-False';
date = new Date(Date.UTC(2023, 1, 15, 15, 40, 0)); // numeric input
expected = '15 February 2023 at 15:40:00 Coordinated Universal Time';
locales = 'en-GB';
options = {
  hour12: false,
  timeStyle: 'full',
  dateStyle: 'long',
};
testFormat(testName, expected, locales, options, date);
expected = [
  { value: '15', type: 'day' },
  { value: ' ', type: 'literal' },
  { value: 'February', type: 'month' },
  { value: ' ', type: 'literal' },
  { value: '2023', type: 'year' },
  { value: ' at ', type: 'literal' },
  { value: '15', type: 'hour' },
  { value: ':', type: 'literal' },
  { value: '40', type: 'minute' },
  { value: ':', type: 'literal' },
  { value: '00', type: 'second' },
  { value: ' ', type: 'literal' },
  { value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatToParts(testName, expected, locales, options, date);

testName = 'Test-Hour12-11HrHourCycle';
date = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
expected = '15 February 2023 at 00:40:00 Coordinated Universal Time';
locales = 'en-GB';
options = {
  hour12: false,
  timeStyle: 'full',
  dateStyle: 'long',
  hourCycle: 'h11',
};
testFormat(testName, expected, locales, options, date);
expected = [
  { value: '15', type: 'day' },
  { value: ' ', type: 'literal' },
  { value: 'February', type: 'month' },
  { value: ' ', type: 'literal' },
  { value: '2023', type: 'year' },
  { value: ' at ', type: 'literal' },
  { value: '00', type: 'hour' },
  { value: ':', type: 'literal' },
  { value: '40', type: 'minute' },
  { value: ':', type: 'literal' },
  { value: '00', type: 'second' },
  { value: ' ', type: 'literal' },
  { value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatToParts(testName, expected, locales, options, date);

testName = 'Test-Hour12-12HrHourCycle';
date = new Date(Date.UTC(2023, 1, 15, 0, 40, 0)); // numeric input
expected = /^February 15, 2023(?:,| at) 00:40:00 Coordinated Universal Time$/;
locales = 'en-US';
options = {
  hour12: false,
  timeStyle: 'full',
  dateStyle: 'long',
  hourCycle: 'h12',
};
testFormat(testName, expected, locales, options, date);
expected = [
  { value: 'February', type: 'month' },
  { value: ' ', type: 'literal' },
  { value: '15', type: 'day' },
  { value: ', ', type: 'literal' },
  { value: '2023', type: 'year' },
  { value: /^(?:,| at) $/, type: 'literal' },
  { value: '00', type: 'hour' },
  { value: ':', type: 'literal' },
  { value: '40', type: 'minute' },
  { value: ':', type: 'literal' },
  { value: '00', type: 'second' },
  { value: ' ', type: 'literal' },
  { value: 'Coordinated Universal Time', type: 'timeZoneName' },
];
testFormatToParts(testName, expected, locales, options, date);
