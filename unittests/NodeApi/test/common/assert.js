// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// The JavaScript code in this file is adopted from the Node.js project.
// See the src\napi\Readme.md about the Node.js copyright notice.

"use strict";

class AssertionError extends Error {
  constructor(options) {
    const { message, actual, expected, method, errorStack } = options;

    super(String(message));

    this.name = "AssertionError";
    this.method = String(method);
    this.actual = String(actual);
    this.expected = String(expected);
    this.errorStack = errorStack || "";
    setAssertionSource(this, method);
  }
}

function setAssertionSource(error, method) {
  let result = { sourceFile: "<Unknown>", sourceLine: 0 };
  const stackArray = (error.errorStack || error.stack).split("\n");
  const methodNamePattern = `${method} (`;
  let methodNameFound = false;
  for (const stackFrame of stackArray) {
    if (methodNameFound) {
      const stackFrameParts = stackFrame.split(":");
      if (stackFrameParts.length >= 2) {
        let sourceFile = stackFrameParts[0];
        if (sourceFile.startsWith("    at ")) {
          sourceFile = sourceFile.substr(7);
        }
        result = { sourceFile, sourceLine: Number(stackFrameParts[1]) };
      }
      break;
    } else {
      methodNameFound = stackFrame.indexOf(methodNamePattern) >= 0;
    }
  }
  Object.assign(error, result);
}

const assert = (module.exports = ok);

assert.fail = function fail(message) {
  message = message || "Failed";
  let errorInfo = message;
  if (typeof message !== "object") {
    errorInfo = { message, method: fail.name };
  }
  throw new AssertionError(errorInfo);
};

function innerOk(fn, argLen, value, message) {
  if (!value) {
    if (argLen === 0) {
      message = "No value argument passed to `assert.ok()`";
    } else if (message == null) {
      message = "The expression evaluated to a falsy value";
    }

    assert.fail({
      message,
      actual: formatValue(value),
      expected: formatValue(true),
      method: fn.name,
    });
  }
}

// Pure assertion tests whether a value is truthy, as determined by !!value.
function ok(...args) {
  innerOk(ok, args.length, ...args);
}
assert.ok = ok;

let compareErrorMessage = undefined;
function innerComparison(
  method,
  compare,
  defaultMessage,
  argLen,
  actual,
  expected,
  message
) {
  if (!compare(actual, expected)) {
    if (argLen < 2) {
      message = `'assert.${method.name}' expects two or more arguments.`;
    } else if (message == null) {
      message = defaultMessage;
    }
    if (typeof compareErrorMessage === "string") {
      message += "; " + compareErrorMessage;
      compareErrorMessage = undefined;
    }
    assert.fail({
      message,
      actual: formatValue(actual),
      expected: formatValue(expected),
      method: method.name,
    });
  }
}

assert.strictEqual = function strictEqual(...args) {
  innerComparison(
    strictEqual,
    Object.is,
    "Values are not strict equal",
    args.length,
    ...args
  );
};

assert.notStrictEqual = function notStrictEqual(...args) {
  innerComparison(
    notStrictEqual,
    negate(Object.is),
    "Values must not be strict equal",
    args.length,
    ...args
  );
};

assert.deepStrictEqual = function deepStrictEqual(...args) {
  innerComparison(
    deepStrictEqual,
    isDeepStrictEqual,
    "Values are not deep strict equal",
    args.length,
    ...args
  );
};

assert.notDeepStrictEqual = function notDeepStrictEqual(...args) {
  innerComparison(
    notDeepStrictEqual,
    negate(isDeepStrictEqual),
    "Values must not be deep strict equal",
    args.length,
    ...args
  );
};

function innerThrows(method, argLen, fn, expected, message) {
  let actual = "Did not throw";
  function succeeds() {
    try {
      fn();
      return false;
    } catch (error) {
      if (typeof expected === "function") {
        if (expected.prototype !== undefined && error instanceof expected) {
          return true;
        } else {
          return expected(error);
        }
      } else if (expected instanceof RegExp) {
        actual = `${error.name}: ${error.message}`;
        return expected.test(actual);
      } else if (expected) {
        actual = `${error.name}: ${error.message}`;
        if (expected.name && expected.name != error.name) {
          return false;
        } else if (expected.message && expected.message != error.message) {
          return false;
        } else if (expected.code && expected.code != error.code) {
          return false;
        }
      }
      return true;
    }
  }

  if (argLen < 1 || typeof fn !== "function") {
    message = `'assert.${method.name}' expects a function parameter.`;
  } else if (message == null) {
    if (expected) {
      message = `'assert.${method.name}' failed to throw an exception that matches '${expected}'.`;
    } else {
      message = `'assert.${method.name}' failed to throw an exception.`;
    }
  }

  if (!succeeds()) {
    throw new AssertionError({
      message,
      actual,
      expected,
      method: method.name,
    });
  }
}

assert.throws = function throws(...args) {
  innerThrows(throws, args.length, ...args);
};

function innerMatch(method, argLen, value, expected, message) {
  let succeeds = false;
  if (argLen < 1 || typeof value !== "string") {
    message = `'assert.${method.name}' expects a string parameter.`;
  } else if (!(expected instanceof RegExp)) {
    message = `'assert.${method.name}' expects a RegExp as a second parameter.`;
  } else {
    succeeds = expected.test(value);
    if (!succeeds && message == null) {
      message = `'assert.${method.name}' failed to match '${expected}'.`;
    }
  }

  if (!succeeds) {
    throw new AssertionError({
      message,
      actual: value,
      expected,
      method: method.name,
    });
  }
}

assert.match = function match(...args) {
  innerMatch(match, args.length, ...args);
};

function negate(compare) {
  return (...args) => !compare(...args);
}

function isDeepStrictEqual(left, right) {
  function check(left, right) {
    if (left === right) {
      return true;
    }
    if (typeof left !== typeof right) {
      compareErrorMessage = `Different types: ${typeof left} vs ${typeof right}`;
      return false;
    }
    if (Array.isArray(left)) {
      return Array.isArray(right) && checkArray(left, right);
    }
    if (typeof left === "number") {
      return isNaN(left) && isNaN(right);
    }
    if (typeof left === "object") {
      return typeof right === "object" && checkObject(left, right);
    }
    return false;
  }

  function checkArray(left, right) {
    if (left.length !== right.length) {
      compareErrorMessage = `Different array lengths: ${left.length} vs ${right.length}`;
      return false;
    }
    for (let i = 0; i < left.length; ++i) {
      if (!check(left[i], right[i])) {
        compareErrorMessage = `Different values at index ${i}: ${left[i]} vs ${right[i]}`;
        return false;
      }
    }
    return true;
  }

  function checkObject(left, right) {
    const leftNames = Object.getOwnPropertyNames(left);
    const rightNames = Object.getOwnPropertyNames(right);
    if (leftNames.length !== rightNames.length) {
      compareErrorMessage = `Different set of property names: ${leftNames.length} vs ${rightNames.length}`;
      return false;
    }
    for (let i = 0; i < leftNames.length; ++i) {
      if (!check(left[leftNames[i]], right[leftNames[i]])) {
        compareErrorMessage = `Different values for property '${leftNames[i]}': ${left[leftNames[i]]} vs ${right[leftNames[i]]}`;
        return false;
      }
    }
    const leftSymbols = Object.getOwnPropertySymbols(left);
    const rightSymbols = Object.getOwnPropertySymbols(right);
    if (leftSymbols.length !== rightSymbols.length) {
      compareErrorMessage = `Different set of symbol names: ${leftSymbols.length} vs ${rightSymbols.length}`;
      return false;
    }
    for (let i = 0; i < leftSymbols.length; ++i) {
      if (!check(left[leftSymbols[i]], right[leftSymbols[i]])) {
        compareErrorMessage = `${leftSymbols[i].toString()}: different value`;
        return false;
      }
    }
    return check(Object.getPrototypeOf(left), Object.getPrototypeOf(right));
  }

  return check(left, right);
}

const mustCallChecks = [];

function runCallChecks() {
  const failed = mustCallChecks.filter((context) => {
    if ("minimum" in context) {
      context.messageSegment = `at least ${context.minimum}`;
      return context.actual < context.minimum;
    }
    context.messageSegment = `exactly ${context.exact}`;
    return context.actual !== context.exact;
  });

  mustCallChecks.length = 0;

  failed.forEach((context) => {
    assert.fail({
      message: `Mismatched ${context.name} function calls`,
      actual: `${context.actual} calls`,
      expected: `${context.messageSegment} calls`,
      method: context.method.name,
      errorStack: context.stack,
    });
  });
};
assert.runCallChecks = runCallChecks;

function getCallSite() {
  try {
    throw new Error("");
  } catch (err) {
    return err.stack;
  }
}

assert.mustNotCall = function mustNotCall(msg) {
  return function mustNotCall(...args) {
    assert.fail({
      message: String(msg || "Function should not have been called"),
      actual:
        args.length > 0
          ? `Called with arguments: ${args.map(String).join(", ")}`
          : "Called without arguments",
      expected: "Not to be called",
      method: mustNotCall.name,
    });
  };
};

assert.mustCall = function mustCall(fn, exact) {
  return _mustCallInner(fn, exact, "exact", mustCall);
};

assert.mustCallAtLeast = function mustCallAtLeast(fn, minimum) {
  return _mustCallInner(fn, minimum, "minimum", mustCallAtLeast);
};

const noop = () => {};

function _mustCallInner(fn, criteria = 1, field, method) {
  if (typeof fn === "number") {
    criteria = fn;
    fn = noop;
  } else if (fn === undefined) {
    fn = noop;
  }

  if (typeof criteria !== "number") {
    throw new TypeError(`Invalid ${field} value: ${criteria}`);
  }

  const context = {
    [field]: criteria,
    actual: 0,
    stack: getCallSite(),
    name: fn.name || "<anonymous>",
    method,
  };

  // Add the exit listener only once to avoid listener leak warnings
  if (mustCallChecks.length === 0) process.on('exit', runCallChecks);

  mustCallChecks.push(context);

  return function () {
    context.actual++;
    return fn.apply(this, arguments);
  };
}

function formatValue(value) {
  let type = typeof value;
  if (type === "object") {
    if (Array.isArray(value)) {
      return "<array> []";
    } else {
      return "<object> {}";
    }
  }
  return `<${type}> ${value}`;
}
