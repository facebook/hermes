// RUN: (! %hermes %s 2>&1 ) | %FileCheck %s

"use strict";
0x

//CHECK: No hexadecimal digits after 0x
