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

const instance = new Temporal.Calendar("gregory");

const fullFields = {
  era: "ce",
  eraYear: 1981,
  year: 1981,
  month: 12,
  monthCode: "M12",
  day: 15,
};

assertEntriesEqual(instance.mergeFields(fullFields, { era: "bce", eraYear: 1 }), [
  ["era", "bce"],
  ["eraYear", 1],
  ["month", 12],
  ["monthCode", "M12"],
  ["day", 15],
], "era and eraYear together exclude year");

assertEntriesEqual(instance.mergeFields(fullFields, { year: -2 }), [
  ["year", -2],
  ["month", 12],
  ["monthCode", "M12"],
  ["day", 15],
], "year excludes era and eraYear");

assertEntriesEqual(instance.mergeFields(fullFields, { month: 5 }), [
  ["era", "ce"],
  ["eraYear", 1981],
  ["year", 1981],
  ["month", 5],
  ["day", 15],
], "month excludes monthCode");

assertEntriesEqual(instance.mergeFields(fullFields, { monthCode: "M05" }), [
  ["era", "ce"],
  ["eraYear", 1981],
  ["year", 1981],
  ["monthCode", "M05"],
  ["day", 15],
], "monthCode excludes month");

// Specific test cases, of mergeFields on information that is not complete
// enough to construct a PlainDate from, as discussed in
// https://github.com/tc39/proposal-temporal/issues/2407:

assertEntriesEqual(instance.mergeFields({ day: 25, monthCode: "M12", year: 1997, era: "bce" }, { eraYear: 1 }), [
  ["day", 25],
  ["monthCode", "M12"],
  ["eraYear", 1],
], "eraYear excludes year and era");

assertEntriesEqual(instance.mergeFields({ day: 25, monthCode: "M12",  era: "bce" }, { eraYear: 1, year: 1997 }), [
  ["day", 25],
  ["monthCode", "M12"],
  ["eraYear", 1],
  ["year", 1997],
], "eraYear and year both exclude era");

assertEntriesEqual(instance.mergeFields({ day: 25, monthCode: "M12", eraYear: 1 }, { era: "bce", year: 1997 }), [
  ["day", 25],
  ["monthCode", "M12"],
  ["era", "bce"],
  ["year", 1997],
], "era and year both exclude eraYear");

assertEntriesEqual(instance.mergeFields({ day: 25, monthCode: "M12", year: 1997, eraYear: 1 }, { era: "bce" }), [
  ["day", 25],
  ["monthCode", "M12"],
  ["era", "bce"],
], "era excludes year and eraYear");

assertEntriesEqual(instance.mergeFields({ day: 25, monthCode: "M12", year: 1997 }, { eraYear: 1, year: 2 }), [
  ["day", 25],
  ["monthCode", "M12"],
  ["year", 2],
  ["eraYear", 1],
], "eraYear excludes year and era, year overwritten");
