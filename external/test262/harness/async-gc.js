// Copyright (C) 2019  Ecma International.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.
/*---
description: >
    Collection of functions used to capture references cleanup from garbage collectors
features: [FinalizationRegistry.prototype.cleanupSome, FinalizationRegistry, Symbol, async-functions]
flags: [non-deterministic]
defines: [asyncGC, asyncGCDeref, resolveAsyncGC]
---*/

function asyncGC(...targets) {
  var finalizationRegistry = new FinalizationRegistry(() => {});
  var length = targets.length;

  for (let target of targets) {
    finalizationRegistry.register(target, 'target');
    target = null;
  }

  targets = null;

  return Promise.resolve('tick').then(() => asyncGCDeref()).then(() => {
    var names = [];

    // consume iterator to capture names
    finalizationRegistry.cleanupSome(name => { names.push(name); });

    if (!names || names.length != length) {
      throw asyncGC.notCollected;
    }
  });
}

asyncGC.notCollected = Symbol('Object was not collected');

async function asyncGCDeref() {
  var trigger;

  // TODO: Remove this when $262.clearKeptObject becomes documented and required
  if ($262.clearKeptObjects) {
    trigger = $262.clearKeptObjects();
  }

  await $262.gc();

  return Promise.resolve(trigger);
}

function resolveAsyncGC(err) {
  if (err === asyncGC.notCollected) {
    // Do not fail as GC can't provide necessary resources.
    $DONE();
    return;
  }

  $DONE(err);
}
