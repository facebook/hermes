/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Introduce some not-called functions to ensure we don't
// depend on all functions having been deserialized.
function notCalled() {
  function alsoNotCalled() {
    alsoNotCalled();
  }
  alsoNotCalled();
  notCalled();
}

// Duplicate some functions to ensure we are robust against function dedup.
function throws6a() {
  throw new Error("Catch me if you can!");
}

function throws6() {
  throw new Error("Catch me if you can!");
}

function throws6b() {
  throw new Error("Catch me if you can!");
}

var obj = {
  throws3: function() {
    eval("throws4();");
  },
  throws5: function() {
    [].forEach.call(arguments, function(arg) { arg(); });
  }
}

function throws4() {
  obj.throws5.apply(this, [throws6, throws6, throws6]);
}

function throws2() {
  obj.throws3();
}

function throws1() {
  (function(){
    throws2();
  })();
}

function throws0() {
  throws1();
}

try {
  throws0();
} catch (err) {
  print(err.stack);
}

