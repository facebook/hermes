// RUN: %hermes -target=HBC -dump-ra -O %s

// Make sure that we are not crashing on this one.
 escape  [ arguments ]
