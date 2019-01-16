// RUN: %hermes -O -dump-ir %s

// This code caused mem2reg to generate PhiInsts with duplicate branches,
// causing an assertion failure later.
switch (x) { case 4:1
   switch (8) { case 4:0
      case.44:

   }
    case.44:
    case.4:
}
