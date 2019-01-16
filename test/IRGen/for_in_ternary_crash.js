// RUN: %hermes -hermes-parser -dump-ir %s 
// RUN: %hermes -hermes-parser -dump-ir %s -O

  for (a[1?2:31] in x) {
  }
