// RUN: %hermes -target=HBC -O -gc-sanitize-handles=0 %s
// Make sure that we don't get a handle count overflow
"use strict";

var a = [];
for(var i = 0; i < 100; ++i)
  a.push({val: i, valueOf : function() {return i;}});

Math.max.apply(null, a);
print.apply(null, a);
