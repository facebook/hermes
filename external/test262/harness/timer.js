// Copyright (C) 2017 Ecma International.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.
/*---
description: |
    Used in website/scripts/sth.js
defines: [setTimeout]
---*/
//setTimeout is not available, hence this script was loaded
if (Promise === undefined && this.setTimeout === undefined) {
  if(/\$DONE()/.test(code))
    throw new Test262Error("Async test capability is not supported in your test environment");
}

if (Promise !== undefined && this.setTimeout === undefined) {
  (function(that) {
     that.setTimeout = function(callback, delay) {
      var p = Promise.resolve();
      var start = Date.now();
      var end = start + delay;
      function check(){
        var timeLeft = end - Date.now();
        if(timeLeft > 0)
          p.then(check);
        else
          callback();
      }
      p.then(check);
    }
  })(this);
}
