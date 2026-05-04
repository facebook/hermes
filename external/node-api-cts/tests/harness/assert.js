if (typeof assert !== 'function') {
  throw new Error('Expected a global assert function');
}

try {
  assert(true, 'assert(true, message) should not throw');
} catch (error) {
  throw new Error(`Global assert(true, message) must not throw: ${String(error)}`);
}

const failureMessage = 'assert(false, message) should throw this message';
let threw = false;

try {
  assert(false, failureMessage);
} catch (error) {
  threw = true;

  if (!(error instanceof Error)) {
    throw new Error(`Global assert(false, message) must throw an Error instance but got: ${String(error)}`);
  }

  const actualMessage = error.message;
  if (actualMessage !== failureMessage) {
    throw new Error(
      `Global assert(false, message) must throw message "${failureMessage}" but got "${actualMessage}"`,
    );
  }
}

if (!threw) {
  throw new Error('Global assert(false, message) must throw');
}
