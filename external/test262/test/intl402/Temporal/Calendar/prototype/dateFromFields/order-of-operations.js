// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.datefromfields
description: Properties on objects passed to dateFromFields() are accessed in the correct order
includes: [compareArray.js, temporalHelpers.js]
features: [Temporal]
---*/

const expected = [
  "get fields.day",
  "get fields.day.valueOf",
  "call fields.day.valueOf",
  "get fields.era",
  "get fields.era.toString",
  "call fields.era.toString",
  "get fields.eraYear",
  "get fields.eraYear.valueOf",
  "call fields.eraYear.valueOf",
  "get fields.month",
  "get fields.month.valueOf",
  "call fields.month.valueOf",
  "get fields.monthCode",
  "get fields.monthCode.toString",
  "call fields.monthCode.toString",
  "get fields.year",
  "get fields.year.valueOf",
  "call fields.year.valueOf",
  "get options.overflow",
  "get options.overflow.toString",
  "call options.overflow.toString",
];
const actual = [];

const instance = new Temporal.Calendar("gregory");

const fields = {
  era: "ce",
  eraYear: 1.7,
  year: 1.7,
  month: 1.7,
  monthCode: "M01",
  day: 1.7,
};
const arg1 = new Proxy(fields, {
  get(target, key) {
    actual.push(`get fields.${key}`);
    if (key === "calendar") return instance;
    const result = target[key];
    return TemporalHelpers.toPrimitiveObserver(actual, result, `fields.${key}`);
  },
  has(target, key) {
    actual.push(`has fields.${key}`);
    return key in target;
  },
});

const options = {
  overflow: "reject",
};
const arg2 = new Proxy(options, {
  get(target, key) {
    actual.push(`get options.${key}`);
    return TemporalHelpers.toPrimitiveObserver(actual, target[key], `options.${key}`);
  },
  has(target, key) {
    actual.push(`has options.${key}`);
    return key in target;
  },
});

const result = instance.dateFromFields(arg1, arg2);
TemporalHelpers.assertPlainDate(result, 1, 1, "M01", 1, "date result", "ce", 1);
assert.sameValue(result.getISOFields().calendar, "gregory", "calendar slot should store a string");
assert.compareArray(actual, expected, "order of operations");
