// Copyright (C) 2023 Justin Grant. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.from
description: Canonical time zone identifiers are never equal to each other
features: [Temporal]
---*/

// supportedValuesOf only returns canonical IDs
const ids = Intl.supportedValuesOf("timeZone");

const forEachDistinctPair = (array, func) => {
  for (let i = 0; i < array.length; i++) {
    for (let j = i + 1; j < array.length; j++) {
      func(array[i], array[j]);
    }
  }
};

forEachDistinctPair(ids, (id1, id2) => {
  const tz = new Temporal.TimeZone(id1);
  assert.sameValue(tz.equals(id2), false);
})

