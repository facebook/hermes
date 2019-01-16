// RUN: %hermes -O -target=HBC %s

function Err() {}
Err.prototype = Error()
var s = new Err()
s.stack = "default"
