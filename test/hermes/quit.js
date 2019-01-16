// RUN: %hermes -O -target=HBC %s

quit();
throw new Error("failed to quit");
