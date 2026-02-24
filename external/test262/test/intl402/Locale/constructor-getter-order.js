// Copyright 2018 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-intl.locale
description: >
    Checks the order of evaluations of arguments and options for the Locale
    constructor.
features: [Intl.Locale]
includes: [compareArray.js]
---*/

const order = [];
new Intl.Locale(
  { toString() { order.push("tag toString"); return "en"; } },
  {
    get language() {
      order.push("get language");
      return {
        toString() {
          order.push("toString language");
          return "de";
        }
      }
    },

    get script() {
      order.push("get script");
      return {
        toString() {
          order.push("toString script");
          return "Latn";
        }
      }
    },

    get region() {
      order.push("get region");
      return {
        toString() {
          order.push("toString region");
          return "DE";
        }
      }
    },

    get calendar() {
      order.push("get calendar");
      return {
        toString() {
          order.push("toString calendar");
          return "gregory";
        }
      }
    },

    get collation() {
      order.push("get collation");
      return {
        toString() {
          order.push("toString collation");
          return "zhuyin";
        }
      }
    },

    get hourCycle() {
      order.push("get hourCycle");
      return {
        toString() {
          order.push("toString hourCycle");
          return "h24";
        }
      }
    },

    get caseFirst() {
      order.push("get caseFirst");
      return {
        toString() {
          order.push("toString caseFirst");
          return "upper";
        }
      }
    },

    get numeric() {
      order.push("get numeric");
      return false;
    },

    get numberingSystem() {
      order.push("get numberingSystem");
      return {
        toString() {
          order.push("toString numberingSystem");
          return "latn";
        }
      }
    },
  }
);

const expected_order = [
  "tag toString",
  "get language",
  "toString language",
  "get script",
  "toString script",
  "get region",
  "toString region",
  "get calendar",
  "toString calendar",
  "get collation",
  "toString collation",
  "get hourCycle",
  "toString hourCycle",
  "get caseFirst",
  "toString caseFirst",
  "get numeric",
  "get numberingSystem",
  "toString numberingSystem"
];

assert.compareArray(order, expected_order);
