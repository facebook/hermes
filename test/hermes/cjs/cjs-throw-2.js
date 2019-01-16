// RUN: true

// Throw if the main module doesn't have x set to 5.

var x = require('./cjs-throw-1.js').x;
if (x !== 5) throw new Error("INVALID X");
exports.y = 42;
