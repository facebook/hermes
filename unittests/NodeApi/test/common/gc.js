// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// The JavaScript code in this file is adopted from the Node.js project.
// See the src\napi\Readme.md about the Node.js copyright notice.
"use strict";

function gcUntil(name, condition) {
  if (typeof name === "function") {
    condition = name;
    name = undefined;
  }
  return new Promise((resolve, reject) => {
    let count = 0;
    function gcAndCheck() {
      setImmediate(() => {
        count++;
        global.gc();
        if (condition()) {
          resolve();
        } else if (count < 10) {
          gcAndCheck();
        } else {
          reject(name === undefined ? undefined : "Test " + name + " failed");
        }
      });
    }
    gcAndCheck();
  });
}

Object.assign(module.exports, {
  gcUntil,
});
