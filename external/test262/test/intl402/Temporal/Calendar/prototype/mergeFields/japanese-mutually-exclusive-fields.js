// Copyright (C) 2023 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.mergefields
description: Calendar-specific mutually exclusive keys in mergeFields
features: [Temporal]
---*/

function assertEntriesEqual(actual, expectedEntries, message) {
  const names = Object.getOwnPropertyNames(actual);
  const symbols = Object.getOwnPropertySymbols(actual);
  const actualKeys = names.concat(symbols);
  assert.sameValue(
    actualKeys.length,
    expectedEntries.length,
    `${message}: expected object to have ${expectedEntries.length} properties, not ${actualKeys.length}:`
  );
  for (var index = 0; index < actualKeys.length; index++) {
    const actualKey = actualKeys[index];
    const expectedKey = expectedEntries[index][0];
    const expectedValue = expectedEntries[index][1];
    assert.sameValue(actualKey, expectedKey, `${message}: key ${index}:`);
    assert.sameValue(actual[actualKey], expectedValue, `${message}: value ${index}:`);
  }
}

const instance = new Temporal.Calendar("japanese");

const lastDayOfShowaFields = { era: "showa", eraYear: 64, year: 1989, month: 1, monthCode: "M01", day: 7 };

assertEntriesEqual(instance.mergeFields(lastDayOfShowaFields, { day: 10 }), [
  ["year", 1989],
  ["month", 1],
  ["monthCode", "M01"],
  ["day", 10],
], "day excludes era and eraYear");

assertEntriesEqual(instance.mergeFields(lastDayOfShowaFields, { month: 2 }), [
  ["year", 1989],
  ["month", 2],
  ["day", 7],
], "month excludes monthCode, era, and eraYear");

assertEntriesEqual(instance.mergeFields(lastDayOfShowaFields, { monthCode: "M03" }), [
  ["year", 1989],
  ["monthCode", "M03"],
  ["day", 7],
], "monthCode excludes month, era, and eraYear");

assertEntriesEqual(instance.mergeFields(lastDayOfShowaFields, { year: 1988 }), [
  ["year", 1988],
  ["month", 1],
  ["monthCode", "M01"],
  ["day", 7],
], "year excludes era and eraYear (within same era)");

assertEntriesEqual(instance.mergeFields(lastDayOfShowaFields, { year: 1990 }), [
  ["year", 1990],
  ["month", 1],
  ["monthCode", "M01"],
  ["day", 7],
], "year excludes era and eraYear (in a different era)");

assertEntriesEqual(instance.mergeFields(lastDayOfShowaFields, { eraYear: 1 }), [
  ["eraYear", 1],
  ["month", 1],
  ["monthCode", "M01"],
  ["day", 7],
], "eraYear excludes year and era");

assertEntriesEqual(instance.mergeFields(lastDayOfShowaFields, { era: "heisei" }), [
  ["era", "heisei"],
  ["month", 1],
  ["monthCode", "M01"],
  ["day", 7],
], "era excludes year and eraYear");
