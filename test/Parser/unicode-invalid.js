// RUN: (! %hermes %s 2>&1 ) | %FileCheck %s

// Verify that invalid Unicode identifiers cause errors
// to be emitted.

 ͛xyz = false;
// Starts with a combining mark
// CHECK: error: unrecognized Unicode character \u35b

᥊abc = false;
// Starts with a digit
// CHECK: error: unrecognized Unicode character \u194a

૬͓͋ = false;
// Starts with a digit with combining mark
// CHECK: unrecognized Unicode character \u34b

﹏ = false;
// Starts with connector puncutation
// CHECK: unrecognized Unicode character \u353
