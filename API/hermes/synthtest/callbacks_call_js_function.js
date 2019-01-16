'use strict';

(function(global) {
  // callbacks execute f
  // read the zeroth element of the return result,
  // execute that as a function with no args,
  // read the zeroth element of the return value and expect it to be false.
  global.f = function() {
    return [
      function() {
        return [false];
      }
    ];
  };
})(this);
