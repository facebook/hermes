// RUN: (! %hermes -O %s 2>&1 ) | %FileCheck %s

var error = {
  stack: {
    toString: function() { return "" },
  },
  toString: function() { return "MY TOSTRING" },
};

throw error;
// CHECK: MY TOSTRING
