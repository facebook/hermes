// Copyright 2013 Mozilla Corporation. All rights reserved.
// This code is governed by the license found in the LICENSE file.

/*---
es5id: 8.0
description: Tests that Intl has Object.prototype as its prototype.
author: Norbert Lindenberg
---*/

assert.sameValue(Object.getPrototypeOf(Intl), Object.prototype, "Intl doesn't have Object.prototype as its prototype.");
